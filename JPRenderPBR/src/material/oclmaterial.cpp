#include "material\oclmaterial.h"
#include <map>
#include "core\oclprogram_manager.h"
#include "core\ocltexture.h"
#include "core/JsonConverter.h"
#include "EncodingConversion.h"
#define BOOST_FILESYSTEM_NO_DEPRECATED
#include "boost\filesystem.hpp"


using namespace std;

OCL_NAMESPACE_START

/*
oclTexture* CreateNormalMap(const STextureDescription& param){
  //获取原始的高度贴图
  oclBuffer<oclStr>* sourceBuf=m_BufMgr->GetoclBuffer(param.fileURL);
  //注意写判断，增加函数逻辑健壮性，你写的是个函数，你逻辑里面就能保证fileURL对应的buffer一定存在？？
  if(sourceBuf){
    oclSize w,h;
    sourceBuf->GetSize(&w,&h);

    //高度贴图fileURL转变为法线帖图的名称
    //不会修改的变量加上const，增加逻辑安全性，部分情况会使编译器优化程序，将const变量直接融合进代码中；
    const oclStr normalMapName=param.fileURL+"_normalMap";
    oclBuffer<oclStr>* outBuf=m_BufMgr->CreateoclBuffer(normalMapName,RT_BUFFER_INPUT,2,RT_FORMAT_FLOAT4);
    //如果为空说明已经生成过一个normalMap
    if(outBuf==nullptr){
      //已存在的buffer通过GetoclBuffer函数，会使其引用计数加1；
      outBuf=m_BufMgr->GetoclBuffer(normalMapName);
    }
    outBuf->SetSize(w,h);
    Bump2Normal(w,h,static_cast<optix::float4*>(sourceBuf->Map()),static_cast<optix::float4*>(outBuf->Map()),2,2);
    outBuf->Unmap();
    sourceBuf->Unmap();

    oclTexture* tex=new oclTexture(m_ctx);
    tex->SetoclBuffer(outBuf);
    tex->Reset(param);
    m_Map_ID_oclTexture[tex->GetID()]=tex;

    // 记得销毁，使其引用计数减一；
    m_BufMgr->DestroyoclBuffer(sourceBuf);
    return tex;
  } else{
    throw "不存在fileURL对应的oclBuffer，请检查外部调用逻辑！";
    return nullptr;
  }
}
*/

static std::map<const oclStr,oclInt> s_mapMaterialNameToIndex={
  {MATERIAL_DEFAULT,MATERIAL_DEFAULT_INDEX},
  {MATERIAL_SHADOW,MATERIAL_SHADOW_INDEX},
  {MATERIAL_METAL,MATERIAL_METAL_INDEX},
  {MATERIAL_METAL_BRUSHED,MATERIAL_METAL_BRUSHED_INDEX},
  {MATERIAL_GLASS,MATERIAL_GLASS_INDEX},
  {MATERIAL_WOOD,MATERIAL_WOOD_INDEX},
  {MATERIAL_LEATHER,MATERIAL_LEATHER_INDEX},
  {MATERIAL_RUBBER,MATERIAL_RUBBER_INDEX},
  {MATERIAL_CARPAINT_UNIQUE_CLEAR,MATERIAL_CARPAINT_UNIQUE_CLEAR_INDEX},
  {MATERIAL_CARPAINT_UNIQUE,MATERIAL_CARPAINT_UNIQUE_INDEX},
  {MATERIAL_CARPAINT_METALLIC,MATERIAL_CARPAINT_METALLIC_INDEX},
  {MATERIAL_MATTE,MATERIAL_MATTE_INDEX},
  {MATERIAL_EMISSIVE,MATERIAL_EMISSIVE_INDEX},
  {MATERIAL_UV,MATERIAL_UV_INDEX},
};

oclMaterial::oclMaterial(RTcontext ctx,oclInt bufID,oclUint offset,oclID ID)
	:m_ctx(ctx),m_nBufferID(bufID),m_nID(ID),m_nBufferOffset(offset){
}

oclMaterial::~oclMaterial(){
	F_DBG_INFO("[DESTRUCT] -> oclMaterial");
	F_SAFE_DELETE(m_materialData);
	ClearTextures_();
  m_ctx=nullptr;
}

oclBool oclMaterial::SetProperty(const oclChar * propName,VARIANT * v,oclInt count){
  if(count==1&&(*v).vt==VT_BSTR){
    oclChar* url=_com_util::ConvertBSTRToString((*v).bstrVal);
    if(strcmp(url,"")==0) return false;
    oclStr strUrl=url;
    delete[] url;
		oclTexture* t=CreateTexture_(strUrl,propName);
		if(t){
			SetTextureSamplerID_(propName,t->GetTextureSamplerID());
			//v->bstrVal=_com_util::ConvertStringToBSTR(t->GetFileNewName().c_str());
    } else return false;
  }
	switch(count){
	case 1:
	jsonconvertor::SetMaterialProperty(propName,v[0],&m_JsonMaterial,m_materialData);
	break;
	case 2:
	jsonconvertor::SetMaterialProperty(propName,v[0],v[1],&m_JsonMaterial,m_materialData);
	break;
	case 3:
	jsonconvertor::SetMaterialProperty(propName,v[0],v[1],v[2],&m_JsonMaterial,m_materialData);
	break;
	case 4:
	default:
	break;
	}
	return true;
}

oclBool oclMaterial::GetProperty(const oclChar * propName,VARIANT * v,oclInt count)const{
	switch(count){
	case 1:
	jsonconvertor::GetMaterialProperty(propName,m_JsonMaterial,&v[0]);
	break;
	case 2:
	jsonconvertor::GetMaterialProperty(propName,m_JsonMaterial,&v[0],&v[1]);
	break;
	case 3:
	jsonconvertor::GetMaterialProperty(propName,m_JsonMaterial,&v[0],&v[1],&v[2]);
	break;
	case 4:
	default:
	break;
	}
	return true;
}

SMaterialDescription oclMaterial::GetDescription() const{
	SMaterialDescription d;
	d.definitionName=GetDefinitionName();
	d.group=m_sGroupName;
	d.ID=m_nID;
	d.jsonString=m_JsonMaterial.toStyledString();
	d.itemIndex=m_nItemIndex;
	d.libID=m_nLibID;
	d.texturePath=m_sTexturePath;
	d.type=m_sMaterialType;
   auto it=m_mapTexuresName.begin();
   while(it!=m_mapTexuresName.end()){
     d.textureMap.insert(make_pair(it->first,it->second));
     ++it;
   }
	return d;
}

oclBool oclMaterial::Reset(const SMaterialDescription& d){
	if(m_materialData){
		delete m_materialData;
	}
	m_materialData=new SData_Material();
	if(strcmp(d.type.c_str(),"")==0)return false;

	/// 1. 设置；
  m_sTexturePath=d.texturePath;
	m_sDefinitionName=d.definitionName;
	m_nItemIndex=d.itemIndex;
	m_nLibID=d.libID;
	m_sMaterialType=d.type;
	m_materialData->materialIndex=s_mapMaterialNameToIndex[m_sMaterialType];
  m_JsonMaterial.clear();
  Json::Reader r;
  r.parse(d.jsonString,m_JsonMaterial);

	/// 2. 清空曾经缓存的纹理；
	ClearTextures_();

	/// 3. 创建新纹理；
	std::map<oclStr,oclStr> mapNames;
	jsonconvertor::ConvertJsonToDataMaterial(m_JsonMaterial,m_materialData,&mapNames);
	for(auto& it :mapNames){
     boost::filesystem::path p(it.second);
		VARIANT v;
		v.vt=VT_BSTR;
		v.bstrVal=_com_util::ConvertStringToBSTR(oclStr(m_sTexturePath+p.filename().string()).c_str());
		SetProperty(it.first.c_str(),&v,1);
	}

	return true;
}

void oclMaterial::FetchProxy(SMaterialProxy* proxy) const{
	proxy->bufferID=m_nBufferID;
	proxy->bufferOffset=m_nBufferOffset;
	proxy->ID=m_nID;
}

void oclMaterial::FetchTextures(std::vector<const oclTexture*>* out)const{
	for(auto v:m_mapTextures){
		out->push_back(v.second);
	}
}

oclTexture* oclMaterial::CreateTexture_(const oclStr& fileURL,const oclStr& propName){
  oclTexture* oldTex=m_mapTextures[propName];
  if(oldTex)
    if(strcmp(oldTex->GetURL().c_str(),fileURL.c_str())==0){
			return oldTex;
    }

  STextureDescription param;
  param.fileURL=fileURL;
  param.format=RT_FORMAT_FLOAT4;
  param.type=RT_BUFFER_INPUT;
  param.wrapModeX=RT_WRAP_REPEAT;
  param.wrapModeY=RT_WRAP_REPEAT;
  param.wrapModeZ=RT_WRAP_REPEAT;
  param.filterModeX=RT_FILTER_LINEAR;
  param.filterModeY=RT_FILTER_LINEAR;
  param.filterModeZ=RT_FILTER_LINEAR;
  param.mipmapCount=1;

  oclTexture* newTex=new oclTexture(m_ctx);
  if(newTex->Reset(param)){
    if(oldTex) delete oldTex;
    m_mapTextures[propName]=newTex;
    m_mapTexuresName[newTex->GetURL()]=newTex->GetFileNewName();
    /*
    //TODO:
    if(strcmp(propName.c_str(),F_STR(M_MPROP_GE_BUMPTEXTUREPATH))==0){
      // 如果是几何贴图的话，需要继续生成法线贴图；
      newTex=CreateNormalMap(param);
      if(!newTex){
        delete newTex;
        return -1;
      }
      oclTexture* oldNormalTex=m_mapTextures[propName+NORMAL_SUFFIX];
      if(oldNormalTex) delete oldNormalTex;
      m_mapTextures[propName+NORMAL_SUFFIX]=newTex;
    }
    */

		return newTex;
  } else{
    delete newTex;
    return oldTex?oldTex:nullptr;
  }
}

void oclMaterial::ClearTextures_(){
  for(auto &ts:m_mapTextures){
    const oclTexture* img=static_cast<const oclTexture*>(ts.second);
    F_SAFE_DELETE(img);
  }
  m_mapTextures.clear();
}

void oclMaterial::SetTextureSamplerID_(const oclStr & propName,oclInt TSID){
  const oclChar* ppName=propName.c_str();
  if(strcmp(ppName,F_STR(M_MPROP_CC_ORANGEPEELTEXTUREPATH))==0){
    m_materialData->CC_texture.TSID=TSID;
  } else if(strcmp(ppName,F_STR(M_MPROP_BS_DIFFUSETEXTUREPATH))==0){
    m_materialData->BS_textureDiffuse.TSID=TSID;
  } else if(strcmp(ppName,F_STR(M_MPROP_BS_GLOSSYTEXTUREPATH))==0){
    m_materialData->BS_textureGlossy.TSID=TSID;
  } else if(strcmp(ppName,F_STR(M_MPROP_GE_BUMPTEXTUREPATH))==0){
    m_materialData->GE_texture.TSID=TSID;
  }
}

void oclMaterial::SetDefinitionName(const oclStr& name){
  if(name=="")return;
  m_sDefinitionName=name;
  m_JsonMaterial["matdefinitionName"]=name;
 }
OCL_NAMESPACE_END
