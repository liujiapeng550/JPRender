#include "geometry\oclgeometry_base.h"
#include "geometry\oclmesh.h"
#include "geometry\oclmath_plane.h"
#include "geometry\oclmath_sphere.h"

OCL_NAMESPACE_START

oclGeometryManager::oclGeometryManager(){
	F_DBG_INFO("[CONSTRUCT] -> oclGeometryManager");
	Init_();
}

oclGeometryManager::~oclGeometryManager(){
	DestroyGeometries();
	F_SAFE_DELETE(m_pMaterialBall);
	F_SAFE_DELETE(m_pGroundPlane);
}

oclMesh* oclGeometryManager::CopyMesh(oclInt ID){
	/*
	TODO: 交由自己实现拷贝，使用工厂方法
	oclMesh* o=GetGeometry<oclMesh>(ID);
	if(o){
		oclMesh* m=new oclMesh(o->GetType(),m_nNextAvaliableGeometryID);
		m_mapGeometries[m_nNextAvaliableGeometryID]=m;
		++m_nNextAvaliableGeometryID;
		m->m_sMeshName=o->m_sMeshName+"_copied_"+std::to_string(m->m_nIndex);
		m->m_AABB=o->m_AABB;
		m->m_nTriangleCount=o->m_nTriangleCount;
		m->m_nVertextCount=o->m_nVertextCount;
		m->m_nNormalCount=o->m_nNormalCount;
		m->m_nUVCount=o->m_nUVCount;
		m->m_nIndexCount=o->m_nIndexCount;
		m->m_ntangentCount=o->m_ntangentCount;

		/// mesh拷贝的话，肯定是共享缓冲区数据；
		/// 这里需要注意的是缓冲区指针的获取方式是通过oclBufferManager，而不是直接拷贝指针；
		oclBufferManager* mgr=oclBufferManager::S();
		m->m_indexoclBuffer=mgr->Get(o->m_indexoclBuffer->key);
		m->m_vertexoclBuffer=mgr->Get(o->m_vertexoclBuffer->key);
		m->m_normaloclBuffer=mgr->Get(o->m_normaloclBuffer->key);
		m->m_tangentoclBuffer=mgr->Get(o->m_tangentoclBuffer->key);
		m->m_biTangentoclBuffer=mgr->Get(o->m_biTangentoclBuffer->key);
		m->m_uvoclBuffer=mgr->Get(o->m_uvoclBuffer->key);
		F_DBG_ASSERT(m->m_indexoclBuffer);
		F_DBG_ASSERT(m->m_vertexoclBuffer);
		F_DBG_ASSERT(m->m_normaloclBuffer);
		F_DBG_ASSERT(m->m_tangentoclBuffer);
		F_DBG_ASSERT(m->m_biTangentoclBuffer);
		F_DBG_ASSERT(m->m_uvoclBuffer);

		return m;
	}
	*/
	return nullptr;
}

oclGeometryBase* oclGeometryManager::CreateGeometry(const SGeometryCreateInfo& info){
	oclGeometryBase* g=nullptr;
	switch(info.type){
	case EGeometryTypes::GEOMETRY_TYPE_MATH_PLANE:
		g=new oclMathPlane(info.ID);
	break;
	case EGeometryTypes::GEOMETRY_TYPE_MATH_SPHERE:
		g=new oclMathSphere(info.ID);
	break;
	case EGeometryTypes::GEOMETRY_TYPE_MESH:
		g=new oclMesh(info.ID);
		static_cast<oclMesh*>(g)->CreateBuffer_();
	break;
	default:
	return nullptr;
	}
	g->SetName(info.name);
	m_mapGeometries[info.ID]=g;
	return g;
}

oclGeometryBase* oclGeometryManager::GetGeometry(oclInt ID){
	auto it=m_mapGeometries.find(ID);
	if(it==m_mapGeometries.end())return nullptr;
	else{
		return it->second;
	}
}

const oclGeometryBase * oclGeometryManager::GetGeometry(oclInt ID) const{
	auto it=m_mapGeometries.find(ID);
	if(it==m_mapGeometries.end())return nullptr;
	else{
		return it->second;
	}
}

oclBool oclGeometryManager::DestroyGeometry(oclInt ID)noexcept{
	oclGeometryBase* g=GetGeometry(ID);
	if(g){
		m_mapGeometries.erase(ID);
		delete g;
		return true;
	}
	return false;
}

void oclGeometryManager::DestroyGeometries(){
	for(auto &it:m_mapGeometries){
		delete it.second;
	}
	m_mapGeometries.clear();
}

void oclGeometryManager::Init_(){
	m_pMaterialBall=new oclMathSphere(-100);
	m_pMaterialBall->SetName("material-ball");
	m_pGroundPlane=new oclMathPlane(-101);
	m_pGroundPlane->SetName("ground-plane");
}

OCL_NAMESPACE_END
