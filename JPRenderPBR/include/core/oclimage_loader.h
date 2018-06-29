#pragma once

#include "core\oclcore.h"

OCL_NAMESPACE_START


class oclImageLoader{
public:
  oclImageLoader(const oclImageLoader&)=delete;
  oclImageLoader operator=(const oclImageLoader&)=delete;
  oclImageLoader();
  ~oclImageLoader();

  /**
   * 只有成功加载才返回true，
   * 可重复加载使用；
   */
  oclBool Load(oclStr fileURL);

  oclFloat4* GetImageData()const{
    return m_data;
  }

  oclUint GetWidth()const{
    return m_uWidth;
  }

  oclUint GetHeight()const{
    return m_uHeight;
  }


private:
  oclFloat4* m_data;
  oclUint m_uWidth,m_uHeight;
  oclBool m_bDown=false;
};

OCL_NAMESPACE_END
