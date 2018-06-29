#include "light/ocllight_manager.h"
#include <atlstr.h>
#include "core/oclbuffer_manager.h"
#include "core\oclbuffer_manager.h"
#include "light/ocllight_source.h"
#include "light/ocllight_source_point.h"
#include "light/ocllight_source_spot.h"
#include "light/ocllight_source_distant.h"
#include "light/ocllight_source_environment.h"
#include "light/ocllight_source_area.h"
#include "EncodingConversion.h"
#define BOOST_FILESYSTEM_NO_DEPRECATED
#include "boost\filesystem.hpp"

using namespace std;
using namespace Json;

OCL_NAMESPACE_START

oclLightManager::oclLightManager(RTcontext ctx) :m_ctx(ctx){
	F_DBG_INFO("[CONSTRUCT] -> oclLightManager");
	Init_();
}

oclLightManager::~oclLightManager(){
	DestroyLights();
  oclBufferManager::S()->Destroy(m_bufferLights);
  oclBufferManager::S()->Destroy(m_bufferMaterialBallLights);
	F_SAFE_DELETE(m_materialBallLight);
	m_bufferLights=nullptr;
	m_bufferMaterialBallLights=nullptr;
  m_ctx=nullptr;
}

oclLightSource* oclLightManager::CreateLight(oclUint lightIndex,const oclChar* lightType){
  oclLightSource *p=nullptr;
  if(strcmp(lightType,L_POINT)==0){
    p=new oclLightSourcePoint(lightIndex);
  } else if(strcmp(lightType,L_DISTSNT)==0){
    p=new oclLightSourceDistant(lightIndex);
  } else if(strcmp(lightType,L_SPOT)==0){
    p=new oclLightSourceSpot(lightIndex);
  } else if(strcmp(lightType,L_ENVIRONMENT)==0){
    p=new oclLightSourceEnvironment(lightIndex,m_ctx);
  } else if(strcmp(lightType,L_AREA_RECTANGLE)==0){
    p=new oclLightSourceArea(lightIndex,m_ctx);
  }
  if(!p)return false;
  p->Init();
  m_mapLights[lightIndex]=p;
	return p;
}

oclStr oclLightManager::GetLightJsonString(const vector<oclInt>& nodeIndexes)const{
  Reader read;
  Value res;
  Value value;
  for(vector<oclInt>::const_iterator it=nodeIndexes.begin(); it<nodeIndexes.end(); it++){
    oclUint index=*it;
    map<oclUint,oclLightSource*>::const_iterator itLight=m_mapLights.find(index);
    if(itLight!=m_mapLights.end()){
      if(res.isNull()){
        read.parse(itLight->second->GetJsonString(),res);
        //read.parse(it)
      }
      if(it!=nodeIndexes.begin()){
        read.parse(itLight->second->GetJsonString(),value);
        res=GetSameBuffer(res,value);
      }
    }
  }
  oclStr str=res.toStyledString();
  return str;
}

Value oclLightManager::GetSameBuffer(Value fstV,Value SecV)const{
  Value::Members propMem1=fstV["Property"].getMemberNames();
  Value::Members propMem2=SecV["Property"].getMemberNames();
  vector<oclStr> deleteMem;
  for(auto propiter1=propMem1.begin(); propiter1<propMem1.end(); propiter1++){
    oclBool del=false;
    for(auto propiter2=propMem2.begin(); propiter2<propMem2.end(); propiter2++){
      if((*propiter1).compare(*propiter2)!=0){

      } else{
        del=true;
        vector<oclStr> deleteMem;
        auto mvit1=fstV["Property"][*propiter1].getMemberNames();
        auto mvit2=SecV["Property"][*propiter2].getMemberNames();

        for(auto propiter1_iter=mvit1.begin(); propiter1_iter<mvit1.end(); propiter1_iter++){
          oclBool del=false;
          for(auto propiter2_iter=mvit2.begin(); propiter2_iter<mvit2.end(); propiter2_iter++){
            if((*propiter1_iter).compare(*propiter2_iter)!=0){

            } else{
              del=true;
              if(fstV["Property"][*propiter1][*propiter1_iter].compare(SecV["Property"][*propiter2][*propiter2_iter])!=0){
                fstV["Property"][*propiter1][*propiter1_iter]["NA"]=true;
              }
              break;
            }
          }
          if(!del){
            deleteMem.push_back(*propiter1_iter);
          }
        }
        for(auto it=deleteMem.begin(); it<deleteMem.end(); it++){
          fstV["Property"][*propiter1].removeMember(*it);
        }
        break;
      }
    }
    if(!del){
      deleteMem.push_back(*propiter1);
    }
  }
  for(auto it=deleteMem.begin(); it<deleteMem.end(); it++){
    fstV["Property"].removeMember(*it);
  }
  for(oclInt i=0; i<fstV["Tab"]["Transform"].size(); i++){
    oclBool ret=true;
    for(oclInt j=0; j<SecV["Tab"]["Transform"].size(); j++){
      if(!fstV["Tab"]["Transform"][i][*fstV["Tab"]["Transform"][i].getMemberNames().begin()].asString().compare(SecV["Tab"]["Transform"][j][*SecV["Tab"]["Transform"][j].getMemberNames().begin()].asString())){
        ret=false;
        Value removed;
        SecV["Tab"]["Transform"].removeIndex(j,&removed);
        removed=removed[*removed.getMemberNames().begin()];
        auto testStr=removed.toStyledString();
        auto testStr1=fstV["TabValue"].toStyledString();
        auto testStr2=removed.asString();
        auto c_str=removed.asString().c_str();
        if(!fstV["TabValue"][removed.asString()]["value"].isNull()){
          for(oclInt valueIndex=0; valueIndex<fstV["TabValue"][removed.asString()]["value"].size(); valueIndex++){
            if(fstV["TabValue"][removed.asString()]["value"][valueIndex]["value"].asFloat()!=SecV["TabValue"][removed.asString()]["value"][valueIndex]["value"].asFloat()){
              fstV["TabValue"][removed.asString()]["value"][valueIndex]["value"]=Value::null;
            }
          }
        }
        break;
      }
    }
    if(ret){
      Value removed;
      fstV["Tab"]["Transform"].removeIndex(i,&removed);
      fstV["Tab"]["TabValue"].removeMember(removed[*removed.getMemberNames().begin()].asString());
    }

  }
  return fstV;
}

void oclLightManager::Upload(){
  vector<const SData_Light*> m_v_Light;
  oclBool bIsActiveDefaultHDR=true;
  oclBool bIsExistedCreateHDR=false;
  for(map<oclUint,oclLightSource*>::reverse_iterator iter=m_mapLights.rbegin(); iter!=m_mapLights.rend(); ++iter){
    const oclLightSource* light=iter->second; 
    if(light->NeedUpload()){
      int nLightType=light->GetLightData()->lightFuncIndex;
      if(nLightType==CFINDEX_LIGHT_ENVIRONMENT){
        bIsActiveDefaultHDR=false;
        if(!bIsExistedCreateHDR){
          m_v_Light.push_back(light->GetLightData());
          bIsExistedCreateHDR=true;
        }
      } else{
        m_v_Light.push_back(light->GetLightData());
      }
    }
  }
  if(bIsActiveDefaultHDR){
    m_v_Light.push_back(m_materialBallLight->GetLightData());
  }

  const oclInt N=m_v_Light.size();
  m_bufferLights->SetSize(N);
  if(N>0){
    for(oclUint i=0; i<N; ++i){
      m_bufferLights->SetData<SData_Light>(m_v_Light[i],1,i);
    }
  }
}

void oclLightManager::DestroyLights(){
	m_bufferLights->SetSize(0);
  for(auto &iter:m_mapLights){
    delete iter.second;
  }
  m_mapLights.clear();
}

Json::Value oclLightManager::GetLightsJson()const{
  Value allbuffer;
  for(auto iter=m_mapLights.begin(); iter!=m_mapLights.end(); iter++){
		oclUint index=iter->first;
		oclFloat matrix[12];
		iter->second->GetWorldMatrix(matrix);

		Json::Value val=iter->second->GetJsonValue();
		for(size_t i=0; i<12; i++){
			val["tmValue"].append(matrix[i]);
		}
		allbuffer[to_string(index)]=val;
	}

#if __OCL_INTERNAL_DEBUG__
  oclStr str=allbuffer.toStyledString();
#endif

	return allbuffer;
}

oclBool oclLightManager::SetLightsProperty(oclStr buf,oclInt offset,oclStr packagePath){
	//TODO:JC:从jsonstring创建光源，转移到JsonConverter中去；
  Value value;
  Reader reader;
  reader.parse(buf,value);
  auto str=value.toStyledString();
  auto mem=value.getMemberNames();
  for(auto iter=mem.begin(); iter<mem.end(); iter++){
    oclFloat matrix[12];
    for(oclInt i=0; i<12; i++){
      matrix[i]=value[*iter]["tmValue"][i].asFloat();
    }
    m_mapLights[atoi((*iter).c_str())+offset]->SetWorldMatrix(matrix);
    value[*iter].removeMember("tmValue");
    m_mapLights[atoi((*iter).c_str())+offset]->InitWithJson(value[*iter]);
    Value tmp=value[*iter]["Property"];
    oclUint lightIndex=atoi((*iter).c_str());
    Value::Members mem=tmp.getMemberNames();
    for(auto iter1=mem.begin();iter1!=mem.end();iter1++){
      Value::Members mem2=tmp[*iter1].getMemberNames();
      for(auto iter2=mem2.begin();iter2!=mem2.end();iter2++){
        oclStr propName=*iter2;
        VARIANT value[3];
        oclFloat f;
        oclBool b;
        oclStr s;
        oclInt i;
        oclInt valueType=tmp[*iter1][*iter2]["valueType"].asInt();
        switch(valueType){
        case 0:// int
          f=tmp[*iter1][*iter2]["realValue"].asInt();
          value->vt=VT_INT;
          value->fltVal=f;
          break;
        case 1:// float
          f=tmp[*iter1][*iter2]["realValue"].asFloat();
          value->vt=VT_R4;
          value->fltVal=f;
          break;
        case 2:// bool
          b=tmp[*iter1][*iter2]["realValue"].asBool();
          value->vt=VT_BOOL;
          value->boolVal=b;
          break;
        case 4:// color/float3
          for(oclInt i=0;i<3;i++){
            f=tmp[*iter1][*iter2]["value"][i]["realValue"].asFloat();
            value[i].fltVal=f;
          }
          break;
        case 5:// string
          s=tmp[*iter1][*iter2]["realValue"].asString();
					boost::filesystem::path boostpath(s);
					oclStr url=packagePath+boostpath.filename().string();
          value->vt=VT_BSTR;
					value->bstrVal=_com_util::ConvertStringToBSTR(url.c_str());
          break;
        }
				oclLightSource* l=GetLight(lightIndex);
				l->SetProperty(propName.c_str(),value);
      }
    }
  }
  return true;
}

oclLightSource* oclLightManager::GetLight(oclUint index)const{
  auto iter=m_mapLights.find(index);
  return iter==m_mapLights.end()?nullptr:iter->second;
}

oclInt oclLightManager::GetBufferID_lights() const{
	return m_bufferLights->GetID();
}

oclInt oclLightManager::GetBufferID_materialBallLights() const{
	return m_bufferMaterialBallLights->GetID();
}

void oclLightManager::FetchTextures(std::vector<const oclTexture*>* out) const{
	for(auto iter=m_mapLights.begin(); iter!=m_mapLights.end(); ++iter){
		oclLightSource* l=iter->second;
		if(l->GetLightType()==LIGHT_TYPE::ENVIRONMENTPROBE){
			//JC:目前只有环境光源有纹理；
			oclLightSourceEnvironment* envl=static_cast<oclLightSourceEnvironment*>(l);
			envl->FetchTextures(out);
		}
	}
}

void oclLightManager::Init_(){
	m_bufferLights=oclBufferManager::S()->Create(GetUniqueBufferKey("LightsBuffer_"),RT_BUFFER_INPUT,RT_FORMAT_USER);
	m_bufferLights->SetElementSize(sizeof(SData_Light));
	m_bufferLights->SetSize(0);
	m_bufferMaterialBallLights=oclBufferManager::S()->Create(GetUniqueBufferKey("MaterialBallLightsBuffer_"),RT_BUFFER_INPUT,RT_FORMAT_USER);
	m_bufferMaterialBallLights->SetElementSize(sizeof(SData_Light));
	m_bufferMaterialBallLights->SetSize(1);


	/// 创建照亮材质球需要的环境光；
	m_materialBallLight=new oclLightSourceEnvironment(-1,m_ctx);
	m_materialBallLight->SetVisibility(true);
	VARIANT v;
	v.vt=VT_BSTR;
	v.bstrVal=_com_util::ConvertStringToBSTR("./texture/env/Default_materialball.hdr");
	m_materialBallLight->SetProperty("HDR_texture",&v);
	m_bufferMaterialBallLights->SetData(m_materialBallLight->GetLightData(),1);
}

OCL_NAMESPACE_END