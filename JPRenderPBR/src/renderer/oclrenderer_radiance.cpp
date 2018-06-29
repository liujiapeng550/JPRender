#include "renderer/oclrenderer_radiance.h"
#include "core/oclbuffer_manager.h"
#include "core\oclprogram_manager.h"
#include "camera/oclcamera.h"

OCL_NAMESPACE_START

oclRendererRadiance::oclRendererRadiance(RTcontext ctx)
  :oclRenderer(ctx){}

oclRendererRadiance::~oclRendererRadiance(){
  /// 1. 销毁RT对象；
  OCL_TRY;
  OCL_CHECK_ERROR(rtCommandListDestroy(m_clDenoiser_off));
  OCL_CHECK_ERROR(rtCommandListDestroy(m_clDenoiser));
  OCL_CHECK_ERROR(rtPostProcessingStageDestroy(m_ppsTonemapStage));
  OCL_CHECK_ERROR(rtPostProcessingStageDestroy(m_ppsDenoiserStage));
  OCL_CATCH;


  /// 2. 销毁缓冲区；
  oclBufferManager* mgr=oclBufferManager::S();
  mgr->Destroy(m_bufferAODensity);
  mgr->Destroy(m_bufferFinalRadiance);
  mgr->Destroy(m_bufferLaunchOutput);
  mgr->Destroy(m_bufferTonemapOutput); 
  mgr->Destroy(m_bufferDenoiserAlbedo);
  mgr->Destroy(m_bufferDenoiserNormal);
  mgr->Destroy(m_bufferDenoiserOutput);
}

void oclRendererRadiance::SetGamma(oclFloat value){
  if(m_postGamma!=value){
    m_postGamma=value;
    OCL_TRY;
    OCL_CHECK_ERROR(rtVariableSet1f(m_varTonemapStageGamma,m_postGamma));
    OCL_CATCH;
  }
}

void oclRendererRadiance::SetBlend(oclFloat value){
  if(m_postBlend!=value){
    m_postBlend=value;
    OCL_TRY;
    OCL_CHECK_ERROR(rtVariableSet1f(m_varDenoiserStageBlend,m_postBlend));
    OCL_CATCH;
  }
}

void oclRendererRadiance::SetExposure(oclFloat value){
  if(m_postExposure!=value){
    m_postExposure=value;
    OCL_TRY;
    OCL_CHECK_ERROR(rtVariableSet1f(m_varTonemapStageExposure,m_postExposure));
    OCL_CATCH;
  }
}

void oclRendererRadiance::EnableDenoise(oclBool value){
	m_data.enableAIdenoise=value;
}

oclBool oclRendererRadiance::IsContinueToRender()const{
  /// 渲染次数大于最大渲染数量后返回false，表示不再渲染；
  return m_data.frame_current<=m_data.samples_per_pixel;
}

void oclRendererRadiance::PreRender(){
	if(m_ref_pCamera->GetNeedUpdateCamera()){
		m_ref_pCamera->FetchData(&m_data);
	}
}

void oclRendererRadiance::Render(){
	OCL_TRY;
	if(m_data.enableAIdenoise){
		OCL_CHECK_ERROR(rtCommandListExecute(m_clDenoiser));
	} else{
		OCL_CHECK_ERROR(rtCommandListExecute(m_clDenoiser_off));
		//test: OCL_CHECK_ERROR(rtContextLaunch2D(m_ctx,EntryPoint::E_ENTRYPOINT_RADIANCE,m_uWidth,m_uHeight));
	}
  OCL_CATCH;
}

void oclRendererRadiance::PostRender(){
  ++m_data.frame_current;
}

void oclRendererRadiance::SetRendererSize(oclUint width,oclUint height){
  m_uWidth=width;
  m_uHeight=height;

  m_bufferAODensity->SetSize(m_uWidth,m_uHeight);
  m_bufferFinalRadiance->SetSize(m_uWidth,m_uHeight);
  m_bufferLaunchOutput->SetSize(m_uWidth,m_uHeight);
  m_bufferTonemapOutput->SetSize(m_uWidth,m_uHeight);
  m_bufferDenoiserAlbedo->SetSize(m_uWidth,m_uHeight);
  m_bufferDenoiserNormal->SetSize(m_uWidth,m_uHeight);
  m_bufferDenoiserOutput->SetSize(m_uWidth,m_uHeight);

  OCL_TRY;
  if(m_clDenoiser_off){
    OCL_CHECK_ERROR(rtCommandListDestroy(m_clDenoiser_off));
    OCL_CHECK_ERROR(rtCommandListDestroy(m_clDenoiser));
  }
  OCL_CHECK_ERROR(rtCommandListCreate(m_ctx,&m_clDenoiser_off));
  OCL_CHECK_ERROR(rtCommandListAppendLaunch2D(m_clDenoiser_off,EntryPoint::E_ENTRYPOINT_RADIANCE,m_uWidth,m_uHeight));
  OCL_CHECK_ERROR(rtCommandListAppendPostprocessingStage(m_clDenoiser_off,m_ppsTonemapStage,m_uWidth,m_uHeight));
  OCL_CHECK_ERROR(rtCommandListFinalize(m_clDenoiser_off));

  /// denoiser
  OCL_CHECK_ERROR(rtCommandListCreate(m_ctx,&m_clDenoiser));
  OCL_CHECK_ERROR(rtCommandListAppendLaunch2D(m_clDenoiser,EntryPoint::E_ENTRYPOINT_RADIANCE,m_uWidth,m_uHeight));
  OCL_CHECK_ERROR(rtCommandListAppendPostprocessingStage(m_clDenoiser,m_ppsTonemapStage,m_uWidth,m_uHeight));
  OCL_CHECK_ERROR(rtCommandListAppendPostprocessingStage(m_clDenoiser,m_ppsDenoiserStage,m_uWidth,m_uHeight));
  OCL_CHECK_ERROR(rtCommandListFinalize(m_clDenoiser));
  OCL_CATCH;
}

oclBool oclRendererRadiance::Init(){
	/// 1. 创建必要缓冲区；
	oclBufferManager* mgr=oclBufferManager::S();
	m_bufferAODensity=mgr->Create(GetUniqueBufferKey("buffer_AOdensity"),RT_BUFFER_OUTPUT,RT_FORMAT_FLOAT);
	m_bufferFinalRadiance=mgr->Create(GetUniqueBufferKey("buffer_finalOutput"),RT_BUFFER_OUTPUT,RT_FORMAT_FLOAT4);
	m_bufferLaunchOutput=mgr->Create(GetUniqueBufferKey("buffer_launchOutput"),RT_BUFFER_OUTPUT,RT_FORMAT_FLOAT4);
	m_bufferTonemapOutput=mgr->Create(GetUniqueBufferKey("buffer_tonemapOutput"),RT_BUFFER_OUTPUT,RT_FORMAT_FLOAT4);
	m_bufferDenoiserAlbedo=mgr->Create(GetUniqueBufferKey("buffer_denoiserAlbedo"),RT_BUFFER_OUTPUT,RT_FORMAT_FLOAT4);
	m_bufferDenoiserNormal=mgr->Create(GetUniqueBufferKey("buffer_denoiserNormal"),RT_BUFFER_OUTPUT,RT_FORMAT_FLOAT4);
	m_bufferDenoiserOutput=mgr->Create(GetUniqueBufferKey("buffer_denoiserOutput"),RT_BUFFER_OUTPUT,RT_FORMAT_FLOAT4);
	F_DBG_ASSERT(m_bufferAODensity);
	F_DBG_ASSERT(m_bufferFinalRadiance);
	F_DBG_ASSERT(m_bufferLaunchOutput);
	F_DBG_ASSERT(m_bufferTonemapOutput);
	F_DBG_ASSERT(m_bufferDenoiserAlbedo);
	F_DBG_ASSERT(m_bufferDenoiserNormal);
	F_DBG_ASSERT(m_bufferDenoiserOutput);


	/// 2. 给上下文设置必要的程式；
	OCL_TRY;
	OCL_CHECK_ERROR(rtContextSetRayGenerationProgram(m_ctx,EntryPoint::E_ENTRYPOINT_RADIANCE,oclProgramManager::S()->GetLaunchProgram_radiance()));
	OCL_CHECK_ERROR(rtContextSetExceptionProgram(m_ctx,EntryPoint::E_ENTRYPOINT_RADIANCE,oclProgramManager::S()->GetExceptionProgram()));
	OCL_CHECK_ERROR(rtContextSetMissProgram(m_ctx,OptixRayTypes::RADIANCE,oclProgramManager::S()->GetMissProgram_radiance()));


	/// 3. 创建后期渲染阶段;
	OCL_CHECK_ERROR(rtPostProcessingStageCreateBuiltin(m_ctx,"TonemapperSimple",&m_ppsTonemapStage));
	OCL_CHECK_ERROR(rtPostProcessingStageDeclareVariable(m_ppsTonemapStage,F_STR(V_C_TONEMAPPED_INPUT_BUFFER),&m_varTonemapInputBuffer));
	OCL_CHECK_ERROR(rtPostProcessingStageDeclareVariable(m_ppsTonemapStage,F_STR(V_C_TONEMAPPED_OUTPUT_BUFFER),&m_varTonemapOutputBuffer));
	OCL_CHECK_ERROR(rtPostProcessingStageDeclareVariable(m_ppsTonemapStage,F_STR(V_C_TONEMAPPED_GAMMA),&m_varTonemapStageGamma));
	OCL_CHECK_ERROR(rtPostProcessingStageDeclareVariable(m_ppsTonemapStage,F_STR(V_C_TONEMAPPED_EXPOSURE),&m_varTonemapStageExposure));
	oclBufferManager::SetoclBufferToRTvariable(m_bufferLaunchOutput,m_varTonemapInputBuffer);
	oclBufferManager::SetoclBufferToRTvariable(m_bufferTonemapOutput,m_varTonemapOutputBuffer);

	OCL_CHECK_ERROR(rtPostProcessingStageCreateBuiltin(m_ctx,"DLDenoiser",&m_ppsDenoiserStage));
	OCL_CHECK_ERROR(rtPostProcessingStageDeclareVariable(m_ppsDenoiserStage,F_STR(V_C_DENOISER_INPUT_BUFFER),&m_varDenoiserInputBuffer));
	OCL_CHECK_ERROR(rtPostProcessingStageDeclareVariable(m_ppsDenoiserStage,F_STR(V_C_DENOISER_ALBEDO_BUFFER),&m_varDenoiserAlbedoBuffer));
	OCL_CHECK_ERROR(rtPostProcessingStageDeclareVariable(m_ppsDenoiserStage,F_STR(V_C_DENOISER_NORMAL_BUFFER),&m_varDenoiserNormalBuffer));
	OCL_CHECK_ERROR(rtPostProcessingStageDeclareVariable(m_ppsDenoiserStage,F_STR(V_C_DENOISER_OUTPUT_BUFFER),&m_varDenoiserOutputBuffer));
	OCL_CHECK_ERROR(rtPostProcessingStageDeclareVariable(m_ppsDenoiserStage,F_STR(V_C_DENOISER_BLEND),&m_varDenoiserStageBlend));
	oclBufferManager::SetoclBufferToRTvariable(m_bufferTonemapOutput,m_varDenoiserInputBuffer);
	oclBufferManager::SetoclBufferToRTvariable(m_bufferDenoiserAlbedo,m_varDenoiserAlbedoBuffer);
	oclBufferManager::SetoclBufferToRTvariable(m_bufferDenoiserNormal,m_varDenoiserNormalBuffer);
	oclBufferManager::SetoclBufferToRTvariable(m_bufferDenoiserOutput,m_varDenoiserOutputBuffer);
	OCL_CATCH;


	/// 4. 设置参数；
	m_data.color_bad.x=1.f;
	m_data.color_bad.y=0.f;
	m_data.color_bad.z=0.f;
	m_data.color_bad.w=1.f;
	m_data.enableGI=true;
	m_data.enableAIdenoise=false;
	m_data.enableAO=false;
	m_data.frame_current=1u;
	//m_data.lightsource_bufID=1000;
	//m_data.lightSource_bufsize=1u;
	m_data.output_AODesityBufID=m_bufferAODensity->GetID();
	m_data.output_FinalRadianceBufID=m_bufferFinalRadiance->GetID();
	m_data.output_launchBufID=m_bufferLaunchOutput->GetID();
	m_data.output_stageAlbedoBufID=m_bufferDenoiserAlbedo->GetID();
	m_data.output_stageNormalBufID=m_bufferDenoiserNormal->GetID();
	m_data.radiance_importanceThreshold=0.0001f;
	m_data.ray_clamp_max=M_MAX_FLOAT;
	m_data.ray_clamp_min=0.00001f;
	m_data.ray_clamp_max_AO=1.f;
	m_data.ray_clamp_min_AO=0.01f;
	m_data.ray_iterateDepth=3u;
	m_data.samples_per_pixel=256u;
	m_data.samples_per_pixel_col=16u;
	m_data.samples_per_pixel_row=16u;
	m_data.skybox_enableBackground=1;
	m_data.skybox_TSID_show=-1;
	m_data.camera_enableDOF=false;
	m_data.camera_focalLength=5.0f;
	m_data.camera_lensRadius=0.1f;
#if __OCL_INTERNAL_DEBUG__
	m_data.color_bad*=100000.f;
#endif
	SetGamma(2.2f);
	SetExposure(4.f);
	SetBlend(0.0f);


	return true;
}

void* oclRendererRadiance::MapOutputBuffer(){
	//test:: return m_bufferLaunchOutput->Map<void,RT_BUFFER_MAP_READ>();
	if(m_data.enableAIdenoise)
		return m_bufferDenoiserOutput->Map<void,RT_BUFFER_MAP_READ>();
	else
		return m_bufferTonemapOutput->Map<void,RT_BUFFER_MAP_READ>();
}

void oclRendererRadiance::UnmapOutputBuffer(){
	//test:: m_bufferLaunchOutput->Unmap(); return;
	if(m_data.enableAIdenoise)
		m_bufferDenoiserOutput->Unmap();
	else
		m_bufferTonemapOutput->Unmap();
}

OCL_NAMESPACE_END
