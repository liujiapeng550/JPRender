#pragma once
#include "oclmaterial_common.h"

struct SData_Material;

OCL_NAMESPACE_START

class oclBuffer;
class oclTexture;
class oclMaterial;

class oclMaterialLibrary final{
public:
	oclMaterialLibrary(RTcontext ctx);
  ~oclMaterialLibrary();

	oclBool Init(const SMaterialLibraryDescription& param);

  /**
	*返回材质库中所有材质的id
	*/
  void GetMaterialIDs(std::vector<oclID>* v)const;

  oclMaterial* GetMaterial(oclID ID);

  const oclMaterial* GetMaterial(oclID ID)const;

  oclMaterial* GetMaterialByIndex(oclInt index);

  oclInt GetMaterialCount()const{ return m_mapMaterials.size(); };

	void DestroyMaterials();

	oclInt GetID()const{ return m_nLibID; }

	oclStr GetLibName()const{ return m_sName; }

	/**
	* 删除参数中指定的所有材质，参数中为材质ID
	*/
  oclBool DestroyMaterials(const std::vector<oclInt>& vecIds);

	//oclBool DestroyUnusedMaterials();

  /**
	* 载入一个常规模型后，由引擎调用该接口创建材质；如果模型需要n个材质，该接口会被调用n遍；
  */
	oclMaterial* CreateMaterial(SMaterialDescription desp);

  /**
  *delete material library config data which exist in tab page tabindex, item number itemindex.
  * this function will not delete current using material instance. They are different
  * material instance essentially.
  * return true if item exist.
  *delete material library config data which exist in tab page tabindex, item number itemindex.
  * @param[in]	 ID 该材质库中的材质ID（也是全局唯一ID）
  */
	oclBool DestroyMaterial(oclID ID);

	/**
	* 获取材质库的字符串描述，用来保存材质库，library也可以从引擎给出的材质库描述创建材质库
	*/
  SMaterialLibraryDescription GetDescription()const;

  void FetchTextures(std::vector<const oclTexture*>*)const;

	void Upload();

private:
  RTcontext m_ctx;
  std::map<oclID,oclMaterial*> m_mapMaterials;
  oclStr m_sName="no-library-name-assigned";
  oclStr m_sLibFilePath="";//材质库文件在硬盘上的地址，新建的材质库该值自然为""，加载的材质库就会记录文件URL；
	oclInt m_nLibID=-1;//材质库ID；
	oclBuffer* m_pBufMaterialData;//储存该材质库所有材质数据的缓冲区；
	oclUint m_nNextMaterialDataBufOffset=0u;//下一个材质数据在buffer中的偏移；
	oclInt m_nBufID=-1;
};

OCL_NAMESPACE_END

