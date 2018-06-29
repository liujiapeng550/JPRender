#pragma once

#include "oclrenderer.h"

OCL_NAMESPACE_START

class oclBuffer;

class oclRendererAO:public oclRenderer{
public:
  oclRendererAO& operator=(oclRendererAO& o)=delete;
  oclRendererAO(oclRendererAO& o)=delete;
  oclRendererAO(RTcontext);
  ~oclRendererAO();

public:
  oclBool IsContinueToRender()const override;

  void PreRender()override;

  void Render()override;

  void PostRender()override;

  oclBool Init()override;

  void SetRendererSize(oclUint,oclUint)override;

  void* MapOutputBuffer()override;

  void UnmapOutputBuffer()override;

private:
  oclBuffer* m_bufferLaunchOutput;

};

OCL_NAMESPACE_END

