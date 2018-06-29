#pragma once

#include <vector>
#include <optixu\optixu_math_namespace.h>
#include <optixu\optixu_matrix_namespace.h>
#include "core\oclcore.h"
#include "../../host_device/data_renderer.h"

OCL_NAMESPACE_START

class oclCameraBase;
class oclTexture;

class oclRenderer{
public:
  oclRenderer& operator=(const oclRenderer&)=delete;
  oclRenderer(const oclRenderer&)=delete;
  oclRenderer(RTcontext ctx);
  virtual ~oclRenderer();

public:
	virtual oclBool Init()=0;

	virtual void SetRendererSize(oclUint,oclUint);

  virtual void* MapOutputBuffer()=0;

  virtual void UnmapOutputBuffer()=0;

  virtual oclBool IsContinueToRender()const=0;

  virtual void PreRender(){};

  virtual void Render()=0;

  virtual void PostRender(){};

  inline void ResetFrameCount(){
		m_data.frame_current=1u;
	}

	void Upload();

	inline oclUint GetRendererElementSize()const{
		return sizeof(oclFloat4);
	}

  oclUint GetRendererWidth()const{ return m_uWidth; }

  oclUint GetRendererHeight()const{ return m_uHeight; }

  oclUint GetRendererSize()const{ return m_uWidth*m_uHeight; }

	oclCameraBase* GetCamera(){ return m_ref_pCamera; }

  const oclCameraBase* GetCamera()const{ return m_ref_pCamera; }

	virtual void SetCamera(oclCameraBase* camera){ m_ref_pCamera=camera; };

	void FetchTextures(std::vector<const oclTexture*>*)const;

  void SetSkyboxTextureURL(oclStr fileURL);

  void SetRayClampMin(oclFloat value){ m_data.ray_clamp_min=value; }

  void SetRayClampMax(oclFloat value){ m_data.ray_clamp_max=value; }

  void SetRayClampMinAO(oclFloat value){ m_data.ray_clamp_min_AO=value; }

  void SetRayClampMaxAO(oclFloat value){ m_data.ray_clamp_max_AO=value; }

  void SetIteratorDepth(oclUint value){ m_data.ray_iterateDepth=value; }

  void SetImportanceThreshold(oclFloat value){ m_data.radiance_importanceThreshold=value; }

  void SetEnableGI(oclBool value){ m_data.enableGI=value; }

  void SetEnableAO(oclBool value){ m_data.enableAO=value; }

  void SetSamplesPerPixel(oclUint value);

  void SetBadColor(oclFloat r,oclFloat g,oclFloat b);

  void SetEnableSkybox_show(oclBool value);

	void SetTopGroup(RTgroup g);

	void SetBufferID_lights(oclInt ID);

protected:
  oclUint m_uWidth=100u;
  oclUint m_uHeight=100u;
  oclFloat m_postBlend;
  oclFloat m_postGamma;
  oclFloat m_postExposure;
  SData_Renderer m_data;

	oclCameraBase* m_ref_pCamera=nullptr;
  RTcontext m_ctx;
  RTvariable m_varRenderer=0;
	RTvariable m_varTopGroup;
	RTgroup m_topGroup;
  oclTexture* m_pSkyboxTexture=nullptr;

};

OCL_NAMESPACE_END
