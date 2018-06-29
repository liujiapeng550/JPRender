#include "core/oclinstance.h"
#include "core\oclbuffer_manager.h"

OCL_NAMESPACE_START

oclInstance::oclInstance(RTcontext ctx,RTmaterial m){
  OCL_TRY;
  OCL_CHECK_ERROR(rtGeometryCreate(ctx,&m_geometry));
  OCL_CHECK_ERROR(rtGeometryInstanceCreate(ctx,&m_geoInstance));
  OCL_CHECK_ERROR(rtGeometryInstanceSetGeometry(m_geoInstance,m_geometry));
	OCL_CHECK_ERROR(rtGeometryInstanceSetMaterialCount(m_geoInstance,1));
	OCL_CHECK_ERROR(rtGeometryInstanceSetMaterial(m_geoInstance,0,m));
  OCL_CHECK_ERROR(rtAccelerationCreate(ctx,&m_acceleration));
  OCL_CHECK_ERROR(rtAccelerationSetBuilder(m_acceleration,TRBVH));
  OCL_CHECK_ERROR(rtAccelerationSetTraverser(m_acceleration,TRBVH));
  OCL_CHECK_ERROR(rtGeometryGroupCreate(ctx,&m_geoGroup));
  OCL_CHECK_ERROR(rtGeometryGroupSetAcceleration(m_geoGroup,m_acceleration));
  OCL_CHECK_ERROR(rtGeometryGroupSetChildCount(m_geoGroup,1));
  OCL_CHECK_ERROR(rtGeometryGroupSetChild(m_geoGroup,0,m_geoInstance));
  OCL_CHECK_ERROR(rtTransformCreate(ctx,&m_transform));
  OCL_CHECK_ERROR(rtTransformSetChild(m_transform,m_geoGroup));
  OCL_CHECK_ERROR(rtGeometryInstanceDeclareVariable(m_geoInstance,F_STR(V_C_DATA_INSTANCE),&m_varInstanceData));
  OCL_CATCH;
	m_buferWorldMatrix=oclBufferManager::S()->Create(GetUniqueBufferKey("instance_"),RT_BUFFER_INPUT,RT_FORMAT_FLOAT);
	m_buferWorldMatrix->SetSize(16);
	m_instanceData.worldMatrixBufferID=m_buferWorldMatrix->GetID();
}

oclInstance::~oclInstance(){
	printf("[DESTRUCT] -> oclInstance\n");

  OCL_TRY;
  OCL_CHECK_ERROR(rtGeometryDestroy(m_geometry));
  OCL_CHECK_ERROR(rtGeometryInstanceDestroy(m_geoInstance));
  OCL_CHECK_ERROR(rtAccelerationDestroy(m_acceleration));
  OCL_CHECK_ERROR(rtGeometryGroupDestroy(m_geoGroup));
  OCL_CHECK_ERROR(rtTransformDestroy(m_transform));
  OCL_CATCH;
	oclBufferManager::S()->Destroy(m_buferWorldMatrix);
	m_buferWorldMatrix=nullptr;
}

void oclInstance::SetMaterialProxy(const SMaterialProxy & proxy){
	m_materialID=proxy.ID;
	m_instanceData.materialBufferID=proxy.bufferID;
	m_instanceData.materialBufferOffset=proxy.bufferOffset;
}

void oclInstance::SetGeometryProxy(const SGeometryProxy & proxy){
  OCL_TRY;
  OCL_CHECK_ERROR(rtGeometrySetPrimitiveCount(m_geometry,proxy.primitiveCount));
  OCL_CATCH;
	m_instanceData.vertexBufferID=proxy.vertexBufferID;
	m_instanceData.norBufferID=proxy.norBufferID;
	m_instanceData.tanBufferID=proxy.tanBufferID;
	m_instanceData.biTanBufferID=proxy.biTanBufferID;
	m_instanceData.uvBufferID=proxy.uvBufferID;
	m_instanceData.indexBufferID=proxy.indexBufferID;
	m_instanceData.tanBufferSize=proxy.tanBufferSize;
	m_instanceData.biTanBufferSize=proxy.biTanBufferSize;
}

void oclInstance::Upload(){
  OCL_TRY
  OCL_CHECK_ERROR(rtVariableSetUserData(m_varInstanceData,sizeof(SData_Instance),&m_instanceData));
  OCL_CATCH
}

void oclInstance::SetWorldMatrix(oclFloat * pMat){
  OCL_TRY;
	OCL_CHECK_ERROR(rtTransformSetMatrix(m_transform,0,pMat,0));
  OCL_CATCH;
	m_buferWorldMatrix->SetData(pMat,16);
}

void oclInstance::GetWorldMatrix(oclFloat * pOut) const{
	m_buferWorldMatrix->GetData(pOut,16);
}

void oclInstance::SetNodeIndex(oclInt nNode){
	m_instanceData.nodeIndex=nNode;
}

oclInt oclInstance::GetNodeIndex()const{
	return m_instanceData.nodeIndex;
}

void oclInstance::FetchProxy(SInstanceProxy * proxy) const{
	proxy->bufferID_bitanget=m_instanceData.biTanBufferID;
	proxy->bufferID_index=m_instanceData.indexBufferID;
	proxy->bufferID_materials=m_instanceData.materialBufferID;
	proxy->bufferID_normal=m_instanceData.norBufferID;
	proxy->bufferID_tangent=m_instanceData.tanBufferID;
	proxy->bufferID_uv=m_instanceData.uvBufferID;
	proxy->bufferID_vertex=m_instanceData.vertexBufferID;
	proxy->bufferID_worldMatrix=m_instanceData.worldMatrixBufferID;
	proxy->bufferOffset_materials=m_instanceData.materialBufferOffset;
	proxy->materialID=m_materialID;
	proxy->nodeIndex=m_instanceData.nodeIndex;
}

void oclInstance::SetAccelerationDirty(){
	OCL_TRY;
	OCL_CHECK_ERROR(rtAccelerationMarkDirty(m_acceleration));
	OCL_CATCH;
}

OCL_NAMESPACE_END
