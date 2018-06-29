#pragma once

#include "oclrenderer.h"

OCL_NAMESPACE_START

class oclBuffer;

class oclRendererNormal:public oclRenderer{
public:
  oclRendererNormal& operator=(oclRendererNormal& o)=delete;
  oclRendererNormal(oclRendererNormal& o)=delete;
  oclRendererNormal(RTcontext);
  ~oclRendererNormal();

public:
  oclBool IsContinueToRender()const override;

  void PreRender()override;

  void Render()override;

  void PostRender()override;

  oclBool Init()override;

  void SetRendererSize(oclUint,oclUint)override;

  void* MapOutputBuffer()override;

  void UnmapOutputBuffer()override;

	void SetShowCameraSpaceNormal(oclBool value);

private:
  oclBuffer* m_bufferLaunchOutput;

};

OCL_NAMESPACE_END