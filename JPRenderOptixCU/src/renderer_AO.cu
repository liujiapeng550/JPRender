#include <optix_device.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu\optixu_matrix_namespace.h>
#include "cu_common.h"
#include "intersection.h"
#include "per_ray_data.h"
#include "../../host_device/data_context.h"
#include "../../host_device/data_renderer.h"
#include "../../host_device/data_instance.h"

using namespace optix;

#include "light_model_data.h"

rtDeclareVariable(uint2,dLd,rtLaunchDim,);
rtDeclareVariable(uint2,dLi,rtLaunchIndex,);
rtDeclareVariable(Ray,dRay,rtCurrentRay,);
rtDeclareVariable(float,dT,rtIntersectionDistance,);
rtDeclareVariable(Intersection,aInset,attribute aInset,);
rtDeclareVariable(rtObject,V_C_TOP_OBJECT,,);
rtDeclareVariable(SData_Context,V_C_DATA_CONTEXT,,);
rtDeclareVariable(SData_Renderer,V_C_DATA_RENDERER,,);
rtDeclareVariable(SData_Instance,V_C_DATA_INSTANCE,,);
rtDeclareVariable(PRD,prdR,rtPayload,);


// callable
rtDeclareVariable(rtCallableProgramId<uint(uint,uint)>,CF_GETRANDOMSEED,,);
rtDeclareVariable(rtCallableProgramId<RandomFloat1(uint&)>,CF_GETSAMPLE1,,);
rtDeclareVariable(rtCallableProgramId<RandomFloat2(uint&)>,CF_GETSAMPLE2,,);
rtDeclareVariable(rtCallableProgramId<Dir(const RandomFloat2&,float,float)>,CF_COSINESAMPLEHEMISPHERE_QUICKER,,);
rtDeclareVariable(rtCallableProgramId<Nor(const SData_Texture&,Intersection)>,CF_GETTEXTURE_NORMAL,,);
rtBuffer<rtCallableProgramId<void(LMD&,uint&)>,1> buf_lm_sample_F;

#define __ONLY_FUNC__
#include "common_AO_launch.h"
#undef __ONLY_FUNC__

RT_PROGRAM void PgmLaunch_AO(){

	/// 1. 计算像素索引，获取种子；
	const int index=dLi.y*dLd.x+dLi.x;
	uint seed=CF_GETRANDOMSEED(index,V_C_DATA_RENDERER.frame_current);


	Radiance4 fRadiance=make_float4(AOCommonLaunch(seed));

	if(V_C_DATA_RENDERER.frame_current>1u){
		float weight=1.f/(float)V_C_DATA_RENDERER.frame_current;
		Radiance4 L=GET_F4_2D(V_C_DATA_RENDERER.output_launchBufID,dLi);
		fRadiance=lerp(L,fRadiance,weight);
		fRadiance.w=1.f;
	} 
	SET_F4_2D(V_C_DATA_RENDERER.output_launchBufID,dLi,fRadiance);

}


RT_PROGRAM void PgmCH_AO(){
	SData_Material& md=
		rtBufferId<SData_Material,1>(V_C_DATA_INSTANCE.materialBufferID)[V_C_DATA_INSTANCE.materialBufferOffset];
	prdR.hp_g=dRay.origin+dT*dRay.direction;
	if(md.materialIndex==MATERIAL_GLASS_INDEX){
		prdR.AO_nextIteration=true;


		LMD lmd;
		lmd.wo_g=-dRay.direction;
		lmd.hitPos_g=prdR.hp_g;
		lmd.sn_g=normalize(rtTransformNormal(RT_OBJECT_TO_WORLD,aInset.geoNor));
		uint seed=prdR.seed;

		const SData_Texture& data=md.GE_texture;
		Nor nor=CF_GETTEXTURE_NORMAL(data,aInset);
		lmd.sn_g=normalize(rtTransformNormal(RT_OBJECT_TO_WORLD,nor));
		lmd.IOR=md.BS_ior;
		const RandomFloat1 u=CF_GETSAMPLE1(seed);
		float fr=fresnel_schlick(fabsf(dot(lmd.wo_g,lmd.sn_g)),md.BS_fresnel.exponent,md.BS_fresnel.fmin,md.BS_fresnel.fmax);
		if(u<fr){
			buf_lm_sample_F[CFINDEX_LM_SPECULAR_REFLECT](lmd,seed);
		} else{
			buf_lm_sample_F[CFINDEX_LM_SPECULAR_REFRACT](lmd,seed);
		}
		prdR.wi_g=lmd.wi_g;
		prdR.seed=seed;

	} else{
		prdR.AO_intensity=md.BS_AOIntensity;
		if(prdR.uDepth==0){
			// AO
			Dir n_g=normalize(rtTransformNormal(RT_OBJECT_TO_WORLD,aInset.geoNor));
			//n_g=faceforward(n_g,-dRay.direction,n_g); // 翻转容易出现黑边；
			prdR.n_g=normalize(n_g);
			++prdR.uDepth;
		} else{
			// result based AO;

			prdR.AO_distancePercent=dT/V_C_DATA_RENDERER.ray_clamp_max_AO;
			/*
			float distanceAttenuation=clamp(dT/V_C_DATA_RENDERER.ray_clamp_max_AO,0.f,1.f);
			float areaAttenuation=V_C_DATA_RENDERER.ray_clamp_min_AO;
			prdR.AO_distancePercent=(distanceAttenuation+areaAttenuation)*.5f;
			*/
		}
	}
}

