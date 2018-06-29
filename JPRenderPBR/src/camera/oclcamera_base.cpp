#include "camera/oclcamera.h"
#include <optixu\optixu_math_namespace.h>
#include <optixu\optixu_matrix_namespace.h>
#include "../../host_device/data_renderer.h"

using namespace optix;

OCL_NAMESPACE_START

void oclCameraBase::FetchData(SData_Renderer* data)const{
  if(m_bNeedRefresh||m_bChangeRendererFresh){
    data->camera_pos=m_pos;
    data->camera_U=m_U;
    data->camera_V=m_V;
    data->camera_W=m_W;
    data->camera_enableDOF=m_bEnableDOF;
    data->camera_focalLength=m_fFlocalLength;
    data->camera_lensRadius=m_fLensRadius;
		const Matrix4x4 current_frame_inv=Matrix4x4::fromBasis(
			normalize(m_U),
			normalize(m_V),
			normalize(-m_W),
			m_target).inverse();
		data->matrix_WorldtoCamera=make_matrix3x3(current_frame_inv);
  }
}

OCL_NAMESPACE_END
