#include <optix_device.h>
#include <optixu\optixu_math_namespace.h>
#include <optixu/optixu_aabb_namespace.h>
#include "cu_common.h"
#include "intersection.h"

using namespace optix;

/*
rtDeclareVariable(float4,sphere,,);
*/

rtDeclareVariable(Ray,dRay,rtCurrentRay,);
rtDeclareVariable(Intersection,aInset,attribute aInset,);

RT_PROGRAM void PgmIntersect_sphere(int primIdx){
	float3 center=make_float3(0.f); //float3 center=make_float3(sphere);
	float3 O=dRay.origin-center;
	float3 D=dRay.direction;
	float radius=1.f; //float radius=sphere.w;

	float b=dot(O,D);
	float c=dot(O,O)-radius*radius;
	float disc=b*b-c;
	if(disc > 0.0f){
		float sdisc=sqrtf(disc);
		float root1=(-b-sdisc);

		float root11=0.0f;
		bool check_second=true;
		if(rtPotentialIntersection(root1+root11)){
			aInset.geoNor=(O+(root1+root11)*D)/radius;
			aInset.nodeIndex=-1;
			if(rtReportIntersection(0))
				check_second=false;
		}
		if(check_second){
			float root2=-b+sdisc;
			if(rtPotentialIntersection(root2)){
				aInset.geoNor=(O+root2*D)/radius;
				aInset.nodeIndex=-1;
				rtReportIntersection(0);
			}
		}
	}
}

RT_PROGRAM void PgmBounds_sphere(int,float result[6]){
	optix::Aabb* aabb=(optix::Aabb*)result;
	aabb->m_min.x=-1.f;
	aabb->m_min.y=-1.f;
	aabb->m_min.z=-1.f;
	aabb->m_max.x=1.f;
	aabb->m_max.y=1.0f;
	aabb->m_max.z=1.f;
}