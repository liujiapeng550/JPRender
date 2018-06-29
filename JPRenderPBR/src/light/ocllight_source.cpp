#include "light/ocllight_source.h"
#include <atlconv.h>
#include <mmsystem.h>
#include <atlstr.h>
#include "core\oclmontecarlo.h"
#include "core\oclrng.h"
#include "ocllight_sh.h"

OCL_NAMESPACE_START

oclLightSource::oclLightSource(oclInt id,LIGHT_TYPE type,oclUint ns)
	:m_type(type),m_nSamples(ns){}

oclLightSource::~oclLightSource(){
	m_currentMatrix=nullptr;
}

void oclLightSource::SetProperty(const oclChar* propName,VARIANT* v){
	//TODO:JC:转移到jsonconverter中去
#if __OCL_INTERNAL_DEBUG__
	oclStr testString=m_props.toStyledString();
#endif
	SetPropertyToData_(propName,v);
	Json::Value::Members mem=m_props["Property"].getMemberNames();
	for(auto iter=mem.begin(); iter!=mem.end(); iter++){
		auto nameMem=m_props["Property"][*iter].getMemberNames();
		for(auto niter=nameMem.begin(); niter!=nameMem.end(); niter++){
			if((*niter).compare(propName)==0){
				Json::Value& tmp=m_props["Property"][*iter][*niter];
				switch(tmp["valueType"].asInt()){
				case LIGHT_VALUE_TYPE::BOOL:
				tmp["realValue"]=v->boolVal;
				break;
				case LIGHT_VALUE_TYPE::COLOR:
				for(oclInt i=0; i<3; i++){
					tmp["value"][i]["realValue"]=v[i].fltVal;
				}
				break;
				case LIGHT_VALUE_TYPE::FLOAT:
				tmp["realValue"]=v->fltVal;
				break;
				case LIGHT_VALUE_TYPE::INT:
				tmp["realValue"]=v->intVal;
				break;
				case LIGHT_VALUE_TYPE::ENUM:
				case LIGHT_VALUE_TYPE::STRING:
				tmp["realValue"]=_com_util::ConvertBSTRToString(v->bstrVal);
				default:
				break;
				}
			}
		}
	}
}

oclBool oclLightSource::GetProperty(const oclChar* propName,oclFloat* v)const{
#if __OCL_INTERNAL_DEBUG__
	oclStr testString=m_props.toStyledString();
#endif
	Json::Value::Members mem=m_props["Property"].getMemberNames();
	for(auto iter=mem.begin(); iter!=mem.end(); iter++){
		auto nameMem=m_props["Property"][*iter].getMemberNames();
		for(auto niter=nameMem.begin(); niter!=nameMem.end(); niter++){
			if((*niter).compare(propName)==0){
				const Json::Value& tmp=m_props["Property"][*iter][*niter];
				switch(tmp["valueType"].asInt()){
				case LIGHT_VALUE_TYPE::BOOL:
				*v=(oclFloat)tmp["realValue"].asBool();
				break;
				case LIGHT_VALUE_TYPE::COLOR:
				for(oclInt i=0; i<3; i++){
					v[i]=tmp["value"][i]["realValue"].asFloat();
				}
				break;
				case LIGHT_VALUE_TYPE::FLOAT:
				*v=tmp["realValue"].asFloat();
				break;
				case LIGHT_VALUE_TYPE::INT:
				*v=(oclFloat)tmp["realValue"].asInt();
				break;
				case LIGHT_VALUE_TYPE::ENUM:
				case LIGHT_VALUE_TYPE::STRING:
				default:
				break;
				}
				return true;
			}
		}
	}
	return false;
}

Json::Value  oclLightSource::CreateTransVal_(oclFloat max,oclFloat min,oclFloat step,oclFloat val){
	Json::Value ret;
	ret["max"]=max;
	ret["min"]=min;
	ret["step"]=step;
	ret["val"]=val;
	ret["value"]=0;
	return ret;
}

void oclLightSource::SetEulerAngle(oclFloat x,oclFloat y,oclFloat z){
	m_props["TabValue"]["mesh_transfrom_euler_rotation"]["value"][0]["value"]=x;
	m_props["TabValue"]["mesh_transfrom_euler_rotation"]["value"][1]["value"]=y;
	m_props["TabValue"]["mesh_transfrom_euler_rotation"]["value"][2]["value"]=z;
}

void oclLightSource::GetWorldMatrix(oclFloat * matrix)const{
	if(m_currentMatrix){
		memcpy(matrix,m_currentMatrix,sizeof(oclFloat)*12u);
	} else
		memset(matrix,0.f,sizeof(oclFloat)*12u);
}

void oclLightSource::SetWorldMatrix(oclFloat* matrix){
	UpdateWorldMatrix(matrix);
	m_currentMatrix=matrix;
}

void oclLightSource::CreateTrans3P_(const oclChar* showName,const oclChar* realName,oclFloat val_x,oclFloat val_y,oclFloat val_z){
	Json::Value Index;
	Index[showName]=realName;
	m_props["TabValue"][realName]["name"]=showName;
	m_props["TabValue"][realName]["type"]=14;
	m_props["TabValue"][realName]["value"].append(CreateTransVal_(1000000,-1000000,0.1,val_x));
	m_props["TabValue"][realName]["value"].append(CreateTransVal_(1000000,-1000000,0.1,val_y));
	m_props["TabValue"][realName]["value"].append(CreateTransVal_(1000000,-1000000,0.1,val_z));

	m_props["Tab"]["Transform"].append(Index);
}

void oclLightSource::CreateTrans3F_(const oclChar* showName,const oclChar* realName,oclFloat val_x,oclFloat val_y,oclFloat val_z){
	Json::Value Index;
	Index[showName]=realName;
	m_props["TabValue"][realName]["name"]=showName;
	m_props["TabValue"][realName]["type"]=14;
	m_props["TabValue"][realName]["value"].append(CreateTransVal_(1000000,-1000000,0.1,val_x));
	m_props["TabValue"][realName]["value"].append(CreateTransVal_(1000000,-1000000,0.1,val_y));
	m_props["TabValue"][realName]["value"].append(CreateTransVal_(1000000,-1000000,0.1,val_z));

	m_props["Tab"]["WH"].append(Index);
}

void oclLightSource::CreateTransBt_(const oclChar* showName,const oclChar* realName){
	Json::Value Index;
	Index[showName]=realName;
	m_props["TabValue"][realName]["name"]=showName;
	m_props["TabValue"][realName]["type"]=47;
	m_props["Tab"]["Transform"].append(Index);
}

oclStr oclLightSource::FindName_(const oclChar* name,oclFloat val){
	Json::Value::Members mem=m_props["Property"].getMemberNames();
	for(auto iter=mem.begin(); iter!=mem.end(); iter++){
		auto nameMem=m_props["Property"][*iter].getMemberNames();
		for(auto niter=nameMem.begin(); niter!=nameMem.end(); niter++){
			if((*niter).compare(name)==0){
				m_props["Property"][*iter][*niter]["realValue"]=val;
				m_props["Property"][*iter][*niter]["NA"]=false;
				return m_props["Property"][*iter][*niter]["realName"].asString();
			}
		}
	}
	return "";
}

oclStr oclLightSource::FindName_(const oclChar* name,const oclChar* val){
	Json::Value::Members mem=m_props["Property"].getMemberNames();
	for(auto iter=mem.begin(); iter!=mem.end(); iter++){
		auto nameMem=m_props["Property"][*iter].getMemberNames();
		for(auto niter=nameMem.begin(); niter!=nameMem.end(); niter++){
			if((*niter).compare(name)==0){
				m_props["Property"][*iter][*niter]["realValue"]=val;
				m_props["Property"][*iter][*niter]["NA"]=false;
				return m_props["Property"][*iter][*niter]["realName"].asString();
			}
		}
	}
	return "";
}

Json::Value oclLightSource::CreateParams_(const oclChar* realName,oclFloat realValue,LIGHT_VALUE_TYPE valueType,oclFloat min,oclFloat max){
	Json::Value result;
	result["realName"]=realName;
	result["realValue"]=realValue;
	result["valueType"]=(oclInt)valueType;
	if(valueType<LIGHT_VALUE_TYPE::BOOL){
		result["min"]=min;
		result["max"]=max;
	}
	result["NA"]=false;
	return result;
}

Json::Value oclLightSource::CreateParams_2_(const oclChar* realName,const oclChar* realValue,LIGHT_VALUE_TYPE valueType){
	Json::Value result;
	result["realName"]=realName;
	if(valueType==LIGHT_VALUE_TYPE::STRING){
		result["realValue"]=realValue;
	}
	result["valueType"]=(oclInt)valueType;
	result["NA"]=false;
	return result;
}

OCL_NAMESPACE_END