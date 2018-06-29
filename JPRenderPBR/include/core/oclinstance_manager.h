#pragma once

#include <vector>
#include <map>
#include "optixu\optixu_math_namespace.h"
#include "oclcore.h"
#include "../../host_device/data_instance.h"

OCL_NAMESPACE_START

class oclInstance;

class oclInstanceManager final{
public:
	oclInstanceManager(const oclInstanceManager&)=delete;
	oclInstanceManager& operator=(const oclInstanceManager&)=delete;
	oclInstanceManager()=delete;
	oclInstanceManager(RTcontext);
	~oclInstanceManager();

	void SetAccelerationDirty();

  void DestroyInstances();

  /**
   * hide/show a mesh,if that mesh exist,then hide/show and return true;
   * if mesh not exist return false;
   * @param[in] ID		mesh的索引
   * @param[in] visible   true or false
   */
	oclBool SetInstanceVisibility(oclID ID,oclBool visible);

  oclBool SetInstanceVisibility(oclInstance* ins,oclBool visible);

	void SetGroundVisible(oclBool visible);

	/**
	* 如果instance存在，返回nullptr，
	* 否则新建instance，并返回
	*/
	oclInstance* CreateInstance(oclID ID);

   inline oclInstance* GetInstance(oclID ID)const{
     auto it=m_mapInstances.find(ID);
     return it==m_mapInstances.end()?nullptr:it->second;
   };

  oclInstance* GetInstance_ground()const;

  oclInstance* GetInstance_materialBall()const;

	void DestoryInstance(oclID ID);

	std::vector<int> GetAllMaterialIDsUsedByInstance()const;

	RTgroup GetTopGroup_radiance()const{
		return m_topGroup;
	}

	RTgroup GetTopGroup_materialball()const{
		return m_topGroup_materialball;
	}

private:
	void Init_();

  oclBool SetInstanceVisibility_(oclInstance*,oclBool visible);

private:
	RTcontext m_ctx;
  RTgroup m_topGroup; RTacceleration m_accTopGroup;
  RTgroup m_dynamicGroup; RTacceleration m_accDynamicGroup;
  RTgroup m_topGroup_materialball; RTacceleration m_accTopGroup_materialball;
	std::map<oclInt,oclInstance*> m_mapInstances;
  oclInt m_nDynamicInstanceCount=0;
	oclInstance* m_pGround=nullptr;
	oclInstance* m_pMaterialBall=nullptr;

};
OCL_NAMESPACE_END
