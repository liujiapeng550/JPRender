#pragma once

#include "OAIDL.h"
#include <comutil.h>
#include <optixu\optixu_math_namespace.h>
#include "oclmaterial_common.h"
#include "../../host_device/data_instance.h"

OCL_NAMESPACE_START

class oclTexture;

class oclMaterial final{
public:
	oclMaterial(RTcontext ctx,oclInt bufID,oclUint bufOffset,oclID ID);
  ~oclMaterial();

  inline void SetMaterialInUse(oclBool value){ m_bIsMaterialInUse=value; };

  inline void SetGroupName(oclStr name){ m_sGroupName=name; }

  void SetDefinitionName(const oclStr& name);

	inline oclInt GetBufferOffset()const{ return m_nBufferOffset; }

	inline const SData_Material& GetMaterialData()const{ return *m_materialData; }

  /**
  *修改材质属性值
  *@param[in] propName        材质属性名
  *@param[in] v               修改后的属性值
  *@param[in] count           属性个数
  */
  oclBool SetProperty(const oclChar* propName,VARIANT* v,oclInt count);

	inline oclID GetID()const{ return m_nID; }
	
	inline oclInt GetIndex()const{ return m_nItemIndex; }

	inline oclInt GetLibID()const{ return m_nLibID; }

	inline oclStr GetDefinitionName()const{ return m_sDefinitionName; }

  inline oclStr GetGroupName()const{ return m_sGroupName; }

  inline oclStr GetType()const{ return m_sMaterialType; }

  inline oclBool GetMaterialInUse()const{ return m_bIsMaterialInUse; };

  oclBool GetProperty(const oclChar* propName,VARIANT* v,oclInt count)const;

	SMaterialDescription GetDescription()const;

	/**
	* 清空旧纹理，重置材质属性；
	*/
	oclBool Reset(const SMaterialDescription& d);

	void FetchProxy(SMaterialProxy* proxy)const;

	/**
	* 获取该材质所有的纹理，用于保存纹理使用
	*/
  void FetchTextures(std::vector<const oclTexture*>*)const;

private:
  oclMaterial()=delete;
  oclMaterial(const oclMaterial&)=delete;
  oclMaterial& operator=(const oclMaterial&)=delete;

  oclTexture* CreateTexture_(const oclStr& fileURL,const oclStr& propName);

  void ClearTextures_();

  void SetTextureSamplerID_(const oclStr& propName,oclInt TSID);

private:
	const oclInt m_nBufferID=-1;//该值是材质数据所在RTbuffer的ID；
	const oclInt m_nBufferOffset=-1;//该值是材质数据在RTbuffer中的偏移，是提交给OptiX使用的变量；
	const oclID m_nID;//材质的全局唯一ID；

  oclStr m_sMaterialType;
  oclStr m_sTexturePath="./texture/";
  oclStr m_sGroupName="no-group-name-assigned";
	oclStr m_sDefinitionName="";
	oclInt m_nItemIndex;//材质在材质库中的索引；
	oclInt m_nLibID;//材质所在材质库的编号；
  oclBool m_bIsMaterialInUse=false;
  SData_Material* m_materialData=nullptr;
  RTcontext m_ctx=0;//optix渲染上下文，这里引用的唯一目的是创建RTtexture时使用；
  Json::Value m_JsonMaterial;//提交引擎，用于渲染GUI使用；
  std::map<oclStr,oclTexture*> m_mapTextures;
  std::map<oclStr,oclStr> m_mapTexuresName;//第一个元素为原始纹理名称，第二个为修改后得纹理名称
};

OCL_NAMESPACE_END
