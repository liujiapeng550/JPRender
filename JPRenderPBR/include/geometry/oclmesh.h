#pragma once

#include "geometry\oclgeometry_base.h"

OCL_NAMESPACE_START

class oclBuffer;

class oclMesh :public oclGeometryBase{
public:
	void SetVertexData(oclInt nVertexCount,oclFloat3* pInVex);
	void SetNormalData(oclInt nVexSize,oclFloat3* pInVex);
	void SetTangentData(oclInt nVexSize,oclFloat3* pInVex);
	void SetBiTangentData(oclInt nVexSize,oclFloat3* pInVex);
	void SetUVData(oclInt nVexSize,oclFloat2* pInVex);
	void SetIndexData(oclInt nVexSize,oclUint3* pInVex);
	void SetAABB(oclFloat Minx,oclFloat Miny,oclFloat Minz,
		oclFloat Maxx,oclFloat Maxy,oclFloat Maxz);
	void UpdateNormalData(oclFloat3* pInVex);
	void UpdateUVData(oclFloat2* pInVex);

	void GetVertexData(oclFloat3* pOutVex)const;
	void GetNormalData(oclFloat3* pOutVex)const;
	void GetTangentData(oclFloat3* pOutVex)const;
	void GetBiTangentData(oclFloat3* pOutVex)const;
	void GetUVData(oclFloat2* pOutVex)const;
	void GetIndexData(oclUint3* pOutVex)const;
	oclInt GetVertexCount()const{ return m_nVertextCount; }
	oclInt GetNormalCount()const{ return m_nNormalCount; }
	oclInt GetUVCount()const{ return m_nUVCount; }
	oclInt GetIndexCount()const{ return m_nIndexCount; }
	oclInt GetTriangleCount()const{ return m_nTriangleCount; }
	void GetAABB(oclFloat& Minx,oclFloat& Miny,oclFloat& Minz,
		oclFloat& Maxx,oclFloat& Maxy,oclFloat& Maxz)const;

	/**
	 * 依次计算引擎传来的网格三角面的面积（顺序就是顶点索引的顺序），然后将面积保存在参数中，并返回vector长度。
	 */
	oclUint GetArea(std::vector<oclFloat>* area)const;

	void FetchProxyInfo(SGeometryProxy* proxy)const override;

private:
	friend oclGeometryManager;

	oclMesh& operator=(const oclMesh&)=delete;
	oclMesh(const oclMesh&)=delete;
	oclMesh()=delete;
	oclMesh(oclInt index);
	~oclMesh();

	void CreateBuffer_();

private:
	oclBuffer* m_vertexoclBuffer=nullptr;
	oclBuffer* m_normaloclBuffer=nullptr;
	oclBuffer* m_tangentoclBuffer=nullptr;
	oclBuffer* m_biTangentoclBuffer=nullptr;
	oclBuffer* m_uvoclBuffer=nullptr;
	oclBuffer* m_indexoclBuffer=nullptr;
	optix::Aabb m_AABB;

	oclInt m_nTriangleCount=0;
	oclInt m_nVertextCount=0;
	oclInt m_nNormalCount=0;
	oclInt m_ntangentCount=0;
	oclInt m_nbiTangentCount=0;
	oclInt m_nUVCount=0;
	oclInt m_nIndexCount=0;
};

OCL_NAMESPACE_END
