#pragma once  

#include "ocllight_source.h"

OCL_NAMESPACE_START

class oclLightSourceDistant:public oclLightSource{
public:
  oclLightSourceDistant(oclInt id);
  ~oclLightSourceDistant();

  oclBool Init()override;

  void SetPropertyToData_(const oclChar* propName,VARIANT* v)override;

  oclBool SetCenter(oclFloat* center)override;

  oclBool UpdateWorldMatrix(oclFloat* mat)override;

	void SHProject(Spectrum *coeffs,const oclFloat4* imgData) const {};

	oclBool NeedUpload()const override;
};

OCL_NAMESPACE_END