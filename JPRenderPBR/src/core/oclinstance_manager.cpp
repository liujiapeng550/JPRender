#include "core/oclinstance_manager.h"
#include "core\oclinstance.h"
#include "core\oclprogram_manager.h"

OCL_NAMESPACE_START

static RTmaterial s_RTmaterial;

oclInstanceManager::oclInstanceManager(RTcontext ctx):m_ctx(ctx){
	F_DBG_INFO("[CONSTRUCT] -> oclInstanceManager");
	Init_();
}

oclInstanceManager::~oclInstanceManager(){
	DestroyInstances();
	F_SAFE_DELETE(m_pGround);
	F_SAFE_DELETE(m_pMaterialBall);
  OCL_TRY;
  OCL_CHECK_ERROR(rtAccelerationDestroy(m_accDynamicGroup));
  OCL_CHECK_ERROR(rtGroupDestroy(m_dynamicGroup));
  OCL_CHECK_ERROR(rtAccelerationDestroy(m_accTopGroup));
  OCL_CHECK_ERROR(rtGroupDestroy(m_topGroup));
  OCL_CHECK_ERROR(rtAccelerationDestroy(m_accTopGroup_materialball));
  OCL_CHECK_ERROR(rtGroupDestroy(m_topGroup_materialball));
  OCL_CATCH;
	m_accTopGroup=nullptr;
	m_topGroup=nullptr;
	m_accDynamicGroup=nullptr;
	m_dynamicGroup=nullptr;
	m_accTopGroup_materialball=nullptr;
	m_topGroup_materialball=nullptr;

	s_RTmaterial=nullptr;
}

void oclInstanceManager::SetAccelerationDirty(){
	OCL_TRY;
	OCL_CHECK_ERROR(rtAccelerationMarkDirty(m_accDynamicGroup));
	OCL_CATCH;
}

void oclInstanceManager::DestroyInstances(){
  m_nDynamicInstanceCount=0;
  OCL_TRY;
  OCL_CHECK_ERROR(rtGroupSetChildCount(m_dynamicGroup,m_nDynamicInstanceCount));
  OCL_CATCH;

	for(auto &it:m_mapInstances){
		delete it.second;
	}
	m_mapInstances.clear();
}

oclBool oclInstanceManager::SetInstanceVisibility(oclID ID,oclBool visible){
	oclInstance* ins=GetInstance(ID);
	return ins?SetInstanceVisibility_(ins,visible):false;
}

oclBool oclInstanceManager::SetInstanceVisibility(oclInstance * ins,oclBool visible){
	return ins?SetInstanceVisibility_(ins,visible):false;
}

void oclInstanceManager::SetGroundVisible(oclBool visible){
  OCL_TRY;
	if(visible){
		OCL_CHECK_ERROR(rtGroupSetChildCount(m_topGroup,2));
		OCL_CHECK_ERROR(rtGroupSetChild(m_topGroup,1,m_pGround->m_transform));
	} else{
		OCL_CHECK_ERROR(rtGroupSetChildCount(m_topGroup,1));
	}
	OCL_CHECK_ERROR(rtAccelerationMarkDirty(m_accTopGroup));
	OCL_CATCH;
}

oclInstance * oclInstanceManager::CreateInstance(oclID ID){
	if(GetInstance(ID))return nullptr;
	oclInstance* ins=new oclInstance(m_ctx,s_RTmaterial);
  OCL_CHECK_ERROR(rtGeometrySetIntersectionProgram(ins->m_geometry,oclProgramManager::S()->GetIntersectProgram_mesh()));
	OCL_CHECK_ERROR(rtGeometrySetBoundingBoxProgram(ins->m_geometry,oclProgramManager::S()->GetBoundProgram_mesh()));
	m_mapInstances[ID]=ins;
	return ins;
}

oclInstance * oclInstanceManager::GetInstance_ground() const{
	return m_pGround;
}

oclInstance * oclInstanceManager::GetInstance_materialBall() const{
	return m_pMaterialBall;
}

void oclInstanceManager::DestoryInstance(oclID ID){
	oclInstance* ins=GetInstance(ID);
	if(ins){
		if(ins->m_bVisible)SetInstanceVisibility_(ins,false);
		delete ins;
		SetAccelerationDirty();
	}
}

void oclInstanceManager::Init_(){
  OCL_TRY;
	/// 1. 初始化
	// dynamic group
  OCL_CHECK_ERROR(rtAccelerationCreate(m_ctx,&m_accDynamicGroup));
  OCL_CHECK_ERROR(rtAccelerationSetBuilder(m_accDynamicGroup,BVH));
  OCL_CHECK_ERROR(rtAccelerationSetTraverser(m_accDynamicGroup,BVH));
  OCL_CHECK_ERROR(rtGroupCreate(m_ctx,&m_dynamicGroup));
  OCL_CHECK_ERROR(rtGroupSetChildCount(m_dynamicGroup,0));
  OCL_CHECK_ERROR(rtGroupSetAcceleration(m_dynamicGroup,m_accDynamicGroup));

	//top group
  OCL_CHECK_ERROR(rtAccelerationCreate(m_ctx,&m_accTopGroup));
  OCL_CHECK_ERROR(rtAccelerationSetBuilder(m_accTopGroup,NOACCEL));
  OCL_CHECK_ERROR(rtAccelerationSetTraverser(m_accTopGroup,NOACCEL));
  OCL_CHECK_ERROR(rtGroupCreate(m_ctx,&m_topGroup));
  OCL_CHECK_ERROR(rtGroupSetChildCount(m_topGroup,2));
  OCL_CHECK_ERROR(rtGroupSetAcceleration(m_topGroup,m_accTopGroup));
	OCL_CHECK_ERROR(rtGroupSetChild(m_topGroup,0,m_dynamicGroup));

	// material ball group
  OCL_CHECK_ERROR(rtAccelerationCreate(m_ctx,&m_accTopGroup_materialball));
  OCL_CHECK_ERROR(rtAccelerationSetBuilder(m_accTopGroup_materialball,NOACCEL));
  OCL_CHECK_ERROR(rtAccelerationSetTraverser(m_accTopGroup_materialball,NOACCEL));
  OCL_CHECK_ERROR(rtGroupCreate(m_ctx,&m_topGroup_materialball));
  OCL_CHECK_ERROR(rtGroupSetChildCount(m_topGroup_materialball,1));
  OCL_CHECK_ERROR(rtGroupSetAcceleration(m_topGroup_materialball,m_accTopGroup_materialball));

	// material
	OCL_CHECK_ERROR(rtMaterialCreate(m_ctx,&s_RTmaterial));
	oclProgramManager* p=oclProgramManager::S();
  OCL_CHECK_ERROR(rtMaterialSetClosestHitProgram(s_RTmaterial,OptixRayTypes::RADIANCE,p->GetCHProgram_radiance()));
  OCL_CHECK_ERROR(rtMaterialSetClosestHitProgram(s_RTmaterial,OptixRayTypes::PICKUP,p->GetCHProgram_pick()));
  OCL_CHECK_ERROR(rtMaterialSetClosestHitProgram(s_RTmaterial,OptixRayTypes::DEPTH,p->GetCHProgram_depth()));
  OCL_CHECK_ERROR(rtMaterialSetClosestHitProgram(s_RTmaterial,OptixRayTypes::AO,p->GetCHProgram_AO()));
  OCL_CHECK_ERROR(rtMaterialSetClosestHitProgram(s_RTmaterial,OptixRayTypes::NORMAL,p->GetCHProgram_normal()));
  OCL_CHECK_ERROR(rtMaterialSetAnyHitProgram(s_RTmaterial,OptixRayTypes::SHADOW,p->GetAHProgram_radiance_shadow()));


	/// 2. 创建阴影地面；
	m_pGround=new oclInstance(m_ctx,s_RTmaterial);
  OCL_CHECK_ERROR(rtGeometrySetIntersectionProgram(m_pGround->m_geometry,oclProgramManager::S()->GetIntersectProgram_plane()));
	OCL_CHECK_ERROR(rtGeometrySetBoundingBoxProgram(m_pGround->m_geometry,oclProgramManager::S()->GetBoundProgram_plane()));
	OCL_CHECK_ERROR(rtGroupSetChild(m_topGroup,1,m_pGround->m_transform));


	/// 3. 创建材质球；
	m_pMaterialBall=new oclInstance(m_ctx,s_RTmaterial);
  OCL_CHECK_ERROR(rtGeometrySetIntersectionProgram(m_pMaterialBall->m_geometry,oclProgramManager::S()->GetIntersectProgram_sphere()));
	OCL_CHECK_ERROR(rtGeometrySetBoundingBoxProgram(m_pMaterialBall->m_geometry,oclProgramManager::S()->GetBoundProgram_sphere()));
	OCL_CHECK_ERROR(rtGroupSetChild(m_topGroup_materialball,0,m_pMaterialBall->m_transform));
	OCL_CATCH;
}

oclBool oclInstanceManager::SetInstanceVisibility_(oclInstance* ins,oclBool visible){
	if(ins->m_bVisible==visible)return false;
	ins->m_bVisible=visible;
	OCL_TRY;
	if(visible){
		OCL_CHECK_ERROR(rtGroupSetChildCount(m_dynamicGroup,m_nDynamicInstanceCount+1));
		OCL_CHECK_ERROR(rtGroupSetChild(m_dynamicGroup,m_nDynamicInstanceCount++,ins->m_transform));
	} else{
		RTobject temp;
		oclInt childIndex=-1;
		for(oclInt index=0;index<m_nDynamicInstanceCount;++index){
			OCL_CHECK_ERROR(rtGroupGetChild(m_dynamicGroup,index,&temp));
			if(temp==ins->m_transform){
				childIndex=index;
				break;
			}
		}
		if(childIndex!=m_nDynamicInstanceCount-1){
			// Get the last child;
			OCL_CHECK_ERROR(rtGroupGetChild(m_dynamicGroup,m_nDynamicInstanceCount-1,&temp));
			OCL_CHECK_ERROR(rtGroupSetChild(m_dynamicGroup,childIndex,temp));
		}
		OCL_CHECK_ERROR(rtGroupSetChildCount(m_dynamicGroup,--m_nDynamicInstanceCount));
	}
	OCL_CHECK_ERROR(rtAccelerationMarkDirty(m_accDynamicGroup));
	OCL_CATCH;
	return true;
}

std::vector<int> oclInstanceManager::GetAllMaterialIDsUsedByInstance()const{
	//TODO:
  std::vector<int> r;
	/*

  for(auto i:m_mapInstances){
    oclMesh* pMesh=(oclMesh*)(i.second);
    int matID=pMesh->GetMaterialID();
    r.push_back(matID);
  }
	*/
  return r;
}

OCL_NAMESPACE_END
