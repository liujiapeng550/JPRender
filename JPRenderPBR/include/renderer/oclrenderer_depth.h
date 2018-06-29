#pragma once

#include "oclrenderer.h"

OCL_NAMESPACE_START

class oclBuffer;

class oclRendererDepth:public oclRenderer{
public:
  oclRendererDepth& operator=(oclRendererDepth& o)=delete;
  oclRendererDepth(oclRendererDepth& o)=delete;
  oclRendererDepth(RTcontext);
  ~oclRendererDepth();

public:
  oclBool IsContinueToRender()const override;

  void PreRender()override;

  void Render()override;

  void PostRender()override;

  oclBool Init()override;

  void SetRendererSize(oclUint,oclUint)override;

  void* MapOutputBuffer()override;

  void UnmapOutputBuffer()override;

  void SetDepthMax(oclFloat value);

  void SetDepthMin(oclFloat value);

private:
  oclBuffer* m_bufferLaunchOutput;

};

OCL_NAMESPACE_END

