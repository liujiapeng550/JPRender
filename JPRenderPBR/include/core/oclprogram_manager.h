#pragma once

#include "oclcore.h"

OCL_NAMESPACE_START

class oclBuffer;

class oclProgramManager final{
public:
	static oclProgramManager* S();

private:
	static oclProgramManager* s_ins;

public:
	void Init(RTcontext,oclStr path);

  const RTprogram GetBoundProgram_mesh()const{ return m_boundProgram_mesh; }
  const RTprogram GetIntersectProgram_mesh()const{ return m_intersectProgram_mesh; }
  const RTprogram GetBoundProgram_plane()const{ return m_boundProgram_plane; }
  const RTprogram GetIntersectProgram_plane()const{ return m_intersectProgram_plane; }
  const RTprogram GetBoundProgram_sphere()const{ return m_boundProgram_sphere; }
  const RTprogram GetIntersectProgram_sphere()const{ return m_intersectProgram_sphere; }

  const RTprogram GetLaunchProgram_radiance()const{ return m_launchProgram_radiance; }
  const RTprogram GetExceptionProgram()const{ return m_exceptionProgram; }
  const RTprogram GetMissProgram_radiance()const{ return m_missProgram_radiance; }
  const RTprogram GetCHProgram_radiance()const{ return m_CHProgram_radiance; }
  const RTprogram GetAHProgram_radiance_shadow()const{ return m_AHProgram_radiance_shadow; }

  const RTprogram GetLaunchProgram_pick()const{ return m_launchProgram_pick; }
  const RTprogram GetCHProgram_pick()const{ return m_CHProgram_pick; }

  const RTprogram GetLaunchProgram_depth()const{ return m_launchProgram_depth; }
  const RTprogram GetCHProgram_depth()const{ return m_CHProgram_depth; }

  const RTprogram GetLaunchProgram_AO()const{ return m_launchProgram_AO; }
  const RTprogram GetCHProgram_AO()const{ return m_CHProgram_AO; }

  const RTprogram GetLaunchProgram_normal()const{ return m_launchProgram_normal; }
  const RTprogram GetCHProgram_normal()const{ return m_CHProgram_normal; }

private:
  oclProgramManager(const oclProgramManager& another)=delete;
  oclProgramManager& operator=(const oclProgramManager&)=delete;
	oclProgramManager();
  ~oclProgramManager();

  void DestroyAllProgram();
  void LoadIntersectProgram_(oclStr path);
  void LoadContextProgram_(oclStr path);
  void LoadCallableProgram_(oclStr path);
  oclInt GetProgramID_(const oclChar* file,const oclChar* name);
  void DeclareProgram_(const oclChar* file,const oclChar* name);
  void DeclareProgramBuffer_(const oclChar * name,oclBuffer* oclBuf);

private:
  RTcontext m_optixCtx;
	
  RTprogram m_intersectProgram_mesh;
  RTprogram m_boundProgram_mesh;
  RTprogram m_intersectProgram_plane;
  RTprogram m_boundProgram_plane;
  RTprogram m_intersectProgram_sphere;
  RTprogram m_boundProgram_sphere;

  RTprogram m_launchProgram_radiance;
  RTprogram m_exceptionProgram;
  RTprogram m_AHProgram_radiance_shadow;
  RTprogram m_CHProgram_radiance;
  RTprogram m_missProgram_radiance;

  RTprogram m_launchProgram_pick;
  RTprogram m_CHProgram_pick;

  RTprogram m_launchProgram_normal;
  RTprogram m_CHProgram_normal;

  RTprogram m_launchProgram_AO;
  RTprogram m_CHProgram_AO;

  RTprogram m_launchProgram_depth;
  RTprogram m_CHProgram_depth;

  oclBuffer* m_bufLightModel_F=0;
  oclBuffer* m_bufLightModel_PDF=0;
  oclBuffer* m_bufLightModel_sample_F=0;
  oclBuffer* m_bufLightModel_albedo=0;
  oclBuffer* m_bufLight_PDF=0;
  oclBuffer* m_bufLight_Power=0;
  oclBuffer* m_bufLight_sample_L=0;
};

OCL_NAMESPACE_END
