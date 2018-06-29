#include <optix_device.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu\optixu_matrix_namespace.h>
#include "cu_common.h"
#include "intersection.h"
#include "../../host_device/data_context.h"
#include "../../host_device/data_renderer.h"

using namespace optix;

rtDeclareVariable(Intersection,aInset,attribute aInset,);
rtDeclareVariable(rtObject,V_C_TOP_OBJECT,,);
rtDeclareVariable(SData_Context,V_C_DATA_CONTEXT,,);
rtDeclareVariable(SData_Renderer,V_C_DATA_RENDERER,,);
rtDeclareVariable(int,prdP,rtPayload,);

////////////////////////////////////////////////////////////////////////////////////////////////////
// launching program
////////////////////////////////////////////////////////////////////////////////////////////////////

RT_PROGRAM void PgmLaunch_pick(){
  /// 1. 获取C端提交到缓冲区中的数据；
  float4 data=GET_F4_1D(V_C_DATA_CONTEXT.picker_bufID,0);
  const Nor rayDir=make_float3(data.x,data.y,data.z);

  /// 2. 生成射线并发射；
  Ray ray=make_Ray(V_C_DATA_RENDERER.camera_pos,rayDir,OptixRayTypes::PICKUP,V_C_DATA_RENDERER.ray_clamp_min,V_C_DATA_RENDERER.ray_clamp_max);
  int w=-1;
  rtTrace(V_C_TOP_OBJECT,ray,w);

  /// 3. 保存获取到的meshID到缓冲区，供C端使用；
	data.w=(float)w;
  SET_F4_1D(V_C_DATA_CONTEXT.picker_bufID,0,data);
}

RT_PROGRAM void PgmCH_pick(){
  prdP=aInset.nodeIndex;
}
