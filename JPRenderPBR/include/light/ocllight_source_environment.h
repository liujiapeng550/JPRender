#pragma once

#include "core\ocltexture.h"
#include "ocllight_source.h"

OCL_NAMESPACE_START

class oclBuffer;
class oclTexture;

class oclLightSourceEnvironment :public oclLightSource {
public:
  oclLightSourceEnvironment(oclInt id,RTcontext ctx);
	~oclLightSourceEnvironment();

	oclBool NeedUpload()const override;

	oclBool UpdateWorldMatrix(oclFloat* matrix)override { return true; };

	oclBool SetCenter(oclFloat* center)override { return true; }

	oclBool Init()override;

	void SHProject(Spectrum *coeffs,const oclFloat4* imgData) const override;

  void FetchTextures(std::vector<const oclTexture*>*)const;

	void SetProperty(const oclChar* propName,VARIANT* v)override;

private:
	oclLightSourceEnvironment() = delete;

	void SetPropertyToData_(const oclChar* propName, VARIANT* v)override;

	void SHReduceRinging_(oclFloat3 *c, oclFloat lambda = 0.005f);

	oclBool HandleDistribution_(oclInt w, oclInt h,const oclFloat4* buf);

	/**
	 * 如果oclBuffer在管理器中存在，返回true;
	 * 不然返回false
	 */
	oclBool AssembleoclBuffer_(oclStr hdrFileURL);

private:
  oclBuffer* m_CDFOB=nullptr;
  oclBuffer* m_LumiOB=nullptr;
  oclBuffer* m_MarginalCDFOB=nullptr;
  oclBuffer* m_IntegrationOB=nullptr;
	oclBuffer* m_buffer_worldMatrix;
  oclBuffer* m_SHco=nullptr;         //SH系数
  oclTexture m_texture;
	optix::float3 m_axis;
};

OCL_NAMESPACE_END
