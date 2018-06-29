#include "geometry\oclmesh.h"
#include "core/oclbuffer_manager.h"

#define M_MESH_PREFIX "oclMesh_"

OCL_NAMESPACE_START

oclMesh::oclMesh(oclInt ID)
	:oclGeometryBase(EGeometryTypes::GEOMETRY_TYPE_MESH,ID){}

oclMesh::~oclMesh(){
	printf("[DESTRUCT] -> oclMesh,m_sMeshName[%s]",GetName().c_str()); 
	oclBufferManager* mgr=oclBufferManager::S();
	mgr->Destroy(m_indexoclBuffer);
	mgr->Destroy(m_vertexoclBuffer);
	mgr->Destroy(m_normaloclBuffer);
	mgr->Destroy(m_tangentoclBuffer);
	mgr->Destroy(m_biTangentoclBuffer);
	mgr->Destroy(m_uvoclBuffer);
}

void oclMesh::CreateBuffer_(){
	oclBufferManager* mgr=oclBufferManager::S();
	m_vertexoclBuffer=mgr->Create(GetUniqueBufferKey(M_MESH_PREFIX),RT_BUFFER_INPUT,RT_FORMAT_FLOAT3);
	m_normaloclBuffer=mgr->Create(GetUniqueBufferKey(M_MESH_PREFIX),RT_BUFFER_INPUT,RT_FORMAT_FLOAT3);
	m_tangentoclBuffer=mgr->Create(GetUniqueBufferKey(M_MESH_PREFIX),RT_BUFFER_INPUT,RT_FORMAT_FLOAT3);
	m_biTangentoclBuffer=mgr->Create(GetUniqueBufferKey(M_MESH_PREFIX),RT_BUFFER_INPUT,RT_FORMAT_FLOAT3);
	m_uvoclBuffer=mgr->Create(GetUniqueBufferKey(M_MESH_PREFIX),RT_BUFFER_INPUT,RT_FORMAT_FLOAT2);
	m_indexoclBuffer=mgr->Create(GetUniqueBufferKey(M_MESH_PREFIX),RT_BUFFER_INPUT,RT_FORMAT_INT3);
	F_DBG_ASSERT(m_vertexoclBuffer);
	F_DBG_ASSERT(m_normaloclBuffer);
	F_DBG_ASSERT(m_tangentoclBuffer);
	F_DBG_ASSERT(m_biTangentoclBuffer);
	F_DBG_ASSERT(m_uvoclBuffer);
	F_DBG_ASSERT(m_indexoclBuffer);

}

void oclMesh::SetBiTangentData(oclInt nVexSize,oclFloat3* pInVex){
	if(pInVex==nullptr) return;
	m_nbiTangentCount=nVexSize;
	m_biTangentoclBuffer->SetSize(nVexSize);
	m_biTangentoclBuffer->SetData(pInVex,nVexSize);
}

void oclMesh::GetBiTangentData(oclFloat3* pOutVex) const{
	m_biTangentoclBuffer->GetData(pOutVex,m_nbiTangentCount);
}

void oclMesh::SetTangentData(oclInt nVexSize,oclFloat3* pInVex){
	if(pInVex==nullptr) return;
	m_ntangentCount=nVexSize;
	m_tangentoclBuffer->SetSize(nVexSize);
	m_tangentoclBuffer->SetData(pInVex,nVexSize);
}

void oclMesh::GetTangentData(oclFloat3* pOutVex) const{
	m_tangentoclBuffer->GetData(pOutVex,m_ntangentCount);
}

void oclMesh::SetIndexData(oclInt nVexSize,oclUint3* pInVex){
	m_nIndexCount=nVexSize;
	m_nTriangleCount=int(nVexSize/3);
	F_DBG_ASSERT(m_nTriangleCount*3==m_nIndexCount);
	m_indexoclBuffer->SetSize(m_nTriangleCount);
	m_indexoclBuffer->SetData(pInVex,m_nTriangleCount);
}

void oclMesh::GetIndexData(oclUint3* pOutVex)const{
	m_indexoclBuffer->GetData(pOutVex,m_nTriangleCount);
}

void oclMesh::FetchProxyInfo(SGeometryProxy * proxy) const{
	proxy->name=GetName();
	proxy->ID=GetID();
	proxy->type=GetType();

	proxy->biTanBufferID=m_biTangentoclBuffer->GetID();
	proxy->biTanBufferSize=m_nbiTangentCount;
	proxy->indexBufferID=m_indexoclBuffer->GetID();
	proxy->norBufferID=m_normaloclBuffer->GetID();
	proxy->tanBufferID=m_tangentoclBuffer->GetID();
	proxy->tanBufferSize=m_ntangentCount;
	proxy->uvBufferID=m_uvoclBuffer->GetID();
	proxy->vertexBufferID=m_vertexoclBuffer->GetID();
	proxy->primitiveCount=m_nTriangleCount;
}

void oclMesh::SetUVData(oclInt nVexSize,oclFloat2* pInVex){
	if(pInVex){
		m_nUVCount=nVexSize;
		m_uvoclBuffer->SetSize(nVexSize);
		m_uvoclBuffer->SetData(pInVex,nVexSize);
	}
}

void oclMesh::GetUVData(oclFloat2* pOutVex)const{
	m_uvoclBuffer->GetData(pOutVex,m_nUVCount);
}

void oclMesh::UpdateUVData(oclFloat2* pInVex){
	m_uvoclBuffer->SetData(pInVex,m_nUVCount);
}

void oclMesh::SetNormalData(oclInt nVexSize,oclFloat3* pInVex){
	m_nNormalCount=nVexSize;
	m_normaloclBuffer->SetSize(nVexSize);
	m_normaloclBuffer->SetData(pInVex,nVexSize);
}

void oclMesh::GetNormalData(oclFloat3* pOutVex)const{
	m_normaloclBuffer->GetData(pOutVex,m_nNormalCount);
}

void oclMesh::UpdateNormalData(oclFloat3* pInVex){
	m_normaloclBuffer->SetData(pInVex,m_nNormalCount);
}

void oclMesh::SetVertexData(oclInt iVertexCount,oclFloat3* pInVex){
	m_nVertextCount=iVertexCount;
	m_vertexoclBuffer->SetSize(iVertexCount);
	m_vertexoclBuffer->SetData(pInVex,iVertexCount);
}

void oclMesh::GetVertexData(oclFloat3* pOutVex)const{
	m_vertexoclBuffer->GetData(pOutVex,m_nVertextCount);
}

void oclMesh::GetAABB(oclFloat& Minx,oclFloat& Miny,oclFloat& Minz,
	oclFloat& Maxx,oclFloat& Maxy,oclFloat& Maxz)const{
	Minx=m_AABB.m_min.x;
	Miny=m_AABB.m_min.y;
	Minz=m_AABB.m_min.z;

	Maxx=m_AABB.m_max.x;
	Maxy=m_AABB.m_max.y;
	Maxz=m_AABB.m_max.z;
}

void oclMesh::SetAABB(oclFloat Minx,oclFloat Miny,oclFloat Minz,
	oclFloat Maxx,oclFloat Maxy,oclFloat Maxz){
	m_AABB.m_min.x=Minx;
	m_AABB.m_min.y=Miny;
	m_AABB.m_min.z=Minz;

	m_AABB.m_max.x=Maxx;
	m_AABB.m_max.y=Maxy;
	m_AABB.m_max.z=Maxz;
}

oclUint oclMesh::GetArea(std::vector<oclFloat>* area) const{
	oclUint n=0u;
	area->clear();
	oclInt3* dataP=m_indexoclBuffer->Map<oclInt3>();
	oclFloat3* pos=m_vertexoclBuffer->Map<oclFloat3>();
	for(oclUint i=0;i<m_nTriangleCount;++i){
		const oclInt3& index=dataP[i];
		const oclFloat3& pos1=pos[index.x];
		const oclFloat3& pos2=pos[index.y];
		const oclFloat3& pos3=pos[index.z];

		oclFloat A=optix::length(optix::cross((pos3-pos1),(pos2-pos1)))*.5f;
		area->push_back(A);
		++n;
	}
	m_indexoclBuffer->Unmap();
	m_vertexoclBuffer->Unmap();
	return n;
}

OCL_NAMESPACE_END
