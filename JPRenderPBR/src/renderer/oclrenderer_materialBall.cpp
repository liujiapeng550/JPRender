#include "renderer/oclrenderer_materialBall.h"
#include "core/oclbuffer_manager.h"
#include "core\oclprogram_manager.h"
#include "core\oclinstance.h"
#include "camera/oclcamera.h"
#include "material\oclmaterial.h"
#include "material\oclmaterial_library.h"
#include "material\oclmaterial_manager.h"

OCL_NAMESPACE_START

oclRendererMaterialBall::oclRendererMaterialBall(RTcontext ctx)
  :oclRendererRadiance(ctx){}

oclRendererMaterialBall::~oclRendererMaterialBall(){
	m_ref_pMaterialMgr=nullptr;
	m_ref_pIns=nullptr;
}

oclBool oclRendererMaterialBall::IsContinueToRender()const{
	if(m_queMateerialIDCacher.size()>0)return true;
	else{
		/// 渲染次数大于最大渲染数量后返回false，表示不再渲染；
		return m_data.frame_current<=m_data.samples_per_pixel;
	}
}

void oclRendererMaterialBall::PreRender(){
	if(m_nCurrentRenderingMaterialID==M_MATERIAL_ID_NONE){
		oclMaterial* m=nullptr;
		do{
			m_nCurrentRenderingMaterialID=m_queMateerialIDCacher.front();
			m_queMateerialIDCacher.pop();
			oclMaterialLibrary* lib;
			m=m_ref_pMaterialMgr->GetMaterial(m_nCurrentRenderingMaterialID,&lib);
		} while(!m&&m_queMateerialIDCacher.size()>0);
		if(m){
			SMaterialProxy proxy;
			m->FetchProxy(&proxy);
			m_ref_pIns->SetMaterialProxy(proxy);
			m_ref_pIns->Upload();
			ResetFrameCount();
		}
	}
}

void oclRendererMaterialBall::PostRender(){
  ++m_data.frame_current;
	if(m_data.frame_current>m_data.samples_per_pixel){
		m_nCurrentRenderingMaterialID=M_MATERIAL_ID_NONE;
	}
}

oclBool oclRendererMaterialBall::Init(){
	oclRendererRadiance::Init();

	/// 1. 覆盖部分父类中的赋值；
	m_data.enableAIdenoise=true;
	m_data.enableAO=true;
	m_data.frame_current=M_MAX_UINT;
	/*
	m_data.samples_per_pixel=256u;
	m_data.samples_per_pixel_col=16u;
	m_data.samples_per_pixel_row=16u;
	m_data.skybox_enableBackground=1;
	m_data.skybox_TSID_show=-1;
	SetGamma(3.2f);
	SetExposure(6.f);
	SetBlend(0.0f);
	*/


	return true;
}

void oclRendererMaterialBall::SetCamera(oclCameraBase * camera){
	m_ref_pCamera=camera;
	m_ref_pCamera->FetchData(&m_data);
}

void oclRendererMaterialBall::PushMaterialID(oclID id){
	if(m_nCurrentRenderingMaterialID==id)ResetFrameCount();
	else m_queMateerialIDCacher.push(id);
}

OCL_NAMESPACE_END
