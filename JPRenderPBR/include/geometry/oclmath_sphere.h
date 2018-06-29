#pragma once

#include "geometry\oclgeometry_base.h"

OCL_NAMESPACE_START

class oclMathSphere :public oclGeometryBase{
public:
	void SetRadius(oclFloat r){
		m_fRadius=r;
	}

	void SetCenter(oclFloat x,oclFloat y,oclFloat z){
		m_center.x=x;
		m_center.y=y;
		m_center.z=z;
	}

	void FetchProxyInfo(SGeometryProxy* proxy)const override;

private:
	friend oclGeometryManager;

	oclMathSphere& operator=(const oclMathSphere&)=delete;
	oclMathSphere(const oclMathSphere&)=delete;
	oclMathSphere()=delete;
	oclMathSphere(oclID ID);
	~oclMathSphere();

private:
	Pos m_center;
	oclFloat m_fRadius;
};

OCL_NAMESPACE_END
