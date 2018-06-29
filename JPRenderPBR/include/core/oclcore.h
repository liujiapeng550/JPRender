#pragma once

#include <string>
#include <optixu\optixu_math_namespace.h>
#include <optixu\optixu_vector_types.h>
#include "../../host_device/CommonMacro.h"

#define MAJOR 0
#define MINOR 8
#define DEPRECATED

#ifdef _DEBUG_
#define __OCL_INTERNAL_DEBUG__ 1
#else
#define __OCL_INTERNAL_DEBUG__ 0
#endif

#if __OCL_INTERNAL_DEBUG__
#include <assert.h>
#include <iostream>
#define F_DBG_INFO(str) std::cout<<"[INFO] "<<str<<std::endl;
#define F_DBG_WARNING(str) std::cout<<"[WARNING] "<<(str)<<std::endl;
#define F_DBG_ERROR(str) std::cout<<"[ERROR] "<<(str)<<std::endl;
#define F_DBG_ASSERT(exp) assert(exp);
#define F_DBG_SASSERT(exp,str) static_assert(exp,str);

#else
#define F_DBG_INFO(str)
#define F_DBG_WARNING(str)
#define F_DBG_ERROR(str)
#define F_DBG_ASSERT(exp)
#define F_DBG_SASSERT(exp,str)
#endif


#if __OCL_INTERNAL_DEBUG__
void SetContext(RTcontext ctx);
void reportErrorMessage(const char* message);
void handleError( RTresult code, const char* file, int line);

struct APIError {
  APIError(RTresult c, const std::string& f, int l)
    : code(c), file(f), line(l) {}
  RTresult     code;
  std::string  file;
  int          line;
};


#define OCL_TRY try{
#define OCL_CATCH }catch( APIError& e ) {           \
    handleError( e.code, e.file.c_str(), e.line );     \
  }                                                                \
  catch( std::exception& e ) {                                     \
    reportErrorMessage( e.what() );                         \
    exit(1);                                                       \
  }




#define OCL_CHECK_ERROR( func )                                     \
  do {                                                             \
    RTresult code = func;                                          \
    if( code != RT_SUCCESS )                                       \
      throw APIError( code, __FILE__, __LINE__ );           \
  } while(0)


#else
#define OCL_TRY
#define OCL_CATCH
#define OCL_CHECK_ERROR( func ) func
#endif

// 这些宏定义与cu项目中的是同名的，用于共享头文件（C端与G端共享）中的预处理；
#define Dir optix::float3
#define Nor optix::float3
#define Pos optix::float3
#define Radiance optix::float3
#define Radiance4 optix::float4
#define Irradiance optix::float3
#define Intensity optix::float3
#define Spectrum optix::float3
#define Color optix::float3
#define Color4 optix::float4
#define Color4_Byte optix::uchar4
// The following 4 marcos are in the range of [0.0f,1.0f]
#define RandomFloat1 float
#define RandomFloat2 optix::float2
#define RandomVariable1 float
#define RandomVariable2 optix::float2
#define RandomVariable optix::float3
// UV坐标并不一定限定在0，1之间。
#define UV2 optix::float2

#define OCL_NAMESPACE_START namespace ocl{
#define OCL_NAMESPACE_END }

OCL_NAMESPACE_START

#define NORMAL_SUFFIX "_normal"
#define MATERIAL_DEFAULT "M_Default"
#define MATERIAL_MATTE "M_Matte"
#define MATERIAL_SHADOW "M_Shadow"
#define MATERIAL_METAL "M_Metal"
#define MATERIAL_GLASS "M_Glass"
#define MATERIAL_RUBBER "M_Rubber"
#define MATERIAL_METAL_BRUSHED "M_Metal_Brushed"
#define MATERIAL_CARPAINT_METALLIC "M_CarPaint_Metallic"
#define MATERIAL_CARPAINT_UNIQUE_CLEAR "M_CarPaint_Unicolor_Clear"
#define MATERIAL_CARPAINT_UNIQUE "M_CarPaint_Unicolor"
#define MATERIAL_WOOD "M_Wood"
#define MATERIAL_SHADOW "M_Shadow"
#define MATERIAL_LEATHER "M_Leather"
#define MATERIAL_EMISSIVE "M_Emissive"
#define MATERIAL_UV        "M_UV"
#define M_LIB_INTERNAL_ID -1
#define M_LIB_SCENE_ID 0
#define M_MATERIAL_GROUND_ID -1
#define M_GEOMETRY_PLANE_ID -1
#define M_GEOMETRY_SPHERE_ID -2
#define M_MATERIAL_UV -10
#define F_STR_EXPAND(s) #s
#define F_STR(s) F_STR_EXPAND(s)
#define F_SAFE_DESTROY(p) if(p){p->destroy(); p=nullptr;}
#define F_SAFE_DELETE(p) if(p){delete p; p=nullptr;}

//==========================================================================================
// typedef
typedef bool oclBool;
typedef char oclChar;
typedef unsigned char oclUchar;
typedef std::string oclStr;
#if defined(_WIN64)
typedef unsigned __int32 oclUint;							//32bits
typedef __int32 oclInt;												//32bits
typedef __int32 oclID;
typedef float oclFloat;												//32bits
typedef unsigned long oclUlong;								//64bits
typedef unsigned __int64 oclSize;							//64bits
#elif defined(_WIN32)
typedef unsigned int oclUint;
typedef int oclInt;
typedef int oclID;
typedef float oclFloat;
typedef unsigned long long oclUlong;					//64bits
typedef unsigned int oclSize;
#endif

typedef optix::uchar4 oclUchar4;
typedef optix::float4 oclFloat4;
typedef optix::float3 oclFloat3;
typedef optix::float2 oclFloat2;
typedef optix::uint3 oclUint3;
typedef optix::uint2 oclUint2;
typedef optix::int3 oclInt3;

enum class EGeometryTypes{
	GEOMETRY_TYPE_MATH_PLANE=0,
	GEOMETRY_TYPE_MATH_SPHERE,
	GEOMETRY_TYPE_MESH,
};

struct SGeometryProxy{
	oclStr name;
	EGeometryTypes type;
	oclID ID;
	
	int vertexBufferID;//float3
	int norBufferID;//float3
	int tanBufferID;//float3，可能会没有值；
	int biTanBufferID;//float3，可能会没有值；
	int uvBufferID;//float2
	int indexBufferID;//mesh三角面的索引缓冲区id，元素是个int3，这个buffer存几个元素，就意味着有几个三角面；

	int tanBufferSize;
	int biTanBufferSize;
	int primitiveCount;
};

struct SMaterialProxy{
	oclID ID;
	int bufferID;
	int bufferOffset;
};

struct SInstanceProxy{
	int bufferID_vertex;//float3
	int bufferID_normal;//float3
	int bufferID_tangent;//float3，可能会没有值；
	int bufferID_bitanget;//float3，可能会没有值；
	int bufferID_uv;//float2
	int bufferID_index;//mesh三角面的索引缓冲区id，元素是个int3，这个buffer存几个元素，就意味着有几个三角面；
	int bufferID_materials;
	int bufferOffset_materials;
	int bufferID_worldMatrix;

	oclID materialID;
	oclInt nodeIndex;
};

void ConvertFloat4ToUchar4(const oclFloat4* source,void** target,oclInt w,oclInt h);

OCL_NAMESPACE_END

