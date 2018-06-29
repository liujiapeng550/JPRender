#pragma once

#include "oclrenderer.h"

OCL_NAMESPACE_START

class oclBuffer;

class oclRendererRadiance:public oclRenderer{
public:
  oclRendererRadiance& operator=(oclRendererRadiance& o)=delete;
  oclRendererRadiance(oclRendererRadiance& o)=delete;
  oclRendererRadiance(RTcontext);
  ~oclRendererRadiance();

  void SetGamma(oclFloat value);

  void SetBlend(oclFloat value);

  void SetExposure(oclFloat value);

  void EnableDenoise(oclBool value);

  virtual oclBool IsContinueToRender()const override;

  virtual void PreRender()override;

  void Render()override;

  virtual void PostRender()override;

  virtual oclBool Init()override;

  void SetRendererSize(oclUint,oclUint)override;

  void* MapOutputBuffer()override;

  void UnmapOutputBuffer()override;

private:
  RTcommandlist m_clDenoiser=0;
  RTcommandlist m_clDenoiser_off=0;
  RTpostprocessingstage m_ppsTonemapStage=0;
  RTpostprocessingstage m_ppsDenoiserStage=0;
  RTvariable m_varTonemapInputBuffer=0;
  RTvariable m_varTonemapOutputBuffer=0;
  RTvariable m_varTonemapStageExposure=0;
  RTvariable m_varTonemapStageGamma=0;
  RTvariable m_varDenoiserInputBuffer=0;
  RTvariable m_varDenoiserOutputBuffer=0;
  RTvariable m_varDenoiserAlbedoBuffer=0;
  RTvariable m_varDenoiserNormalBuffer=0;
  RTvariable m_varDenoiserStageBlend=0;
  oclBuffer* m_bufferAODensity=0;
  oclBuffer* m_bufferFinalRadiance=0;
  oclBuffer* m_bufferLaunchOutput=0;
  oclBuffer* m_bufferTonemapOutput=0;
  oclBuffer* m_bufferDenoiserAlbedo=0;
  oclBuffer* m_bufferDenoiserNormal=0;
  oclBuffer* m_bufferDenoiserOutput=0;

};

OCL_NAMESPACE_END

