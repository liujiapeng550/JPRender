#pragma once  

#include "ocllight_source.h"

OCL_NAMESPACE_START

class oclLightSourceSpot:public oclLightSource{
public:
  oclLightSourceSpot(oclInt id);
  ~oclLightSourceSpot();

  oclBool Init()override;

  void SetPropertyToData_(const oclChar* propName,VARIANT* v)override;

  oclBool SetCenter(oclFloat* center)override;

  oclBool UpdateWorldMatrix(oclFloat* matrix)override;

	void SHProject(Spectrum *coeffs,const oclFloat4* imgData) const {};

	oclBool NeedUpload()const override;
};

OCL_NAMESPACE_END
