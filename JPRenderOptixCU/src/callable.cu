#include <optix_device.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu\optixu_matrix_namespace.h>
#include "cu_common.h"
#include "../../host_device/data_renderer.h"
#include "../../host_device/data_light.h";
#include "../../host_device/data_instance.h"
#include "intersection.h"

using namespace optix;

#include "light_model_data.h"

RT_FUNC_F float DistanceSquared(const Pos& p1,const Pos& p2){
  const Dir tmp(p1-p2);
  return dot(tmp,tmp);
}

/*
RT_FUNC_F float4 Multiply(const Matrix4x4& m,const Dir& d){
	return m.getCol(0)*d.x+m.getCol(1)*d.y+m.getCol(2)*d.z;
}
rtDeclareVariable(SData_Renderer,V_C_DATA_RENDERER,,);
*/


#pragma region MONTECARLO
/**
* v1 and v2 must be at same space with n;
*/
RT_FUNC_F bool SameHemisphere(const Dir& v1,const Dir& v2,const Nor& n){
	return dot(v1,n)*dot(v2,n)>0.f;
}

RT_FUNC_S Dir UniformSampleHemisphere(float u1,float u2){
	float z=u1;
	float r=sqrtf(max(0.f,1.f-z*z));
	float phi=M_2PIf * u2;
	float x=r * cosf(phi);
	float y=r * sinf(phi);
	return make_float3(x,y,z);
}

RT_FUNC_S Dir UniformSampleSphere(float u1,float u2){
	float z=1.f-2.f * u1;
	float r=sqrtf(max(0.f,1.f-z*z));
	float phi=M_2PIf * u2;
	float x=r * cosf(phi);
	float y=r * sinf(phi);
	return make_float3(x,y,z);
}

RT_FUNC_F float UniformSpherePdf(){
	return 1.f/M_4PIf;
}

RT_CALLABLE_PROGRAM void CF_UNIFORMSAMPLETRIANGLE(RandomFloat1 u1,RandomFloat1 u2,RandomVariable1& u,RandomVariable1& v){
	float su1=sqrtf(u1);
	u=1.f-su1;
	v=u2 * su1;
}

RT_CALLABLE_PROGRAM void CF_CONCENTRICSAMPLEDISK(RandomFloat1 u1,RandomFloat1 u2,RandomVariable1* dx,RandomVariable1* dy){
	float r,theta;
	// Map uniform random numbers to $[-1,1]^2$
	float sx=2.f*u1-1.f;
	float sy=2.f*u2-1.f;

	// Map square to $(r,\theta)$

	// Handle degeneracy at the origin
	if(sx==0.0f && sy==0.0f){
		*dx=0.0f;
		*dy=0.0f;
		return;
	}
	if(sx>=-sy){
		if(sx>sy){
			// Handle first region of disk
			r=sx;
			if(sy>0.0f) theta=sy/r;
			else theta=8.0f+sy/r;
		} else{
			// Handle second region of disk
			r=sy;
			theta=2.0f-sx/r;
		}
	} else{
		if(sx<=sy){
			// Handle third region of disk
			r=-sx;
			theta=4.0f-sy/r;
		} else{
			// Handle fourth region of disk
			r=-sy;
			theta=6.0f+sx/r;
		}
	}
	theta*=M_PIf*.25f;
	*dx=r * cosf(theta);
	*dy=r * sinf(theta);
}

/**
* returns a vector in hemisphere local space, which has z as surface normal;
* 采样具有cos半球分部；
* 均匀的圆盘采样分布点映射到半球面后会使其更加集中于法线方向；
*/
RT_FUNC_S Dir CosineSampleHemisphere(RandomFloat1 u1,RandomFloat1 u2){
	Dir ret;
	ConcentricSampleDisk(u1,u2,&ret.x,&ret.y);
	ret.z=sqrtf(fmaxf(0.f,1.f-ret.x*ret.x-ret.y*ret.y));
	return ret;
}

RT_FUNC_S Dir CosineSampleHemisphere_hack(RandomFloat1 u1,RandomFloat1 u2){
	const float phi=M_2PIf * u2;
	Dir p;
	p.x=u1 * cosf(phi);
	p.y=u1 * sinf(phi);

	// Project up to hemisphere.
	p.z=sqrtf(fmaxf(0.0f,1.0f-p.x*p.x-p.y*p.y));
	return p;
}

RT_CALLABLE_PROGRAM Dir CF_COSINESAMPLEHEMISPHERE_QUICKER(const RandomFloat2& u,float quadrant,float max=4.f){
	// Uniformly sample disk.
	const float r=sqrtf(u.x);
	const float phi=(quadrant+u.y)*M_2PIf/max;
	Dir p;
	p.x=r * cosf(phi);
	p.y=r * sinf(phi);

	// Project up to hemisphere.
	p.z=sqrtf(fmaxf(0.0f,1.0f-p.x*p.x-p.y*p.y));
	return p;
}

RT_CALLABLE_PROGRAM float CF_CHECKERBOARD3(float3 hitPos,float size){
	hitPos+=make_float3(0.001f); // small epsilon so planes don't coincide with scene geometry
	int3 c;

	c.x=abs((int)floor((hitPos.x/size)));
	c.y=abs((int)floor((hitPos.y/size)));
	c.z=abs((int)floor((hitPos.z/size)));

	if((c.x%2)^(c.y%2)^(c.z%2)) return 1.0f;
	return 0.0f;
}

/**
* d1,d2 must be normalized
*/

// 右手坐标系  Z 朝上
RT_FUNC_F Nor SphericalDirection(float sintheta,float costheta,float phi){
	return make_float3(sintheta * cosf(phi),sintheta * sinf(phi),costheta);
}

RT_FUNC_F float CosTheta(const Nor& v,const Nor& n){
	return dot(v,n);
}
RT_FUNC_F float Cos2Theta(const Nor& v,const Nor& n){
	float f=dot(v,n);
	return f*f;
}
RT_FUNC_F float Sin2Theta(const Nor& v,const Nor& n){
	return fmaxf(0.f,1.f-Cos2Theta(v,n));
}
RT_FUNC_F float SinTheta(const Nor& v,const Nor& n){
	return sqrtf(Sin2Theta(v,n));
}

RT_FUNC_F float TanTheta(const Nor& v,const Nor& n){
	return SinTheta(v,n)/CosTheta(v,n);
}
RT_FUNC_F float Tan2Theta(const Nor& v,const Nor& n){
	return Sin2Theta(v,n)/Cos2Theta(v,n);
}
RT_FUNC_F float CosPhi(const Nor& v,const Nor& n,const Nor& t){
	float f=SinTheta(v,n);
	return f==0.f?1.f:clamp(dot(n,t)/f,-1.f,1.f);
}
RT_FUNC_F float SinPhi(const Nor& v,const Nor& n,const Nor& b){
	float f=SinTheta(v,n);
	return f==0.f?0.f:clamp(dot(n,b)/f,-1.f,1.f);
}
RT_FUNC_F float Cos2Phi(const Nor& v,const Nor& n,const Nor& t){
	return CosPhi(v,n,t)*CosPhi(v,n,t);
}
RT_FUNC_F float Sin2Phi(const Nor& v,const Nor& n,const Nor& b){
	return SinPhi(v,n,b)*SinPhi(v,n,b);
}

RT_FUNC_S float ErfInv(float x){
	float w,p;
	x=clamp(x,-.99999f,.99999f);
	w=-logf((1.f-x) * (1.f+x));
	if(w<5){
		w=w-2.5f;
		p=2.81022636e-08f;
		p=3.43273939e-07f+p * w;
		p=-3.5233877e-06f+p * w;
		p=-4.39150654e-06f+p * w;
		p=0.00021858087f+p * w;
		p=-0.00125372503f+p * w;
		p=-0.00417768164f+p * w;
		p=0.246640727f+p * w;
		p=1.50140941f+p * w;
	} else{
		w=sqrtf(w)-3.f;
		p=-0.000200214257f;
		p=0.000100950558f+p * w;
		p=0.00134934322f+p * w;
		p=-0.00367342844f+p * w;
		p=0.00573950773f+p * w;
		p=-0.0076224613f+p * w;
		p=0.00943887047f+p * w;
		p=1.00167406f+p * w;
		p=2.83297682f+p * w;
	}
	return p * x;
}

RT_FUNC_S float Erf(float x){
	// constants
	float a1=0.254829592f;
	float a2=-0.284496736f;
	float a3=1.421413741f;
	float a4=-1.453152027f;
	float a5=1.061405429f;
	float p=0.3275911f;

	// Save the sign of x
	int sign=1;
	if(x<0) sign=-1;
	x=abs(x);

	// A&S formula 7.1.26
	float t=1.f/(1.f+p * x);
	float y=1.f-(((((a5 * t+a4) * t)+a3) * t+a2) * t+a1) * t * expf(-x * x);

	return sign * y;
}


#pragma endregion

#pragma region RANDOM
namespace{

template<unsigned int N>
static __device__ __inline__ unsigned int tea(unsigned int val0,unsigned int val1){
	unsigned int v0=val0;
	unsigned int v1=val1;
	unsigned int s0=0;

	for(unsigned int n=0; n<N; n++){
		s0+=0x9e3779b9;
		v0+=((v1<<4)+0xa341316c)^(v1+s0)^((v1>>5)+0xc8013ea4);
		v1+=((v0<<4)+0xad90777d)^(v0+s0)^((v0>>5)+0x7e95761e);
	}

	return v0;
}

// Generate random unsigned int in [0, 2^24)
static __device__ __inline__ unsigned int lcg(unsigned int &prev){
	const unsigned int LCG_A=1664525u;
	const unsigned int LCG_C=1013904223u;
	prev=(LCG_A * prev+LCG_C);
	return prev&0x00FFFFFF;
}

/*
static __host__ __device__ __inline__ unsigned int lcg2(unsigned int &prev){
	prev=(prev*8121+28411)%134456;
	return prev;
}
// Generate random float in [0, 1)
static __host__ __device__ __inline__ float rnd(unsigned int &prev){
	return ((float)lcg(prev)/(float)0x01000000);
}

static __host__ __device__ __inline__ unsigned int rot_seed(unsigned int seed,unsigned int frame){
	return seed ^ frame;
}
*/

/**
* 一维StratifiedSample抽样
* 分层抽样必须提前知道样本点有多少，不然没有办法划分区间
* "dec_dataContext.h"里有生成的样本总数参数V_C_DATA_CONTEXT
* "dec_frameNumber.h"里有样本的下标参数V_C_FRAME_NUMBER_UINT
*
* @param[in] index  表示样本下标
RT_FUNC_S void GetSample2_stratified(uint& seed,float2& out){
	const float interval=1.f/V_C_DATA_RENDERER.samples_per_pixel_col;
	float delta=rnd(seed);
	seed=rot_seed(seed,V_C_DATA_RENDERER.frame_current*2u);
	out.x=fminf(((int)V_C_DATA_RENDERER.frame_current/V_C_DATA_RENDERER.samples_per_pixel_col+delta)*interval,M_1_MINUS_EPSILON);

	delta=rnd(seed);
	seed=rot_seed(seed,V_C_DATA_RENDERER.frame_current*2u+1u);
	out.y=fminf(((int)V_C_DATA_RENDERER.frame_current % V_C_DATA_RENDERER.samples_per_pixel_row+delta)*interval,M_1_MINUS_EPSILON);
}
*/
}


RT_CALLABLE_PROGRAM uint CF_GETRANDOMSEED(uint index,uint frameCount){
	unsigned int s0=0;

	for(unsigned int n=0; n<16; n++){
		s0+=0x9e3779b9;
		index+=((frameCount<<4)+0xa341316c)^(frameCount+s0)^((frameCount>>5)+0xc8013ea4);
		frameCount+=((index<<4)+0xad90777d)^(index+s0)^((index>>5)+0x7e95761e);
	}

	return index;
}

/**
* Sample one float at range of [0,1)
*/
RT_CALLABLE_PROGRAM RandomFloat1 CF_GETSAMPLE1(uint& seed){
	return ((float)lcg(seed)/(float)0x01000000);
}

/*
* Sample two float at range of [0,1)
*/
RT_CALLABLE_PROGRAM RandomFloat2 CF_GETSAMPLE2(uint& seed){
	RandomFloat2 samples;
	samples.x=((float)lcg(seed)/(float)0x01000000);
	samples.y=((float)lcg(seed)/(float)0x01000000);
	return samples;
}

#pragma endregion

#pragma region DISTRIBUTION

/**
* endIndex 是不能返回的，就像正常的数组一样。
* 返回的值是在整体上的索引，并非在startIndex基础上；
*/
RT_FUNC_F int FindLowBoundOffset_(int BufferID,int startIndex,int endIndex,float value){
	if(value<=0.f)return startIndex;
	if(value>=1.f)return endIndex-1;

	int mid=startIndex+1;
	while(startIndex<endIndex-1){
		mid=(int)((startIndex+endIndex)*.5f);
		float v=rtBufferId<float,1>(BufferID)[mid];
		if(v>value)endIndex=mid;
		if(v<=value)startIndex=mid;
	}
	//rtPrintf("start[%d],end[%d],mid[%d],v[%f]",startIndex,endIndex,mid,rtBufferId<float,1>(BufferID)[mid]);
	return mid-1;
}

RT_CALLABLE_PROGRAM void CF_DISTRIBUTION1D_SAMPLEDISCRETE(const SData_Light& data,RandomFloat1 rf,int& offset,float& PDF){
	int CDFBufferID=data.meshAreaCDFBufferID;
	float integration=data.meshAreaIntegration;
	int valueBufferID=data.meshAreaBufferID;
	int size=data.meshCount;
	//Assert(rf<1.0f);
	offset=FindLowBoundOffset_(CDFBufferID,0,size+1,rf);
	//Assert(offset<=size);

	float v=rtBufferId<float,1>(valueBufferID)[offset];
	PDF=v/(integration*size);
}


/*
RT_FUNC void SampleDiscrete(const RandomFloatSample2& rs,UV& uv,float& PDF){
//Assert(rs.x<1.0f && rs.y<1.0f);
int off_y=FindLowBoundOffset_(MarginalCDFBufferID,0,size.y+1,rs.y);
uv.y=float(off_y)/float(size.y);
//Assert(uv.y<=1.f);

int startIndex=off_y*(size.x+1);
int endIndex=(off_y+1)*(size.x+1);
int off_x=FindLowBoundOffset_(CDFBufferID,startIndex,endIndex,rs.x);
uv.x=float(off_x-startIndex)/float(size.x);
//Assert(uv.x<=1.f);

PDF=GETLVALUE(off_x)/GETIVALUE(size.y);

}
*/
RT_CALLABLE_PROGRAM void CF_DISTRIBUTION2D_SAMPLECONTINUOUS(const SData_Light& data,const RandomFloat2& rs,RandomVariable2& uv,float& PDF){
	uint2 size=data.HDR_size;
	int CDFBufferID=data.HDR_CDFBufferID;
	int MarginalCDFBufferID=data.HDR_MarginalCDFBufferID;
	int IntegrationBufferID=data.HDR_IntegrationBufferID;
	int LuminanceBufferID=data.HDR_LuminanceBufferID;

	//Assert(rs.x < 1.0f && rs.y < 1.0f);
	int off_y=FindLowBoundOffset_(MarginalCDFBufferID,0,size.y+1,rs.y);
	//rtPrintf("wow!CDFBufferID[%d],MarginalCDFBufferID[%d],IntegrationBufferID[%d],LuminanceBufferID[%d]",CDFBufferID,MarginalCDFBufferID,IntegrationBufferID,LuminanceBufferID);

	//discrete sample only:
	//float pdf_y=GETIVALUE(off_y)/GETIVALUE(size.y);

	//continues sampling;
#define GETMCDFVALUE(index)(rtBufferId<float,1>(MarginalCDFBufferID)[index])
	float du=(rs.y-GETMCDFVALUE(off_y))/(GETMCDFVALUE(off_y+1)-GETMCDFVALUE(off_y));
#undef GETMCDFVALUE

	// 下面没有使用公式uv.y=1.f-(off_y+du)/(size.y+1.f)计算uv.y，
	// 是因为图像本身的Y轴是颠倒的。
	uv.y=(off_y+du)/float(size.y);

	const int startIndex=off_y*(size.x+1);
	const int endIndex=(off_y+1)*(size.x+1);
	int off_x=FindLowBoundOffset_(CDFBufferID,startIndex,endIndex,rs.x);
#if 0
	rtPrintf("off_y off_x is:%d,%d\n",off_y,off_x-startIndex);
#endif

	//discrete sample only:float pdf_x=GETLVALUE(off_x)/GETIVALUE(off_y);
#define GETCDFVALUE(index)(rtBufferId<float,1>(CDFBufferID)[index])
	float dv=(rs.x-GETCDFVALUE(off_x))/(GETCDFVALUE(off_x+1)-GETCDFVALUE(off_x));
	uv.x=(off_x-startIndex+dv)/(size.x);
#undef GETCDFVALUE

	//PDF=pdf_y*pdf_x;
	PDF=rtBufferId<float,1>(LuminanceBufferID)[off_x]/rtBufferId<float,1>(IntegrationBufferID)[size.y];
};

RT_CALLABLE_PROGRAM float CF_DISTRIBUTION2D_PDF(const SData_Light& data,UV2& uv){
	uint2 size=data.HDR_size;
	int IntegrationBufferID=data.HDR_IntegrationBufferID;
	int LuminanceBufferID=data.HDR_LuminanceBufferID;
	//Assert(0.f <= uv.x < 1.0f);
	//Assert(0.f <= uv.y < 1.0f);
	int iu=uv.x*size.x;
	int iv=uv.y*size.y;
	int index=iv*size.x+iu;
	return rtBufferId<float,1>(LuminanceBufferID)[index]/rtBufferId<float,1>(IntegrationBufferID)[size.y];

}

struct PhongDistribution{
public:

	RT_FUNC float D(const LMD& data,const Nor& wh_g) const{
		const float costhetah=fabsf(dot(wh_g,data.sn_g));

		const float d=1.f-costhetah * costhetah;
		if(d<=0.f) return 0.f;
		const float HdotT=dot(wh_g,data.tangent_g);
		const float HdotB=dot(wh_g,data.bitangent_g);
		const float e=(data.anisotropicUV.x * HdotT*HdotT+data.anisotropicUV.y * HdotB*HdotB)/d;
		const float r=sqrtf((data.anisotropicUV.x+2.f) * (data.anisotropicUV.y+2.f)) * M_1_2PIf * powf(costhetah,e);
		return r;
	}

	RT_FUNC float Pdf(const LMD& data,const Nor& wh_g) const{
		// Compute PDF for $\wi$ from anisotropic distribution
		const float costhetah=fabsf(dot(wh_g,data.sn_g));
		const float ds=1.f-costhetah * costhetah;
		if(ds>0.f && dot(data.wo_g,wh_g)>0.f){
			const float HdotT=dot(wh_g,data.tangent_g);
			const float HdotB=dot(wh_g,data.bitangent_g);
			const float e=(data.anisotropicUV.x * HdotT*HdotT+data.anisotropicUV.y *HdotB*HdotB)/ds;
			const float d=sqrtf((data.anisotropicUV.x+1.f) * (data.anisotropicUV.y+1.f)) * M_1_2PIf * powf(costhetah,e);
			return d;
		}
		return 0.f;
	}


	RT_FUNC_F void Sample_wh(const LMD&data,RandomVariable2& u,Nor& wh_g) const{
		// Sample from first quadrant and remap to hemisphere to sample $\wh$
		float phi,costheta;
		if(u.x<.25f){
			u.x*=4.f;
			sampleFirstQuadrant(data,u,phi,costheta);
		} else if(u.x<.5f){
			u.x=4.f * (.5f-u.x);
			sampleFirstQuadrant(data,u,phi,costheta);
			phi=M_PIf-phi;
		} else if(u.x<.75f){
			u.x=4.f * (u.x-.5f);
			sampleFirstQuadrant(data,u,phi,costheta);
			phi+=M_PIf;
		} else{
			u.x=4.f * (1.f-u.x);
			sampleFirstQuadrant(data,u,phi,costheta);
			phi=M_2PIf-phi;
		}
		float sintheta=sqrtf(fmaxf(0.f,1.f-costheta*costheta));
		wh_g=SphericalDirection(sintheta,costheta,phi);
		wh_g=normalize(wh_g);

		float3 result=wh_g.x*data.tangent_g+wh_g.y*data.bitangent_g+wh_g.z*data.sn_g;
		if(!SameHemisphere(result,data.wo_g,data.sn_g)){ result*=-1.f; }
		wh_g=result;
	}

	RT_FUNC void sampleFirstQuadrant(const LMD& data,const RandomVariable2& u,float& phi,float& costheta) const{
		if(data.anisotropicUV.x==data.anisotropicUV.y) phi=M_PI_2f * u.x;
		else phi=atanf(sqrtf((data.anisotropicUV.x+1.f)/(data.anisotropicUV.y+1.f)) * tanf(M_PI_2f * u.x));
		const float cosphi=cosf(phi);
		const float sinphi=sinf(phi);
		costheta=powf(u.y,1.f/(data.anisotropicUV.x * cosphi * cosphi+data.anisotropicUV.y * sinphi * sinphi+1.f));
	}

};

#pragma endregion

#pragma region TEXTURE
RT_CALLABLE_PROGRAM Color4 CF_GETTEXTURE_GLOSSY(int bufId,int offset,UV2 uv){
	SData_Texture& data=rtBufferId<SData_Material,1>(bufId)[offset].BS_textureGlossy;
	if(data.enable){
		uv.x*=data.UVrepeat.x;
		uv.y*=data.UVrepeat.y;
		if(fabsf(data.rotation)>M_MIN_FLOAT){
			const float c=cos(data.rotation);
			const float s=sin(data.rotation);
			const float temp=c*uv.x-s*uv.y;
			uv.y=s*uv.x+c*uv.y;
			uv.x=temp;
		}
		Color4 c=rtTex2D<float4>(data.TSID,uv.x,uv.y);
		c.x=powf(c.x,data.invGamma);
		c.y=powf(c.y,data.invGamma);
		c.z=powf(c.z,data.invGamma);
		c.w=1.f;
		return c;
	} else return data.color;
}

RT_CALLABLE_PROGRAM Color4 CF_GETTEXTURE_DIFFUSE(int bufId,int offset, UV2 uv){
	SData_Texture& data=rtBufferId<SData_Material,1>(bufId)[offset].BS_textureDiffuse;
	if(data.enable){
		uv.x*=data.UVrepeat.x;
		uv.y*=data.UVrepeat.y;
		if(fabsf(data.rotation)>M_MIN_FLOAT){
			const float c=cos(data.rotation);
			const float s=sin(data.rotation);
			const float temp=c*uv.x-s*uv.y;
			uv.y=s*uv.x+c*uv.y;
			uv.x=temp;
		}
		Color4 c=rtTex2D<float4>(data.TSID,uv.x,uv.y);
		c.w=1.f;
		return c;
	} else return data.color;
}

RT_CALLABLE_PROGRAM Nor CF_GETTEXTURE_NORMAL(const SData_Texture& data, Intersection aInset){
	if(data.enable){
		aInset.texCoord.x*=data.UVrepeat.x;
		aInset.texCoord.y*=data.UVrepeat.y;
		if(fabsf(data.rotation)>M_MIN_FLOAT){
			const float c=cos(data.rotation);
			const float s=sin(data.rotation);
			const float temp=c*aInset.texCoord.x-s*aInset.texCoord.y;
			aInset.texCoord.y=s*aInset.texCoord.x+c*aInset.texCoord.y;
			aInset.texCoord.x=temp;
		}
		Color4 c=rtTex2D<float4>(data.TSID,aInset.texCoord.x,aInset.texCoord.y);
		c.w=1.f;
		Nor nor=make_float3(F_COLOR_TO_NORMAL(c));
		nor=nor.x*aInset.tbn.Tangent+nor.y*aInset.tbn.Bitangent+nor.z*aInset.tbn.Normal;
		nor=lerp(nor,aInset.geoNor,data.bumpscale+1.f);
		return nor;
	} else return aInset.geoNor;
}

RT_CALLABLE_PROGRAM Radiance4 CF_GETSKYBOXRADIANCE(int TSID,const Dir& dir_g){
	if(TSID<0)return make_float4(0.f);

	UV2 uv;
	float phi=atan2f(dir_g.x,dir_g.z);
	uv.x=phi*M_1_2PIf;
	float theta=acos(dir_g.y);
	uv.y=1.f-theta*M_1_PIf;

	return rtTex2D<float4>(TSID,uv.x,uv.y);
}

/**
* 调用该函数需要注意的是：球谐函数使用的是标准的右手纹理坐标系，需要将wi_g转化
* 另一个需要注意的是：通过freeimage加载的图片是上下颠倒的；
* 内容的实现依据以下论文：
* https://cseweb.ucsd.edu/~ravir/papers/envmap/envmap.pdf
*/
RT_CALLABLE_PROGRAM Radiance4 CF_GETRADIANCEFROMSH(int IdOfSHCbuffer,Nor wi_g){

	float tmpy=wi_g.y,tmpx=wi_g.x;
	wi_g.x=wi_g.z;
	wi_g.y=tmpx;
	wi_g.z=-tmpy;


#define GETSHCOF(idx) rtBufferId<float3, 1>(IdOfSHCbuffer)[idx]

#if 0
	float3 value=GETSHCOF(4);
	rtPrintf("wow,wi_g[%f,%f,%f],ldtype[%d],value[%f,%f,%f],,",wi_g.x,wi_g.y,wi_g.z,ld.lightType,value.x,value.y,value.z);
#endif

	Radiance3 Li=make_float3(0.f);
	Li+=GETSHCOF(0u)*Y00;
	Li+=GETSHCOF(1u)*Y1_1(wi_g.y);
	Li+=GETSHCOF(2u)*Y10(wi_g.z);
	Li+=GETSHCOF(3u)*Y11(wi_g.x);
	Li+=GETSHCOF(4u)*Y2_2(wi_g.x,wi_g.y);
	Li+=GETSHCOF(5u)*Y2_1(wi_g.y,wi_g.z);
	Li+=GETSHCOF(6u)*Y20(wi_g.z);
	Li+=GETSHCOF(7u)*Y21(wi_g.x,wi_g.z);
	Li+=GETSHCOF(8u)*Y22(wi_g.x,wi_g.y);

#undef GETSHCOF

	return make_float4(Li,1.f);
}

RT_CALLABLE_PROGRAM float CF_GETENVRADIANCE(int TSID,int bufferID,const Nor& rayDir,float blend,Color4& outColor){
	int N=rtBufferId<SData_Light,1>(bufferID).size();
	for(uint i=0u; i<N; ++i){
		const SData_Light& ld=rtBufferId<SData_Light,1>(bufferID)[i];
		if(ld.lightFuncIndex==CFINDEX_LIGHT_ENVIRONMENT){
			Radiance4 l=CF_GETSKYBOXRADIANCE(TSID,rayDir);
			Radiance4 sh=CF_GETRADIANCEFROMSH(ld.SH_cBufferID,rayDir)*ld.energyBalance;
			Radiance4 L=lerp(sh,l,blend);
			const float& F{ld.HDR_inv_gamma};
			outColor=make_float4(powf(L.x,F),powf(L.y,F),powf(L.z,F),1.0f);;
			return ld.HDR_exposure;
		}
	}
	return 0.f;
}

RT_CALLABLE_PROGRAM Nor CF_GETTRIPLANENORMAL(int TSID,const Pos& pos,const Nor& gn){
	/**
	* 参考资料
	https://medium.com/@bgolus/normal-mapping-for-a-triplanar-shader-10bf39dca05a
	*/

	Nor norX=make_float3(F_COLOR_TO_NORMAL(rtTex2DLod<float4>(TSID,pos.z,pos.y,0)));
	Nor norY=make_float3(F_COLOR_TO_NORMAL(rtTex2DLod<float4>(TSID,pos.x,pos.z,0)));
	Nor norZ=make_float3(F_COLOR_TO_NORMAL(rtTex2DLod<float4>(TSID,pos.x,pos.y,0)));

	if(gn.x<0.f) norX.z*=-1.f;
	if(gn.y<0.f) norY.z*=-1.f;
	if(gn.z<0.f) norZ.z*=-1.f;

#define F 2.f
	float3 blend=make_float3(powf(gn.x,F),powf(gn.y,F),powf(gn.z,F));
	blend/=dot(blend,make_float3(1.f));

	float3 X=make_float3(norX.z,norX.y,norX.x)*blend.x;
	float3 Y=make_float3(norY.x,norY.z,norY.y)*blend.y;
	float3 Z=make_float3(norZ.x,norZ.y,norZ.z)*blend.z;
	return normalize(X+Y+Z);
#undef F


};
#pragma endregion

#pragma region LIGHTMODEL

RT_FUNC_F float FrCond(float cosi){
	//折射率
	const float eta=0.2f;
	//吸收系数
	const float k=3.f;
	const float tmp=(eta*eta+k*k) * cosi*cosi;
	const float Rparl2=(tmp-(2.f * eta * cosi)+1)/(tmp+(2.f * eta * cosi)+1);
	const float tmp_f=eta*eta+k*k;
	const float Rperp2=(tmp_f-(2.f * eta * cosi)+cosi*cosi)/(tmp_f+(2.f * eta * cosi)+cosi*cosi);
	return (Rparl2+Rperp2)*.5f;
}

RT_FUNC_F float D(const Nor &wh,const Nor& n_g,float roughness){
	const float costhetah=fabsf(dot(n_g,wh));
	return roughness * M_1_2PIf* powf(costhetah,roughness);
}

RT_FUNC_F  float G(const Nor &wo,const Nor& n_g,const Nor &wi,const Nor &wh){
	const float tmp=2.f*fabsf(dot(n_g,wh)/dot(wo,wh));
	const float NdotWo=fabsf(dot(n_g,wo));
	const float NdotWi=fabsf(dot(n_g,wi));
	return min(1.f,min(NdotWo*tmp,NdotWi*tmp));
}

#pragma region Lambertian
RT_CALLABLE_PROGRAM void Lambertian_Pdf(LMD& data,uint& seed){
	//RT_CALLABLE_PROGRAM float Lambertian_Pdf(const Nor& n_g,const Nor& wo_g,const Nor& wi_g){
		/*
		 * 这里需要注意，由于计算机精度问题导致随机的半球法线不可能完美的被限制在半球内部，所以导致SampleHemisphiere返回false
		 * 而整个Lambertian_Pdf函数返回0.f
		 * 而0.f的话由于存在除以0.f的逻辑，从而导致结果无穷大。需要注意；
		 * 所以下面的逻辑不能用
		return SameHemisphere(wo_g,wi_g,prd.n_g)?fabsf(dot(wi_g,prd.n_g)) * M_1_PIf:0.f;
		*/
	data.PDF=fabsf(dot(data.wi_g,data.sn_g)) * M_1_PIf;
}

RT_CALLABLE_PROGRAM void Lambertian_f(LMD& data,uint& seed){ data.BRDF=M_1_PIf; }
//RT_CALLABLE_PROGRAM float Lambertian_f(const Nor& n_g,const Nor& wo_g,const Nor& wi_g,float roughness){ return M_1_PIf; }

RT_CALLABLE_PROGRAM void Lambertian_Sample_f(LMD& data,uint& seed){
	//RT_CALLABLE_PROGRAM float Lambertian_Sample_f(const Nor& n_g,const Nor& wo_g,Nor& wi_g,float& pdf,uint& seed,float roughness){
	if(dot(data.sn_g,data.wo_g)<0.f)data.wi_g=data.sn_g;
	else{
		Onb onb(data.sn_g);
		RandomVariable2 samples=CF_GETSAMPLE2(seed);
		data.wi_g=CF_COSINESAMPLEHEMISPHERE_QUICKER(samples,0.f,1.f);
		/**
		* 目前不做图元双面渲染，原因是
		* 1 真实世界中不存在数学中的2D平面图元；
		* 2 消耗性能；
		* 3 无法良好判断图元的正反面；
		if(dot(wo_g,n_g)<0.f)wi_g.z*=-1.f;
		*/
		onb.inverse_transform(data.wi_g);
		data.NormalizeWi();
	}
	Lambertian_Pdf(data,seed);
	Lambertian_f(data,seed);
}

RT_CALLABLE_PROGRAM void Lambertian_Albedo(LMD& data){
	//RT_CALLABLE_PROGRAM float Lambertian_Albedo(const Nor& nor,const Nor& wo_g,const Nor& wi_g){
	data.albedo=M_1_PIf;
}
#pragma endregion

#pragma region TorranceSparrow
RT_CALLABLE_PROGRAM void TorranceSparrow_Pdf(LMD& data,uint& seed){
	//RT_CALLABLE_PROGRAM float TorranceSparrow_Pdf(const Nor& n_g,const Nor& wo_g,const Nor& wi_g){
	// TODO:这个PDF计算是错的；
	data.PDF=data.WoWiAtSamehemisphere()?fabsf(dot(data.wi_g,data.sn_g))*M_1_PIf:0.f;
}

RT_CALLABLE_PROGRAM void TorranceSparrow_f(LMD& data,uint& seed){
	//RT_CALLABLE_PROGRAM float TorranceSparrow_f(const Nor& n_g,const Nor& wo_g,const Nor& wi_g,float roughness){
	const float cosTheta=fabsf(data.WiDotN()*data.WoDotN());
	if(cosTheta<=M_EPSILON) data.BRDF=0.f;
	else{
		Nor wh_g=data.GetWh_g();
		if(dot(wh_g,wh_g)<=M_EPSILON) data.BRDF=0.f;
		else{
			data.BRDF=D(wh_g,data.sn_g,data.roughness)*G(data.wo_g,data.sn_g,data.wi_g,wh_g)*FrCond(dot(data.wi_g,wh_g))/(4.f * cosTheta);
		}
	}
}

RT_CALLABLE_PROGRAM void TorranceSparrow_Sample_f(LMD& data,uint& seed){
	//RT_CALLABLE_PROGRAM float TorranceSparrow_Sample_f(const Nor& n_g,const Nor& wo_g,Nor& wi_g,float& pdf,uint& seed,float roughness){
		//roughness [1~ +inf)
	RandomVariable2 samples=CF_GETSAMPLE2(seed);
	if(samples.x<0.01f)samples.x+=0.01f;
	const float costheta=powf(samples.x,1.f/data.roughness);
	const float sintheta=sqrtf(max(0.f,1.f-costheta*costheta));
	const float phi=samples.y * M_2PIf;
	Nor wh_g=SphericalDirection(sintheta,costheta,phi);
	Onb onb(data.sn_g);
	onb.inverse_transform(wh_g);

	data.wi_g=normalize(reflect(-data.wo_g,wh_g));
	// Compute PDF for $\wi$ from Blinn distribution
	if(!data.WoWiAtSamehemisphere()){
		data.wi_g*=-1.f;
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	//pdf=(roughness * powf(costheta,roughness))/(M_8PIf *  fabsf(dot(wo_g,wh_g)));
	// 优化上面为下一行：
	data.PDF=(data.roughness * samples.x)/(M_8PIf *  fabsf(dot(data.wo_g,wh_g)));
	////////////////////////////////////////////////////////////////////////////////////////////////////
	TorranceSparrow_f(data,seed);
}

RT_CALLABLE_PROGRAM void TorranceSparrow_Albedo(LMD& data){
	//RT_CALLABLE_PROGRAM float TorranceSparrow_Albedo(const Nor& n_g,const Nor& wo_g,const Nor& wi_g){
	data.albedo=1.f;
}
#pragma endregion

#pragma region SpecularReflect
RT_CALLABLE_PROGRAM void SpecularReflect_Pdf(LMD& data,uint& seed){
	data.PDF=0.f;
}

RT_CALLABLE_PROGRAM void SpecularReflect_f(LMD& data,uint& seed){
	data.BRDF=0.f;
}

RT_CALLABLE_PROGRAM void SpecularReflect_Sample_f(LMD& data,uint& seed){
	data.wi_g=reflect(-data.wo_g,data.sn_g);
	data.NormalizeWi();
	data.PDF=1.f;
	data.BRDF=1.f/fabsf(data.WiDotN());
}

RT_CALLABLE_PROGRAM void SpecularReflect_Albedo(LMD& data,uint& seed){
	data.albedo=1.f;
}
#pragma endregion

#pragma region SpecularRefract
RT_CALLABLE_PROGRAM void SpecularRefract_Pdf(LMD& data,uint& seed){
	data.PDF=0.f;
}

RT_CALLABLE_PROGRAM void SpecularRefract_f(LMD& data,uint& seed){
	data.BRDF=0.f;
}

RT_CALLABLE_PROGRAM void SpecularRefract_Sample_f(LMD& data,uint& seed){
	if(!refract(data.wi_g,-data.wo_g,data.sn_g,data.IOR)){
		//全反射
		data.wi_g=reflect(-data.wo_g,data.sn_g);
	}
	data.NormalizeWi();
	data.PDF=1.f;
	data.BRDF=1.f/fabsf(data.WiDotN());
}

RT_CALLABLE_PROGRAM void SpecularRefract_Albedo(LMD& data){
	data.albedo=1.f;
}
#pragma endregion

#pragma region Ashikhmin-Shirley
RT_CALLABLE_PROGRAM void AshikminShirley_Pdf(LMD& data,uint& seed){
	const Nor wh_g=data.GetWh_g();
	PhongDistribution dtrb;

	// 下面是平衡自适应，乘以0.5f的意思是给分子乘以2（pdf做分母）具体参考平衡自适应，如果
	// 采用幂自适应，收敛会更快；
	data.PDF=.5f * (fabsf(data.WiDotN())* M_1_PIf+dtrb.Pdf(data,wh_g)/(4.f*dot(data.wo_g,wh_g)));
}

RT_CALLABLE_PROGRAM void AshikminShirley_f(LMD& data,uint& seed){
	//diffuse partial
	const float diffuse=(28.f/(23.f*M_PIf)) * data.reflatance.x * (1.f-data.reflatance.y) *(1.f-powf(1.f-.5f * fabsf(data.WiDotN()),5.f)) *(1.f-powf(1.f-.5f * fabsf(data.WoDotN()),5.f));
	Nor wh_g=data.GetWh_g();
	if(length(wh_g)==0) data.BRDF=0.f;
	else{

		//specular partial
		const float maxFactor=fmaxf(fmaxf(fabsf(data.WiDotN()),fabsf(data.WoDotN())),M_MIN_RADIANCE);
		const float absTheta=fmaxf(fabsf(dot(wh_g,data.wi_g)),M_MIN_RADIANCE);
		const float f=fresnel_schlick(absTheta,5.f,data.reflatance.y,1.f);

		PhongDistribution dtrb;
		const float specular=dtrb.D(data,wh_g)*f/(4.f * absTheta * maxFactor);

		if(data.WoWiAtSamehemisphere()){
			data.BRDF=data.reflatance.x*diffuse+data.reflatance.y*specular;
		} else{
			data.BRDF=0.f;
		}
	}
}

RT_CALLABLE_PROGRAM void AshikminShirley_Sample_f(LMD& data,uint& seed){
	RandomFloat2 u=GetSample2(seed);
	if(u.x<0.5f){
		u.x*=2.f;
		// Cosine-sample the hemisphere, flipping the direction if necessary
		data.wi_g=CosineSampleHemisphere(u.x,u.y);
		if(data.WoDotN()<0.f)data.wi_g.z*=-1.f;
		Onb onb(data.sn_g);
		onb.inverse_transform(data.wi_g);
		data.NormalizeWi();
	} else{
		u.x=2.f * (u.x-.5f);
		Nor wh_g;
		PhongDistribution dtrb;
		dtrb.Sample_wh(data,u,wh_g);
		data.wi_g=reflect(-data.wo_g,wh_g);

		if(!data.WoWiAtSamehemisphere()){
			data.BRDF=0.f;
			return;
		}
	}
	AshikminShirley_Pdf(data,seed);
	AshikminShirley_f(data,seed);
}

RT_CALLABLE_PROGRAM void AshikminShirley_Albedo(LMD& data){
	data.albedo=1.f;
}
#pragma endregion

#pragma endregion

#pragma region LIGHTS
RT_FUNC_S float Falloff(float costheta,float cosTotalWidth,float cosFalloffStart){
	if(costheta<cosTotalWidth||costheta<=0.f)return 0.;
	if(costheta>cosFalloffStart)return 1.;
	// Compute falloff inside spotlight cone
	float delta=(costheta-cosTotalWidth)/(cosFalloffStart-cosTotalWidth);
	return delta*delta*delta*delta;
}
RT_FUNC_S void CalcWithMatrix(float3& p,int id){
	float3 tmpp=p;
	p.x=tmpp.x*GET_F_1D(id,0)+tmpp.y*GET_F_1D(id,1)+tmpp.z*GET_F_1D(id,2)+GET_F_1D(id,3);
	p.y=tmpp.x*GET_F_1D(id,4)+tmpp.y*GET_F_1D(id,5)+tmpp.z*GET_F_1D(id,6)+GET_F_1D(id,7);
	p.z=tmpp.x*GET_F_1D(id,8)+tmpp.y*GET_F_1D(id,9)+tmpp.z*GET_F_1D(id,10)+GET_F_1D(id,11);
}
RT_FUNC_S void UVToNor(const UV2& uv,Nor& v,float& sintheta){
	const float theta=(1.f-uv.y)*M_PIf;
	const float phi=uv.x*M_2PIf;

	v.y=cosf(theta);
	sintheta=sinf(theta);
	v.x=sintheta*sinf(phi);
	v.z=sintheta*cosf(phi);
}

/**
* 使用的前提是HDR贴图没有旋转。
*/
RT_FUNC_S void DirToUV(const Dir& dir,UV2& uv){
	float phi=atan2f(dir.x,dir.z);
	uv.x=phi*M_1_2PIf;
	float theta=acos(dir.y);
	uv.y=1-theta*M_1_PIf;
}

#pragma region POINT_LIGHTS
RT_CALLABLE_PROGRAM void PointLight_Sample_L(const SData_Light& data,LMD& lmd,uint& seed){
	//RT_CALLABLE_PROGRAM Radiance4 PointLight_Sample_L(const SData_Light& data,const Pos& pos_g,Dir& wi_g,float& pdf,uint& seed){
	lmd.wi_g=normalize(data.pos-lmd.hitPos_g);
	lmd.PDF=1.f;
	lmd.L=data.diffuse*data.intensity/DistanceSquared(data.pos,lmd.hitPos_g);
}

RT_CALLABLE_PROGRAM Radiance4 PointLight_Power(const SData_Light& data){ return M_4PIf*data.diffuse*data.intensity; }

RT_CALLABLE_PROGRAM float PointLight_Pdf(const SData_Light&,const Pos &p,const Dir &wi){ return 0.f; }
//RT_FUNC float3 Le(const RayDifferential &r) const;
//RT_FUNC void SHProject(const Pos &p,float pEpsilon,int lmax,float3 *coeffs){ }
#pragma endregion

#pragma region SPOT_LIGHTS
RT_CALLABLE_PROGRAM void SpotLight_Sample_L(const SData_Light& data,LMD& lmd,uint& seed){
	//RT_CALLABLE_PROGRAM float4 SpotLight_Sample_L(const SData_Light& data,const Pos& pos_g,Dir& wi_g,float& pdf,uint& seed){
	lmd.wi_g=normalize(data.pos-lmd.hitPos_g);
	lmd.PDF=1.f;
	float cosTotalWidth=cosf(F_RADIAN(data.totalTheta));
	float cosFalloffStart=cosf(F_RADIAN(data.fullLightTheta));
	float cosTheta=dot(-lmd.wi_g,data.dir);
	lmd.L=data.diffuse*data.intensity * Falloff(cosTheta,cosTotalWidth,cosFalloffStart)/DistanceSquared(data.pos,lmd.hitPos_g);
}
RT_CALLABLE_PROGRAM Radiance4 SpotLight_Power(const SData_Light& data){ return M_4PIf*data.diffuse*data.intensity; }

/**
* 针对未与场景几何体对象碰撞的光线，无穷远区域光源须向其“贡献”辐射度。对此，可向Light基类加入一方法，沿未碰撞光线并根据当前光源返回发射辐射度。
* 针对其他类型光源，默认实现并不返回辐射度。
*/
RT_CALLABLE_PROGRAM float SpotLight_Pdf(const SData_Light&,const Pos &p,const Dir &wi){ return 0.f; }
//RT_FUNC float3 Le(const RayDifferential &r) const;
//RT_FUNC void SHProject(const Pos &p,float pEpsilon,int lmax,float3 *coeffs){ }
#pragma endregion

#pragma region DISTANT_LIGHTS
RT_CALLABLE_PROGRAM void DistantLight_Sample_L(const SData_Light& data,LMD& lmd,uint& seed){
	//RT_CALLABLE_PROGRAM Irradiance4 DistantLight_Sample_L(const SData_Light& data,const Pos& pos_g,Dir& wi_g,float& pdf,uint& seed){
	lmd.wi_g=-data.dir;
	lmd.PDF=1.f;
	lmd.L=data.diffuse*data.intensity;
}

RT_CALLABLE_PROGRAM float4 DistantLight_Power(){ return make_float4(0.f); }

/**
* 针对未与场景几何体对象碰撞的光线，无穷远区域光源须向其“贡献”辐射度。对此，可向Light基类加入一方法，沿未碰撞光线并根据当前光源返回发射辐射度。
* 针对其他类型光源，默认实现并不返回辐射度。
*/
RT_CALLABLE_PROGRAM float DistantLight_Pdf(const SData_Light&,const Pos &p,const Dir &wi){ return 0.f; }
//RT_FUNC float3 Le(const RayDifferential &r) const;
//RT_FUNC void SHProject(const Pos &p,float pEpsilon,int lmax,float3 *coeffs){ }
#pragma endregion

#pragma region AREA_LIGHTS
/**
* 调用者负责传递场景内一点的世界空间位置，以及光照采样的时刻，最终返回到达该点的辐射度。
* 该函数还负责初始化相对于光源的入射方向wi，以及初始化VisibilityTester对象；
* 如果光源不是点光源，该方法需要对光源表面上的一点进行随机采样。
*/
RT_CALLABLE_PROGRAM void AreaLight_Sample_L(const SData_Light& data,LMD& lmd,uint& seed){
	//RT_CALLABLE_PROGRAM Irradiance4 AreaLight_Sample_L(const SData_Light& data,const Pos& pos_g,Dir& wi_g,float& PDF,uint& seed){

	int offset;
	float mapPDF;
	RandomFloat1 component=CF_GETSAMPLE1(seed);
	CF_DISTRIBUTION1D_SAMPLEDISCRETE(data,component,offset,mapPDF);

	// local space;
	const int3 v_idx=GET_I3_1D(data.bufferID_index,offset);
	Pos p0=GET_F3_1D(data.bufferID_vertex,v_idx.x);
	Pos p1=GET_F3_1D(data.bufferID_vertex,v_idx.y);
	Pos p2=GET_F3_1D(data.bufferID_vertex,v_idx.z);

	//to world space;
	//**************************************************start**************************************************
	// 下面函数导致将p0,p1,p2点转化为了当前mesh的坐标，从而出现错误。
	//p0=rtTransformPoint(RT_OBJECT_TO_WORLD,p0);
	//p1=rtTransformPoint(RT_OBJECT_TO_WORLD,p1);
	//p2=rtTransformPoint(RT_OBJECT_TO_WORLD,p2);
	// 应该为：
	CalcWithMatrix(p0,data.bufferID_worldMatrix);
	CalcWithMatrix(p1,data.bufferID_worldMatrix);
	CalcWithMatrix(p2,data.bufferID_worldMatrix);
	//***************************************************end*************************************************
	/*
	rtPrintf("%f,%f,%f\n",p0.x,p0.y,p0.z);
	rtPrintf("%f,%f,%f\n",p1.x,p1.y,p1.z);
	rtPrintf("%f,%f,%f\n",p2.x,p2.y,p2.z);
	*/

	RandomFloat2 ls=CF_GETSAMPLE2(seed);
	RandomVariable1 u,v;
	CF_UNIFORMSAMPLETRIANGLE(ls.x,ls.y,u,v);
	// centric coord.
	Pos p=u*p0+v*p1+(1.f-u-v)*p2;
	lmd.wi_g=p-lmd.hitPos_g;
	Nor n=normalize(cross(p1-p0,p2-p0));

	//**************************************************start**************************************************
	// 下面不做点积判断，因为那样会对可见区域过小的区域光造成大量采样丢失；从而噪点无法收敛；
	//if(dot(n,-wi_g)>0){
	//same side;
	//***************************************************end*************************************************
	const float dis=length(lmd.wi_g);
	//**************************************************start**************************************************
	// 以前这里有单位化，去掉的原因是外部需要知道wi_g的长度，从而发射阴影查询射线；
	//wi_g/=dis;
	//***************************************************end*************************************************

	// calc PDF;
	const float area=rtBufferId<float,1>(data.meshAreaBufferID)[offset];

	//**************************************************start**************************************************
	// 下面这句
	//PDF=(dis*dis)/((AbsCosTheta(-wi_g,n)*area)/dis);
	//优化为：
	lmd.PDF=(dis*dis*dis)/(fabsf(dot(lmd.wi_g,n))*area);
	//***************************************************end*************************************************

	const SData_Material& md=rtBufferId<SData_Material,1>(data.bufferID_materials)[data.bufferOffset_materials];
	lmd.L=md.BS_textureDiffuse.color*md.BS_AOIntensity/(dis*dis);
}

RT_CALLABLE_PROGRAM float4 AreaLight_Power(){ return make_float4(0.f);/*TODO:return L*M_PIf*data.sumArea;*/ }

/**
* 针对未与场景几何体对象碰撞的光线，无穷远区域光源须向其“贡献”辐射度。对此，可向Light基类加入一方法，沿未碰撞光线并根据当前光源返回发射辐射度。
* 针对其他类型光源，默认实现并不返回辐射度。
*/
RT_CALLABLE_PROGRAM float AreaLight_Pdf(const SData_Light&,const Pos &p,const Dir &wi){ return 0.f; }
//RT_FUNC float3 Le(const RayDifferential &r) const;
//RT_FUNC void SHProject(const Pos &p,float pEpsilon,int lmax,float3 *coeffs){ }
#pragma endregion

#pragma region ENV_LIGHTS
/**
* 调用者负责传递场景内一点的世界空间位置，以及光照采样的时刻，最终返回到达该点的辐射度。
* 该函数还负责初始化相对于光源的入射方向wi，以及初始化VisibilityTester对象；
* 如果光源不是点光源，该方法需要对光源表面上的一点进行随机采样。
*/
RT_CALLABLE_PROGRAM void EnvLight_Sample_L(const SData_Light& data,LMD& lmd,uint& seed){
	//RT_CALLABLE_PROGRAM Irradiance4 EnvLight_Sample_L(const SData_Light& data,const Pos& pos_g,Dir& wi_g,float& pdf,uint& seed){
	RandomFloat2 rf2=CF_GETSAMPLE2(seed);
	UV2 uv; float mapPDF;
	CF_DISTRIBUTION2D_SAMPLECONTINUOUS(data,rf2,uv,mapPDF);
	//dtrb2D.SampleDiscrete(ls,uv,mapPDF);

	//rtPrintf("rt2[%f,%f],PDF[%f],,",rf2.x,rf2.y,mapPDF);
#if 0
	//Testing
	PDF=1.0f;
	wi_g=make_float3(0.f,1.f,0.f);
	return make_float3(mapPDF);
#endif

	if(mapPDF==0.f){
		lmd.L=make_float4(0.f);
		return;
	}

	float sintheta;
	UVToNor(uv,lmd.wi_g,sintheta);

#if 0
			if(uint(lmd.dim.x*.5f)==lmd.index.x && uint(lmd.dim.y*.5f)==lmd.index.y){
				rtPrintf("sintheta[%f],,",sintheta);
			}
#endif

	if(sintheta<=0.f){
		lmd.L=make_float4(0.f);
	} else{
#if 0
		rtPrintf("wi_g is:%f,%f,%f\n",wi_g.x,wi_g.y,wi_g.z);
#endif
		CalcWithMatrix(lmd.wi_g,data.bufferID_worldMatrix);
		lmd.wi_g*=M_MAX_FLOAT;
		lmd.PDF=mapPDF/(M_2PIf*M_PIf*sintheta);
		lmd.L=data.HDR_Intensity*rtTex2D<float4>(data.HDR_textureSamplerID,uv.x,uv.y); //test:Radiance4 L=make_float4(0.1f);;
	}
}

RT_CALLABLE_PROGRAM float3 EnvLight_Power(){ return make_float3(0.f); }

RT_CALLABLE_PROGRAM float EnvLight_Pdf(const SData_Light& data,const Pos&,const Dir &wi){
	UV2 uv;
	DirToUV(wi,uv);
	const float sintheta=sinf(uv.y*M_PIf);
	float tmp=CF_DISTRIBUTION2D_PDF(data,uv);
	return tmp/(M_2PIf*M_PIf*sintheta);
}
/**
* 针对未与场景几何体对象碰撞的光线，无穷远区域光源须向其“贡献”辐射度。对此，可向Light基类加入一方法，沿未碰撞光线并根据当前光源返回发射辐射度。
* 针对其他类型光源，默认实现并不返回辐射度。
*/
//RT_FUNC float3 Le(const RayDifferential &r) const;
//RT_FUNC void SHProject(const Pos &p,float pEpsilon,int lmax,float3 *coeffs){ }
#pragma endregion

#pragma endregion
