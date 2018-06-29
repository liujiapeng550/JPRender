#include "camera/oclcamera.h"
#include <optixu\optixu_math_namespace.h>
#include <optixu\optixu_matrix_namespace.h>
#include "../../host_device/data_renderer.h"

using namespace optix;

OCL_NAMESPACE_START

oclBool oclCameraPerspective::UpdateCamera(const Pos & eye,const Pos & target,const Dir & up,oclFloat HFOV,oclFloat VFOV){
  if(m_pos.x==eye.x && m_fHFOV==HFOV && m_fVFOV==VFOV && m_up.y==up.y){
    m_bNeedRefresh=false;
  } else{
		m_up=up;
		m_target=target;
		m_W=target-eye;
		Dir tmp=cross(up,m_W);
		tmp=normalize(cross(m_W,tmp));
    const oclFloat len=length(m_W);
    if(len>0.f){
      m_U=cross(m_W,tmp);
      m_V=cross(m_U,m_W);
      m_U*=tanf(F_RADIAN(HFOV*0.5f));
      m_V*=tanf(F_RADIAN(VFOV*0.5f))/len;
      m_fHFOV=HFOV;
      m_fVFOV=VFOV;
      m_pos=eye;
      m_bNeedRefresh=true;
    }

  }
  return m_bNeedRefresh;
}

oclBool oclCameraOrth::UpdateCamera(const Pos & eye,const Pos & lookat,const Dir & up,oclFloat HFOV,oclFloat VFOV){
  return m_bNeedRefresh;
}

OCL_NAMESPACE_END
