#include "rmprocessor.h"
#include <time.h>
#include "core\oclprogram_manager.h"
#include "core\oclbuffer_manager.h"
#include "core\JsonConverter.h"
#include "core\ocltexture.h"
#include "core/oclinstance.h"
#include "core/oclinstance_manager.h"
#include "material\oclmaterial.h"
#include "material\oclmaterial_library.h"
#include "material\oclmaterial_manager.h"
#include "geometry/oclmesh.h"
#include "geometry\oclgeometry_base.h"
#include "light\ocllight_source.h"
#include "light\ocllight_source_area.h"
#include "light\ocllight_manager.h"
#include "camera/oclcamera.h"
#include "primes.h"
#include "oclmesh_picker.h"
#include "renderer/oclrenderer_radiance.h"
#include "renderer/oclrenderer_AO.h"
#include "renderer/oclrenderer_depth.h"
#include "renderer/oclrenderer_normal.h"
#define BOOST_FILESYSTEM_NO_DEPRECATED
#include "boost\filesystem.hpp"
using namespace std;
using namespace optix;

// 该值每次Init必须清零；
static int s_nNextAvaliableGeometryID=0;
// 该值不能清零；
static int s_nNextAvaliableMaterialID=0;

static int GetNextAvaliableLibID(){
	static int s_index=0;
	return ++s_index;
}

static const wchar_t *GetWC(const char *c){
	const size_t cSize=strlen(c)+1;
	wchar_t* wc=new wchar_t[cSize];
	mbstowcs(wc,c,cSize);

	return wc;
}

OCL_NAMESPACE_START

RenderMax::IBaseProcess* rmProcessor::getInstance(){
  F_DBG_INFO("getInstance");

	if(!m_instance){
		m_instance=new rmProcessor();
	}
	return m_instance;
}

rmProcessor* rmProcessor::m_instance=nullptr;

rmProcessor::rmProcessor(){
#if __OCL_INTERNAL_DEBUG__
	AllocConsole();// 开启控制台窗口  
	// Redirect CRT standard input, output and error handles to the console window.
	FILE * pNewStdout=nullptr;
	FILE * pNewStderr=nullptr;
	FILE * pNewStdin=nullptr;

	freopen_s(&pNewStdout,"CONOUT$","w",stdout);
	freopen_s(&pNewStderr,"CONOUT$","w",stderr);
	freopen_s(&pNewStdin,"CONIN$","r",stdin);

	// Clear the error state for all of the C++ standard streams. Attempting to accessing the streams before they refer
	// to a valid target causes the stream to enter an error state. Clearing the error state will fix this problem,
	// which seems to occur in newer version of Visual Studio even when the console has not been read from or written
	// to yet.
	cout.clear();
	cerr.clear();
	cin.clear();
	F_DBG_INFO("Test console print!");
#endif

	RTresult code=rtContextCreate(&ctx);
	if(code!=RT_SUCCESS){
		F_DBG_INFO("Context creation failure!");
		throw "Context creation failure!";
		return;
	}


#if __OCL_INTERNAL_DEBUG__
	SetContext(ctx);// debug用的。
	OCL_CHECK_ERROR(rtContextSetPrintEnabled(ctx,1));
	OCL_CHECK_ERROR(rtContextSetPrintBufferSize(ctx,1024u));
	OCL_CHECK_ERROR(rtContextSetExceptionEnabled(ctx,RT_EXCEPTION_ALL,1));
	//OCL_CHECK_ERROR(rtContextSetTimeoutCallback
	oclUint c;
	OCL_CHECK_ERROR(rtContextGetDeviceCount(ctx,&c));
	//OCL_CHECK_ERROR(rtDeviceGetAttribute

#endif


	OCL_TRY;
	OCL_CHECK_ERROR(rtContextSetStackSize(ctx,512u));
	OCL_CHECK_ERROR(rtContextSetEntryPointCount(ctx,EntryPoint::E_ENTRYPOINT_TOTAL));
	OCL_CHECK_ERROR(rtContextSetRayTypeCount(ctx,OptixRayTypes::TOTAL));
	OCL_CHECK_ERROR(rtContextDeclareVariable(ctx,F_STR(V_C_DATA_CONTEXT),&m_varContext));
	OCL_CATCH;

}

rmProcessor::~rmProcessor(){
  F_DBG_INFO("rmProcessor");
	DestroyDefaultTextures();
	oclBufferManager* bufMgr=oclBufferManager::S();
	bufMgr->Destroy(m_bufferPrime);

	F_SAFE_DELETE(m_pRender_radiance);
	F_SAFE_DELETE(m_pRender_AO);
	F_SAFE_DELETE(m_pRender_depth);
	F_SAFE_DELETE(m_pRender_normal);
	F_SAFE_DELETE(m_pRender_materialBall);
	F_SAFE_DELETE(m_pMeshPicker);
	F_SAFE_DELETE(m_pLightMgr);
	F_SAFE_DELETE(m_pGeoMgr);
   F_SAFE_DELETE(m_pMaterialMgr);
   F_SAFE_DELETE(m_pInsMgr);
	F_SAFE_DELETE(m_pCamera);
	F_SAFE_DELETE(m_pCameraMaterialBall);
	OCL_CHECK_ERROR(rtContextDestroy(ctx));
}

#pragma region light
void rmProcessor::createLight(int lightIndex,const char* lightType){
	F_DBG_INFO("createLight,lightIndex,type["+to_string(lightIndex)+","+oclStr(lightType)+"]");
	m_pLightMgr->CreateLight(lightIndex,lightType);
}

void rmProcessor::HideSelectLight(int index,bool bShow){
  F_DBG_INFO("HideSelectLight,index,bShow["+to_string(index)+"]");
	oclLightSource* l=m_pLightMgr->GetLight(index);
	if(l){
		l->SetVisibility(bShow);
		m_pLightMgr->Upload();
		m_pRender_actived->ResetFrameCount();
	}
}

void rmProcessor::setLightProperty(unsigned int lightIndex,const char* propName,VARIANT* v,int count){
  F_DBG_INFO("setLightProperty,lightIndex,propName["+to_string(lightIndex)+","+oclStr(propName)+"]");
	oclLightSource* l=m_pLightMgr->GetLight(lightIndex);
	if(l){
		if(l->GetLightType()==LIGHT_TYPE::AREA){
			if(strcmp(propName,"nodeID")==0){
				oclInt nodeIndex=v->intVal;
				const pair<oclGeometryBase*,oclInstance*>& p=m_mapNodeToObject[nodeIndex];
				oclMesh* mesh=static_cast<oclMesh*>(p.first);
				oclInstance* ins=p.second;
				if(mesh){
					vector<oclFloat> area;
					mesh->GetArea(&area);
					SInstanceProxy insp;
					ins->FetchProxy(&insp);
					static_cast<oclLightSourceArea*>(l)->SetAreaLightAccessory(area,insp);
				}
			}
		}
		l->SetProperty(propName,v);
		m_pLightMgr->Upload();
		m_pRender_actived->ResetFrameCount();
	}
}

string rmProcessor::getLightBuffer(unsigned int lightIndex){
  F_DBG_INFO("getLightBuffer");

	oclLightSource* l=m_pLightMgr->GetLight(lightIndex);
	return l?l->GetJsonString():"";
}

string rmProcessor::getLightBuffer(vector<int> nodeIndexes){
  F_DBG_INFO("getLightBuffer");

	return m_pLightMgr->GetLightJsonString(nodeIndexes);
}

/**
* 在保存模型时候，由引擎调用，旨在获取场景中光源的信息，以便保存进我们的模型文件中
* 不过HDRlight有环境光（一个HDR文件），这些文件交由SaveFile接口一并实现；
* 从接口的命名上来看非常明确，“获取光源信息”，并没有要保存纹理的意思
* 接口定义一定要非常明确，不怕名字起的长，首先要明确；
*/
string rmProcessor::getAllLightBuffer(int saveMode){
  F_DBG_INFO("getAllLightBuffer");

	Json::Value j=m_pLightMgr->GetLightsJson();
	return j.toStyledString();
}

/**
* 加载我们的模型后调用；
* 起的这名，其逻辑命名仅仅是给已创建的光源赋值；
*/
void rmProcessor::createAllLight(string buf,int offset){
	F_DBG_INFO("createAllLight,offset["+to_string(offset)+"]");
	//F_DBG_INFO("createAllLight,buf,offset["+buf+","+to_string(offset)+"]");
	m_pLightMgr->SetLightsProperty(buf,offset,m_openedModelFileInfo.m_dir);
	m_pLightMgr->Upload();
}

void rmProcessor::deleteAllLight(){
  F_DBG_INFO("deleteAllLight");

	m_pLightMgr->DestroyLights();
	m_pLightMgr->Upload();
}

void rmProcessor::getLightProperty(unsigned int lightIndex,const char* propName,float * v){
  F_DBG_INFO("getLightProperty");

	oclLightSource* l=m_pLightMgr->GetLight(lightIndex);
	if(l)l->GetProperty(propName,v);
}
#pragma endregion

#pragma region MaterialLib

string rmProcessor::GetMatLibItem(int libIndex,int itemIndex){
  F_DBG_INFO("GetMatLibItem");
  //TODO: ERROR
	oclMaterialLibrary* lib=m_pMaterialMgr->GetLibrary(libIndex);
	oclMaterial* m=lib->GetMaterialByIndex(itemIndex);
	SMaterialDescription d=m->GetDescription();
	oclStr out;
	jsonconvertor::ConvertMaterialDescriptionToJson(d,&out);
	return out;
}

bool rmProcessor::DeleteMatItem(int libIndex,int ID){
  F_DBG_INFO("DeleteMatItem");
  oclMaterialLibrary* lib=m_pMaterialMgr->GetLibrary(libIndex);
	return lib?lib->DestroyMaterial(ID):false;
}

bool rmProcessor::ClearSceneUnusedMats(vector<int> usedmatindexs){
  F_DBG_INFO("ClearSceneUnusedMats");
  //TODO:JC:需要实现;
	return true;
	/*
	vector<int> usedmatindexs2,usedmatindexs3;
	usedmatindexs2=m_pGeoMgr->GetScenUseMaterial();
	usedmatindexs3.insert(usedmatindexs3.end(),usedmatindexs.begin(),usedmatindexs.end());
	usedmatindexs3.insert(usedmatindexs3.end(),usedmatindexs2.begin(),usedmatindexs2.end());
	return  m_pMaterialMgr->CleanSceneUnusedMaterial(usedmatindexs3);
	*/
}

int rmProcessor::GetSceneMatCount(){
  F_DBG_INFO("GetSceneMatCount");
  oclMaterialLibrary* lib=m_pMaterialMgr->GetLibrary(M_LIB_SCENE_ID);
	return lib->GetMaterialCount();
}

string rmProcessor::GetSceneMatList(){
  F_DBG_INFO("GetSceneMatList");
  oclMaterialLibrary* lib=m_pMaterialMgr->GetLibrary(M_LIB_SCENE_ID);
	if(lib){
		SMaterialLibraryDescription d=lib->GetDescription();
		oclStr out;
		jsonconvertor::ConvertMaterialLibraryDescriptionToJson_scene(d,&out);
		return out;
	}
	return "";
}

/**
* 保存模型时候调用，将场景中使用的材质的Json数据以字符串格式返回引擎，供其统一保存
*/
string rmProcessor::GetAllMaterialBuf(){
	F_DBG_INFO("GetAllMaterialBuf");
	oclMaterialLibrary* lib=m_pMaterialMgr->GetLibrary(M_LIB_SCENE_ID);
	if(lib){
		SMaterialLibraryDescription d=lib->GetDescription();
		oclStr out;
		jsonconvertor::ConvertMaterialLibraryDescriptionToJson_save(d,&out);
		return out;
	}
	return "";
}

string rmProcessor::GetMatLibrary(int libIndex){
  F_DBG_INFO("GetMatLibrary");
  oclMaterialLibrary* lib=m_pMaterialMgr->GetLibrary(libIndex);
	if(lib){
		SMaterialLibraryDescription d=lib->GetDescription();
		oclStr out;
		jsonconvertor::ConvertMaterialLibraryDescriptionToJson(d,&out);
		return out;
	}
	return "";
}

/**
* 加载已经保存的材质库. 当点击材质库Open按钮时调用,或者点击创建新材质库按钮时调用；
* 返回材质库ID
*/
int rmProcessor::OpenMatLibrary(const char* path,bool){
  F_DBG_INFO("OpenMatLibrary");
  RM_FileInfo info=path;
	SMaterialLibraryDescription d;
	d.filePath=info.m_absolutePath;
	d.ID=GetNextAvaliableLibID();
	if(strcmp(d.filePath.c_str(),"")!=0){
    ifstream file(d.filePath,ios::in);
    istreambuf_iterator<char> beg(file),end;
    oclStr temp(beg,end);
    file.close();
		if(!jsonconvertor::ConvertJsonToMaterialLibraryDescription(temp,&d))return -1;
		for(auto& it:d.vecMaterial){
			s_nNextAvaliableMaterialID+=it.ID;
		}
	}
	oclMaterialLibrary* lib= m_pMaterialMgr->CreateLibrary(d);
	if(lib){
		lib->Upload();
		return lib->GetID();
	}
	return -1;
}

bool rmProcessor::SaveMatLibrary(const char* path,int libIndex){
  F_DBG_INFO("SaveMatLibrary");
  oclMaterialLibrary* lib=m_pMaterialMgr->GetLibrary(libIndex);
	if(lib){
		boost::filesystem::path p(path);
		if(!boost::filesystem::exists(p.parent_path())) boost::filesystem::create_directories(p.parent_path());
		SMaterialLibraryDescription d=lib->GetDescription();
		oclStr out;
		jsonconvertor::ConvertMaterialLibraryDescriptionToJson(d,&out);
		boost::filesystem::save_string_file(p,out);
		return true;
	}
	return false;
}

bool rmProcessor::CloseMatLibrary(int libIndex){
  F_DBG_INFO("CloseMatLibrary");
  oclMaterialLibrary* lib=m_pMaterialMgr->GetLibrary(libIndex);
	return m_pMaterialMgr->DestroyLibrary(lib);
}

bool rmProcessor::AssignlookToSingleMesh(int meshindex,int ID){
  F_DBG_INFO("AssignlookToSingleMesh");
  //TODO:JC:检查与实现；
 
  if(ID==M_MATERIAL_UV){
     oclMaterialLibrary* scenelib=m_pMaterialMgr->GetLibrary(M_LIB_SCENE_ID);
    oclMaterial* m=scenelib->GetMaterial(0);
    F_DBG_ASSERT(m);
    oclMaterialLibrary* InternalLib=m_pMaterialMgr->GetLibrary(M_LIB_INTERNAL_ID);
    oclMaterial* UVMaterial=InternalLib->GetMaterial(M_MATERIAL_UV);
    SMaterialDescription d=UVMaterial->GetDescription();
    d.libID=m->GetLibID();
    d.itemIndex=m->GetIndex();
    m->Reset(d);
    SMaterialProxy mproxy;
    m->FetchProxy(&mproxy);
    oclInstance* ins=m_pInsMgr->GetInstance(meshindex);
    if(ins){
      ins->SetMaterialProxy(mproxy);
      ins->Upload();
    }
    scenelib->Upload();
    m_pRender_actived->ResetFrameCount();
    return true;
  }
	return false;
}

/**
* 将材质库中的材质拖动到场景mesh上释放时调用。
*/
bool rmProcessor::AssignlookToSelection(vector<int> meshes
	,int matlibIndex,int matID,int assignMode,int& NewMatIndex){
  F_DBG_INFO("AssignlookToSelection");

	if(meshes.size()<=0)return false;
	if(matlibIndex<0||matID<0) return false;

	/// 获取将要使用的材质对象；
	oclMaterialLibrary* lib=m_pMaterialMgr->GetLibrary(matlibIndex);
	oclMaterial* newm=lib->GetMaterial(matID);
	F_DBG_ASSERT(newm);
	SMaterialDescription d=newm->GetDescription();

	/// 默认选取要改变材质的所有mesh中第一个mesh原先使用的材质；
	/// 该材质肯定是场景材质库中的材质；
	oclMaterialLibrary* scenelib=m_pMaterialMgr->GetLibrary(M_LIB_SCENE_ID);
	oclMaterial* m=scenelib->GetMaterial(m_pInsMgr->GetInstance(meshes[0])->GetMaterialID());
	F_DBG_ASSERT(m);

	/// 改变旧材质的部分属性；
	d.libID=m->GetLibID();
	d.itemIndex=m->GetIndex();
	m->Reset(d);

	SMaterialProxy mproxy;
	m->FetchProxy(&mproxy);
	switch(assignMode){
	case 0://shape 
	case 1://group
	{
		//此处在上一层已经处理过shape和group 无需再次处理,此处只关乎节点所以逻辑相同
		//遍历所有mesh，更换为同一材质索引
		for(auto & i:meshes){
			oclInstance* ins=m_pInsMgr->GetInstance(i);
			if(ins){
				ins->SetMaterialProxy(mproxy);
				ins->Upload();
			}
		}
	}
	break;
	case 2://look
	{
		//TODO:CJ:检查与实现；
		//此处关心节点所用材质
		//找到场景中用此材质的模型，然后更换材质索引
		/*
		for(auto &i:meshes){
			oclMesh* mesh=m_pGeoMgr->GetMesh(i);
			if(!mesh) continue;
			oclInt cmatIndex=mesh->GetMaterialIndex();
			for(auto &j:meshes){
				oclMesh* pMesh=m_pGeoMgr->GetMesh(j);
				oclInt matIndex_=pMesh->GetMaterialIndex();
				if(matIndex_==cmatIndex){
					//m_pMaterialMgr->CopyMaterial(pMesh->GetMaterialIndex(),matIndex,NewMatIndex);
					//pMesh->SetMaterialIndex(NewMatIndex);
					OCL_TRY;
					OCL_CHECK_ERROR(rtGeometryInstanceSetMaterialCount(pMesh->GetGeometryInstance(),1));
					OCL_CHECK_ERROR(rtGeometryInstanceSetMaterial(pMesh->GetGeometryInstance(),0,m_pMaterialMgr->GetMaterial(matIndex)->GetRawMaterial()));
					OCL_CATCH;
				}
			}
		}
*/
	}
	break;
	default:
	break;
	}

	NewMatIndex=m->GetID();
	scenelib->Upload();

	m_pRender_actived->ResetFrameCount();
	return true;
}

/**
* 拷贝材质时候调用，比如拖动材质库中的材质到材质库中（就是给材质库添加一个一样的新材质）；
*/
bool rmProcessor::CopyMatItem(int srcMat,int desMatlib,int& newItemIndex){
  F_DBG_INFO("CopyMatItem");
  oclMaterialLibrary* lib=m_pMaterialMgr->GetLibrary(desMatlib);
	if(lib){
		oclMaterial* m=lib->GetMaterial(srcMat);
		if(m){
			SMaterialDescription d=m->GetDescription();
			d.ID=s_nNextAvaliableMaterialID++;
			oclMaterial* newm=lib->CreateMaterial(d);
			newItemIndex=newm->GetIndex();
			return true;
		}
	}
	return false;
}

/**
* 从材质库UI界面创建材质时候调用该接口；
* 参数二返回新创建材质在libID材质库中的索引；
* 返回新创建的材质的唯一ID;
*
* libIndex													材质库索引（在map中的key值）；
* itemIndex													材质库中材质的索引（并不是材质ID）；
*/
int rmProcessor::CreateMaterial(unsigned int libIndex,int * itemIndex){
  F_DBG_INFO("CreateMaterial");
  oclMaterialLibrary* lib=m_pMaterialMgr->GetLibrary(libIndex);
	if(lib){
		m_bFlagUploadMaterialData=true;
		SMaterialDescription p;
		p.ID=s_nNextAvaliableMaterialID++;
		p.type=MATERIAL_MATTE;
		p.jsonString=GetTypeJson(p.type).toStyledString();
		p.definitionName+=to_string(p.ID);
		oclMaterial* m=lib->CreateMaterial(p);
		*itemIndex=m->GetIndex();
		return m->GetID();
	}
	return -1;
}

#pragma endregion

#pragma region Material
/**
*TODO:JC:材质球渲染以后需要引擎设计的更加友好，目前只能返回绘制1帧的结果；
* 参数二：libID 作废，引擎提供的永远是-1
*	该接口对PT渲染器不友好，以后需要改动；
*/
void rmProcessor::GetMatballBuffer(int matID,int,int width,int height,void* pbuf){
	F_DBG_INFO("GetMatballBuffer begine");
	oclUint size=m_pRender_materialBall->GetRendererSize();
	if(size!=width*height){
		m_pRender_materialBall->SetRendererSize(width,height);
	}
	oclMaterialLibrary* lib;
	oclMaterial* m=m_pMaterialMgr->GetMaterial(matID,&lib);
	if(m){
		SMaterialProxy mp;
		m->FetchProxy(&mp);
		oclInstance* ins=m_pInsMgr->GetInstance_materialBall();
		ins->SetMaterialProxy(mp);
		ins->Upload();

		m_pRender_materialBall->ResetFrameCount();
		m_pRender_materialBall->PreRender();
		m_pRender_materialBall->Upload();
		m_pRender_materialBall->Render();
		m_pRender_materialBall->PostRender();
		void* img=m_pRender_materialBall->MapOutputBuffer();
		ConvertFloat4ToUchar4(static_cast<oclFloat4*>(img),&pbuf,width,height);
		m_pRender_materialBall->UnmapOutputBuffer();
	}
	F_DBG_INFO("GetMatballBuffer end");
}

string rmProcessor::getMaterialName(int ID){
  F_DBG_INFO("getMaterialName");
  oclMaterialLibrary* lib;
	oclMaterial* m=m_pMaterialMgr->GetMaterial(ID,&lib);
	return m?m->GetDefinitionName():"";
}

void rmProcessor::destroySceneMaterial(){
  F_DBG_INFO("destroySceneMaterial");
  oclMaterialLibrary* lib=m_pMaterialMgr->GetLibrary(M_LIB_SCENE_ID);
	lib->DestroyMaterials();
}

bool rmProcessor::SetMatProperty(int ID,const char* propName,VARIANT* v,int count){
  F_DBG_INFO("SetMatProperty");
  if(ID>=0){
		oclMaterialLibrary* lib;
		oclMaterial* m=m_pMaterialMgr->GetMaterial(ID,&lib);
		if(m&&m->SetProperty(propName,v,count)){
			lib->Upload();
			if(lib->GetID()==M_LIB_SCENE_ID){
				m_pRender_actived->ResetFrameCount();
			}
			return true;
		}
	}
	return false;
}

bool rmProcessor::GetMatProperty(int ID,const char* propName,VARIANT* v,int count){
  F_DBG_INFO("GetMatProperty");
  if(ID>=0){
		oclMaterialLibrary* lib;
		oclMaterial* m=m_pMaterialMgr->GetMaterial(ID,&lib);
		return m&& m->GetProperty(propName,v,count);
	}
	return false;
}

/**
* 加载我们自己格式的模型后，引擎调用该接口
* jsonbuffs 是所有该模型要使用的材质数据
* TODO:JC: 第二三个参数什么含义？
*/
void rmProcessor::setMaterialRes(string jsonbuffs
	,map<string ,int>& fileGuidIndexMap
	,map<string,int>& allGuidIndexMap){
	F_DBG_INFO("setMaterialRes");
	//F_DBG_INFO("setMaterialRes,jsonbuffs["+jsonbuffs+"]");

	//================================================================================
	//TODO:jc:这里纯粹是为解决ID而写的，需要引擎好好规划下ID的管理；
	oclInt maxID=0;
	//================================================================================

	SMaterialLibraryDescription d;
	jsonconvertor::ConvertJsonToMaterialLibraryDescription_save(jsonbuffs,&d);
	oclMaterialLibrary* lib=m_pMaterialMgr->GetLibrary(M_LIB_SCENE_ID);
	for(SMaterialDescription& md:d.vecMaterial){
		md.texturePath=m_openedModelFileInfo.m_dir;
		//TODO:JC:这里是一个坑，李帅以后会改动ID的唯一管理地方，要么引擎，要么渲染器；
		if(md.ID>maxID)maxID=md.ID;
		md.ID+=s_nNextAvaliableMaterialID;
		lib->CreateMaterial(md);
	}
	s_nNextAvaliableMaterialID+=maxID+1;
	m_bFlagUploadMaterialData=true;
}

string rmProcessor::GetMatTypes(){
	F_DBG_INFO("GetMatTypes");
	string a=GetTypeNames();
	return a;
}

/**
* 载入一个常规模型后，由引擎发起调用，模型有几个材质就调用几次
*/
void rmProcessor::CreateMaterial(int ID,OPENNP::Material* pMat){
	s_nNextAvaliableMaterialID=ID+1;
	oclMaterialLibrary* lib=m_pMaterialMgr->GetLibrary(M_LIB_SCENE_ID);
	SMaterialDescription p;
	p.ID=ID;
	p.type=MATERIAL_METAL;
	p.jsonString=GetTypeJson(p.type).toStyledString();
   //p.definitionName=pMat->materialName.C_Str();
	lib->CreateMaterial(p);
	m_bFlagUploadMaterialData=true;
}

/**
* 场景中鼠标点击选中一个mesh后，需要获取该mesh关联的材质ID，从而引擎好渲染对应的材质UI
*/
int rmProcessor::GetMatIndex(int meshIndex){
	F_DBG_INFO("GetMatIndex,meshIndex["+to_string(meshIndex)+"]");
	oclInstance* ins=m_pInsMgr->GetInstance(meshIndex);
	return ins?ins->GetMaterialID():-1;
}

bool rmProcessor::SetMatDefinitionName(int ID,const char* matName){
  F_DBG_INFO("SetMatDefinitionName");
  oclMaterialLibrary* lib;
	oclMaterial* m=m_pMaterialMgr->GetMaterial(ID,&lib);
	if(m){
		m->SetDefinitionName(matName);
		return true;
	} 
	return false;
}

int rmProcessor::GetMatCounts(){
  F_DBG_INFO("GetMatCounts");
  oclMaterialLibrary* lib=m_pMaterialMgr->GetLibrary(M_LIB_SCENE_ID);
	return lib->GetMaterialCount();
}

bool rmProcessor::ChangeMatType(int ID,const char* typeName){
	F_DBG_INFO("ChangeMatType,ID,typeName["+to_string(ID)+","+oclStr(typeName)+"]");
	if(ID>=0){
		oclMaterialLibrary* lib;
		oclMaterial* m=m_pMaterialMgr->GetMaterial(ID,&lib);
		if(m){
			//TODO:这里通过SMaterialDescription的方式显得有些笨重；
			SMaterialDescription d=m->GetDescription();
			d.type=typeName;
         Json::Value tmp;
         tmp=GetTypeJson(d.type);
         tmp["matdefinitionName"]=d.definitionName;
			d.jsonString=tmp.toStyledString();
			d.texturePath="../../../Bin/texture/";
			if(m->Reset(d)){
				lib->Upload();
				m_pRender_actived->ResetFrameCount();
				return true;
			}
		}
	}
	return false;
}
 
/**
* 获取全局唯一ID的材质的JsonString，（不确定在哪个材质库中)
*/
string rmProcessor::GetMatBuff(int ID){
	F_DBG_INFO("GetMatBuff,ID["+to_string(ID)+"]");
	if(ID>=0){
		oclMaterialLibrary* lib;
		oclMaterial* m=m_pMaterialMgr->GetMaterial(ID,&lib);
		return m?m->GetDescription().jsonString:"";
	}
	return "";
}

// 给常规模型设置材质属性；
void rmProcessor::SetMaterial(int matIndex,OPENNP::Material* pMaterial){
	//TODO 给常规模型设置材质属性；
	oclInt i{10};
}

int rmProcessor::GetMatOffset(){
  F_DBG_INFO("GetMatOffset");
  return s_nNextAvaliableMaterialID;
}

#pragma endregion

#pragma region mesh
void rmProcessor::HideSelectMesh(int meshIndex,bool bShow,int nodeType){
  F_DBG_INFO("HideSelectMesh");
  m_pInsMgr->SetInstanceVisibility(meshIndex,bShow);
	m_pRender_actived->ResetFrameCount();
}

void rmProcessor::deleteObjBase(int index){
  F_DBG_INFO("deleteObjBase");
  if(index>=0){
		m_pInsMgr->DestoryInstance(index);
		m_pGeoMgr->DestroyGeometry(index);
	}
}

void rmProcessor::setCenter(float*center,int nodeIndex,int nodeType){
  F_DBG_INFO("setCenter");
  switch(nodeType){
	case TreeObject_ObjectLight:
	case TreeObject_ObjectLight_Distant:
	case TreeObject_ObjectLight_Point:
	case TreeObject_ObjectLight_Spot:
	case TreeObject_ObjectLight_Photometric:
	case TreeObject_ObjectLight_AreaCylinder:
	case TreeObject_ObjectLight_AreaDisc:
	case TreeObject_ObjectLight_AreaRectangle:
	case TreeObject_ObjectLight_AreaSphere:
	case TreeObject_ObjectLight_AreaDiscSpot:
	case TreeObject_ObjectLight_AreaRectangleSpot:
	case TreeObject_ObjectLight_EnvironmentProbe:
	{
		oclLightSource* l=m_pLightMgr->GetLight(nodeIndex);
    l->SetCenter(center);
	}
	break;
	case TreeObject_ClipPlane:
	case TreeObject_Group:
	case TreeObject_Surrounding:
	case TreeObject_Camera:
	case TreeObject_Model:
	case TREEOBJ_MASK:
	default:
	break;
	}
}

void rmProcessor::setEulerAngle(int index,float x,float y,float z,int nodeType){
  F_DBG_INFO("setEulerAngle");
  switch(nodeType){
	case TreeObject_Polygon:
	case TreeObject_ObjectLight:
	case TreeObject_ObjectLight_Distant:
	case TreeObject_ObjectLight_Point:
	case TreeObject_ObjectLight_Spot:
	case TreeObject_ObjectLight_Photometric:
	case TreeObject_ObjectLight_AreaCylinder:
	case TreeObject_ObjectLight_AreaDisc:
	case TreeObject_ObjectLight_AreaRectangle:
	case TreeObject_ObjectLight_AreaSphere:
	case TreeObject_ObjectLight_AreaDiscSpot:
	case TreeObject_ObjectLight_AreaRectangleSpot:
	case TreeObject_ObjectLight_EnvironmentProbe:
	{
		oclLightSource* l=m_pLightMgr->GetLight(index);
		l->SetEulerAngle(x,y,z);
	}
	break;
	case TreeObject_ClipPlane:
	case TreeObject_Surrounding:
	case TreeObject_Camera:
	case TreeObject_Model:
	case TREEOBJ_MASK:
	default:
	break;
	}
}

void rmProcessor::GetBindBox(int index,Vector3D& vMin,Vector3D& vMax){
  F_DBG_INFO("GetBindBox");
  oclMesh* mesh=static_cast<oclMesh*>(m_pGeoMgr->GetGeometry(index));
	if(mesh!=nullptr){
		mesh->GetAABB(vMin.x,vMin.y,vMin.z,vMax.x,vMax.y,vMax.z);
	}
}

void rmProcessor::writeMesh(int index,char* meshName,string& materialName,int& mateiralIndex
	,float* pInVex,float* normals,unsigned int* indices,float* textures,float* Tangents,float* Bitangents
	,float* maxbox,float* minbox,string& entityparams){
  F_DBG_INFO("writeMesh");
  oclMesh* mesh=static_cast<oclMesh*>(m_pGeoMgr->GetGeometry(index));
	if(mesh){
		strcpy(meshName,mesh->GetName().c_str());
		mateiralIndex=m_pInsMgr->GetInstance(index)->GetMaterialID();
		mesh->GetVertexData((oclFloat3*)pInVex);
		mesh->GetIndexData((oclUint3*)indices);
		mesh->GetNormalData((oclFloat3*)normals);
		mesh->GetUVData((oclFloat2*)textures);
		mesh->GetTangentData((oclFloat3*)Tangents);
		mesh->GetAABB(minbox[0],minbox[1],minbox[2],maxbox[0],maxbox[1],maxbox[2]);
	}
}

void rmProcessor::getMeshVertexIndexCount(int index,int& indexCount,int& vertexCount,int& meshNameLen){
  F_DBG_INFO("getMeshVertexIndexCount");
  oclMesh* mesh=static_cast<oclMesh*>(m_pGeoMgr->GetGeometry(index));
	if(mesh){
		indexCount=mesh->GetIndexCount();
		vertexCount=mesh->GetVertexCount();
		meshNameLen=strlen(mesh->GetName().c_str())+1;
	}
}

void rmProcessor::getVertexAndIndexOfMesh(int index,float* vertex,unsigned int* indices){
  F_DBG_INFO("getVertexAndIndexOfMesh");
  oclMesh* mesh=static_cast<oclMesh*>(m_pGeoMgr->GetGeometry(index));
	if(mesh){
		mesh->GetVertexData((oclFloat3*)vertex);
		mesh->GetIndexData((oclUint3*)indices);
	}
}

void rmProcessor::getMeshNormalAndTexture(int index,float* normals,float* textures){
  F_DBG_INFO("getMeshNormalAndTexture");
  oclMesh* mesh=static_cast<oclMesh*>(m_pGeoMgr->GetGeometry(index));
	if(mesh){
		mesh->GetNormalData((oclFloat3*)normals);
		mesh->GetUVData((oclFloat2*)textures);
	}
}

/**
* 更新UV坐标后，引擎传入的新坐标
*/
void rmProcessor::UpdateVecInfo(int index,int nVexSize,const float* pVex
	,const float* pNormal,const float* pUV,int nIndexSize,const unsigned int* pIndex){
	F_DBG_INFO("UpdateVecInfo,index,vertexSize,indexSize["+to_string(index)+","+to_string(nVexSize)+","+to_string(nIndexSize)+"]");
	oclMesh* mesh=static_cast<oclMesh*>(m_pGeoMgr->GetGeometry(index));
	if(mesh){
		mesh->SetVertexData(nVexSize,(oclFloat3*)pVex);
		mesh->SetNormalData(nVexSize,(oclFloat3*)pNormal);
		mesh->SetUVData(nVexSize,(oclFloat2*)pUV);
		mesh->SetIndexData(nIndexSize,(oclUint3*)pIndex);

		oclInstance* ins=m_pInsMgr->GetInstance(index);
		ins->SetAccelerationDirty();
		m_pInsMgr->SetAccelerationDirty();
		m_pRender_actived->ResetFrameCount();
	}
}

void rmProcessor::CopyNode(int srcNodeid,int newNodeId,TreeObject_Type nodeType){
	F_DBG_INFO("CopyNode");
	switch(nodeType){
	case TreeObject_Polygon:
	{
		//TODO:实现节点的拷贝功能；
		/*
		oclMesh* pmesh=m_pGeoMgr->CopyMesh(srcNodeid);
		if(pmesh){
			pmesh->SetNodeIndex(newNodeId);
			pmesh->UploadToGPU();
		}
		*/
	}
	break;
	case TreeObject_Group:
	case TreeObject_ObjectLight:
	case TreeObject_ObjectLight_Distant:
	case TreeObject_ObjectLight_Point:
	case TreeObject_ObjectLight_Spot:
	case TreeObject_ObjectLight_Photometric:
	case TreeObject_ObjectLight_AreaCylinder:
	case TreeObject_ObjectLight_AreaDisc:
	case TreeObject_ObjectLight_AreaRectangle:
	case TreeObject_ObjectLight_AreaSphere:
	case TreeObject_ObjectLight_AreaDiscSpot:
	case TreeObject_ObjectLight_AreaRectangleSpot:
	case TreeObject_ClipPlane:
	case TreeObject_Surrounding:
	case TreeObject_Camera:
	case TreeObject_Model:
	case TREEOBJ_MASK:
	default:
	break;
	}
}

void rmProcessor::setMeshRUVAndNormal(int index,float* textures,float* normals){
  F_DBG_INFO("setMeshRUVAndNormal");
  oclMesh* mesh=static_cast<oclMesh*>(m_pGeoMgr->GetGeometry(index));
	if(mesh){
		if(normals!=nullptr)
			mesh->UpdateNormalData((oclFloat3*)normals);
		if(textures!=nullptr)
			mesh->UpdateUVData((oclFloat2*)textures);
	}
}

bool rmProcessor::GetPickOutID(int x,int y,int& nOutID){
	nOutID=m_pMeshPicker->GetPickedMeshID(m_pRender_actived,x,y);
	F_DBG_INFO("GetPickOutID,x,y,outID["+to_string(x)+","+to_string(y)+","+to_string(nOutID)+"]");
	return true;
}

void rmProcessor::processMesh(bool isSurround,int surroundid
	,const char* meshName,const char* materialName,int nodeIndex
	,int ID,int iVertexCount,int indexCount,const float* pInVex,const float* normals
	,const unsigned int* indices,const float* textures,const float* Tangents
	,const float* Bitangent,float* worldMat,const float* maxbox,const float* minbox
	,int meshCount,int materialCount,const char* entityParams){
  F_DBG_INFO("processMesh");
  oclMaterialLibrary* lib=m_pMaterialMgr->GetLibrary(M_LIB_SCENE_ID);
	oclMaterial* m=lib->GetMaterial(ID);
	SMaterialProxy mproxy;
	if(m){
		m->SetDefinitionName(materialName);
		m->FetchProxy(&mproxy);
	} else{
		F_DBG_ERROR("Big Error, There is no OptixMaterial which has index of ??");
		F_DBG_ASSERT(false);
	}

	SGeometryCreateInfo info;
	info.type=EGeometryTypes::GEOMETRY_TYPE_MESH;
	info.name=meshName;
	info.ID=s_nNextAvaliableGeometryID++;

	oclMesh* mesh=static_cast<oclMesh*>(m_pGeoMgr->CreateGeometry(info));
	mesh->SetVertexData(iVertexCount,(oclFloat3*)pInVex);
	mesh->SetNormalData(iVertexCount,(oclFloat3*)normals);
	mesh->SetUVData(iVertexCount,(oclFloat2*)textures);
	mesh->SetTangentData(iVertexCount,(oclFloat3*)Tangents);
	mesh->SetBiTangentData(iVertexCount,(oclFloat3*)Bitangent);
	mesh->SetIndexData(indexCount,(oclUint3*)indices);
	mesh->SetAABB(minbox[0],minbox[1],minbox[2],maxbox[0],maxbox[1],maxbox[2]);
	SGeometryProxy proxy;
	mesh->FetchProxyInfo(&proxy);


	oclInstance* ins=m_pInsMgr->CreateInstance(info.ID);
	ins->SetWorldMatrix(worldMat);
	ins->SetNodeIndex(nodeIndex);
	ins->SetMaterialProxy(mproxy);
	ins->SetGeometryProxy(proxy);

	ins->Upload();

	m_mapNodeToObject[nodeIndex]={mesh,ins};
   //TODO::这个函数会被调用好多次；
   m_pLightMgr->Upload();
}

/**
* 返回即将要创建的mesh的ID（index）
*/
int rmProcessor::GetNewMeshIndex(){
  F_DBG_INFO("GetNewMeshIndex");
  return s_nNextAvaliableGeometryID;
}

int rmProcessor::getMeshCount(){
  F_DBG_INFO("getMeshCount");
  return s_nNextAvaliableGeometryID;
}
#pragma endregion

#pragma region misc
void rmProcessor::initObjBase(){
	if(m_bIsInited)return;
	m_bIsInited=true;
	F_DBG_INFO("initObjBase");

	/// 1. 创建通用缓冲区，并写入数据（通用缓冲区用在各个不同的渲染器上）；
	oclBufferManager* bufMgr=oclBufferManager::S();
	bufMgr->Init(ctx);
	m_bufferPrime=bufMgr->Create("buffer_primes",RT_BUFFER_INPUT,RT_FORMAT_INT);
	m_bufferPrime->SetSize(1000);
	m_bufferPrime->SetData(s_primes,1000);


	/// 2. 创建其他必要组件；
	oclProgramManager::S()->Init(ctx,"./ptx/");
	LoadMaterialConfigFile("./MatLibraryOptix/MatType.dat");
	m_pMaterialMgr=new oclMaterialManager(ctx);
	m_pGeoMgr=new oclGeometryManager();
	m_pInsMgr=new oclInstanceManager(ctx);
	m_pLightMgr=new oclLightManager(ctx);
	m_pMeshPicker=new oclMeshPicker(ctx);
	m_pCamera=new oclCameraPerspective();
	m_pCameraMaterialBall=new oclCameraPerspective();
	m_pCameraMaterialBall->UpdateCamera(
		optix::make_float3(0.f,0.f,3.f),optix::make_float3(0.f),optix::make_float3(0.f,1.f,0.f),45.f,45.f);


	/// 3.创建必要实例；
	// 渲染器专用材质库，不受引擎影响；
	SMaterialLibraryDescription d;
	d.libName="Internal";
	d.ID=M_LIB_INTERNAL_ID;
	oclMaterialLibrary* lib=m_pMaterialMgr->CreateLibrary(d);

	// 地面阴影材质；
	SMaterialDescription md;
	md.ID=M_MATERIAL_GROUND_ID;
	md.type=MATERIAL_SHADOW;
	md.jsonString=GetTypeJson(md.type).toStyledString();
	md.definitionName="inline_ground";
	oclMaterial* m=lib->CreateMaterial(md);
	lib->Upload();

   //UV材质创建
   md.ID=M_MATERIAL_UV;
   md.type=MATERIAL_UV;
   md.jsonString=GetTypeJson(md.type).toStyledString();
   md.definitionName="inline_UV";
   oclMaterial* UVMaterial=lib->CreateMaterial(md);
   lib->Upload();



	// 创建场景材质库；
	d.ID=M_LIB_SCENE_ID;
	d.libName="Scene";
	m_pMaterialMgr->CreateLibrary(d);

	// 设置地面几何体与材质；
	SMaterialProxy proxy;
	m->FetchProxy(&proxy);
	oclInstance* ins=m_pInsMgr->GetInstance_ground();
	ins->SetMaterialProxy(proxy);
	oclGeometryBase* plane=m_pGeoMgr->GetGroundPlane();
	SGeometryProxy gp;
	plane->FetchProxyInfo(&gp);
	ins->SetGeometryProxy(gp);
	ins->Upload();

	// 设置材质球几何体；
	oclGeometryBase* g=m_pGeoMgr->GetMaterialBall();
	g->FetchProxyInfo(&gp);
	ins=m_pInsMgr->GetInstance_materialBall();
	ins->SetGeometryProxy(gp);
	ins->SetMaterialProxy(proxy);
	ins->Upload();
	/// info:材质球实例的材质是在用户选中模型或者材质库中的材质后引擎通知渲染的，到时候会动态的赋予材质属性；


	/// 4. 创建渲染器；
	m_pRender_radiance=new oclRendererRadiance(ctx);
	m_pRender_radiance->Init();
	m_pRender_radiance->SetSkyboxTextureURL("./texture/env/Default.hdr");
	m_pRender_radiance->SetCamera(m_pCamera);
	m_pRender_radiance->SetTopGroup(m_pInsMgr->GetTopGroup_radiance());
	m_pRender_radiance->SetBufferID_lights(m_pLightMgr->GetBufferID_lights());
	m_pRender_AO=new oclRendererAO(ctx);
	m_pRender_AO->Init();
	m_pRender_AO->SetCamera(m_pCamera);
	m_pRender_AO->SetTopGroup(m_pInsMgr->GetTopGroup_radiance());
	m_pRender_AO->SetBufferID_lights(m_pLightMgr->GetBufferID_lights());
	m_pRender_depth=new oclRendererDepth(ctx);
	m_pRender_depth->Init();
	m_pRender_depth->SetCamera(m_pCamera);
	m_pRender_depth->SetTopGroup(m_pInsMgr->GetTopGroup_radiance());
	m_pRender_depth->SetBufferID_lights(m_pLightMgr->GetBufferID_lights());
	m_pRender_normal=new oclRendererNormal(ctx);
	m_pRender_normal->Init();
	m_pRender_normal->SetCamera(m_pCamera);
	m_pRender_normal->SetTopGroup(m_pInsMgr->GetTopGroup_radiance());
	m_pRender_normal->SetBufferID_lights(m_pLightMgr->GetBufferID_lights());
	m_pRender_materialBall=new oclRendererRadiance(ctx);
	m_pRender_materialBall->Init();
	m_pRender_materialBall->SetSkyboxTextureURL("./texture/env/Default.hdr");
	m_pRender_materialBall->SetCamera(m_pCameraMaterialBall);
	m_pRender_materialBall->SetTopGroup(m_pInsMgr->GetTopGroup_materialball());
	m_pRender_materialBall->SetBufferID_lights(m_pLightMgr->GetBufferID_materialBallLights());
	m_pRender_actived=m_pRender_radiance;
	m_vecRenderer_renderList.push_back(m_pRender_radiance);


	/// 5. 提交上下文数据到OptiX；
	OCL_TRY;
	dataCtx.bufID_prime=m_bufferPrime->GetID();
	dataCtx.picker_bufID=m_pMeshPicker->GetPickBufferID();
	OCL_CHECK_ERROR(rtVariableSetUserData(m_varContext,sizeof(SData_Context),&dataCtx));
	OCL_CATCH;


	/// 6. validate
	m_pRender_materialBall->SetRendererSize(100u,100u);
	m_pRender_materialBall->ResetFrameCount();
	m_pRender_materialBall->PreRender();
	m_pRender_materialBall->Upload();
	m_pRender_materialBall->Render();
	m_pRender_materialBall->PostRender();
}

bool rmProcessor::Render(void* hwnd){
	if(m_bFlagUploadMaterialData){
		// JC:引擎不通知渲染器数据何时结束，先用这种方式代替；
		m_bFlagUploadMaterialData=false;
		oclMaterialLibrary* lib=m_pMaterialMgr->GetLibrary(M_LIB_SCENE_ID);
		lib->Upload();
	}
	/// 渲染队列，用于支持多视口、材质球等渲染需求；
	for(auto it:m_vecRenderer_renderList){
		if(it->IsContinueToRender()){
			it->PreRender();
			it->Upload();
			it->Render();
			it->PostRender();
		}
	}
	return true;
}

void rmProcessor::SetOutputBufReSize(unsigned int width,unsigned int height,void* hwnd){
	oclUint w,h;
	w=m_pRender_actived->GetRendererWidth();
	h=m_pRender_actived->GetRendererHeight();
	if(w==width && h==height)return;
	F_DBG_INFO("resized to["+to_string(width)+","+to_string(height)+"]");

#if __OCL_INTERNAL_DEBUG__
	const oclUint hw=width*.5f;
	const oclUint hh=height*.5f;
	OCL_TRY;
	OCL_CHECK_ERROR(rtContextSetPrintLaunchIndex(ctx,hw,hh,0));
	OCL_CATCH;
#endif

	// TODO: 这里有个问题：场景在拖动的时候引擎会缩小宽高，停止释放还原，于是这个函数在不停的调用，缓冲区也是在不停的申请释放；
	m_pRender_radiance->SetRendererSize(width,height);
	m_pRender_AO->SetRendererSize(width,height);
	m_pRender_depth->SetRendererSize(width,height);
	m_pRender_normal->SetRendererSize(width,height);
	m_pRender_actived->ResetFrameCount();
}

void rmProcessor::getWidAndHei(unsigned int& width,unsigned int& height){
	F_DBG_INFO("getWidAndHei w,h["+to_string(width)+","+to_string(height)+"]");
	width=m_pRender_actived->GetRendererWidth();
	height=m_pRender_actived->GetRendererHeight();
}

void rmProcessor::UpdatePinholeCamera(Vector3D eye_,Vector3D lookat,Vector3D up_,float HFOV,float VFOV){
  Pos target,eye;
	Dir up;
	memcpy(&eye,&eye_,sizeof(oclFloat3));
	memcpy(&target,&lookat,sizeof(oclFloat3));
	memcpy(&up,&up_,sizeof(oclFloat3));
	if(m_pRender_actived->GetCamera()->UpdateCamera(eye,target,up,HFOV,VFOV)){
		m_pRender_actived->ResetFrameCount();
	}
}

/**
* 保存Scene材质库所有材质的纹理
* 环境贴图纹理；
* 光源纹理；
*/
void rmProcessor::SaveFile(RM_FileInfo targetPathInfo){
	F_DBG_INFO("SaveFile,URL["+targetPathInfo.m_absolutePath+"]");
	vector<const oclTexture*> vec;
	m_pMaterialMgr->GetLibrary(M_LIB_SCENE_ID)->FetchTextures(&vec);
	m_pRender_radiance->FetchTextures(&vec);
	m_pLightMgr->FetchTextures(&vec);
	if(vec.size()>0){
		/// 1. 目录/文件操作；
		boost::filesystem::path desFolder(targetPathInfo.m_absolutePath);
		desFolder.replace_extension("");
		if(!boost::filesystem::exists(desFolder)){
			boost::filesystem::create_directories(desFolder);
		}

		/// 2. 文件拷贝；
		//TODO:JC:这里有bug，如果纹理名相同，但在不同的目录，这样会覆盖掉同名文件；
		for(const oclTexture* t:vec){
			boost::filesystem::path from(t->GetURL());
			boost::filesystem::path to(desFolder);
			to/=t->GetFileNewName();
			if(!boost::filesystem::exists(to)){
				boost::filesystem::copy_file(from,to,boost::filesystem::copy_option::overwrite_if_exists);
			}
		}
	}
}

/**
* 加载我们自己的模型文件后，引擎调用该接口，告诉渲染器该文件的相关路径
*/
void rmProcessor::ImportFile(RM_FileInfo info){
	F_DBG_INFO("ImportFile,URL["+info.m_absolutePath+"]");
  m_openedModelFileInfo=info;
}

size_t rmProcessor::GetOutBufSize(){
	return m_pRender_actived->GetRendererSize();
}

unsigned int rmProcessor::GetElementSize(){ 
	return m_pRender_actived->GetRendererElementSize();
}

void* rmProcessor::OutPutMap(){
	return m_pRender_actived->MapOutputBuffer();
}

void rmProcessor::OutPutUnMap(){
	m_pRender_actived->UnmapOutputBuffer();
}

void rmProcessor::Init(){
	F_DBG_INFO("rmprocessor::Init() called.");
	setMeshIndex(0);
	oclMaterialLibrary* lib=m_pMaterialMgr->GetLibrary(M_LIB_SCENE_ID);
	lib->DestroyMaterials();
	m_pLightMgr->DestroyLights();
	m_pGeoMgr->DestroyGeometries();
	m_pInsMgr->DestroyInstances();
	m_pInsMgr->SetAccelerationDirty();
	m_mapNodeToObject.clear();
	s_nNextAvaliableGeometryID=0;
}

void rmProcessor::ReleaseObject(){ 
	F_DBG_INFO("ReleaseObject,转发给了Init");
	// TODO:JC:这个接口干嘛使得？或者Init（）干嘛使得？
	Init();
}

void rmProcessor::updateWorldMat(float* mat,int instanceIndex,int nodeType){
  F_DBG_INFO("updateWorldMat");
  switch(nodeType){
	case TreeObject_Polygon:
	{
		oclInstance* ins=m_pInsMgr->GetInstance(instanceIndex);
		if(ins){
			ins->SetWorldMatrix(mat);
			m_pInsMgr->SetAccelerationDirty();
		}
	}
	break;
	case TreeObject_ObjectLight:
	case TreeObject_ObjectLight_Distant:
	case TreeObject_ObjectLight_Point:
	case TreeObject_ObjectLight_Spot:
	case TreeObject_ObjectLight_Photometric:
	case TreeObject_ObjectLight_AreaCylinder:
	case TreeObject_ObjectLight_AreaDisc:
	case TreeObject_ObjectLight_AreaRectangle:
	case TreeObject_ObjectLight_AreaSphere:
	case TreeObject_ObjectLight_AreaDiscSpot:
	case TreeObject_ObjectLight_AreaRectangleSpot:
	case TreeObject_ObjectLight_EnvironmentProbe:
	{
		oclLightSource* l=m_pLightMgr->GetLight(instanceIndex);
		l->SetWorldMatrix(mat);
		m_pLightMgr->Upload();
	}
	break;
	case TreeObject_Group:
	case TreeObject_ClipPlane:
	case TreeObject_Surrounding:
	case TreeObject_Camera:
	case TreeObject_Model:
	case TREEOBJ_MASK:
	default:
	break;
	}
	m_pRender_actived->ResetFrameCount();
}

void rmProcessor::SetEnvProperty(const char* propName,VARIANT* v,int argCount,EnviromentType t){
  F_DBG_INFO("SetEnvProperty");
  oclStr name{propName};
	if(strcmp(name.substr(0,8).c_str(),"Renderer")==0){
		if(strcmp(propName,"RendererDepth")==0){
			m_pRender_actived=m_pRender_depth;
		} else if(strcmp(propName,"RendererAO")==0){
			m_pRender_actived=m_pRender_AO;
		} else if(strcmp(propName,"Renderer")==0){
			m_pRender_actived=m_pRender_radiance;
		} else if(strcmp(propName,"RendererNormal")==0){
			m_pRender_actived=m_pRender_normal;
		} else{
			F_DBG_ASSERT(false);
		}
		m_vecRenderer_renderList.clear();
		m_vecRenderer_renderList.push_back(m_pRender_actived);
		m_pRender_actived->GetCamera()->SetNeedRefreshCamera();

	} else{

		if(strcmp(propName,"EnableAO")==0){
			m_pRender_actived->SetEnableAO(oclBool(v->boolVal));
		} else if(
			strcmp(propName,"AOR_AORayDistanceMin")==0||
			strcmp(propName,"AORayDistanceMin")==0){
			m_pRender_actived->SetRayClampMinAO(v->fltVal);
		} else if(
			strcmp(propName,"AOR_AORayDistanceMax")==0||
			strcmp(propName,"AORayDistanceMax")==0){
			m_pRender_actived->SetRayClampMaxAO(v->fltVal);
		} else if(
			strcmp(propName,"AOR_RayDistanceMin")==0||
			strcmp(propName,"RayDistanceMin")==0){
			m_pRender_actived->SetRayClampMin(v->fltVal);
		} else if(
			strcmp(propName,"AOR_RayDistanceMax")==0||
			strcmp(propName,"RayDistanceMax")==0){
			m_pRender_actived->SetRayClampMax(v->fltVal);
		} else if(
			strcmp(propName,"SPP")==0||
			strcmp(propName,"DepthR_SPP")==0||
			strcmp(propName,"AOR_SPP")==0||
			strcmp(propName,"NorR_SPP")==0){
			m_pRender_actived->SetSamplesPerPixel(oclUint(v->fltVal));
		} else if(strcmp(propName,"EnableSkybox")==0){
			m_pRender_actived->SetEnableSkybox_show(oclBool(v->boolVal));
		} else if(strcmp(propName,"BadColor")==0){
			m_pRender_actived->SetBadColor(v[0].fltVal,v[1].fltVal,v[2].fltVal);
		} else if(strcmp(propName,"RayMaxDepth")==0){
			m_pRender_actived->SetIteratorDepth(oclUint(v->fltVal));
		} else if(strcmp(propName,"ImportanceThreshold")==0){
			m_pRender_actived->SetImportanceThreshold(oclFloat(v->fltVal));
		} else if(strcmp(propName,"EnableGI")==0){
			m_pRender_actived->SetEnableGI(oclBool(v->boolVal));
		} else if(strcmp(propName,"LensRadius")==0){
			m_pRender_actived->GetCamera()->SetLensRadius(oclFloat(v->fltVal));
		} else if(strcmp(propName,"FocalLength")==0){
			m_pRender_actived->GetCamera()->SetFocalLength(oclFloat(v->fltVal));
		} else if(strcmp(propName,"EnableDOF")==0){
			m_pRender_actived->GetCamera()->SetEnableDOF(oclBool(v->boolVal));
		} else if(strcmp(propName,"PostBlend")==0){
			m_pRender_radiance->SetBlend(oclFloat(v->fltVal));
		} else if(strcmp(propName,"PostExposure")==0){
			m_pRender_radiance->SetExposure(oclFloat(v->fltVal));
		} else if(strcmp(propName,"PostGamma")==0){
			m_pRender_radiance->SetGamma(oclFloat(v->fltVal));
		} else if(strcmp(propName,"EnableAIdenoise")==0){
			m_pRender_radiance->EnableDenoise(oclBool(v->boolVal));
		} else if(strcmp(propName,"EnvTexturePath")==0){
			oclChar* url=_com_util::ConvertBSTRToString(v->bstrVal);
			m_sEnvHDRFileURL=oclStr(url);
			m_pRender_radiance->SetSkyboxTextureURL(m_sEnvHDRFileURL);
		} else if(strcmp(propName,"DepthR_DepthMin")==0){
			m_pRender_depth->SetDepthMin(oclFloat(v->fltVal));
		} else if(strcmp(propName,"DepthR_DepthMax")==0){
			m_pRender_depth->SetDepthMax(oclFloat(v->fltVal));
		} else if(strcmp(propName,"NorR_CameraSpaceNormal")==0){
			m_pRender_normal->SetShowCameraSpaceNormal(oclBool(v->boolVal));


		} else if(
			strcmp(propName,"ShadowAttenuation")==0||
			strcmp(propName,"AOIntensity")==0){
			oclMaterialLibrary* lib=m_pMaterialMgr->GetLibrary(M_LIB_INTERNAL_ID);
			oclMaterial* m=lib->GetMaterial(M_MATERIAL_GROUND_ID);
			m->SetProperty(propName,v,1);
			lib->Upload();
		} else if(strcmp(propName,"EnableGround")==0){
			m_pInsMgr->SetGroundVisible(v->boolVal);
		}

	}
	m_pRender_actived->ResetFrameCount();

}

std::string rmProcessor::GetMatDefinitionName(int matIndex){
  F_DBG_INFO("GetMatDefinitionName");
  oclStr ss=getMaterialName(matIndex);
  F_DBG_INFO("DefiniationName,URL["+ss+"]");
  return getMaterialName(matIndex);
}

bool rmProcessor::CreateMatForVariant(int matlibIndex,int oldMatId,int& newMatIndex){
  F_DBG_INFO("CreateMatForVariant");
  if(matlibIndex==0){
    newMatIndex=oldMatId;
  } else{
    oclMaterialLibrary* lib;
    oclMaterial* m=m_pMaterialMgr->GetMaterial(oldMatId,&lib);
    if(lib){
      if(m){
        SMaterialDescription d=m->GetDescription();
        d.ID=s_nNextAvaliableMaterialID++;
        oclMaterial* newm=m_pMaterialMgr->GetLibrary(0)->CreateMaterial(d);
        newMatIndex=d.ID;
        return true;
      }
    }
    return false;
  }
  return true;
};

#pragma endregion

OCL_NAMESPACE_END

