#include "material/oclmaterial_common.h"
#include "core\JsonConverter.h"
#include "core\oclbuffer_manager.h"
#include "core\oclimage_loader.h"
#include <fstream>
#include <sstream>

using namespace std;

OCL_NAMESPACE_START 

static std::map<oclStr,Json::Value> s_jsonTypes;
static std::vector<oclBuffer*> m_vecCachedTextureBuffer;

const Json::Value & GetTypeJson(oclStr type){
	return s_jsonTypes[type];
}

oclStr GetTypeNames(){
  Json::Value v;
  oclInt pos=0;
  for(auto &i:s_jsonTypes){
    v[pos]=i.first;
    ++pos;
  }
  return v.toStyledString();
}

const void LoadMaterialConfigFile(const oclStr& URL){
	/// 1.加载材质配置文件；
	Json::Value v;
	Json::Reader r;
	ifstream file(URL);
	if(r.parse(file,v)){
		auto mem=v.getMemberNames();
		for(auto it=mem.begin(); it!=mem.end(); it++){
			s_jsonTypes[*it]=v[*it];
		}
	}

	/// 2. 预加载材质文件中配置的默认纹理数据；
	vector<oclStr> out;
	jsonconvertor::GetTypesTextureFileNames(s_jsonTypes,&out);
	for(oclStr& name:out){
		oclStr path="./texture/"+name;
		oclBuffer* buf=oclBufferManager::S()->Create(path,RT_BUFFER_INPUT,RT_FORMAT_FLOAT4);
		if(buf){
			oclImageLoader loader;
			if(loader.Load(path)){
				buf->SetSize(loader.GetWidth(),loader.GetHeight());
				buf->SetData(loader.GetImageData(),loader.GetWidth()*loader.GetHeight());
				m_vecCachedTextureBuffer.push_back(buf);
			} else{
				oclBufferManager::S()->Destroy(buf);
				buf=nullptr;
				F_DBG_ERROR("image读取失败，检查问题；");
				F_DBG_ASSERT(false);
			}
		}
	}
}

void DestroyDefaultTextures(){
  for(oclBuffer* buf:m_vecCachedTextureBuffer){
    oclBufferManager::S()->Destroy(buf);
  }
  m_vecCachedTextureBuffer.clear();
}

OCL_NAMESPACE_END
