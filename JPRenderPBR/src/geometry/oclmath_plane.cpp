#include "geometry\oclmath_plane.h"

OCL_NAMESPACE_START

oclMathPlane::oclMathPlane(oclInt ID)
	:oclGeometryBase(EGeometryTypes::GEOMETRY_TYPE_MATH_PLANE,ID){}

void oclMathPlane::FetchProxyInfo(SGeometryProxy * proxy) const{
	proxy->primitiveCount=1;
	proxy->ID=GetID();
	proxy->name=GetName();
}

oclMathPlane::~oclMathPlane(){
}

OCL_NAMESPACE_END
