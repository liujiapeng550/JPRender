#include "core/oclimage_loader.h"
#include "FreeImage.h"

OCL_NAMESPACE_START

static void ReadRGBFData(FIBITMAP* dib,oclInt m_uWidth,oclInt m_uHeight,oclFloat4* result){
  FIRGBF* bits=(FIRGBF*)FreeImage_GetBits(dib);
  for(oclInt y=0; y<m_uHeight; y++){
    for(oclInt x=0; x<m_uWidth; x++){
      oclUint index=(y*m_uWidth+x);
      result[index].x=bits[index].red;
      result[index].y=bits[index].green;
      result[index].z=bits[index].blue;
      result[index].w=1.0f;
    }
  }
}

static void ReadRGBAFData(FIBITMAP* dib,oclInt m_uWidth,oclInt m_uHeight,oclFloat4* result){
  FIRGBAF* bits=(FIRGBAF*)FreeImage_GetBits(dib);
  for(oclInt y=0; y<m_uHeight; y++){
    for(oclInt x=0; x<m_uWidth; x++){
      oclUint index=(y*m_uWidth+x);
      result[index].x=bits[index].red;
      result[index].y=bits[index].green;
      result[index].z=bits[index].blue;
      result[index].w=bits[index].alpha;
    }
  }
}

static void ReadR8G8B8Data(FIBITMAP* dib,oclInt m_uWidth,oclInt m_uHeight,oclFloat4* result){
  if(result==nullptr)return;
  RGBQUAD color;
  oclUint buf_index=0;
  const float F{1.0f/255.0f};
  for(oclInt j=0; j<m_uHeight; ++j){
    for(oclInt i=0; i<m_uWidth; ++i){
      FreeImage_GetPixelColor(dib,i,j,&color);
      buf_index=j*m_uWidth+i;
      result[buf_index].x=color.rgbRed*F;
      result[buf_index].y=color.rgbGreen*F;
      result[buf_index].z=color.rgbBlue*F;
      result[buf_index].w=1.0f;
    }
  }
}

static void ReadR8G8B8A8Data(FIBITMAP* dib,oclUint m_uWidth,oclUint m_uHeight,oclFloat4* result){
  if(result==nullptr)return;
  RGBQUAD color;
  oclUint buf_index=0;
  const float F{1.0f/255.0f};
  for(oclInt j=0; j<m_uHeight; ++j){
    for(oclInt i=0; i<m_uWidth; ++i){
      FreeImage_GetPixelColor(dib,i,j,&color);
      buf_index=j*m_uWidth+i;
      result[buf_index].x=color.rgbRed*F;
      result[buf_index].y=color.rgbGreen*F;
      result[buf_index].z=color.rgbBlue*F;
      result[buf_index].w=color.rgbReserved*F;
    }
  }
}

static void ReadR8Data(FIBITMAP* dib,oclUint m_uWidth,oclUint m_uHeight,oclFloat4* result){
  if(result==nullptr)return;
  oclUint buf_index=0;
  const float F{1.0f/255.0f};
  for(oclInt j=0; j<m_uHeight; ++j){
    BYTE *bits=FreeImage_GetScanLine(dib,j);
    for(oclInt i=0; i<m_uWidth; ++i){
      buf_index=j*m_uWidth+i;
      oclFloat value=(oclInt)bits[i]*F;
      result[buf_index].x=value;
      result[buf_index].y=value;
      result[buf_index].z=value;
      result[buf_index].w=value;
    }
  }
}

oclImageLoader::oclImageLoader(){}

oclImageLoader::~oclImageLoader(){
  if(m_bDown){
    delete[]m_data;
  }
}

oclBool oclImageLoader::Load(oclStr fileURL){
  if(m_bDown) delete[]m_data;
  m_bDown=false;

  FREE_IMAGE_FORMAT fif=FreeImage_GetFileType(fileURL.c_str());
  if(fif==FIF_UNKNOWN) fif=FreeImage_GetFIFFromFilename(fileURL.c_str());
  if(fif==FIF_UNKNOWN) return false;

  FIBITMAP * dib=FreeImage_Load(fif,fileURL.c_str(),0);
  if(dib==nullptr) return false;
  m_uWidth=FreeImage_GetWidth(dib);
  m_uHeight=FreeImage_GetHeight(dib);
  const oclUint size=m_uWidth*m_uHeight;
  oclBool failFlag{true};

  FREE_IMAGE_TYPE image_type=FreeImage_GetImageType(dib);
  switch(image_type){
  case FIT_UNKNOWN:
    return false;
  case FIT_BITMAP:
  {
    oclUint bitsPP=FreeImage_GetBPP(dib);
    oclUint bytesPP=bitsPP/8;
    switch(bytesPP){
    case 1:
      m_data=new oclFloat4[size];
      ReadR8Data(dib,m_uWidth,m_uHeight,m_data);
      failFlag=false;
      break;
    case 2:
      break;
    case 3:
    {
      m_data=new oclFloat4[size];
      ReadR8G8B8Data(dib,m_uWidth,m_uHeight,m_data);
      failFlag=false;
      break;
    }
    case 4:
    {
      m_data=new oclFloat4[size];
      ReadR8G8B8A8Data(dib,m_uWidth,m_uHeight,m_data);
      failFlag=false;
      break;
    }
    default:
    {
      FIBITMAP* bitmapTar32=FreeImage_ConvertTo32Bits(dib);
      FreeImage_DestroyICCProfile(dib);
      dib=bitmapTar32;
      m_data=new oclFloat4[size];
      ReadR8G8B8A8Data(dib,m_uWidth,m_uHeight,m_data);
      failFlag=false;
      break;
    }
    }
  }
  break;
  case FIT_UINT16:
    break;
  case FIT_INT16:
    break;
  case FIT_UINT32:
    break;
  case FIT_INT32:
    break;
  case FIT_FLOAT:
    break;
  case FIT_DOUBLE:
    break;
  case FIT_COMPLEX:
    break;
  case FIT_RGB16:
    break;
  case FIT_RGBA16:
    break;
  case FIT_RGBF:
    m_data=new oclFloat4[size];
    ReadRGBFData(dib,m_uWidth,m_uHeight,m_data);
    failFlag=false;
    break;
  case FIT_RGBAF:
    m_data=new oclFloat4[size];
    ReadRGBAFData(dib,m_uWidth,m_uHeight,m_data);
    failFlag=false;
    break;
  default:
    break;
  }
  FreeImage_DestroyICCProfile(dib);

  // check success;
  m_bDown=!failFlag;
  return m_bDown;
}

OCL_NAMESPACE_END
