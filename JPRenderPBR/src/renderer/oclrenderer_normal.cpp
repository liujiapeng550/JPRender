#include "renderer/oclrenderer_normal.h"
#include "core/oclbuffer_manager.h"
#include "core\oclprogram_manager.h"
#include "camera/oclcamera.h"

OCL_NAMESPACE_START

oclRendererNormal::oclRendererNormal(RTcontext ctx)
  :oclRenderer(ctx){}

oclRendererNormal::~oclRendererNormal(){
  /// x. 销毁缓冲区；
  oclBufferManager* mgr=oclBufferManager::S();
  mgr->Destroy(m_bufferLaunchOutput->key);
}

oclBool oclRendererNormal::IsContinueToRender()const{
  /// 渲染次数大于最大渲染数量后返回false，表示不再渲染；
	return m_data.frame_current<=m_data.samples_per_pixel;
}

void oclRendererNormal::PreRender(){
	if(m_ref_pCamera->GetNeedUpdateCamera()){
		m_ref_pCamera->FetchData(&m_data);
	}
}

void oclRendererNormal::Render(){
  OCL_TRY;
  OCL_CHECK_ERROR(rtContextLaunch2D(m_ctx,EntryPoint::E_ENTRYPOINT_NORMAL,m_uWidth,m_uHeight));
	OCL_CATCH;
}

void oclRendererNormal::PostRender(){
	++m_data.frame_current;
}

oclBool oclRendererNormal::Init(){
  /// 1. 共享Renderer_radiance的缓冲区；
  oclBufferManager* mgr=oclBufferManager::S();
	m_bufferLaunchOutput=mgr->Create(GetUniqueBufferKey("buffer_normal_launchOutput"),RT_BUFFER_OUTPUT,RT_FORMAT_FLOAT4);
  F_DBG_ASSERT(m_bufferLaunchOutput);


  /// 2. 给上下文设置必要的程式；
  OCL_TRY;
  OCL_CHECK_ERROR(rtContextSetRayGenerationProgram(m_ctx,EntryPoint::E_ENTRYPOINT_NORMAL,oclProgramManager::S()->GetLaunchProgram_normal()));
  OCL_CHECK_ERROR(rtContextSetExceptionProgram(m_ctx,EntryPoint::E_ENTRYPOINT_NORMAL,oclProgramManager::S()->GetExceptionProgram()));
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
	m_data.enableShowCameraSpaceNormal=false;

  return true;
}

void oclRendererNormal::SetRendererSize(oclUint width,oclUint height){
  m_uWidth=width;
  m_uHeight=height;
	m_bufferLaunchOutput->SetSize(width,height);
}

void* oclRendererNormal::MapOutputBuffer(){
	return m_bufferLaunchOutput->Map<void,RT_BUFFER_MAP_READ>();
}

void oclRendererNormal::UnmapOutputBuffer(){
	m_bufferLaunchOutput->Unmap();
}

void oclRendererNormal::SetShowCameraSpaceNormal(oclBool value){
	m_data.enableShowCameraSpaceNormal=value;
}

OCL_NAMESPACE_END
