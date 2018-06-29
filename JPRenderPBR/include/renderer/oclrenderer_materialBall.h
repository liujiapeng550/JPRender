#pragma once

#include "oclrenderer_radiance.h"
#include <queue>

OCL_NAMESPACE_START

class oclBuffer;
class oclMaterialManager;
class oclInstance;

class oclRendererMaterialBall:public oclRendererRadiance{
public:
	oclRendererMaterialBall& operator=(oclRendererMaterialBall& o)=delete;
	oclRendererMaterialBall(oclRendererMaterialBall& o)=delete;
	oclRendererMaterialBall(RTcontext);
  ~oclRendererMaterialBall();

  oclBool IsContinueToRender()const override;

  void PreRender()override;

  void PostRender()override;

  oclBool Init()override;

	void SetCamera(oclCameraBase* camera)override;

	void PushMaterialID(oclID id);

	void SetMaterialManager(oclMaterialManager* mgr){
		m_ref_pMaterialMgr=mgr;
	}

	void SetMaterialBallInstance(oclInstance* ins){
		m_ref_pIns=ins;
	}

private:
	/*
	用于缓存将要渲染的材质ID，因为材质是一个一个渲染的，前面未渲染完成，后面按照目前的设计只能等待；
	渲染完成指的是一共需要渲染的帧数大于既定帧数；
	该vec为空的话材质球渲染器暂停工作，
	每渲染完成一个pop一个ID；
	如果用户将正在渲染的材质属性变动的话：重置该渲染器framecount
	正在渲染的材质id是已经pop出该vector的；
	*/
	std::queue<oclID> m_queMateerialIDCacher;
	oclMaterialManager* m_ref_pMaterialMgr=nullptr;
	oclID m_nCurrentRenderingMaterialID=M_MATERIAL_ID_NONE;
	oclInstance* m_ref_pIns=nullptr;
};

OCL_NAMESPACE_END

