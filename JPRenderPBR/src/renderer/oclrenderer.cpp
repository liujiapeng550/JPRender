#include "renderer/oclrenderer.h"
#include "core/oclbuffer_manager.h"
#include "core\ocltexture.h"

OCL_NAMESPACE_START

oclRenderer::oclRenderer(RTcontext ctx)
  :m_ctx(ctx){
	F_DBG_INFO("[CONSTRUCT] -> oclRenderer");
	OCL_TRY;
	OCL_CHECK_ERROR(rtContextQueryVariable(m_ctx,F_STR(V_C_DATA_RENDERER),&m_varRenderer));
	if(!m_varRenderer) OCL_CHECK_ERROR(rtContextDeclareVariable(m_ctx,F_STR(V_C_DATA_RENDERER),&m_varRenderer));
	OCL_CHECK_ERROR(rtContextQueryVariable(m_ctx,F_STR(V_C_TOP_OBJECT),&m_varTopGroup));
	if(!m_varTopGroup) OCL_CHECK_ERROR(rtContextDeclareVariable(m_ctx,F_STR(V_C_TOP_OBJECT),&m_varTopGroup));
	OCL_CATCH;
}

oclRenderer::~oclRenderer(){
	if(m_pSkyboxTexture)delete m_pSkyboxTexture;
	m_ref_pCamera=nullptr;
  m_ctx=nullptr;
}

void oclRenderer::SetRendererSize(oclUint w,oclUint h){
  m_uWidth=w;
  m_uHeight=h;
}

void oclRenderer::Upload(){
	OCL_TRY;
	OCL_CHECK_ERROR(rtVariableSetObject(m_varTopGroup,m_topGroup));
	OCL_CHECK_ERROR(rtVariableSetUserData(m_varRenderer,sizeof(SData_Renderer),&m_data));
	OCL_CATCH;
}

void oclRenderer::FetchTextures(std::vector<const oclTexture*>* out) const{
	out->push_back(m_pSkyboxTexture);
}

void oclRenderer::SetSkyboxTextureURL(oclStr fileURL){
	if(!m_pSkyboxTexture)m_pSkyboxTexture=new oclTexture(m_ctx);
  if(strcmp(m_pSkyboxTexture->GetURL().c_str(),fileURL.c_str())==0)return;
  STextureDescription param;
  param.fileURL=fileURL; 
  param.format=RT_FORMAT_FLOAT4;
  param.type=RT_BUFFER_INPUT;
  param.wrapModeX=RT_WRAP_REPEAT;
  param.wrapModeY=RT_WRAP_REPEAT;
  param.wrapModeZ=RT_WRAP_REPEAT;
  param.filterModeX=RT_FILTER_LINEAR;
  param.filterModeY=RT_FILTER_LINEAR;
  param.filterModeZ=RT_FILTER_LINEAR;
  param.mipmapCount=1;

	m_pSkyboxTexture->Reset(param);
  m_data.skybox_TSID_show=m_pSkyboxTexture->GetTextureSamplerID();
  m_data.frame_current=1u;
}

void oclRenderer::SetSamplesPerPixel(oclUint value){
  oclUint temp=sqrtf(value);
  m_data.samples_per_pixel=temp*temp;
  m_data.samples_per_pixel_col=temp;
  m_data.samples_per_pixel_row=temp;
}

void oclRenderer::SetBadColor(oclFloat r,oclFloat g,oclFloat b){
  m_data.color_bad.x=r;
  m_data.color_bad.y=g;
  m_data.color_bad.z=b;
  m_data.color_bad.w=1.f;
#if __OCL_INTERNAL_DEBUG__
  m_data.color_bad*=100000.f;
#endif
}

void oclRenderer::SetEnableSkybox_show(oclBool value){
	m_data.skybox_enableBackground=value;
}

void oclRenderer::SetTopGroup(RTgroup g){
	m_topGroup=g;
}

void oclRenderer::SetBufferID_lights(oclInt ID){
	m_data.bufferID_lights=ID;
}

OCL_NAMESPACE_END
