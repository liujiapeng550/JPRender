#pragma once

#include "oclmaterial_common.h"

OCL_NAMESPACE_START

class oclBuffer;
class oclMaterial;
class oclMaterialLibrary;

class oclMaterialManager final{

public:
  oclMaterialManager()=delete;
  oclMaterialManager(const oclMaterialManager&)=delete;
	oclMaterialManager(RTcontext ctx);
  ~oclMaterialManager();

	/**
	* 材质库创建失败返回nullptr
	*/
	oclMaterialLibrary* CreateLibrary(SMaterialLibraryDescription desp);

	/**
	* 成功销毁，返回true，否则false
	*/
	oclBool DestroyLibrary(oclMaterialLibrary* lib);

	/**
	* 获取参数关联的材质库，不存在的话返回nullptr，此时你可能需要调用CreateLibrary创建材质库
	*/
	oclMaterialLibrary* GetLibrary(oclInt libIndex);

	const oclMaterialLibrary* GetLibrary(oclInt libIndex)const;

	/**
	* 遍历所有的材质库，查找材质ID对应的材质；
	* 参数返回找到的材质所在的材质库；
	*/
	oclMaterial* GetMaterial(oclID ID,oclMaterialLibrary** lib);

  /**
  *由材质ID获取其在材质库中的索引
  */
	oclInt GetLibraryIndex(oclID ID)const;

  /**
  *
  * @param[in] definitionName       所创建材质自定义的名称
  * @param[in] matindex             场景中的材质和材质库中的材质在m_mapMaterials中的索引
  * @param[in] type                 创建材质的类型Default Metal 等
  * @param[in] isAddMatToScene      创建的材质是否添加到场景中
  * @param[in] jsonString           当第五个默认参数str=""时用于创建默认材质，不为空时用于创建已经保存过的材质，str里为保存的材质属性
  * @param[in] path                 已保存的材质库路径
  */
  //oclMaterial* CreateMaterialAndExchangeMaterialIndex(oclStr definitionName,oclInt matindex,oclStr type,oclBool isAddMatToScene=true,oclStr jsonString="",oclStr path="./texture/");

private:
  /**
  * create necessary oclbuffers;
  * load disk material type file(*.dat).
  * If exist and load successfully return true;
  * else return false;
  */
  oclBool Init_();

  /**
  *创建保存材质环境贴图路径
  *@param[in] topath               要保存的贴图路径
  *@param[in] oldEnvPath           贴图原始路径
  */
  oclBool SaveMaterialEnvTexPath(oclStr topath,oclStr oldEnvPath);

private:
  RTcontext m_ctx;
  std::map<oclInt,oclMaterialLibrary*> m_mapLibs; 
};

OCL_NAMESPACE_END
