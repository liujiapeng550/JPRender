#include "light\ocllight_source_area.h"
#include "core\ocldistribution.h"
#include "core\oclbuffer_manager.h"

using namespace optix;

OCL_NAMESPACE_START

oclLightSourceArea::oclLightSourceArea(oclInt id,RTcontext ctx)
	:oclLightSource(id,LIGHT_TYPE::AREA,1u),m_ctx(ctx){
	m_lightData.lightFuncIndex=CFINDEX_LIGHT_AREA;
	m_lightData.diffuse=make_float4(1.f);
	m_lightData.intensity=1.f;
}

oclLightSourceArea::~oclLightSourceArea(){
	oclBufferManager* bufMgr=oclBufferManager::S();
	bufMgr->Destroy(m_CDFOB);
	bufMgr->Destroy(m_valueOB);
}

oclBool oclLightSourceArea::Init(){
	/// 1. 创建UI界面需要的Json数据；
	Json::Value Property;
	// meshID相关信息的输入是在PRB项目中，因为要获取模型相关数据，而Lights无权获取；
	Property["Base"]["nodeID"]=CreateParams_("nodeID",-1,LIGHT_VALUE_TYPE::INT,0,10000);
	/*
	//JC:区域光的颜色与强度从发光材质获取
	Property["Base"]["Intensity"]=CreateParams_("Intensity",m_lightData.intensity,LIGHT_VALUE_TYPE::FLOAT,0,10000);
	Property["Base"]["Diffuse"]["valueType"]=(oclInt)LIGHT_VALUE_TYPE::COLOR;
	Property["Base"]["Diffuse"]["value"].append(CreateParams_("x",m_lightData.diffuse.x,LIGHT_VALUE_TYPE::COLOR,0,1));
	Property["Base"]["Diffuse"]["value"].append(CreateParams_("y",m_lightData.diffuse.y,LIGHT_VALUE_TYPE::COLOR,0,1));
	Property["Base"]["Diffuse"]["value"].append(CreateParams_("z",m_lightData.diffuse.z,LIGHT_VALUE_TYPE::COLOR,0,1));
	*/
	m_props["Property"]=Property;


	/// 2. 创建缓冲区；
	oclBufferManager* bufMgr=oclBufferManager::S();
	m_CDFOB=bufMgr->Create(GetUniqueBufferKey("light_area_"),RT_BUFFER_INPUT,RT_FORMAT_FLOAT);
	m_valueOB=bufMgr->Create(GetUniqueBufferKey("light_area_"),RT_BUFFER_INPUT,RT_FORMAT_FLOAT);

	return true;
}

void oclLightSourceArea::SetAreaLightAccessory(const std::vector<oclFloat>& areas,const SInstanceProxy& proxy){
	m_oldMeshNodeIndex=proxy.nodeIndex;
	const oclUint N=areas.size();


	/// 1. 计算面积的一维分布；
	oclDistribution1D dis1D(&areas[0],N);
	m_CDFOB->SetSize(N+1);
	oclFloat* data=m_CDFOB->Map<oclFloat>();
	dis1D.CopyCDF(data);
	m_CDFOB->Unmap();


	/// 2. 拷贝面积具体数据到缓冲区；
	m_valueOB->SetSize(N);
	m_valueOB->SetData(&areas[0],N);


	/// 3. 赋值；
	oclInt sumArea=0;
	for(int i=0; i<N; i++){
		sumArea+=areas[i];
	}
	m_lightData.sumArea=sumArea;
	m_lightData.meshAreaIntegration=dis1D.GetIntegration();
	m_lightData.meshAreaCDFBufferID=m_CDFOB->GetID();
	m_lightData.meshAreaBufferID=m_valueOB->GetID();
	m_lightData.meshCount=N;
	m_lightData.nodeIndex=proxy.nodeIndex;
	m_lightData.bufferID_index=proxy.bufferID_index;
	m_lightData.bufferID_vertex=proxy.bufferID_vertex;
	m_lightData.bufferID_worldMatrix=proxy.bufferID_worldMatrix;
	m_lightData.bufferID_materials=proxy.bufferID_materials;
	m_lightData.bufferOffset_materials=proxy.bufferOffset_materials;
}

void oclLightSourceArea::SetPropertyToData_(const oclChar* propName,VARIANT* v){
	/*
	//JC:区域光的颜色与强度从发光材质获取
	if(strcmp("Diffuse",propName)==0){
		m_lightData.diffuse.x=v[0].fltVal;
		m_lightData.diffuse.y=v[1].fltVal;
		m_lightData.diffuse.z=v[2].fltVal;
	} else if(strcmp("Intensity",propName)==0){
		m_lightData.intensity=v->fltVal;
	}
	*/
}

oclBool oclLightSourceArea::UpdateWorldMatrix(oclFloat* matrix){
	return true;
}

oclBool oclLightSourceArea::SetCenter(oclFloat* center){
	return true;
}

oclBool oclLightSourceArea::NeedUpload() const{
	oclBool l=luminanceCIE_(m_lightData.diffuse*m_lightData.intensity)>M_MIN_IMPORTANCE;
	return (m_lightData.sumArea>M_EPSILON)&&l&& m_bActived;
}

OCL_NAMESPACE_END