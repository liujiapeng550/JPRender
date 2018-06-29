#pragma once
#ifndef __ONLY_FUNC__
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

rtDeclareVariable(optix::uint2,dLd,rtLaunchDim,);
rtDeclareVariable(optix::uint2,dLi,rtLaunchIndex,);
rtDeclareVariable(rtObject,V_C_TOP_OBJECT,,);
rtDeclareVariable(SData_Renderer,V_C_DATA_RENDERER,,);


// callable
//rtDeclareVariable(rtCallableProgramId<RandomFloat1(uint&)>,CF_GETSAMPLE1,,);
rtDeclareVariable(rtCallableProgramId<RandomFloat2(uint&)>,CF_GETSAMPLE2,,);
rtDeclareVariable(rtCallableProgramId<Dir(const RandomFloat2&,float,float)>,CF_COSINESAMPLEHEMISPHERE_QUICKER,,);
#endif


RT_FUNC_S float AOCommonLaunch(uint& seed){

	/// 1. 计算射线方向；
	RandomFloat2 tmp=CF_GETSAMPLE2(seed)+make_float2(dLi);
	tmp=tmp/make_float2(dLd)*2.f-1.f;
	tmp.y*=-1.f;

	Ray ray;
	ray.ray_type=OptixRayTypes::AO;
	ray.tmin=V_C_DATA_RENDERER.ray_clamp_min;
	ray.tmax=V_C_DATA_RENDERER.ray_clamp_max;

	PRD prd;
	prd.uDepth=0;
	prd.seed=seed;
	prd.hp_g=V_C_DATA_RENDERER.camera_pos;
	prd.wi_g=tmp.x*V_C_DATA_RENDERER.camera_U+tmp.y*V_C_DATA_RENDERER.camera_V+V_C_DATA_RENDERER.camera_W;

#if 0
	if(uint(dLd.x*.5f)==dLi.x && uint(dLd.y*.5f)==dLi.y){
		rtPrintf("directions[%f,%f,%f]\n",direction.x,direction.y,direction.z);
	}
#endif

	int n=4;

	do{
		prd.AO_nextIteration=false;
		ray.origin=prd.hp_g;
		ray.direction=prd.wi_g;
		rtTrace(V_C_TOP_OBJECT,ray,prd);
		--n;
	} while(prd.AO_nextIteration && n>0);

	float sum=1.f;
	if(prd.uDepth>0&&prd.AO_intensity>M_EPSILON){
		const float firstGeometryAOIntensity=prd.AO_intensity;
		sum=0.f;
		ray.origin=prd.hp_g;
		ray.tmin=V_C_DATA_RENDERER.ray_clamp_min_AO;
		ray.tmax=V_C_DATA_RENDERER.ray_clamp_max_AO;
		Onb onb(prd.n_g);
#define MAX 4
		int i=0;
		while(i++<MAX){
			prd.AO_distancePercent=1.f;
			prd.AO_intensity=1.f;
			const RandomFloat2 samples=CF_GETSAMPLE2(prd.seed);
			Nor wi_g=CF_COSINESAMPLEHEMISPHERE_QUICKER(samples,float(i),4.f);
			onb.inverse_transform(wi_g);
			ray.direction=normalize(wi_g);
			rtTrace(V_C_TOP_OBJECT,ray,prd);

			sum+=clamp(prd.AO_distancePercent/(prd.AO_intensity*firstGeometryAOIntensity),0.f,1.f);
		}
		sum/=MAX;
	}

	return sum;

}
