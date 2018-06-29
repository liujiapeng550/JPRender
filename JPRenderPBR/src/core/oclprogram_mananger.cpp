#include "core\oclprogram_manager.h"
#include "dirent.h"
#include "core\oclbuffer_manager.h"

OCL_NAMESPACE_START

oclProgramManager* oclProgramManager::s_ins=nullptr;

oclProgramManager* oclProgramManager::S(){
  if(s_ins==nullptr)s_ins=new oclProgramManager();
  return s_ins;
}

oclProgramManager::oclProgramManager(){}

oclProgramManager::~oclProgramManager(){
  DestroyAllProgram();
  m_optixCtx=nullptr;
}

void oclProgramManager::Init(RTcontext ctx,oclStr path){
	m_optixCtx=ctx;
	LoadIntersectProgram_(path);
  LoadContextProgram_(path);
  LoadCallableProgram_(path+"callable.cu.ptx");
}

void oclProgramManager::DestroyAllProgram(){
  OCL_TRY;
  OCL_CHECK_ERROR(rtProgramDestroy(m_CHProgram_pick));
  OCL_CHECK_ERROR(rtProgramDestroy(m_intersectProgram_mesh));
  OCL_CHECK_ERROR(rtProgramDestroy(m_boundProgram_mesh));
  OCL_CHECK_ERROR(rtProgramDestroy(m_launchProgram_radiance));
  OCL_CHECK_ERROR(rtProgramDestroy(m_launchProgram_pick));
  OCL_CHECK_ERROR(rtProgramDestroy(m_missProgram_radiance));
  OCL_CHECK_ERROR(rtProgramDestroy(m_exceptionProgram));
  OCL_CHECK_ERROR(rtProgramDestroy(m_launchProgram_depth));
  OCL_CHECK_ERROR(rtProgramDestroy(m_CHProgram_depth));
  OCL_CHECK_ERROR(rtProgramDestroy(m_CHProgram_radiance));
  OCL_CHECK_ERROR(rtProgramDestroy(m_AHProgram_radiance_shadow));
  OCL_CHECK_ERROR(rtProgramDestroy(m_launchProgram_AO));
  OCL_CHECK_ERROR(rtProgramDestroy(m_CHProgram_AO));

  oclBufferManager::S()->Destroy(m_bufLightModel_F);
  oclBufferManager::S()->Destroy(m_bufLightModel_PDF);
  oclBufferManager::S()->Destroy(m_bufLightModel_sample_F);
  oclBufferManager::S()->Destroy(m_bufLightModel_albedo);
  oclBufferManager::S()->Destroy(m_bufLight_PDF);
  oclBufferManager::S()->Destroy(m_bufLight_Power);
  oclBufferManager::S()->Destroy(m_bufLight_sample_L);

  OCL_CATCH;
}

void oclProgramManager::LoadIntersectProgram_(oclStr path){
  OCL_TRY;
	oclStr URL=path+"IntersectionTriangleMesh.cu.ptx";
  OCL_CHECK_ERROR(rtProgramCreateFromPTXFile(m_optixCtx,URL.c_str(),"PgmIntersect_mesh_triangle",&m_intersectProgram_mesh));
  OCL_CHECK_ERROR(rtProgramCreateFromPTXFile(m_optixCtx,URL.c_str(),"PgmBounds_mesh_triangle",&m_boundProgram_mesh));
	URL=path+"IntersectionPlane.cu.ptx";
  OCL_CHECK_ERROR(rtProgramCreateFromPTXFile(m_optixCtx,URL.c_str(),"PgmIntersect_plane",&m_intersectProgram_plane));
  OCL_CHECK_ERROR(rtProgramCreateFromPTXFile(m_optixCtx,URL.c_str(),"PgmBounds_plane",&m_boundProgram_plane));
	URL=path+"IntersectionSphere.cu.ptx";
  OCL_CHECK_ERROR(rtProgramCreateFromPTXFile(m_optixCtx,URL.c_str(),"PgmIntersect_sphere",&m_intersectProgram_sphere));
  OCL_CHECK_ERROR(rtProgramCreateFromPTXFile(m_optixCtx,URL.c_str(),"PgmBounds_sphere",&m_boundProgram_sphere));
  OCL_CATCH;
}

void oclProgramManager::LoadContextProgram_(oclStr path){
  OCL_TRY;
  oclStr const fileName=path+"renderer_radiance.cu.ptx";
  OCL_CHECK_ERROR(rtProgramCreateFromPTXFile(m_optixCtx,fileName.c_str(),"PgmLaunch_radiance",&m_launchProgram_radiance));
  OCL_CHECK_ERROR(rtProgramCreateFromPTXFile(m_optixCtx,fileName.c_str(),"PgmException",&m_exceptionProgram));

  oclStr const fileNamedepth=path+"renderer_depth.cu.ptx";
  OCL_CHECK_ERROR(rtProgramCreateFromPTXFile(m_optixCtx,fileNamedepth.c_str(),"PgmLaunch_depth",&m_launchProgram_depth));
  OCL_CHECK_ERROR(rtProgramCreateFromPTXFile(m_optixCtx,fileNamedepth.c_str(),"PgmCH_depth",&m_CHProgram_depth));

  oclStr const fileNameAO=path+"renderer_AO.cu.ptx";
  OCL_CHECK_ERROR(rtProgramCreateFromPTXFile(m_optixCtx,fileNameAO.c_str(),"PgmLaunch_AO",&m_launchProgram_AO));
  OCL_CHECK_ERROR(rtProgramCreateFromPTXFile(m_optixCtx,fileNameAO.c_str(),"PgmCH_AO",&m_CHProgram_AO));

  oclStr const fileNameNormal=path+"renderer_normal.cu.ptx";
  OCL_CHECK_ERROR(rtProgramCreateFromPTXFile(m_optixCtx,fileNameNormal.c_str(),"PgmLaunch_normal",&m_launchProgram_normal));
  OCL_CHECK_ERROR(rtProgramCreateFromPTXFile(m_optixCtx,fileNameNormal.c_str(),"PgmCH_normal",&m_CHProgram_normal));

  oclStr const ptxPickFileURL=path+"PickProgram.cu.ptx";
  OCL_CHECK_ERROR(rtProgramCreateFromPTXFile(m_optixCtx,ptxPickFileURL.c_str(),"PgmLaunch_pick",&m_launchProgram_pick));
  OCL_CHECK_ERROR(rtProgramCreateFromPTXFile(m_optixCtx,ptxPickFileURL.c_str(),"PgmCH_pick",&m_CHProgram_pick));

  oclStr const fileURL_material=path+"Material.cu.ptx";
  OCL_CHECK_ERROR(rtProgramCreateFromPTXFile(m_optixCtx,fileURL_material.c_str(),"PgmCH_radiance",&m_CHProgram_radiance));
  OCL_CHECK_ERROR(rtProgramCreateFromPTXFile(m_optixCtx,fileURL_material.c_str(),"PgmAH_radiance_shadow",&m_AHProgram_radiance_shadow));
  OCL_CHECK_ERROR(rtProgramCreateFromPTXFile(m_optixCtx,fileURL_material.c_str(),"PgmMiss_radiance",&m_missProgram_radiance));
  OCL_CATCH;
}

void oclProgramManager::LoadCallableProgram_(oclStr path){
  OCL_TRY;
  const oclChar* file=path.c_str();

  DeclareProgram_(file,F_STR(CF_GETTEXTURE_DIFFUSE));
  DeclareProgram_(file,F_STR(CF_GETTEXTURE_GLOSSY));
  DeclareProgram_(file,F_STR(CF_GETTEXTURE_NORMAL));
  DeclareProgram_(file,F_STR(CF_GETSKYBOXRADIANCE));
  DeclareProgram_(file,F_STR(CF_GETRADIANCEFROMSH));
  DeclareProgram_(file,F_STR(CF_GETRANDOMSEED));
  DeclareProgram_(file,F_STR(CF_GETSAMPLE1));
  DeclareProgram_(file,F_STR(CF_GETSAMPLE2));
  DeclareProgram_(file,F_STR(CF_UNIFORMSAMPLETRIANGLE));
  DeclareProgram_(file,F_STR(CF_COSINESAMPLEHEMISPHERE_QUICKER));
  DeclareProgram_(file,F_STR(CF_CONCENTRICSAMPLEDISK));
  DeclareProgram_(file,F_STR(CF_DISTRIBUTION1D_SAMPLEDISCRETE));
  DeclareProgram_(file,F_STR(CF_DISTRIBUTION2D_SAMPLECONTINUOUS));
  DeclareProgram_(file,F_STR(CF_DISTRIBUTION2D_PDF));
  DeclareProgram_(file,F_STR(CF_GETENVRADIANCE));
  DeclareProgram_(file,F_STR(CF_CHECKERBOARD3));
  DeclareProgram_(file,F_STR(CF_GETTRIPLANENORMAL));

  /// load light model relative funcs.
  m_bufLightModel_F=oclBufferManager::S()->Create(GetUniqueBufferKey(""),RT_BUFFER_INPUT,RT_FORMAT_PROGRAM_ID);
  m_bufLightModel_F->SetSize(5);
  oclInt* data=m_bufLightModel_F->Map<oclInt,RT_BUFFER_MAP_WRITE>();
  data[CFINDEX_LM_LAMBERTIAN]=GetProgramID_(file,"Lambertian_f");
  data[CFINDEX_LM_TORRANCESPARROW]=GetProgramID_(file,"TorranceSparrow_f");
  data[CFINDEX_LM_SPECULAR_REFLECT]=GetProgramID_(file,"SpecularReflect_f");
  data[CFINDEX_LM_SPECULAR_REFRACT]=GetProgramID_(file,"SpecularRefract_f");
  data[CFINDEX_LM_ASHIKHMIN_SHIRLEY]=GetProgramID_(file,"AshikminShirley_f");
  m_bufLightModel_F->Unmap();
  DeclareProgramBuffer_("buf_lm_F",m_bufLightModel_F);

  m_bufLightModel_PDF=oclBufferManager::S()->Create(GetUniqueBufferKey(""),RT_BUFFER_INPUT,RT_FORMAT_PROGRAM_ID);
  m_bufLightModel_PDF->SetSize(5);
  data=m_bufLightModel_PDF->Map<oclInt,RT_BUFFER_MAP_WRITE>();
  data[CFINDEX_LM_LAMBERTIAN]=GetProgramID_(file,"Lambertian_Pdf");
  data[CFINDEX_LM_TORRANCESPARROW]=GetProgramID_(file,"TorranceSparrow_Pdf");
  data[CFINDEX_LM_SPECULAR_REFLECT]=GetProgramID_(file,"SpecularReflect_Pdf");
  data[CFINDEX_LM_SPECULAR_REFRACT]=GetProgramID_(file,"SpecularRefract_Pdf");
  data[CFINDEX_LM_ASHIKHMIN_SHIRLEY]=GetProgramID_(file,"AshikminShirley_Pdf");
  m_bufLightModel_PDF->Unmap();
  DeclareProgramBuffer_("buf_lm_PDF",m_bufLightModel_PDF);

  m_bufLightModel_sample_F=oclBufferManager::S()->Create(GetUniqueBufferKey(""),RT_BUFFER_INPUT,RT_FORMAT_PROGRAM_ID);
  m_bufLightModel_sample_F->SetSize(5);
  data=m_bufLightModel_sample_F->Map<oclInt,RT_BUFFER_MAP_WRITE>();
  data[CFINDEX_LM_LAMBERTIAN]=GetProgramID_(file,"Lambertian_Sample_f");
  data[CFINDEX_LM_TORRANCESPARROW]=GetProgramID_(file,"TorranceSparrow_Sample_f");
  data[CFINDEX_LM_SPECULAR_REFLECT]=GetProgramID_(file,"SpecularReflect_Sample_f");
  data[CFINDEX_LM_SPECULAR_REFRACT]=GetProgramID_(file,"SpecularRefract_Sample_f");
  data[CFINDEX_LM_ASHIKHMIN_SHIRLEY]=GetProgramID_(file,"AshikminShirley_Sample_f");
  m_bufLightModel_sample_F->Unmap();
  DeclareProgramBuffer_("buf_lm_sample_F",m_bufLightModel_sample_F);

  m_bufLightModel_albedo=oclBufferManager::S()->Create(GetUniqueBufferKey(""),RT_BUFFER_INPUT,RT_FORMAT_PROGRAM_ID);
  m_bufLightModel_albedo->SetSize(5);
  data=m_bufLightModel_albedo->Map<oclInt,RT_BUFFER_MAP_WRITE>();
  data[CFINDEX_LM_LAMBERTIAN]=GetProgramID_(file,"Lambertian_Albedo");
  data[CFINDEX_LM_TORRANCESPARROW]=GetProgramID_(file,"TorranceSparrow_Albedo");
  data[CFINDEX_LM_SPECULAR_REFLECT]=GetProgramID_(file,"SpecularReflect_Albedo");
  data[CFINDEX_LM_SPECULAR_REFRACT]=GetProgramID_(file,"SpecularRefract_Albedo");
  data[CFINDEX_LM_ASHIKHMIN_SHIRLEY]=GetProgramID_(file,"AshikminShirley_Albedo");
  m_bufLightModel_albedo->Unmap();
  DeclareProgramBuffer_("buf_lm_albedo",m_bufLightModel_albedo);


  /// load light relative funcs.
  m_bufLight_PDF=oclBufferManager::S()->Create(GetUniqueBufferKey(""),RT_BUFFER_INPUT,RT_FORMAT_PROGRAM_ID);
  m_bufLight_PDF->SetSize(5u);
  oclInt* pdf=m_bufLight_PDF->Map<oclInt,RT_BUFFER_MAP_WRITE>();
  pdf[CFINDEX_LIGHT_POINT]=GetProgramID_(file,"PointLight_Pdf");
  pdf[CFINDEX_LIGHT_SPOT]=GetProgramID_(file,"SpotLight_Pdf");
  pdf[CFINDEX_LIGHT_DISTANT]=GetProgramID_(file,"DistantLight_Pdf");
  pdf[CFINDEX_LIGHT_AREA]=GetProgramID_(file,"AreaLight_Pdf");
  pdf[CFINDEX_LIGHT_ENVIRONMENT]=GetProgramID_(file,"EnvLight_Pdf");
  m_bufLight_PDF->Unmap();
  DeclareProgramBuffer_("buf_light_PDF",m_bufLight_PDF);



  m_bufLight_Power=oclBufferManager::S()->Create(GetUniqueBufferKey(""),RT_BUFFER_INPUT,RT_FORMAT_PROGRAM_ID);
  m_bufLight_Power->SetSize(5u);
  oclInt* power=m_bufLight_Power->Map<oclInt,RT_BUFFER_MAP_WRITE>();
  power[CFINDEX_LIGHT_POINT]=GetProgramID_(file,"PointLight_Power");
  power[CFINDEX_LIGHT_SPOT]=GetProgramID_(file,"SpotLight_Power");
  power[CFINDEX_LIGHT_DISTANT]=GetProgramID_(file,"DistantLight_Power");
  power[CFINDEX_LIGHT_AREA]=GetProgramID_(file,"AreaLight_Power");
  power[CFINDEX_LIGHT_ENVIRONMENT]=GetProgramID_(file,"EnvLight_Power");
  m_bufLight_Power->Unmap();
  DeclareProgramBuffer_("buf_light_Power",m_bufLight_Power);




  m_bufLight_sample_L=oclBufferManager::S()->Create(GetUniqueBufferKey(""),RT_BUFFER_INPUT,RT_FORMAT_PROGRAM_ID);
  m_bufLight_sample_L->SetSize(5u);
  oclInt* sampleL=m_bufLight_sample_L->Map<oclInt,RT_BUFFER_MAP_WRITE>();
  sampleL[CFINDEX_LIGHT_POINT]=GetProgramID_(file,"PointLight_Sample_L");
  sampleL[CFINDEX_LIGHT_SPOT]=GetProgramID_(file,"SpotLight_Sample_L");
  sampleL[CFINDEX_LIGHT_DISTANT]=GetProgramID_(file,"DistantLight_Sample_L");
  sampleL[CFINDEX_LIGHT_AREA]=GetProgramID_(file,"AreaLight_Sample_L");
  sampleL[CFINDEX_LIGHT_ENVIRONMENT]=GetProgramID_(file,"EnvLight_Sample_L");
  m_bufLight_sample_L->Unmap();
  DeclareProgramBuffer_("buf_light_sample_L",m_bufLight_sample_L);
  OCL_CATCH;
}

oclInt oclProgramManager::GetProgramID_(const oclChar * file,const oclChar * name){
  RTprogram tmpp;
  oclInt tmpid;
  OCL_TRY;
  OCL_CHECK_ERROR(rtProgramCreateFromPTXFile(m_optixCtx,file,name,&tmpp));
  OCL_CHECK_ERROR(rtProgramGetId(tmpp,&tmpid));
  OCL_CATCH;
  return tmpid;
}

void oclProgramManager::DeclareProgram_(const oclChar * file,const oclChar * name){
  RTprogram tmpp;
  oclInt tmpid;
  OCL_TRY;
  OCL_CHECK_ERROR(rtProgramCreateFromPTXFile(m_optixCtx,file,name,&tmpp));
  OCL_CHECK_ERROR(rtProgramGetId(tmpp,&tmpid));

  RTvariable RTvar;
  OCL_CHECK_ERROR(rtContextDeclareVariable(m_optixCtx,name,&RTvar));
  OCL_CHECK_ERROR(rtVariableSet1i(RTvar,tmpid));
  OCL_CATCH;
}

void oclProgramManager::DeclareProgramBuffer_(const oclChar * name,oclBuffer* oclBuf){
  RTvariable RTvar;
  OCL_TRY;
  OCL_CHECK_ERROR(rtContextDeclareVariable(m_optixCtx,name,&RTvar));
  OCL_CATCH;
  oclBufferManager::SetoclBufferToRTvariable(oclBuf,RTvar);
}

OCL_NAMESPACE_END

