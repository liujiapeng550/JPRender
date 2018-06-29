#pragma once

#include <map>
#include "oclcore.h"

OCL_NAMESPACE_START

class oclBufferManager;

oclStr GetUniqueBufferKey(oclStr prefix);

class oclBuffer final{
public:
  const oclStr key;

  void SetSize(oclInt d1);
  void SetSize(oclInt d1,oclInt d2);
  void SetSize(oclInt d1,oclInt d2,oclInt d3);
  void GetSize(oclInt* d1)const;
  void GetSize(oclInt* d1,oclInt* d2)const;
  void GetSize(oclInt* d1,oclInt* d2,oclInt* d3)const;
  void SetElementSize(oclInt sz);
  oclInt GetID()const;
  oclInt GetElementSize()const;

  template<typename T>
  void SetData(const T* data,oclUint elemCount,oclUint offsetOfTypeT=0u,oclUint level=0u){
    if(elemCount==0u)return;
    OCL_TRY;
    T* dataStream;
    OCL_CHECK_ERROR(rtBufferMapEx(m_buffer,RT_BUFFER_MAP_WRITE,level,0,(void**)(&dataStream)));
    dataStream+=offsetOfTypeT;

#if __OCL_INTERNAL_DEBUG__
    oclUint n=elemCount*sizeof(T);
#endif

    memcpy(dataStream,data,elemCount*sizeof(T));
    OCL_CHECK_ERROR(rtBufferUnmapEx(m_buffer,level));
    OCL_CATCH;
  }

  template<typename T>
  void GetData(T* data,oclUint elemCount,oclUint offsetOfTypeT=0u,oclUint level=0u){
    if(elemCount==0u)return;
    OCL_TRY;
    T* dataStream;
    OCL_CHECK_ERROR(rtBufferMapEx(m_buffer,RT_BUFFER_MAP_READ,level,0,(void**)(&dataStream)));
    dataStream+=offsetOfTypeT;

#if __OCL_INTERNAL_DEBUG__
    oclUint n=elemCount*sizeof(T);
#endif

    memcpy(data,dataStream,elemCount*sizeof(T));
    OCL_CHECK_ERROR(rtBufferUnmapEx(m_buffer,level));
    OCL_CATCH;
  }
  void Unmap();
  template<typename T,RTbuffermapflag flag=RT_BUFFER_MAP_READ_WRITE>
  T* Map(){
    void* dataStream;
    OCL_TRY;
    OCL_CHECK_ERROR(rtBufferMapEx(m_buffer,flag,0u,0,&dataStream));
    OCL_CATCH;
    return static_cast<T*>(dataStream);
  }

private:
  friend oclBufferManager;

  oclBuffer()=delete;
  oclBuffer(const oclBuffer&)=delete;
  oclBuffer& operator=(const oclBuffer&)=delete;
  oclBuffer(RTcontext ctx,RTbuffertype type,oclStr key);
  ~oclBuffer();

private:
  RTbuffer m_buffer;
  oclInt m_d1,m_d2,m_d3;
  oclInt m_refCount;

};

class oclBufferManager final{

private:
  static oclBufferManager* ins;
public:
  static oclBufferManager* S();

  static void SetoclBufferToRTtexturesampler(oclBuffer*,RTtexturesampler TS);
  static void SetoclBufferToRTvariable(oclBuffer*,RTvariable var);

public:
  void Init(RTcontext);
  void Destroy();

  /**
   * 成功删除才返回true
   */
  oclBool Destroy(const oclStr& key);
  oclBool Destroy(oclBuffer* buf);

  /**
   * 如果key已经存在，就返回nullptr;
   * 如果不存在，创建oclBuffer，并返回；
   *
   * @param[in] key            键值
   * @param[in] type           枚举值
   * @param[in] RTformat       枚举类型，表示每个数据元素的类型
   */
  oclBuffer* Create(oclStr&& key,RTbuffertype type,RTformat format);
  oclBuffer* Create(oclStr& key,RTbuffertype type,RTformat format);

  /**
   * 查找是否存在该key的oclBuffer，
   * 若存在便返回
   * 如果不存在，则返回nullptr.
   * NOTE: this method will not create a new oclBuffer instance.
   *
   * @param[in] key      oclBufferID
   */
  oclBuffer* Get(const oclStr& key);
  oclBool Exist(const oclStr& key)const;

private:
  oclBufferManager(const oclBufferManager&)=delete;
  oclBufferManager& operator=(const oclBufferManager&)=delete;
  oclBufferManager();
  ~oclBufferManager();

private:
  RTcontext m_ctx=0;
  std::map<oclStr,oclBuffer*> m_map_oclStr_oclBuffer;

  /**
   * 查询m_map_ID_oclBuffer中是否有Key值为key的oclBuffer对象，若有则返回；若无则返回nullptr
   * @param[in] ID 待查找的oclBuffer的key
   */
  inline oclBuffer* Find_(const oclStr& key)const;
};


OCL_NAMESPACE_END
