#pragma once  

#include "ocllight_source.h"

OCL_NAMESPACE_START

class oclBuffer;
class oclMeshManager;

class oclLightSourceArea:public oclLightSource{
public:
	oclLightSourceArea(oclInt id, RTcontext ctx);
  ~oclLightSourceArea();
  oclBool Init()override;
  void SetPropertyToData_(const oclChar* propName,VARIANT* v)override;
	void SetAreaLightAccessory(const std::vector<oclFloat>& area,const SInstanceProxy& proxy);
  oclBool SetCenter(oclFloat* center)override;
  oclBool UpdateWorldMatrix(oclFloat* mat)override;
	void SHProject(Spectrum *coeffs,const oclFloat4* imgData) const {};
	oclBool NeedUpload()const override;

private:
  RTcontext m_ctx;
  oclBuffer* m_CDFOB=nullptr;
  oclBuffer* m_valueOB=nullptr;
  oclInt m_oldMeshNodeIndex=-1;
};

OCL_NAMESPACE_END
