#include "renderer/oclrenderer_depth.h"
#include "core/oclbuffer_manager.h"
#include "core\oclprogram_manager.h"
#include "camera/oclcamera.h"

OCL_NAMESPACE_START

oclRendererDepth::oclRendererDepth(RTcontext ctx)
  :oclRenderer(ctx){
}

oclRendererDepth::~oclRendererDepth(){
  /// x. 销毁缓冲区；
  oclBufferManager* mgr=oclBufferManager::S();
  mgr->Destroy(m_bufferLaunchOutput->key);
}

oclBool oclRendererDepth::IsContinueToRender()const{
  /// 渲染次数大于最大渲染数量后返回false，表示不再渲染；
	return m_data.frame_current<=m_data.samples_per_pixel;
}

void oclRendererDepth::PreRender(){
	if(m_ref_pCamera->GetNeedUpdateCamera()){
		m_ref_pCamera->FetchData(&m_data);
	}
}

void oclRendererDepth::Render(){
	OCL_TRY;
  OCL_CHECK_ERROR(rtContextLaunch2D(m_ctx,EntryPoint::E_ENTRYPOINT_DEPTH,m_uWidth,m_uHeight));
	OCL_CATCH;
}

void oclRendererDepth::PostRender(){
	++m_data.frame_current;
}

oclBool oclRendererDepth::Init(){
  /// 1. 共享Renderer_radiance的缓冲区；
  oclBufferManager* mgr=oclBufferManager::S();
	m_bufferLaunchOutput=mgr->Create(GetUniqueBufferKey("buffer_depth_launchOutput"),RT_BUFFER_OUTPUT,RT_FORMAT_FLOAT4);
  F_DBG_ASSERT(m_bufferLaunchOutput);


  /// 2. 给上下文设置必要的程式；
  OCL_TRY;
  OCL_CHECK_ERROR(rtContextSetRayGenerationProgram(m_ctx,EntryPoint::E_ENTRYPOINT_DEPTH,oclProgramManager::S()->GetLaunchProgram_depth()));
  OCL_CHECK_ERROR(rtContextSetExceptionProgram(m_ctx,EntryPoint::E_ENTRYPOINT_DEPTH,oclProgramManager::S()->GetExceptionProgram()));
  OCL_CATCH;


  /// 5. 设置其他参数；
  m_data.frame_current=1u;
  m_data.output_launchBufID=m_bufferLaunchOutput->GetID();
  m_data.radiance_importanceThreshold=0.0001f;
  m_data.ray_clamp_max=M_MAX_FLOAT;
  m_data.ray_clamp_min=0.00001f;
  m_data.samples_per_pixel=256u;
  m_data.samples_per_pixel_col=16u;
  m_data.samples_per_pixel_row=16u;
  m_data.skybox_enableBackground=1;
  m_data.skybox_TSID_show=-1;
  m_data.DepthRenderer_depth_max=1.f;
  m_data.DepthRenderer_depth_min=0.1f;

  return true;
}

void oclRendererDepth::SetRendererSize(oclUint width,oclUint height){
  m_uWidth=width;
  m_uHeight=height;
	m_bufferLaunchOutput->SetSize(width,height);
}

void* oclRendererDepth::MapOutputBuffer(){
	return m_bufferLaunchOutput->Map<void,RT_BUFFER_MAP_READ>();
}

void oclRendererDepth::UnmapOutputBuffer(){
	m_bufferLaunchOutput->Unmap();
}

void oclRendererDepth::SetDepthMax(oclFloat value){
  m_data.DepthRenderer_depth_max=value;
}

void oclRendererDepth::SetDepthMin(oclFloat value){
  m_data.DepthRenderer_depth_min=value;
}

OCL_NAMESPACE_END
