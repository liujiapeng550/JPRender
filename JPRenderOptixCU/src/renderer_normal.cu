#include <optix_device.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu\optixu_matrix_namespace.h>
#include "cu_common.h"
#include "../../host_device/data_context.h"
#include "../../host_device/data_renderer.h"
#include "../../host_device/data_instance.h"
#include "intersection.h"

using namespace optix;

struct SData_prd_normalRendering{
  Nor sn_g;
  bool hitted;
};

rtDeclareVariable(Ray,dRay,rtCurrentRay,);
rtDeclareVariable(float,dT,rtIntersectionDistance,);
rtDeclareVariable(uint2,dLd,rtLaunchDim,);
rtDeclareVariable(uint2,dLi,rtLaunchIndex,);
rtDeclareVariable(SData_Context,V_C_DATA_CONTEXT,,);
rtDeclareVariable(SData_Renderer,V_C_DATA_RENDERER,,);
rtDeclareVariable(SData_Instance,V_C_DATA_INSTANCE,,);
rtDeclareVariable(rtObject,V_C_TOP_OBJECT,,);
rtDeclareVariable(SData_prd_normalRendering,prd_normalRendering,rtPayload,);
rtDeclareVariable(Intersection,aInset,attribute aInset,);

rtDeclareVariable(rtCallableProgramId<uint(uint,uint)>,CF_GETRANDOMSEED,,);
rtDeclareVariable(rtCallableProgramId<RandomFloat1(uint&)>,CF_GETSAMPLE1,,);
rtDeclareVariable(rtCallableProgramId<RandomFloat2(uint&)>,CF_GETSAMPLE2,,);
rtDeclareVariable(rtCallableProgramId<Nor(int,const Pos&,const Nor&)>,CF_GETTRIPLANENORMAL,,);
rtDeclareVariable(rtCallableProgramId<Color4(int,int,UV2)>,CF_GETTEXTURE_DIFFUSE,,);
rtDeclareVariable(rtCallableProgramId<Nor(const SData_Texture&,Intersection)>,CF_GETTEXTURE_NORMAL,,);

#define __ONLY_FUNC__
#include "common_texture_operation.h"
#include "prepare_launch.h"
#undef __ONLY_FUNC__


RT_PROGRAM void PgmLaunch_normal(){
	Nor direction; uint seed;
	PrepareLaunch(direction,seed);

  SData_prd_normalRendering prd;
  prd.hitted=false;
	prd.sn_g=make_float3(0.f);
  Ray ray=make_Ray(V_C_DATA_RENDERER.camera_pos,direction,OptixRayTypes::NORMAL,V_C_DATA_RENDERER.ray_clamp_min,V_C_DATA_RENDERER.ray_clamp_max);
  rtTrace(V_C_TOP_OBJECT,ray,prd);
  Radiance4 L;
  if(prd.hitted){

		if(V_C_DATA_RENDERER.enableShowCameraSpaceNormal){
			Nor normal_eyespace=(length(prd.sn_g)>0.f)?normalize(V_C_DATA_RENDERER.matrix_WorldtoCamera * prd.sn_g):make_float3(0.,0.,1.);
			L=make_float4(F_NORMAL_TO_COLOR(normal_eyespace),1.f);
		} else{
			L=make_float4(F_NORMAL_TO_COLOR(prd.sn_g),1.f);
		}
		if(V_C_DATA_RENDERER.frame_current>1u){
			const float weight=1.f/(float)V_C_DATA_RENDERER.frame_current;
			Radiance4 tmp=GET_F4_2D(V_C_DATA_RENDERER.output_launchBufID,dLi);
			L=lerp(tmp,L,weight);
		} 
	} else{

		if(V_C_DATA_RENDERER.enableShowCameraSpaceNormal){
			L=make_float4(0.f,0.f,1.f,1.f);
		} else{
			L=make_float4(1.f);
		}
  }
	SET_F4_2D(V_C_DATA_RENDERER.output_launchBufID,dLi,L);

}


RT_PROGRAM void PgmCH_normal(){
	Nor geoNor_g;
	const SData_Material& md=
		rtBufferId<SData_Material,1>(V_C_DATA_INSTANCE.materialBufferID)[V_C_DATA_INSTANCE.materialBufferOffset];
	switch(md.materialIndex){
	case MATERIAL_LEATHER_INDEX:
	{
		Pos hitPos_g=dRay.origin+dRay.direction*dT;
		GetNormal_triplanar(md.GE_texture,hitPos_g,geoNor_g);
	}
	break;
	default:
	{
		const SData_Texture& data=md.GE_texture;
		Nor nor=CF_GETTEXTURE_NORMAL(data,aInset);
		geoNor_g=normalize(rtTransformNormal(RT_OBJECT_TO_WORLD,nor));
	}
	break;
	}
	prd_normalRendering.sn_g=geoNor_g;
  prd_normalRendering.hitted=true;
}

