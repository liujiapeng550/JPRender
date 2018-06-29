#pragma once

#include "../../host_device/CommonMacro.h"

//==========================================================================================
#define RT_FUNC_F __forceinline__ __device__ 
#define RT_FUNC __device__ 
#define RT_FUNC_S __device__ static


//==========================================================================================
//#define __optix_debug__
#define _out_
#define _in_
#define _opt_
//==========================================================================================
typedef optix::float3 Nor;
typedef optix::float3 Dir;
typedef optix::float3 Pos;
typedef optix::float3 Albedo3;
typedef optix::float4 Albedo4;
typedef optix::float3 Radiance3;
typedef optix::float4 Radiance4;
typedef optix::float3 Irradiance3;
typedef optix::float4 Irradiance4;
typedef optix::float3 Intensity3;
typedef optix::float4 Intensity4;
typedef optix::float3 Spectrum3;
// This macro defines that the variable is in the range of [0.0f,1.0f];
typedef optix::float3 Color3;
typedef optix::float4 Color4;

// The following 4 marcos are in the range of [0.0f,1.0f]
typedef float RandomFloat1;
typedef optix::float2 RandomFloat2;
typedef float RandomVariable1;
typedef optix::float2 RandomVariable2;
typedef optix::float3 RandomVariable3;

// UV坐标并不一定限定在0，1之间。
typedef optix::float2 UV2;


//==========================================================================================
#define F_COLOR_TO_NORMAL(c) ((c)*2.0f-1.0f)
#define F_NORMAL_TO_COLOR(n) (((n)+1.0f)*.5f)
#define F_SET_FLOAT4_TO_BUFFER(bufferID,index,value) (rtBufferId<float4,1>(bufferID)[index]=value)
#define SET_F4_2D(bufferID,index,value) (rtBufferId<float4,2>(bufferID)[index]=value)
#define SET_I_2D(bufferID,index,value) (rtBufferId<int,2>(bufferID)[index]=value)
#define GET_F_1D(bufferID,index) (rtBufferId<float,1>(bufferID)[index])
#define GET_F2_1D(bufferID,index) (rtBufferId<float2,1>(bufferID)[index])
#define GET_F3_1D(bufferID,index) (rtBufferId<float3,1>(bufferID)[index])
#define F_GET_FLOAT4_FROM_BUFFER(bufferID,index) (rtBufferId<float4,1>(bufferID)[index])
#define GET_F4_2D(bufferID,index) (rtBufferId<float4,2>(bufferID)[index])
#define GET_I_2D(bufferID,index) (rtBufferId<int,2>(bufferID)[index])
#define F_GET_INT_FROM_BUFFER(bufferID,index) (rtBufferId<int,1>(bufferID)[index])
#define GET_I3_1D(bufferID,index) (rtBufferId<int3,1>(bufferID)[index])
//==========================================================================================

//impossible valur range
#define MY_EXCEPTION_0 RT_EXCEPTION_USER + 0
//assertion failure.
#define MY_EXCEPTION_1 RT_EXCEPTION_USER + 1
// imposbile value
#define MY_EXCEPTION_2 RT_EXCEPTION_USER + 2
#define Assert(exp) if(!(exp))optix::rt_throw(MY_EXCEPTION_1);

//==========================================================================================
#define V_D_PRD_RADIANCE dPrdRadiance
#define V_D_PRD_AO dPrdAO
#define V_D_PRD_SHADOW dPrdShadow
#define V_D_CURRENT_RAY dRay
#define V_D_HIT_T_FLOAT dHitT
#define V_D_LAUNCH_DIM_UINT2 dLaunchDim
#define V_D_LAUNCH_INDEX_UINT2 dLaunchIndex
#define V_A_TBN aTBN
#define V_A_INTERSECTION aInset
#define V_A_PICKED_GEO_ID_INT pickedGeoID

//==========================================================================================
// integrator types
/*
#define M_INTGRATORTYPE_NONE 0u
#define M_INTGRATORTYPE_MATTE 1u
#define M_INTGRATORTYPE_GLOSSY 2u
#define M_INTGRATORTYPE_SPECULAR 3u
*/
