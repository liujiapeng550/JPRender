#pragma once

#include "geometry\oclgeometry_base.h"

OCL_NAMESPACE_START

class oclMathPlane :public oclGeometryBase{
public:
// 可扩展诸如位置，法向、边长等成员函数；
//目前使用的地方是地面，位置在原点，法线y+，无穷大（这些也是默认参数);
	void FetchProxyInfo(SGeometryProxy* proxy)const override;

private:
	friend oclGeometryManager;

	oclMathPlane& operator=(const oclMathPlane&)=delete;
	oclMathPlane(const oclMathPlane&)=delete;
	oclMathPlane()=delete;
	oclMathPlane(oclInt index);
	~oclMathPlane();

private:
	Pos m_pos;//平面的中心点
	Nor m_normal;
	oclFloat m_width,m_height;
	
};

OCL_NAMESPACE_END
