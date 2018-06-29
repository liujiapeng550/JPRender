#pragma once

#include "core/oclcore.h"

struct SData_Renderer;

OCL_NAMESPACE_START

class oclCameraBase {
public:
	oclCameraBase(){
		F_DBG_INFO("[CONSTRUCT] -> oclCameraBase");
	};
	virtual ~oclCameraBase(){};

	virtual oclBool UpdateCamera(const Pos& eye,const Pos& lookat,const Dir& up,oclFloat HFOV,oclFloat VFOV)=0;

	void FetchData(SData_Renderer* data)const;

	Pos GetPos()const{ return m_pos; }

	Dir GetU()const{ return m_U; }

	Dir GetV()const{ return m_V; }

	Dir GetW()const{ return m_W; }

	void SetFocalLength(oclFloat value){ m_fFlocalLength=value; }

	oclFloat GetFocalLength()const{ return m_fFlocalLength; }

	void SetLensRadius(oclFloat value){ m_fLensRadius=value; }

	oclFloat GetLensRadius()const{ return m_fLensRadius; }

	void SetEnableDOF(oclBool value){ m_bEnableDOF=value; }

	oclBool GetEnableDOF()const{ return m_bEnableDOF; }

	void SetNeedRefreshCamera(){ m_bChangeRendererFresh=true; }

	inline bool GetNeedUpdateCamera()const{
		return m_bNeedRefresh||m_bChangeRendererFresh;
	}

protected:
	Pos m_pos;
	Dir m_up;
	oclFloat m_fHFOV;
	oclFloat m_fVFOV;
	Dir m_U,m_V,m_W;
	Dir m_target;
	oclBool m_bEnableDOF=false;
	oclFloat m_fLensRadius=0.f;
	oclFloat m_fFlocalLength=1.f;
	oclBool m_bNeedRefresh=true;
	oclBool m_bChangeRendererFresh=false;
};

class oclCameraPerspective:public oclCameraBase{
public:
	oclCameraPerspective(){};
	~oclCameraPerspective(){};

	oclBool UpdateCamera(const Pos& eye,const Pos& lookat,const Dir& up,oclFloat HFOV,oclFloat VFOV)override;

};

class oclCameraOrth:public oclCameraBase{
public:
	oclCameraOrth(){};
	~oclCameraOrth(){};

	oclBool UpdateCamera(const Pos& eye,const Pos& lookat,const Dir& up,oclFloat HFOV,oclFloat VFOV)override;
};

OCL_NAMESPACE_END