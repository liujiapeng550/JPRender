#include <optix_device.h>
#include <optixu\optixu_math_namespace.h>
#include <optixu/optixu_aabb_namespace.h>
#include "cu_common.h"
#include "intersection.h"

using namespace optix;

/*
rtDeclareVariable(float3,plane,,);
rtDeclareVariable(float3,v1,,);
rtDeclareVariable(float3,v2,,);
rtDeclareVariable(float3,anchor,,);
*/

rtDeclareVariable(Ray,dRay,rtCurrentRay,);
rtDeclareVariable(Intersection,aInset,attribute aInset,);

RT_PROGRAM void PgmIntersect_plane(int primIdx){
	float t=-dRay.origin.y/dRay.direction.y;
	if(t>dRay.tmin && t<dRay.tmax){
		if(rtPotentialIntersection(t)){
			aInset.geoNor=aInset.meshNor=make_float3(0.f,1.f,0.f);
			aInset.nodeIndex=-1;
			rtReportIntersection(0);
		}
	}
}

RT_PROGRAM void PgmBounds_plane(int,float result[6]){
	optix::Aabb* aabb=(optix::Aabb*)result;
	aabb->m_min.x=-M_MAX_FLOAT;
	aabb->m_min.y=-0.01f;
	aabb->m_min.z=-M_MAX_FLOAT;
	aabb->m_max.x=M_MAX_FLOAT;
	aabb->m_max.y=0.0f;
	aabb->m_max.z=M_MAX_FLOAT;
}