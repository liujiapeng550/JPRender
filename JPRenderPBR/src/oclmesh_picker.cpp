#include "oclmesh_picker.h"
#include "core\oclbuffer_manager.h"
#include "core\oclprogram_manager.h"
#include "camera/oclcamera.h"
#include "renderer/oclrenderer.h"

OCL_NAMESPACE_START

oclMeshPicker::oclMeshPicker(RTcontext ctx):m_ctx(ctx){
  /// 1. 创建拾取缓冲区;
  m_bufferPicker=oclBufferManager::S()->Create(GetUniqueBufferKey("oclMeshPicker_"),RT_BUFFER_INPUT_OUTPUT,RT_FORMAT_FLOAT4);
  F_DBG_ASSERT(m_bufferPicker);
  m_bufferPicker->SetSize(1);

  /// 2. 给上下文设置必要的程式；
  OCL_TRY;
  OCL_CHECK_ERROR(rtContextSetRayGenerationProgram(m_ctx,EntryPoint::E_ENTRYPOINT_PICK,oclProgramManager::S()->GetLaunchProgram_pick()));
  OCL_CATCH;
}

oclMeshPicker::~oclMeshPicker(){
  oclBufferManager::S()->Destroy(m_bufferPicker);
  m_ctx=nullptr;
}

oclInt oclMeshPicker::GetPickedMeshID(const oclRenderer* renderer,oclInt mousePosInRenderScreen_x,oclInt mousePosInRenderScreen_y){
  // 1. 获取指定渲染器的相机；
  const oclCameraBase* camera=renderer->GetCamera();

  // 2. 根据相机属性与渲染窗口大小，计算射线的方向与起点（worldspace)；
  const Pos origin=camera->GetPos();
  const oclFloat sw=(float)renderer->GetRendererWidth();
  const oclFloat sh=(float)renderer->GetRendererHeight();
  oclFloat2 d;
  d.x=mousePosInRenderScreen_x/sw*2.f-1.f;
  d.y=mousePosInRenderScreen_y/sh*2.f-1.f;
  m_dir=d.x*camera->GetU()+d.y*camera->GetV()+camera->GetW();
  m_dir=optix::normalize(m_dir);

	// 3.pick
  oclFloat4 f4;
  f4.x=m_dir.x;
  f4.y=m_dir.y;
  f4.z=m_dir.z;
  f4.w=-1.f;
  m_bufferPicker->SetData(&f4,1);
  OCL_TRY;
  OCL_CHECK_ERROR(rtContextLaunch1D(m_ctx,EntryPoint::E_ENTRYPOINT_PICK,1));
  OCL_CATCH;
  m_bufferPicker->GetData(&f4,1);
  return oclInt(f4.w);
}

oclInt oclMeshPicker::GetPickBufferID()const{
	return m_bufferPicker->GetID();
};

OCL_NAMESPACE_END
