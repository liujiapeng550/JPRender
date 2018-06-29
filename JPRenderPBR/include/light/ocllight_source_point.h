#pragma once  

#include "ocllight_source.h"

OCL_NAMESPACE_START

class oclLightSourcePoint:public oclLightSource{
public:
  oclLightSourcePoint(oclInt id);

  ~oclLightSourcePoint();

  oclBool Init()override;

  void SetPropertyToData_(const oclChar* propName,VARIANT* v)override;

  oclBool SetCenter(oclFloat* center)override;

  oclBool UpdateWorldMatrix(oclFloat* matirx)override;

	void SHProject(Spectrum *coeffs,const oclFloat4* imgData) const override {}

	oclBool NeedUpload()const override;
};

OCL_NAMESPACE_END