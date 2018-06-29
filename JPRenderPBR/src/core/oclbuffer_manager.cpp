#include "core\oclbuffer_manager.h"
#include <optix_host.h>
#include "core\ocltexture.h"

OCL_NAMESPACE_START

oclStr GetUniqueBufferKey(oclStr prefix){
  static oclUint ID=0;
  return prefix+std::to_string(++ID);
}

#pragma region buffer
oclBuffer::oclBuffer(RTcontext ctx,RTbuffertype type,oclStr k):key(k){
  OCL_TRY;
  OCL_CHECK_ERROR(rtBufferCreate(ctx,type,&m_buffer));
  OCL_CATCH;
};
oclBuffer::~oclBuffer(){
  OCL_TRY;
  printf("[DESTRUCT] -> oclBuffer,key[%s]\n",key.c_str());
  OCL_CHECK_ERROR(rtBufferDestroy(m_buffer));
  OCL_CATCH;
};
void oclBuffer::SetSize(oclInt d1){
  if(d1==m_d1)return;
  OCL_TRY;
  OCL_CHECK_ERROR(rtBufferSetSize1D(m_buffer,d1));
  OCL_CATCH;
  m_d1=d1;
}
void oclBuffer::SetSize(oclInt d1,oclInt d2){
  if(d1==m_d1 && d2==m_d2)return;
  OCL_TRY;
  OCL_CHECK_ERROR(rtBufferSetSize2D(m_buffer,d1,d2));
  OCL_CATCH;
  m_d1=d1;
  m_d2=d2;
}
void oclBuffer::SetSize(oclInt d1,oclInt d2,oclInt d3){
  if(d1==m_d1 && d2==m_d2 && d3==m_d3)return;
  OCL_TRY;
  OCL_CHECK_ERROR(rtBufferSetSize3D(m_buffer,d1,d2,d3));
  OCL_CATCH;
  m_d1=d1;
  m_d2=d2;
  m_d3=d3;
}

void oclBuffer::GetSize(oclInt* d1)const{
  *d1=m_d1;
}
void oclBuffer::GetSize(oclInt* d1,oclInt* d2)const{
  *d1=m_d1;
  *d2=m_d2;
}
void oclBuffer::GetSize(oclInt* d1,oclInt* d2,oclInt* d3)const{
  *d1=m_d1;
  *d2=m_d2;
  *d3=m_d3;
}
oclInt oclBuffer::GetID()const{
  oclInt result;
  OCL_TRY;
  OCL_CHECK_ERROR(rtBufferGetId(m_buffer,&result));
  OCL_CATCH;
  return result;
}
oclInt oclBuffer::GetElementSize()const{
  oclSize sz;
  OCL_TRY;
  OCL_CHECK_ERROR(rtBufferGetElementSize(m_buffer,&sz));
  OCL_CATCH;
  return oclInt(sz);
}
void oclBuffer::SetElementSize(oclInt sz){
  OCL_TRY;
  OCL_CHECK_ERROR(rtBufferSetElementSize(m_buffer,oclSize(sz)));
  OCL_CATCH;
}
void oclBuffer::Unmap(){
  OCL_TRY;
  OCL_CHECK_ERROR(rtBufferUnmapEx(m_buffer,0u));
  OCL_CATCH;
}

#pragma endregion

#pragma region buffermanager

void oclBufferManager::Init(RTcontext ctx){
  m_ctx=ctx;
}

oclBufferManager::oclBufferManager(){}
oclBufferManager::~oclBufferManager(){
  Destroy();
  m_ctx=nullptr;
}

void oclBufferManager::Destroy(){
  std::map<oclStr,oclBuffer*>::iterator it=m_map_oclStr_oclBuffer.begin();
  for(; it!=m_map_oclStr_oclBuffer.end(); ++it){
    delete it->second;
  }
  m_map_oclStr_oclBuffer.clear();
}

oclBool oclBufferManager::Destroy(const oclStr& key){
  oclBuffer* tmp=Find_(key);
  return Destroy(tmp);
}

oclBool oclBufferManager::Destroy(oclBuffer* buf){
  if(buf){
    --buf->m_refCount;
    if(buf->m_refCount<=0){
      m_map_oclStr_oclBuffer.erase(buf->key);
      delete buf;
      return true;
    }
  }
  return false;
}

oclBuffer* oclBufferManager::Create(oclStr&& key,RTbuffertype type,RTformat format){
  oclStr tmp=key;
  return Create(tmp,type,format);
}
oclBuffer* oclBufferManager::Create(oclStr& key,RTbuffertype type,RTformat format){
  if(Exist(key)){
    return nullptr;
  } else{
    oclBuffer* tmp=new oclBuffer(m_ctx,type,key);
		tmp->m_d1=tmp->m_d2=tmp->m_d3=-1;
    tmp->m_refCount=1;
    OCL_TRY;
    OCL_CHECK_ERROR(rtBufferSetFormat(tmp->m_buffer,format));
    OCL_CATCH;
    m_map_oclStr_oclBuffer[key]=tmp;
    return tmp;
  }
}

oclBuffer* oclBufferManager::Get(const oclStr& key){
  oclBuffer* tmp=Find_(key);
  if(tmp){
    ++tmp->m_refCount;
  }
  return tmp;
}

oclBool oclBufferManager::Exist(const oclStr& key)const{
  auto it=m_map_oclStr_oclBuffer.find(key);
  return it==m_map_oclStr_oclBuffer.end()?false:true;
}

oclBufferManager* oclBufferManager::ins=nullptr;
oclBufferManager* oclBufferManager::S(){
  if(ins==nullptr)ins=new oclBufferManager();
  return ins;
}

void oclBufferManager::SetoclBufferToRTtexturesampler(oclBuffer * oclBuf,RTtexturesampler TS){
  OCL_TRY;
  OCL_CHECK_ERROR(rtTextureSamplerSetBuffer(TS,0u,0u,oclBuf->m_buffer));
  OCL_CATCH;
}

void oclBufferManager::SetoclBufferToRTvariable(oclBuffer * oclBuf,RTvariable var){
  OCL_TRY;
  OCL_CHECK_ERROR(rtVariableSetObject(var,oclBuf->m_buffer));
  OCL_CATCH;
}

inline oclBuffer* oclBufferManager::Find_(const oclStr& key)const{
  auto it=m_map_oclStr_oclBuffer.find(key);
  return it==m_map_oclStr_oclBuffer.end()?nullptr:it->second;
}
#pragma endregion

OCL_NAMESPACE_END
