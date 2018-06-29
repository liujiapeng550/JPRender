#pragma once

#include "IBaseProcess.h"
#include "RenderMath.h"
#include "core\oclcore.h"
#include "../../host_device/data_context.h"

OCL_NAMESPACE_START

class oclBuffer;
class oclTexture;
class oclMaterialManager;
class oclGeometryBase;
class oclMesh;
class oclGeometryManager;
class oclInstance;
class oclInstanceManager;
class oclLightManager;
class oclCameraBase;
class oclRenderer;
class oclRendererAO;
class oclRendererDepth;
class oclRendererRadiance;
class oclRendererNormal;
class oclMeshPicker;

class rmProcessor: public RenderMax::IBaseProcess{
  static rmProcessor* m_instance;
public:
  static IBaseProcess* getInstance(); 

  void ImportFile(RM_FileInfo info);
  void SaveFile(RM_FileInfo info);
  bool RenderAsync(){ return false; }
  bool RenderMatBall(){ return false; }
  bool RenderMaterialBall(int matIndex){ return false; }
  bool RenderMaterialLibBall(int matlibIndex,int& matlibItemIndex){ return false; }
  void RenderModelForMatBall(MeshData* meshData){}
  void LoadMatBallSceneData(MatBallSceneData* data){}
  void cancelRenderBall(){}
  void deleteSectionPlane(){}
  bool removeClip(int nNodeIndex){ return false; }
  void setMeshCount(int count){};
  void GetHitPos(float& x,float& y,float& z){}
  void GetHandleRayPoint(float* HTCHandleViveInfo){};
  void GetRootworld(float* rootworld){}
  int GetMeshNodeIndex(int meshIndex){ return 0; }
  void SetStereo(bool b){}
  void SetEyeDist(float dist){}
  void SetScreenDist(float dist){}
  bool GetStereo(){ return false; }
  float GetEyeDist(){ return 0.f; }
  float GetScreenDist(){ return 0.f; }
  bool GetVRMode(){ return false; }
  void setMeshIndex(int meshIndex){}
  int GetNewMeshIndex();
  void TestInterface(){}
  bool GetViewPort(void*hwnd,int&x,int&y,int&width,int&hight){ return false; }
  void SetNodeParams(int meshindex,NodeParams nodepar){}
  void GetNodeParams(int meshindex,NodeParams& nodepar){}
  void EngineOnIdle(){}
  std::string GetConsoleData(){ return ""; }
  void SetConsoleVal(std::string bug){}
  float GetTerrainSize(){ return 0.f; }
  bool RenderComplete(){ return false; };
  void LoadVegetationFile(bool isSurround,const char* meshName,const char* materialName,int nodeIndex,int mateiralIndex,int vertexSize,int indexCount,const float* pInVex,const float* normals,
                          const unsigned int* indices,const float* textures,const float* Tangents,const float* Bitangent,float* worldMat,const float* maxbox,const float* minbox,int meshCount,int materialCount,int nodeid){}
  void SetVegetationToolMode(int toolmode){}
  int GetVegetationToolMode(){ return 0; }
  std::shared_ptr<InstanceValueMap> VegetationButtonDown(Vector3D worldPoint,size_t instanceId,size_t nodeindex){
    return nullptr;
  };
  Vector3D VeiwToWorldTerrain(int x,int y){ return Vector3D(0.f); }
  void GetVegetationNodeBufferByNodeMeshid(int nodeid,int & indexCount,int & vertexCount,float ** vertex,unsigned int ** index,float ** normals,float ** textrues,int meshindex){}
  void MoveSelectedVegetation(const Vector3D & offset,bool bFollowTerrain){}
  void RotateSelectedVegetation(const Vector3D & rot){}
  void ScaleSelected(float fScale){}
  void SetSelectInstance(std::set<size_t> & selectedInstance){}
  void SetVegetationPosCallBack(void * callBack){}
  void SetVegetationRoteCallBack(void * callBack){}
  void SetVegetationScaleCallBack(void * callBack){}
  void SetVegetationRemoveCallBack(void * callBack){}
  float GetVegetationBrushRadius(){ return 0.f; }
  int GetMaterialIndexByNodeIndex(int nodeindex){ return 0; }
  void FinishProcessVegetationProcess(int nodeId){}
  EngineBlock * SaveVegetation(size_t & EngineBlockLength){ return nullptr; };
  void LoadVegetation(std::vector<int> & nodeIndex,EngineBlock * block,size_t EngineBlockLength){};
  void BeginRecordVegetationEvent(int eventType){};
  void EndRecordVegetationEvent(){};
  void VegetationUnDo(){};
  void VegetationReDo(){};
  void SetVegetationAddCallBack(void * callBack){};
  void SetVegetationHiddenByNodeId(size_t nodeid,bool hidden){};
  void DeleteSelectVegetationInstance(){};



  void RemoveNode(size_t nodeid){}
  void* GetVegetationValueChangerByNodeId(size_t nodeid){ return nullptr; }
  void SetVegetationBrushRadius(float radius){}
  void createSurroundLight(int surroundid,int lightIndex,const char* lightType){}
  void createSurroundAllLight(int surroundid,std::string buf,int offset){}
  //分配材质id
  int GetMatOffset();
  std::string GetMatDefinitionName(int matIndex);

  //初始化
  void initObjBase();

  //渲染操作
  bool RenderBegin(void* hwnd=nullptr){ return false; }
  bool Render(void* hwnd=nullptr);
  bool RenderEnd(void* hwnd=nullptr){ return false; }

  void RefreshAcceleration(){};
  //释放对象
  void ReleaseObject();

  void SetMaterial(int matIndex,OPENNP::Material* pMaterial);
  string GetAllMaterialBuf();
  void SetIsSplitScreen(bool b){};
	void GetMatballBuffer(int matID,int libID,int width,int height,void* pbuf);

#pragma region material
#pragma endregion

  string GetSceneMatList();
  string GetMatLibrary(int matlibIndex);
  int OpenMatLibrary(const char* path,bool isNew);
  bool SaveMatLibrary(const char* path,int matllibIndex);
  bool CloseMatLibrary(int matllibIndex);
  bool GetUVMatImagePath(std::string &imagepath,int meshindex){ return false; }
	bool GetMatShader(int matindex,const char* propName,int& shaderType){ return false; }
  bool ClearSceneUnusedMats(std::vector<int> usedmatindexs);
  bool DeleteMatItem(int tabindex,int itemindex);
  bool SetUserLevel(int matindex,int level){ return false; }
  string GetMatLibItem(int matlibIndex,int matitemIndex);
  void SetVideoTextureFilePath(std::string filePath){};
  void BegineEncodeVideoTexture(){};
  void EndEncodeVideoTexture(){};

  /**
  * 将材质赋给mesh
  * 如果assignMode是0（shape）或者1（group）的话，表示将matIndex对应的材质赋给meshes中的所有对应的mesh
  * 上面是针对具体的形状；
  * 如果assignMode是2（look）的话，就是将场景中所有与meshes使用相同的材质的mesh换成matIndex对应的材质；
  * 上面是针对ID相同的材质。
  *
  * _in_ meshes            mesh在引擎中的ID
  * _in_ matlibIndex       材质库ID
  * _in_ matIndex          材质ID
  * _in_ assignMode        模式
  * _ot_ NewMatIndex       新材质ID，需要返回给引擎；
  */
  bool AssignlookToSelection(vector<int> meshes,int matlibIndex,int matIndex,int assignMode,int& NewMatIndex);
  string GetMatBuff(int matIndex);

  void removeAllMesh(){}
  void removeSelectMesh(int index,bool bHide,int type){}

  //light method
  void removeLight(unsigned int lightIndex){};
  void createLight(int lightIndex,const char* lightType);
  void setLightProperty(unsigned int lightIndex,const char* propName,VARIANT* v,int count);
  void setLightPreDelete(int,bool){}
  void ImportLightResources(){}
  string getLightBuffer(unsigned int lightIndex);
  string getLightBuffer(std::vector<int> nodeIndexes);
  string getAllLightBuffer(int saveMode);
  void createAllLight(string buf,int offset);
  void deleteAllLight();
  void getLightProperty(unsigned int lightIndex,const char* propName,float* v);
  bool getLightProperty(unsigned int lightIndex,const char* propName,VARIANT* v,int count){ return true; };
  void setLightAllProperty(int index,string json){};
  bool setEnvPath(string path){ return false; };
  void SetEnvProperty(const char* propName,VARIANT* v,int argCount,EnviromentType t);
	void GetEnvProperty(const char* propname,VARIANT* v,int count,EnviromentType t){};
  void GetEnvironmentJsonData(VARIANT* v,EnviromentType t){}
  void UpdateVecInfo(int index,int nVexSize,const float* pVex,const float* pNormal,const float* pUV,int nIndexSize,const unsigned int* pIndex);

  //mesh相关操作:
  // When a node in scene copied ,the corresponding entity which was bound to that node will copy too.
  void CopyNode(int srcNodeid,int newNodeId,TreeObject_Type nodeType);


  void setMeshRUVAndNormal(int index,float* textures,float* normals); //设置mesh的纹理和法线坐标

  void processMesh(bool isSurround,int surroundID,const char* meshName,const char* materialName,int nodeIndex,int mateiralIndex,int vertexSize,int indexCount,const float* pInVex,const float* normals,
                   const unsigned int* indices,const float* textures,const float* Tangents,const float* Bitangent,float* worldMat,const float* maxbox,const float* minbox,int meshCount,int materialCount,const char* entityParams);//从模型读取mesh

                                                                                                                                                                                                                                    // To achieve mesh data.
  void writeMesh(int index,char* meshName,string& materialName,int& mateiralIndex,float* pInVex,float* normals,
                 unsigned int* indices,float* textures,float* Tangents,float* Bitangents,float* maxbox,float* minbox,std::string& entityparams);//从optix读取mesh


  void getMeshVertexIndexCount(int i,int& indexCount,int& vertexCount,int& meshNameLen);//获得mesh的索引顶点和名字的长度

  void getMeshNormalAndTexture(int index,float* normals,float* textures); //获得mesh的法线和索引

  void getVertexAndIndexOfMesh(int index,float* vertex,unsigned int* indices);//获取mesh的顶点和索引数据

  int getMeshCount();//获得mesh数目

  void updateWorldMat(float* mat,int instanceIndex,int nodeType);//更新mesh的数据矩阵
  void setCenter(float*center,int nodeIndex,int nodeType);
  void setCenter(int,float,float,float){}
  void setEulerAngle(int index,float x,float y,float z,int nodeType);
  void Init();//删除所有mesh

  void deleteObjBase(int key);//mesh管理器清除对应mesh

  void GetBindBox(int nInstanceIndex,Vector3D& vMin,Vector3D& vMax);
  void UpdateBindBox(Vector3D vMin,Vector3D vMax){};
  virtual RenderAABBV3& GetLocalBBOX(int index){
    RenderAABBV3 aa;
    return aa;
  };
  virtual RenderAABBV3& GetWorldBBOX(int index){
    RenderAABBV3 aa;
    return aa;
  };
  virtual RenderAABBV3& GetSceneBBOX(){
    RenderAABBV3 aa;
    return aa;
  };

  RenderAABBV3& getAABB(){
    RenderAABBV3 aa;
    return aa;
  }
  void setGroundPositionMin(){};
  void updateAABB(){};
  void updateAABB(RenderAABBV3 aabb,int index){}

  //计算mesh包围盒的各个轴向的最小值来设置地面（暂时只用来设置地面，后续可能有别的用处）
  bool calWorldMinXYZ(float& minX,float& minY,float& minZ){ return true; };
  bool calWorldMaxXYZ(float& maxX,float& maxY,float& maxZ){ return true; };
  bool getAABBLength(float& Length){ return true; };

  //surround操作
  void removeSurroundChild(int surroundid){}
  void HideSurroundLight(int surroundid,bool bShow){}
  void HideSurroundMesh(int surroundid,bool bHide){}
  void deleteSurroundLight(int surroundid){}
  int getSurroundInstanceCount(){ return 0; };//获取surround的mesh数量
  void setBackScene(int index,bool isShow){}
  void setAllBackScene(bool isShow){}

	void ShowUnSelectMesh(int* index,int size,bool bShow){};

  // Set visibility of selected mesh;
  // If bShow is false ,then hide;
  // other wise;
  //z_spark: 建议改为SetSelectedMeshVisibility(int meshIndex,bool b,int nodeType);
  void HideSelectMesh(int meshIndex,bool bShow,int nodeType);

  void CreateOutPutBuf(unsigned int/*glvboid*/,unsigned int/*width*/,unsigned int/*height*/){};
  void SetOutputBufReSize(unsigned int/*width*/,unsigned int/*height*/,void* hwnd=nullptr);
  void UnregisterGLBuffer(){};
  void RegisterGLBuffer(){};
  unsigned int/*glvboid*/GetGLBOId(){ return 0; }
  size_t GetOutBufSize();
  unsigned int GetElementSize();
  unsigned char * GetImageBuf(){ throw"这个方法停用了"; return nullptr; };
  void* OutPutMap() override;
  void OutPutUnMap() override;

  void AddLayer(unsigned int layerType){};
  void RemoveLayer(unsigned int layerType){};
  void * GetD3DDevice(){ return nullptr; };
  void * GetD3DDeviceContext(){ return nullptr; };

  void SetMatBallOutputBufSize(unsigned int/*width*/,unsigned int/*height*/){};
  void* OutPutMatBallMap(){ return nullptr; };
  void* OutPutLayerBuffer(unsigned int layerType,unsigned int quality=100){ return nullptr; };

#pragma region Camera
  bool GetPickOutID(int x,int y,int& nOutID);
  void getWidAndHei(unsigned int& width,unsigned int& height);
  void TurnTable(float angle,bool direction){}

  // Set main camera's data according to current mesh.
  // This fn. must be called before getCameraInfo().
  bool processSurroundCamera(){ return false; };
  void getCameraInfo(Vector3D& eye,Vector3D& lookat,Vector3D& up,float& fov){};
  void UpdatePinholeCamera(Vector3D eye,Vector3D lookat,Vector3D up,float HFOV,float VFOV);
  void GetOrthographicWith(float &with){};
  void SetOrthographicWith(float with){};
  void SetCameraType(CameraType camtype){};
#pragma endregion

public:
  void processClipPlane(int nNodeIndex,float* pMat,bool bLight){};
  void ChangeClipLight(int nInstanceIndex,bool bLight){};
  bool GetClipLight(int nInstanceIndex){ return false; };

public:
  void SetRenderLevel(int nLevel){};
  int GetRenderLevel(){ return 0; }
  void SetRayDepthReflect(int type,int value){};
	void SetTexturePackagePath(string path){};
	void SetTexturePackage(int saveMode){};
  void HideSelectLight(int index,bool bShow);

#pragma region Material
public:

  int CreateMaterial(unsigned int matlibIndex,int *matlibItemIndex);
  void CreateMaterial(int index,OPENNP::Material* pMat);
  int GetSceneMatCount(); //获取材质数量
  string getMaterialName(int index); //根据材质索引号查询对应材质名字
  void destroySceneMaterial(); //释放场景中的材质
  void SetMatToBall(int matIndex){};

  bool SetMatProperty(int matIndex,const char* propName,VARIANT* v,int count);
  bool GetMatProperty(int meshIndex,const char* propName,VARIANT* v,int count);//撤销操做调用，获取之前的材质参数
  bool GetMatProperty(int matIndex,const char* propName,float* pv){ return false; }
  bool SetTextureSpaceProp(int meshIndex,const char* propName,VARIANT* v){ return false; };
  bool SetTextureSpaceType(int meshIndex,string tsm){ return false; };
  string GetMatTextureSpaceBuff(int meshIndex){ return ""; };

	/**
	* 读入已保存模型时调用
	*/
  void setMaterialRes(std::string jsonbuffs,std::map<std::string,int>& fileGuidIndexMap,std::map<std::string,int>&allGuidIndexMa);
  string GetMatTypes();
  std::string GetMatType(int matIndex){ return ""; };
  bool ChangeMatType(int matIndex,const char* typeName);

  string GetMatlibList(int matlibIndex){ return ""; }

  int GetMatCounts();

  //void SetNewMaterialRes(int meshIndex, const char* typeName);
  //void SetSceneMaterial(int meshIndex, int materialIndex);
  std::string MakeUniqMatName(std::string srcName){ return ""; };
  std::string GetMatUniqueName(int matIndex){ return ""; };

  bool SetMatDefinitionName(int matIndex,const char*  matName);
  string GetDefinitionName(int matIndex){ return ""; };

  int GetMatIndex(int meshIndex);

  bool AssignlookToSingleMesh(int meshindex,int matindex);
  bool AssignlookToMat(int oldmatindex,int newmatindex,int& outmatindex,bool beCopy){ return false; }

  int GetMatLibIndex(int matindex){ return -1; };
  void CancelRender(){};
  void SetViewMatCallBack(std::function<void(float[3],float[3],float[3],float)>){};
  void UnLockBuffer(){};
  bool ImageRender(Vector3D& eye,Vector3D& lookat,Vector3D& up,float& HFOV,float& VFOV){ return false; };

  bool ChangeMatShader(int matindex,const char* propName,int type){ return false; };
#pragma endregion


  //HsessionView
public:
  string GetHsessionViewOptionsBuff(){ return ""; };
  bool GetHsessionViewOptionsBuff(VARIANT *v){ return false; };
  bool SetHsessionViewOption(const char* propertyName,VARIANT* v){ return false; };
  void SetAllHsessionViewOption(string jsonbuff){};

  bool ExportCurrentRenderLayer(const string& fileName,const unsigned int layerType=0,unsigned int quality=100){
    return true;
  };


  //Environment
public:
  string GetEnvironmentOptionsBuff(){ return ""; };
  bool SetEnvironmentType(const char* EnvType){ return false; };
  bool GetEnvironmentOptionsBuff(VARIANT *v){ return false; };
  bool SetEnvironmentOption(const char* propertyName,VARIANT* v){ return false; };
  bool GetEnvFunctionBuff(VARIANT *v){ return false; };
  bool SetEnvFunction(const char* propertyName,VARIANT* v){ return false; };
  bool GetBackplateOptionsBuff(VARIANT *v){ return false; };
  bool SetBackplateOption(const char* propertyName,VARIANT* v){ return false; };
  bool GetToneMapperOptionsBuff(VARIANT *v){ return false; };
  bool SetToneMapperOption(const char* propertyName,VARIANT* v){ return false; };
  void SetAllEnvironmentOption(string jsonbuff){};

  string GetEnvFunctionBuff(){ return ""; };
  bool SetEnvFunctionType(const char* type){ return false; };
  void SetAllEnvFunOption(string jsonbuff){};

  //set baclpate
  string GetBackplateOptionsBuff(){ return ""; };
  void SetBackplateType(const char* EnvType){};
  void SetAllBackplateOption(string jsonbuff){};

  //set ToneMapper
  string GetToneMapperOptionsBuff(){ return ""; };
  void SetToneMapperType(string m_ToneMapper){};
  void SetAllToneMapperOption(string jsonbuff){};

  void ExportScene(){};		//set ToneMapper
  bool CreateMatForVariant(int mat,int oldMatId,int& newMatIndex);
  bool TestConsoleVar_RT(const char* param,int value){ return false; };
	bool GetConsoleVar_RT(const char* param,VARIANT& value){ return false; };
  string MatIndex2Id(std::string oldid){ return " "; };
  void ImportResources(const char*){};
  std::list<int> GetlightShowList(){ std::list<int> a; return a; }
  void SetFarPlane(float t_far){};
  void SetNearPlane(float t_near){};
  void * GetFactory(){ return nullptr; }
  void GetHandleRayPoint(float& src_x,float& src_y,float& src_z,float& dir_x,float& dir_y,float& dir_z,float& quat_x,float&quat_y,float&quat_z,float&quat_w){};
  void GetPickOutIDByHTCHandle(Vector3D srcPos,Vector3D dir,bool bNetLeader,Vector3D& t_hitPos,int& nOutID){};
  void SetHTCNetPeoplePos(int SelfClientID,HTC_PosMessage* t_mess,bool Allshow){};
  void SetHTCHandle(Vector3D pos,Vector3D up,Vector3D dir,bool isRight){}
  void SetHTCHandle(Vector3D Lpos,Vector3D Lup,Vector3D Ldir,Vector3D Rpos,Vector3D Rup,Vector3D Rdir){}
  void SetHTCHandle(bool isShow,bool){};
  void SetHTCHandlePosOffset(float x,float y,float z,bool isMeta){};
  void SetHTCCameraOffset(float x,float y,float z,bool isMeta){}
  void SetHTCCameraScale(float t_scale){ return; }
  void GetHtcHandleViveMat(float* mat){}
  void SetHTCVive(bool b,bool isMeta){}
  void SetHTCSelectFeedback(){};
  void SetHTCNetPeoplePos(int NetID,Vector3D pos,QuaternionD qua,bool show,bool Allshow){};
  void SetHTCLeftHandleRayMat(Vector3D t_pos){};
  void GetHTCCameraInfo(int t_eye,float& t_fov,float& eye_x,float& eye_y,float& eye_z,
                        float& up_x,float& up_y,float& up_z,
                        float& dir_x,float& dir_y,float& dir_z,float& t_modulatedHalfIpd,float& aspectRatio,float& transfAsymH,float& transfAsymV){}
  std::map<int,float> GetHTCHandleButtonState(){
    std::map<int,float> tmp;
    return tmp;
  }
#pragma region MaterialCollections
  bool AssignCollection(int matlibindex,int* matitems,int num,const char* collection,bool isadd){ return 0; }
  bool ModifyCollection(int matlibindex,const char* oldname,const char* newname){ return 0; }
  bool RemoveCollection(int matlibindex,const char* name){ return 0; }
  bool AddCollection(int matlibindex,const char* name){ return 0; }
#pragma endregion MaterialCollections
  void GetPickOutIDByHTCHandle(float srcx,float srcy,float srcz,float dirx,float diry,float dirz,int& nOutID){};
  void GetSuperSampling(int& t_supersampling){};
  void SetNormalSaveSign(bool bSave){};
  void SetOperateHandleShow(bool isShow){}
  void SetOperateHandle(float pos_x,float pos_y,float pos_z,float up_x,float up_y,float up_z,float dir_x,float dir_y,float dir_z){}
  bool CopyMatItem(int srcMat,int desMatlib,int& newItemIndex);
  void SetActiveStereo(bool b){}
  bool LoadActiveStereoParam(){ return false; }
  void GetWorldRay(float* srcRay,float* rayDir){}
private:
  rmProcessor();
  ~rmProcessor();

  RTcontext ctx=0;
  oclBuffer* m_bufferPrime;
  RTvariable m_varContext;

  oclMaterialManager* m_pMaterialMgr=nullptr;
  oclGeometryManager* m_pGeoMgr=nullptr;
	oclInstanceManager* m_pInsMgr=nullptr;
  oclLightManager* m_pLightMgr;
  oclMeshPicker* m_pMeshPicker=nullptr;
	oclCameraBase* m_pCamera=nullptr;
	oclCameraBase* m_pCameraMaterialBall=nullptr;

  oclStr m_sEnvHDRFileURL="./texture/env/Default.hdr";
  oclBool m_bIsInited=false;
  SData_Context dataCtx;
	RM_FileInfo m_openedModelFileInfo="";
  std::vector<oclRenderer*> m_vecRenderer_renderList;
  oclRenderer* m_pRender_actived;
  oclRendererRadiance* m_pRender_radiance=0;
  oclRendererAO* m_pRender_AO=0;
  oclRendererDepth* m_pRender_depth=0;
  oclRendererNormal* m_pRender_normal=0;
  oclRendererRadiance* m_pRender_materialBall=0;
	std::map<int,std::pair<oclGeometryBase*,oclInstance*>> m_mapNodeToObject;
	oclBool m_bFlagUploadMaterialData=false;
};

OCL_NAMESPACE_END
