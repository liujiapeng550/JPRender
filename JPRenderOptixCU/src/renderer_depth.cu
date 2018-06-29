#include <optix_device.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu\optixu_matrix_namespace.h>
#include "cu_common.h"
#include "../../host_device/data_context.h"
#include "../../host_device/data_renderer.h"

using namespace optix;

struct SData_prd_depthRendering{
  float depth;
};

rtDeclareVariable(uint2,dLd,rtLaunchDim,);
rtDeclareVariable(uint2,dLi,rtLaunchIndex,);
rtDeclareVariable(float,dT,rtIntersectionDistance,);
rtDeclareVariable(rtObject,V_C_TOP_OBJECT,,);
rtDeclareVariable(SData_Renderer,V_C_DATA_RENDERER,,);
rtDeclareVariable(SData_prd_depthRendering,prd_depthRendering,rtPayload,);

//rtDeclareVariable(rtCallableProgramId<RandomFloat1(uint&)>,CF_GETSAMPLE1,,);
rtDeclareVariable(rtCallableProgramId<uint(uint,uint)>,CF_GETRANDOMSEED,,);
rtDeclareVariable(rtCallableProgramId<RandomFloat2(uint&)>,CF_GETSAMPLE2,,);

#define __ONLY_FUNC__
#include "prepare_launch.h"
#undef __ONLY_FUNC__

RT_PROGRAM void PgmLaunch_depth(){
	Nor direction; uint seed;
	PrepareLaunch(direction,seed);

#if 0
		if(uint(dLd.x*.5f)==dLi.x && uint(dLd.y*.5f)==dLi.y){
			rtPrintf("directions[%f,%f,%f]\n",direction.x,direction.y,direction.z);
		}
#endif

  SData_prd_depthRendering prd;
  prd.depth=M_MAX_FLOAT;
  Ray ray=make_Ray(V_C_DATA_RENDERER.camera_pos,direction,OptixRayTypes::DEPTH,V_C_DATA_RENDERER.ray_clamp_min,V_C_DATA_RENDERER.ray_clamp_max);
  rtTrace(V_C_TOP_OBJECT,ray,prd);


  Radiance4 L=make_float4(0.f);
  const float delta=V_C_DATA_RENDERER.DepthRenderer_depth_max-prd.depth;
  if(delta>0.f){
    float pct=delta/(V_C_DATA_RENDERER.DepthRenderer_depth_max-V_C_DATA_RENDERER.DepthRenderer_depth_min);
		L=make_float4(pct);

		if(V_C_DATA_RENDERER.frame_current>1u){
			const float weight=1.f/(float)V_C_DATA_RENDERER.frame_current;
			Radiance4 tmp=GET_F4_2D(V_C_DATA_RENDERER.output_launchBufID,dLi);
			L=lerp(tmp,L,weight);
		}
  }
	L.w=1.f;

	SET_F4_2D(V_C_DATA_RENDERER.output_launchBufID,dLi,L);
}

RT_PROGRAM void PgmCH_depth(){

  prd_depthRendering.depth=dT;

#if 0
  if(uint(dLd.x*.5f)==dLi.x && uint(dLd.y*.5f)==dLi.y){
    rtPrintf("inv_depth:%f\n",prd_depthRendering.depth);
  }
  return;
#endif
}