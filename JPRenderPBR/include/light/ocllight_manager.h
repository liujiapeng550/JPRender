#pragma once

#include <comutil.h>
#include <map>
#include <optixu\optixu_matrix_namespace.h>
#include "json.h"
#include "core\oclcore.h"
#include "../../host_device/data_light.h"

namespace Json{
class Value;
};

OCL_NAMESPACE_START

class oclBuffer;
class oclTexture;
class oclLightSource;
class oclLightSourceEnvironment;

class oclLightManager final{
public:
  oclLightManager()=delete;
  oclLightManager(const oclLightManager&)=delete;
  oclLightManager& operator=(const oclLightManager&)=delete;
  oclLightManager(RTcontext ctx);
  ~oclLightManager();

  oclLightSource* CreateLight(oclUint id,const oclChar* lightType);

  oclStr GetLightJsonString(const std::vector<oclInt>& nodeIndexes)const;

  Json::Value GetLightsJson()const;

  /**
  *从选中灯光两个json数据中获取相同的属性
  * @param[in] fstV	 一个灯光属性json
  * @param[in] SecV	 一个灯光属性json
  */
  Json::Value GetSameBuffer(Json::Value fstV,Json::Value SecV)const;

  /**
  *将灯光信息传到GPU端，判断是否使用默认的HDR，当用户自己创建HDR灯光，默认灯光关闭，使用最新创建的HDR灯光
  */
  void Upload();

  /**
  *创建之前保存的灯光；
  * @param[in] buf	以保存灯光的属性数据
  * @param[in] offset 灯光数量
  * @param[in] packagePath 加载保存灯光的的路径
  */
  oclBool SetLightsProperty(oclStr buf,oclInt offset,oclStr packagePath);

  void DestroyLights();

  oclLightSource* GetLight(oclUint index)const;

	oclInt GetBufferID_lights()const;

	oclInt GetBufferID_materialBallLights()const;

  void FetchTextures(std::vector<const oclTexture*>*)const;

private:
  void Init_();

private:
  RTcontext m_ctx=0;
  std::map<oclUint,oclLightSource*> m_mapLights;
  oclBuffer* m_bufferLights=nullptr;
  oclBuffer* m_bufferMaterialBallLights=nullptr;
	oclLightSourceEnvironment* m_materialBallLight=nullptr;

};

OCL_NAMESPACE_END