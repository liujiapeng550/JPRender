#include "geometry\oclmath_sphere.h"
#include "core/oclbuffer_manager.h"

OCL_NAMESPACE_START

oclMathSphere::oclMathSphere(oclInt ID)
	:oclGeometryBase(EGeometryTypes::GEOMETRY_TYPE_MATH_SPHERE,ID){
	m_center=optix::make_float3(0.f);
	m_fRadius=1.f;
}

void oclMathSphere::FetchProxyInfo(SGeometryProxy * proxy) const{
	proxy->primitiveCount=1;
	proxy->ID=GetID();
	proxy->name=GetName();
}

oclMathSphere::~oclMathSphere(){
}

OCL_NAMESPACE_END
