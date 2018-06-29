#include "light/ocllight_source_spot.h"
#include <optixu/optixu_math_namespace.h>

OCL_NAMESPACE_START

oclLightSourceSpot::oclLightSourceSpot(oclInt id)
	:oclLightSource(id,LIGHT_TYPE::SPOT,1u){
	m_lightData.lightFuncIndex=CFINDEX_LIGHT_SPOT;
	m_lightData.pos.z=200.f;
	m_lightData.diffuse.x=0.f;
	m_lightData.diffuse.y=0.5f;
	m_lightData.diffuse.z=0.f;
	m_lightData.radius=1000.f;
	m_lightData.fullLightTheta=90.f;
	m_lightData.totalTheta=180.f;
	m_lightData.intensity=1.f;
}

oclLightSourceSpot::~oclLightSourceSpot(){}

oclBool oclLightSourceSpot::Init(){
	Json::Value Property;
	Property["Base"]["Radius"]=CreateParams_("Radius",30,LIGHT_VALUE_TYPE::FLOAT,0,100000);
	Property["Base"]["TotalTheta"]=CreateParams_("TotalTheta",180,LIGHT_VALUE_TYPE::FLOAT,0,180);
	Property["Base"]["FullLightTheta"]=CreateParams_("FullLightTheta",30,LIGHT_VALUE_TYPE::FLOAT,0,90);
	Property["Base"]["Intensity"]=CreateParams_("Intensity",m_lightData.intensity,LIGHT_VALUE_TYPE::FLOAT,0,100);
	Property["Base"]["Diffuse"]["valueType"]=(oclInt)LIGHT_VALUE_TYPE::COLOR;
	Property["Base"]["Diffuse"]["value"].append(CreateParams_("x",m_lightData.diffuse.x,LIGHT_VALUE_TYPE::COLOR,0,1));
	Property["Base"]["Diffuse"]["value"].append(CreateParams_("y",m_lightData.diffuse.y,LIGHT_VALUE_TYPE::COLOR,0,1));
	Property["Base"]["Diffuse"]["value"].append(CreateParams_("z",m_lightData.diffuse.z,LIGHT_VALUE_TYPE::COLOR,0,1));
	m_props["Property"]=Property;
	return true;
}

void oclLightSourceSpot::SetPropertyToData_(const oclChar* propName,VARIANT* v){
	if(strcmp("Diffuse",propName)==0){
		m_lightData.diffuse.x=v[0].fltVal;
		m_lightData.diffuse.y=v[1].fltVal;
		m_lightData.diffuse.z=v[2].fltVal;
	} else if(strcmp("Radius",propName)==0){
		m_lightData.radius=v->fltVal;
	} else if(strcmp("Intensity",propName)==0){
		m_lightData.intensity=v->fltVal;
	} else if(strcmp("TotalTheta",propName)==0){
		m_lightData.totalTheta=v->fltVal;
	} else if(strcmp("FullLightTheta",propName)==0){
		m_lightData.fullLightTheta=v->fltVal;
	}
}

oclBool oclLightSourceSpot::UpdateWorldMatrix(oclFloat* matrix){
	oclFloat3 vec3;
	vec3.x=matrix[3];
	vec3.y=matrix[7];
	vec3.z=matrix[11];
	m_lightData.pos.x=vec3.x;
	m_lightData.pos.y=vec3.y;
	m_lightData.pos.z=vec3.z;

	oclFloat3 centerDir;
	centerDir.x=matrix[0];
	centerDir.y=matrix[4];
	centerDir.z=matrix[8];
	centerDir=optix::normalize(centerDir);
	m_lightData.dir.x=centerDir.x;
	m_lightData.dir.y=centerDir.y;
	m_lightData.dir.z=centerDir.z;

	m_props["TabValue"]["mesh_transform_translation"]["value"][0]["value"]=vec3.x;
	m_props["TabValue"]["mesh_transform_translation"]["value"][1]["value"]=vec3.y;
	m_props["TabValue"]["mesh_transform_translation"]["value"][2]["value"]=vec3.z;
	return true;
}

oclBool oclLightSourceSpot::NeedUpload() const{
	oclBool l=luminanceCIE_(m_lightData.diffuse*m_lightData.intensity)>M_MIN_IMPORTANCE;
	return l&& m_bActived;
}

oclBool oclLightSourceSpot::SetCenter(oclFloat* center){
	m_props["TabValue"]["mesh_transform_center"]["value"][0]["value"]=center[3]-(2*m_lightData.pos.x);
	m_props["TabValue"]["mesh_transform_center"]["value"][1]["value"]=center[7]-(2*m_lightData.pos.y);
	m_props["TabValue"]["mesh_transform_center"]["value"][2]["value"]=center[11]-(2*m_lightData.pos.z);
	return true;
}

OCL_NAMESPACE_END