#pragma once

#include <optixu\optixu_math_namespace.h>
#include "core\oclcore.h"

OCL_NAMESPACE_START

class oclBuffer;
class oclRenderer;

/**
 * mesh拾取器是将需要计算拾取的所有信息保存在一个float4中
 * （前3个字段是方向，第四个拾取到的meshID）；
 * 然后通过一个buffer提交给OptiX，最后依然是通过该buffer获取到MeshID
 */
class oclMeshPicker final{
public:
  oclMeshPicker(const oclMeshPicker&)=delete;
  oclMeshPicker operator=(const oclMeshPicker&)=delete;
	oclMeshPicker(RTcontext ctx);
  ~oclMeshPicker();

  /**
   * mousePosInRenderScreen_x|y   鼠标在渲染区域左上角的相对坐标；
   */
  oclInt GetPickedMeshID(const oclRenderer* renderer,oclInt mousePosInRenderScreen_x,oclInt mousePosInRenderScreen_y);

	oclInt GetPickBufferID()const;

private:
  oclBuffer* m_bufferPicker=nullptr;
  RTcontext m_ctx;
  Nor m_dir;


};

OCL_NAMESPACE_END
