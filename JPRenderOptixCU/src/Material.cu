#include <optix_device.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu\optixu_matrix_namespace.h>
#include "cu_common.h"
#include "intersection.h"
#include "per_ray_data.h"
#include "../../host_device/data_renderer.h"
#include "../../host_device/data_light.h"
#include "../../host_device/data_instance.h"

using namespace optix;

#include "light_model_data.h"

struct PRD_Shadow{
	bool canSeeLight;
};

rtDeclareVariable(Ray,dRay,rtCurrentRay,);
rtDeclareVariable(float,dT,rtIntersectionDistance,);
rtDeclareVariable(PRD,prdR,rtPayload,);
rtDeclareVariable(PRD_Shadow,prdS,rtPayload,);
rtDeclareVariable(Intersection,aInset,attribute aInset,);
rtDeclareVariable(SData_Instance,V_C_DATA_INSTANCE,,);
rtDeclareVariable(SData_Renderer,V_C_DATA_RENDERER,,);
rtDeclareVariable(rtObject,V_C_TOP_OBJECT,,);

rtDeclareVariable(rtCallableProgramId<RandomFloat1(uint&)>,CF_GETSAMPLE1,,);
rtDeclareVariable(rtCallableProgramId<RandomFloat2(uint&)>,CF_GETSAMPLE2,,);
rtDeclareVariable(rtCallableProgramId<Color4(int TSID,const Dir&)>,CF_GETSKYBOXRADIANCE,,);
rtDeclareVariable(rtCallableProgramId<float(int,int,const Nor&,float,Color4&)>,CF_GETENVRADIANCE,,);
rtDeclareVariable(rtCallableProgramId<float(float3,float)>,CF_CHECKERBOARD3,,);
rtDeclareVariable(rtCallableProgramId<Color4(int,int,UV2)>,CF_GETTEXTURE_DIFFUSE,,);
rtDeclareVariable(rtCallableProgramId<Color4(int,int,UV2)>,CF_GETTEXTURE_GLOSSY,,);
rtDeclareVariable(rtCallableProgramId<Nor(const SData_Texture&,Intersection)>,CF_GETTEXTURE_NORMAL,,);
rtDeclareVariable(rtCallableProgramId<Nor(int,const Pos&,const Nor&)>,CF_GETTRIPLANENORMAL,,);
rtDeclareVariable(rtCallableProgramId<uint(uint,uint)>,CF_GETRANDOMSEED,,);

// light;
rtBuffer<rtCallableProgramId<float(const SData_Light&,const Pos&,const Dir&)>,1> buf_light_PDF;
rtBuffer<rtCallableProgramId<Radiance4(const SData_Light&)>,1> buf_light_Power;
rtBuffer<rtCallableProgramId<void(const SData_Light&,LMD&,uint&)>,1> buf_light_sample_L;

// light model;
//rtBuffer<rtCallableProgramId<void(LMD&,uint&)>,1> buf_lm_PDF;
rtBuffer<rtCallableProgramId<void(LMD&,uint&)>,1> buf_lm_F;
rtBuffer<rtCallableProgramId<void(LMD&,uint&)>,1> buf_lm_sample_F;
//rtBuffer<rtCallableProgramId<void(LMD&)>,1> buf_lm_albedo;


#define __ONLY_FUNC__
#include "common_texture_operation.h"
#undef __ONLY_FUNC__


#pragma region helper
RT_FUNC_S Color4 MergeColor(Color4 source,Color4 target,float gamma){
	Color4 tmp=source*source.w+(1-source.w)*target;
	const float F{1.f/gamma};
	tmp.x=powf(tmp.x,F);
	tmp.y=powf(tmp.y,F);
	tmp.z=powf(tmp.z,F);
	tmp.w=1.f;
	return tmp;
}

RT_FUNC_F float luminanceCIE(const float4& rgba){
	const float4 ntsc_luminance={0.30f, 0.59f, 0.11f,0.f};
	return dot(rgba,ntsc_luminance);
}

RT_FUNC_S void HandelLight(const uint BRDFindex,LMD& lmd,uint& seed){
#if 0
	//test:
	return make_float4(1.f);
#endif

	Radiance4 L_out=make_float4(0.f);
	Color4 C_out=make_float4(1.f);

	int N=rtBufferId<SData_Light,1>(V_C_DATA_RENDERER.bufferID_lights).size();
	for(uint i=0u; i<N; ++i){
		const SData_Light& ld=rtBufferId<SData_Light,1>(V_C_DATA_RENDERER.bufferID_lights)[i];

		buf_light_sample_L[ld.lightFuncIndex](ld,lmd,seed);
		if(luminanceCIE(lmd.L)<=M_MIN_RADIANCE)continue;
		if(lmd.PDF<=M_MIN_PDF)continue;
		const float dis=length(lmd.wi_g);
		lmd.wi_g=lmd.wi_g/dis;

		const float cosTheta=lmd.WiDotN();
		if(cosTheta>0.f){
			Ray ray=make_Ray(lmd.hitPos_g,lmd.wi_g,OptixRayTypes::SHADOW,V_C_DATA_RENDERER.ray_clamp_min,dis);
			PRD_Shadow prd;
			prd.canSeeLight=true;
			rtTrace(V_C_TOP_OBJECT,ray,prd);

			if(prd.canSeeLight){
				buf_lm_F[BRDFindex](lmd,seed);
				L_out+=lmd.BRDF*lmd.L*cosTheta/lmd.PDF;
			}
		}
	}// end for
#if 0
	if(uint(dLd.x*.5f)==dLi.x && uint(dLd.y*.5f)==dLi.y){
		rtPrintf("C_out[%f,%f,%f],,",C_out.x,C_out.y,C_out.z);
	}
#endif
	lmd.L=L_out;
}

RT_FUNC_S void Miss(){
#if 0
	//test
	if(uint(dLd.x*.5f)==dLi.x && uint(dLd.y*.5f)==dLi.y){
		rtPrintf("missed - in PgmMiss");
	}
#endif
	Nor rayDir=dRay.direction;
	if(prdR.uDepth==0u){
		if(V_C_DATA_RENDERER.skybox_enableBackground){
			prdR.color_out=CF_GETSKYBOXRADIANCE(V_C_DATA_RENDERER.skybox_TSID_show,rayDir);
			prdR.luminance_out=make_float4(1.f);
		} else{
			prdR.luminance_out=make_float4(0.f);
		}

		if(V_C_DATA_RENDERER.enableAIdenoise){
			prdR.albedo=make_float4(0.f,0.f,0.f,1.f);
			prdR.n_g=make_float3(0.f);
		}
	} else{
		Color4 color;
		prdR.luminance_out=make_float4(CF_GETENVRADIANCE(V_C_DATA_RENDERER.skybox_TSID_show,V_C_DATA_RENDERER.bufferID_lights,rayDir,prdR.lastMaterialEnvBlend,color));
		prdR.color_out=color;
	}

	// 终止射线，使其不要继续迭代；
	prdR.missed=true;
}

RT_FUNC_F void AdjustVecBySize(const float3& f3,const float size,RandomFloat2* rf2){
	float3 d,d2;
	const float INV_SIZE=1.f/size;
	float3 v1=make_float3(1.4142f,-1.4142f,0.f);
	float3 v2=make_float3(0,1.4142f,-1.4142f);
	float3 v3=make_float3(1.4142f,0.f,-1.4142f);
	d.x=int(floor(dot(v1,f3)*INV_SIZE));
	d.y=int(floor(dot(v2,f3)*INV_SIZE));
	d.z=int(floor(dot(v3,f3)*INV_SIZE));
	d*=size;

	v1=make_float3(0.935f,0.25f,0.25f);
	v2=make_float3(0.25f,0.935f,0.25f);
	v3=make_float3(0.25f,0.25f,0.935f);
	d2.x=int(floor(dot(v1,f3)*INV_SIZE));
	d2.y=int(floor(dot(v2,f3)*INV_SIZE));
	d2.z=int(floor(dot(v3,f3)*INV_SIZE));
	d2*=size;

	float a=sin(d.x*12.9898f+d.y*78.233f+d.z*45.164f+d2.x*94.673f+d2.y*45.164f+d2.z*78.233f);
	a=fabsf(a*43758.5453f);
	rf2->x=a-int(a);

	a=sin(d.y*12.9898f+d.z*78.233f+d2.x*45.164f+d2.y*94.673f+d2.z*45.164f+d.x*78.233f);
	a=fabsf(a*43758.5453f);
	rf2->y=a-int(a);

}

RT_FUNC_S void HandleMetallicFlakeAndBase(LMD& lmd,uint& seed,const Color4& color_diffuse){
	//uint index=uint(length(lmd.hitPos_g)*0xFFFFFF)%0xFFFFFFFF;
	//uint newSeed=CF_GETRANDOMSEED(aInset.texCoord.x,11);
	SData_Material& md=rtBufferId<SData_Material,1>(V_C_DATA_INSTANCE.materialBufferID)[V_C_DATA_INSTANCE.materialBufferOffset];
	RandomFloat2 v2;
	AdjustVecBySize(lmd.hitPos_g,md.CF_size,&v2);
	if(v2.x<md.CF_density){
		//flake;
		v2.x/=md.CF_density;
		const float theta=atanf(md.CF_irregularity*sqrtf(-logf(1.f-v2.x)));
		const float sintheta=sinf(theta);
		const float costheta=cosf(theta);
		//const float phi=M_PI_2f;
		const float phi=M_2PIf*v2.y;

		Onb onb(lmd.sn_g);
		Nor flakeNor=make_float3(sintheta * cosf(phi),sintheta * sinf(phi),costheta);
		onb.inverse_transform(flakeNor);
		flakeNor=normalize(flakeNor);

		//buf_lm_sample_F[CFINDEX_LM_SPECULAR_REFLECT](lmd,seed);
		//change to :
		lmd.wi_g=reflect(-lmd.wo_g,flakeNor);
		if(lmd.WiDotN()<0.f){
			lmd.wi_g*=-1.f;
		}
		lmd.NormalizeWi();
		lmd.PDF=1.f;
		lmd.BRDF=1.f/fabsf(lmd.WiDotN());


		//lmd.sn_g=oldSn_g;
		prdR.color_out=md.CF_color;
		if(prdR.lastMaterialEnvBlend>1.f) prdR.lastMaterialEnvBlend=1.f;
		lmd.L=make_float4(0.f);
	} else{
		//base;
		HandelLight(CFINDEX_LM_LAMBERTIAN,lmd,seed);
		buf_lm_sample_F[CFINDEX_LM_LAMBERTIAN](lmd,seed);
		prdR.color_out=color_diffuse;
		prdR.lastMaterialEnvBlend=0.f;
		if(prdR.uDepth<V_C_DATA_RENDERER.ray_iterateDepth-1u){
			prdR.uDepth=V_C_DATA_RENDERER.ray_iterateDepth-1u;
		}
	}
}
#pragma endregion



RT_PROGRAM void PgmAH_radiance_shadow(){
	prdS.canSeeLight=false;
	rtTerminateRay();
}




RT_PROGRAM void PgmCH_radiance(){
#if 0
	//test
	if(uint(dLd.x*.5f)==dLi.x && uint(dLd.y*.5f)==dLi.y){
		rtPrintf("closet hit - \n");
	}
#endif


	/// 2. basic data;
	LMD lmd;
	lmd.wo_g=-dRay.direction;
	lmd.hitPos_g=dRay.origin+dT*dRay.direction;
	lmd.sn_g=normalize(rtTransformNormal(RT_OBJECT_TO_WORLD,aInset.geoNor));
	uint seed=prdR.seed;
	const SData_Material& md=
		rtBufferId<SData_Material,1>(V_C_DATA_INSTANCE.materialBufferID)[V_C_DATA_INSTANCE.materialBufferOffset];


	/// 3. material color;
	//Color4 color_diffuse=CF_GETTEXTURE_DIFFUSE(md.BS_textureDiffuse,aInset.texCoord);
	Color4 color_diffuse=CF_GETTEXTURE_DIFFUSE(V_C_DATA_INSTANCE.materialBufferID
		,V_C_DATA_INSTANCE.materialBufferOffset,aInset.texCoord);
	Color4 color_glossy=CF_GETTEXTURE_GLOSSY(V_C_DATA_INSTANCE.materialBufferID
		,V_C_DATA_INSTANCE.materialBufferOffset,aInset.texCoord);

#if 0
	//test
	rtPrintf("color_diffuse,[%f,%f,%f]\n",color_diffuse.x,color_diffuse.y,color_diffuse.z);
#endif

	/// 4. BRDF;
	switch(md.materialIndex){
	case MATERIAL_EMISSIVE_INDEX:
	{
		if(prdR.uDepth==0u){
			prdR.color_out=color_diffuse*md.BS_AOIntensity;
			prdR.luminance_out=make_float4(1.f);
		} else{
			prdR.luminance_out=make_float4(0.f);
		}
		prdR.missed=true;
		return;
	}
	break;
	case MATERIAL_SHADOW_INDEX:
	{
		Miss();
		int N=rtBufferId<SData_Light,1>(V_C_DATA_RENDERER.bufferID_lights).size();
		for(uint i=0u; i<N; ++i){
			const SData_Light& ld=rtBufferId<SData_Light,1>(V_C_DATA_RENDERER.bufferID_lights)[i];

			buf_light_sample_L[ld.lightFuncIndex](ld,lmd,seed);
			const float dis=length(lmd.wi_g);
			lmd.wi_g=lmd.wi_g/dis;

			Ray ray=make_Ray(lmd.hitPos_g,lmd.wi_g,OptixRayTypes::SHADOW,V_C_DATA_RENDERER.ray_clamp_min,dis);
			PRD_Shadow prd;
			prd.canSeeLight=true;
			rtTrace(V_C_TOP_OBJECT,ray,prd);

			if(!prd.canSeeLight){
				prdR.luminance_out*=md.GE_shadowAttenuation;
			}
		}// end for
		prdR.seed=seed;
		return;
	}
	break;
	case MATERIAL_GLASS_INDEX:
	{
		const SData_Texture& data=md.GE_texture;
		Nor nor=CF_GETTEXTURE_NORMAL(data,aInset);
		lmd.sn_g=normalize(rtTransformNormal(RT_OBJECT_TO_WORLD,nor));
		lmd.IOR=md.BS_ior;
		const RandomFloat1 u=CF_GETSAMPLE1(seed);
		float fr=fresnel_schlick(fabsf(dot(lmd.wo_g,lmd.sn_g)),md.BS_fresnel.exponent,md.BS_fresnel.fmin,md.BS_fresnel.fmax);
		if(u<fr){
			buf_lm_sample_F[CFINDEX_LM_SPECULAR_REFLECT](lmd,seed);
			prdR.color_out=color_glossy;
		} else{
			buf_lm_sample_F[CFINDEX_LM_SPECULAR_REFRACT](lmd,seed);
			prdR.color_out=color_diffuse;
		}
		lmd.L=make_float4(0.f);
		if(prdR.lastMaterialEnvBlend>1.f) prdR.lastMaterialEnvBlend=1.f;
	}
	break;
	case MATERIAL_METAL_INDEX:
	{
		const SData_Texture& data=md.GE_texture;
		Nor nor=CF_GETTEXTURE_NORMAL(data,aInset);
		lmd.sn_g=normalize(rtTransformNormal(RT_OBJECT_TO_WORLD,nor));
		lmd.roughness=1.f/md.BS_roughness;
		HandelLight(CFINDEX_LM_TORRANCESPARROW,lmd,seed);
		prdR.color_out=color_diffuse*color_glossy;
		buf_lm_sample_F[CFINDEX_LM_TORRANCESPARROW](lmd,seed);
		const float blend=1.f-md.BS_roughness;
		if(prdR.lastMaterialEnvBlend>blend) prdR.lastMaterialEnvBlend=blend;
		if(prdR.uDepth<V_C_DATA_RENDERER.ray_iterateDepth-1u){
			prdR.uDepth=V_C_DATA_RENDERER.ray_iterateDepth-1u;
		}
	}
	break;
	case MATERIAL_METAL_BRUSHED_INDEX:
	{
		const SData_Texture& data=md.GE_texture;
		Nor nor=CF_GETTEXTURE_NORMAL(data,aInset);
		lmd.sn_g=normalize(rtTransformNormal(RT_OBJECT_TO_WORLD,nor));
		Pos center_g=normalize(rtTransformPoint(RT_OBJECT_TO_WORLD,make_float3(md.BS_center)));

		Dir to_center_g=center_g-lmd.hitPos_g;
		////////////////////////////////////////////////////////////////////////////////////////////////////
		//TODO: fix in the future;
		//if(data.M_MPROP_BS_usePlanar) tangent=::normalize(::cross(n_g,center-hp_g));
		//else tangent=::normalize(::cross(n_g,center));
		// replace the following with above;
		lmd.tangent_g=normalize(cross(lmd.sn_g,to_center_g));
		////////////////////////////////////////////////////////////////////////////////////////////////////
		lmd.bitangent_g=normalize(cross(lmd.sn_g,lmd.tangent_g));
		lmd.reflatance.x=md.BS_reflectanceDiffuse;
		lmd.reflatance.y=md.BS_reflectanceSpecular;
		lmd.anisotropicUV.x=md.BS_anisotropyU;
		lmd.anisotropicUV.y=md.BS_anisotropyV;
		HandelLight(CFINDEX_LM_ASHIKHMIN_SHIRLEY,lmd,seed);
		prdR.color_out=color_diffuse*color_glossy;
		buf_lm_sample_F[CFINDEX_LM_ASHIKHMIN_SHIRLEY](lmd,seed);
		if(prdR.lastMaterialEnvBlend>1.f) prdR.lastMaterialEnvBlend=1.f;
		if(prdR.uDepth<V_C_DATA_RENDERER.ray_iterateDepth-1u){
			prdR.uDepth=V_C_DATA_RENDERER.ray_iterateDepth-1u;
		}
	}
	break;
	case MATERIAL_WOOD_INDEX:
	{
		const RandomFloat1 u=CF_GETSAMPLE1(seed);
		if(u<md.CC_weight){
			// coat
			const SData_Texture& data=md.CC_texture;
			Nor nor=CF_GETTEXTURE_NORMAL(data,aInset);
			lmd.sn_g=normalize(rtTransformNormal(RT_OBJECT_TO_WORLD,nor));
			buf_lm_sample_F[CFINDEX_LM_SPECULAR_REFLECT](lmd,seed);
			lmd.L=make_float4(0.f);
			prdR.color_out=md.CC_texture.color;
			if(prdR.lastMaterialEnvBlend>1.f) prdR.lastMaterialEnvBlend=1.f;
		} else{
			const SData_Texture& data=md.GE_texture;
			Nor nor=CF_GETTEXTURE_NORMAL(data,aInset);
			lmd.sn_g=normalize(rtTransformNormal(RT_OBJECT_TO_WORLD,nor));
			HandelLight(CFINDEX_LM_LAMBERTIAN,lmd,seed);
			prdR.color_out=color_diffuse;
			buf_lm_sample_F[CFINDEX_LM_LAMBERTIAN](lmd,seed);
			prdR.lastMaterialEnvBlend=0.f;
			prdR.uDepth=V_C_DATA_RENDERER.ray_iterateDepth;
		}
	}
	break;
	case MATERIAL_RUBBER_INDEX:
	{
		const SData_Texture& data=md.GE_texture;
		Nor nor=CF_GETTEXTURE_NORMAL(data,aInset);
		lmd.sn_g=normalize(rtTransformNormal(RT_OBJECT_TO_WORLD,nor));
		float fr;
		if(md.BS_fresnel.fmax<md.BS_fresnel.fmin){
			fr=fresnel_schlick(fabsf(lmd.WoDotN()),md.BS_fresnel.exponent,md.BS_fresnel.fmax,md.BS_fresnel.fmin);
			fr=1.f-fr;
		} else{
			fr=fresnel_schlick(fabsf(lmd.WoDotN()),md.BS_fresnel.exponent,md.BS_fresnel.fmin,md.BS_fresnel.fmax);
		}

		const RandomFloat1 u1=CF_GETSAMPLE1(seed);
		if(u1<fr){
			buf_lm_sample_F[CFINDEX_LM_SPECULAR_REFLECT](lmd,seed);
			prdR.color_out=md.BS_colorReflect;
			if(prdR.lastMaterialEnvBlend>1.f) prdR.lastMaterialEnvBlend=1.f;
			lmd.L=make_float4(0.f);
		} else{
			HandelLight(CFINDEX_LM_LAMBERTIAN,lmd,seed);
			prdR.color_out=color_diffuse*color_glossy;
			buf_lm_sample_F[CFINDEX_LM_LAMBERTIAN](lmd,seed);
			prdR.lastMaterialEnvBlend=0.f;
			prdR.uDepth=V_C_DATA_RENDERER.ray_iterateDepth;
		}
	}
	break;
	case MATERIAL_LEATHER_INDEX:
	{
		GetNormal_triplanar(md.GE_texture,lmd.hitPos_g,lmd.sn_g);
		float fr;
		if(md.BS_fresnel.fmax<md.BS_fresnel.fmin){
			fr=fresnel_schlick(fabsf(lmd.WoDotN()),md.BS_fresnel.exponent,md.BS_fresnel.fmax,md.BS_fresnel.fmin);
			fr=1.f-fr;
		} else{
			fr=fresnel_schlick(fabsf(lmd.WoDotN()),md.BS_fresnel.exponent,md.BS_fresnel.fmin,md.BS_fresnel.fmax);
		}

		const RandomFloat1 u1=CF_GETSAMPLE1(seed);
		if(u1<fr){
			buf_lm_sample_F[CFINDEX_LM_SPECULAR_REFLECT](lmd,seed);
			prdR.color_out=md.BS_colorReflect;
			if(prdR.lastMaterialEnvBlend>1.f) prdR.lastMaterialEnvBlend=1.f;
			lmd.L=make_float4(0.f);
		} else{
			HandelLight(CFINDEX_LM_LAMBERTIAN,lmd,seed);
			prdR.color_out=color_diffuse*color_glossy;
			buf_lm_sample_F[CFINDEX_LM_LAMBERTIAN](lmd,seed);
			prdR.lastMaterialEnvBlend=0.f;
			prdR.uDepth=V_C_DATA_RENDERER.ray_iterateDepth;
		}
	}
	break;
	case MATERIAL_CARPAINT_UNIQUE_CLEAR_INDEX:
	{
		const RandomFloat2 u2=CF_GETSAMPLE2(seed);
		if(u2.x<md.CC_weight){

			float fr=fresnel_schlick(fabsf(lmd.WoDotN()),md.CC_fresnel.exponent,md.CC_fresnel.fmin,md.CC_fresnel.fmax);
			if(u2.y<fr){
				// coat
				const SData_Texture& data=md.CC_texture;
				Nor nor=CF_GETTEXTURE_NORMAL(data,aInset);
				lmd.sn_g=normalize(rtTransformNormal(RT_OBJECT_TO_WORLD,nor));
				buf_lm_sample_F[CFINDEX_LM_SPECULAR_REFLECT](lmd,seed);
				prdR.color_out=md.CC_texture.color;
				if(prdR.lastMaterialEnvBlend>1.f) prdR.lastMaterialEnvBlend=1.f;
				lmd.L=make_float4(0.f);
			} else{
				HandelLight(CFINDEX_LM_LAMBERTIAN,lmd,seed);
				prdR.color_out=color_diffuse;
				buf_lm_sample_F[CFINDEX_LM_LAMBERTIAN](lmd,seed);
				prdR.lastMaterialEnvBlend=0.f;
				if(prdR.uDepth<V_C_DATA_RENDERER.ray_iterateDepth-1u){
					prdR.uDepth=V_C_DATA_RENDERER.ray_iterateDepth-1u;
				}
			}

		} else{
			HandelLight(CFINDEX_LM_LAMBERTIAN,lmd,seed);
			prdR.color_out=color_diffuse;
			buf_lm_sample_F[CFINDEX_LM_LAMBERTIAN](lmd,seed);
			prdR.lastMaterialEnvBlend=0.f;
			if(prdR.uDepth<V_C_DATA_RENDERER.ray_iterateDepth-1u){
				prdR.uDepth=V_C_DATA_RENDERER.ray_iterateDepth-1u;
			}
		}
	}
	break;
	case MATERIAL_CARPAINT_METALLIC_INDEX:
	{

		const RandomFloat2 u2=CF_GETSAMPLE2(seed);
		if(u2.x<md.CC_weight){

			float fr=fresnel_schlick(fabsf(lmd.WoDotN()),md.CC_fresnel.exponent,md.CC_fresnel.fmin,md.CC_fresnel.fmax);
			if(u2.y<fr){
				// coat
				const SData_Texture& data=md.CC_texture;
				Nor nor=CF_GETTEXTURE_NORMAL(data,aInset);
				lmd.sn_g=normalize(rtTransformNormal(RT_OBJECT_TO_WORLD,nor));
				buf_lm_sample_F[CFINDEX_LM_SPECULAR_REFLECT](lmd,seed);
				prdR.color_out=md.CC_texture.color;
				if(prdR.lastMaterialEnvBlend>1.f) prdR.lastMaterialEnvBlend=1.f;
				lmd.L=make_float4(0.f);
			} else{
				HandleMetallicFlakeAndBase(lmd,seed,color_diffuse);
			}

		} else{
			HandleMetallicFlakeAndBase(lmd,seed,color_diffuse);
			}
		}
	break;
	case MATERIAL_CARPAINT_UNIQUE_INDEX:
	case MATERIAL_MATTE_INDEX:
	{
		HandelLight(CFINDEX_LM_LAMBERTIAN,lmd,seed);
		prdR.color_out=color_diffuse;
		buf_lm_sample_F[CFINDEX_LM_LAMBERTIAN](lmd,seed);
		prdR.lastMaterialEnvBlend=0.f;
		prdR.uDepth=V_C_DATA_RENDERER.ray_iterateDepth;
#if 0
		//test
		rtPrintf("bufid[%d],bufoffset[%d]\n",V_C_DATA_INSTANCE.materialBufferID,V_C_DATA_INSTANCE.materialBufferOffset);
		rtPrintf("dc[%f]\n",md.BS_textureDiffuse.color.x);
#endif
	}
	break;
   case MATERIAL_UV_INDEX:
   {
     HandelLight(CFINDEX_LM_LAMBERTIAN,lmd,seed);
     prdR.color_out=color_diffuse;
     buf_lm_sample_F[CFINDEX_LM_LAMBERTIAN](lmd,seed);
     prdR.lastMaterialEnvBlend=0.f;
     prdR.uDepth=V_C_DATA_RENDERER.ray_iterateDepth;
   }
   break;
	case MATERIAL_DEFAULT_INDEX:
	default:
	{
		prdR.color_out=color_diffuse*(.05f+.95f*CF_CHECKERBOARD3(lmd.hitPos_g,md.BS_checkboardSize));
		prdR.luminance_out=make_float4(1.f);
		prdR.missed=true;
		return;
	}
	break;
	}

	/// 5. return;
	prdR.materialIndex=md.materialIndex;
	prdR.luminance_out=lmd.L;
	prdR.BRDF=lmd.BRDF;
	prdR.PDF=lmd.PDF;
	prdR.seed=seed;
	prdR.hp_g=lmd.hitPos_g;
	prdR.n_g=lmd.sn_g;
	prdR.wi_g=lmd.wi_g;
	if(V_C_DATA_RENDERER.enableAIdenoise) prdR.albedo=prdR.color_out;
}




RT_PROGRAM void PgmMiss_radiance(){
	Miss();
}

#if 0
//test
if(uint(dLd.x*.5f)==dLi.x && uint(dLd.y*.5f)==dLi.y){
	//rtPrintf("prd data is:uv[%f,%f],wo_g[%f,%f,%f],hp_g[%f,%f,%f]",prdR.uv.x,prdR.uv.y,prdR.wo_g.x,prdR.wo_g.y,prdR.wo_g.z,prdR.hp_g.x,prdR.hp_g.y,prdR.hp_g.z);
	//rtPrintf("wi_g[%f,%f,%f]",prdR.wi_g.x,prdR.wi_g.y,prdR.wi_g.z);
	rtPrintf("gn_g[%f,%f,%f],depth[%d],BRDF[%d]\n",prdR.gn_g.x,prdR.gn_g.y,prdR.gn_g.z,prdR.uDepth,prdR.BRDFindex);
}
#endif
