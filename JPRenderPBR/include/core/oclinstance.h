#pragma once

#include "optixu\optixu_math_namespace.h"
#include "oclcore.h"
#include "../../host_device/data_instance.h"

OCL_NAMESPACE_START

class oclInstanceManager;
class oclBuffer;

class oclInstance final{
public:
	void SetMaterialProxy(const SMaterialProxy& proxy);

	void SetGeometryProxy(const SGeometryProxy& proxy);

	void Upload();

	oclID GetMaterialID()const{ return m_materialID; }

	void SetWorldMatrix(oclFloat* pMat);

	void GetWorldMatrix(oclFloat* pOut)const;

	/**
	 * 设置节点索引（用于拾取）
	 */
	void SetNodeIndex(oclInt nNode);

	oclInt GetNodeIndex()const;

	void FetchProxy(SInstanceProxy* proxy)const;

	void SetAccelerationDirty();

private:
	oclInstance(const oclInstance&)=delete;
	oclInstance& operator=(const oclInstance&)=delete;
	oclInstance()=delete;
	oclInstance(RTcontext,RTmaterial);
	~oclInstance();

private:
	friend oclInstanceManager;

  RTtransform m_transform;
  RTgeometrygroup m_geoGroup; RTacceleration m_acceleration;
  RTgeometryinstance m_geoInstance; RTvariable m_varInstanceData;
  RTgeometry m_geometry;
	SData_Instance m_instanceData;
  oclBool m_bVisible=false;
	oclInt m_materialID;
	oclBuffer* m_buferWorldMatrix;//模型空间转世界空间的矩阵；
};

OCL_NAMESPACE_END
