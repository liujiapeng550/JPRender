#pragma once


#ifndef __ONLY_FUNC__
#include <optix_device.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu\optixu_matrix_namespace.h>
#include "cu_common.h"
#include "../../host_device/data_renderer.h"

using namespace optix;

rtDeclareVariable(uint2,dLd,rtLaunchDim,);
rtDeclareVariable(uint2,dLi,rtLaunchIndex,);
rtDeclareVariable(SData_Renderer,V_C_DATA_RENDERER,,);
rtDeclareVariable(rtCallableProgramId<uint(uint,uint)>,CF_GETRANDOMSEED,,);
rtDeclareVariable(rtCallableProgramId<RandomFloat2(uint&)>,CF_GETSAMPLE2,,);
#endif


RT_FUNC_F void PrepareLaunch(Dir& dir,uint& seed){

	const int index=dLi.y*dLd.x+dLi.x;
	seed=CF_GETRANDOMSEED(index,V_C_DATA_RENDERER.frame_current);
	RandomFloat2 tmp=CF_GETSAMPLE2(seed)+make_float2(dLi);
	tmp=tmp/make_float2(dLd)*2.f-1.f;
	//tmp.y*=-1.f;
	dir=tmp.x*V_C_DATA_RENDERER.camera_U+tmp.y*V_C_DATA_RENDERER.camera_V+V_C_DATA_RENDERER.camera_W;
}
