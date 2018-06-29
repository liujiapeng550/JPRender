#ifndef VIRTUALBASEPROCESS_H_
#define VIRTUALBASEPROCESS_H_
#include "RenderMath.h"
#include <OAIdl.h>
#include <functional>
#include <set>
#include "VegetationValueChanger.h"
#include "DataType.h"
#include <list>
//#include "ITmaxSystem.h"

namespace OPENNP{
class OPENFuncs;
struct Material;
}
typedef std::function<void(int nodeIndex, int meshIndex)> NotifyMeshIndexChange;
namespace RenderMax {
	class IBaseProcess {
	public:
		virtual ~IBaseProcess() {};

		//virtual bool CreateViewport(void* pHwnd) { return false; }
		//初始化
		virtual void ImportFile(RM_FileInfo) = 0;
		virtual void ImportResources(const char*) = 0;
		virtual void SaveFile(RM_FileInfo) = 0;
		virtual void initObjBase() = 0;

		//渲染操作
		virtual bool RenderAsync() = 0;
		virtual bool RenderMatBall() = 0;

		//释放对象
		virtual void ReleaseObject() = 0;
		virtual void removeAllMesh() = 0;

		//light method
		virtual void removeLight(unsigned int lightIndex) = 0;
		virtual void createLight(int lightIndex, const char* lightType) = 0;
		virtual void setLightProperty(unsigned int lightIndex, const char* propName, VARIANT* v, int count) = 0;
		virtual void getLightProperty(unsigned int lightIndex, const char* propName, float*) = 0;
		virtual bool getLightProperty(unsigned int lightIndex, const char* propName, VARIANT* v, int count) = 0;

		virtual void setLightPreDelete(int, bool) = 0;
		virtual std::list<int> GetlightShowList() = 0;
		virtual std::string getLightBuffer(unsigned int lightIndex) = 0;
		virtual std::string getLightBuffer(std::vector<int>) = 0; //此函数应改为纯虚函数
		virtual std::string getAllLightBuffer(int saveMode) = 0;
		virtual void createAllLight(std::string buf, int offset) = 0;
		virtual void setLightAllProperty(int index, std::string json) = 0;
		virtual void setEulerAngle(int, float, float, float, int nodeType) = 0;
		//virtual void ImportLightResources() = 0;
		//set the path of the environment
		virtual bool setEnvPath(std::string path) = 0;
		virtual void createSurroundLight(int surroundid, int lightIndex, const char* lightType) = 0;
		virtual void createSurroundAllLight(int surroundid, std::string buf, int offset) = 0;
		virtual void SetViewProMat(UINT* pData) {}
		//clip
		virtual void deleteSectionPlane() = 0;
		virtual bool removeClip(int nNodeIndex) = 0;

		//mesh相关操作:
		//设置mesh的纹理和法线坐标
		virtual void setMeshRUVAndNormal(int index, float* textures, float* normals) = 0;

		//更新mesh的纹理和法线坐标和顶点
		virtual void UpdateVecInfo(int index, int nVexSize, const float* pVex, const float* pNormal, const float* pUV, int nIndexSize, const unsigned int* pIndex) = 0;
		virtual void CopyNode(int srcNodeid, int newNodeId, TreeObject_Type nodeType) = 0;
		virtual void UpdateMeshForCutPlane(int meshIndex, int nVexSize, int nIndexSize, const VERTEX_P3F_CU_T2F* vers, const float* normals, const unsigned int* indices) {}
		virtual void getMeshDataForCutPlane(int meshIndex, VERTEX_P3F_CU_T2F* vers, float* normals, unsigned int* indices) {}

		virtual void InitMatballRenderWnd(void * hWnd) {}
		virtual void GetMatballBuffer(int matID, int libID, int width, int height, void* pbuf) {}
		virtual void GetSpecularMapBuffer(void* pbuf) {}
		virtual void GetNormalMapBuffer(void* pbuf) {}
		/**
		* @param[in] meshName              mesh名字
		* @param[in] materialName          材质名字
		* @param[in] nodeIndex             节点索引
		* @param[in] mateiralIndex         材质索引
		* @param[in] iVertexCount          顶点数量
		* @param[in] indexCount            索引数量
		* @param[in] pInVex                顶点数据
		* @param[in] normals               法线数据
		* @param[in] indices               索引数据
		* @param[in] textures              纹理数据
		* @param[in] Tangents              切线数据
		* @param[in] Bitangent             并未使用
		* @param[in] worldMat              世界矩阵数据
		* @param[in] maxbox                包围盒右上角x,y,z数据
		* @param[in] minbox                包围盒左上角x,y,z数据
		* @param[in] meshCount             mesh数量
		* @param[in] materialCount         材质数量
		*/
		virtual void processMesh(bool isSurround,int surroundid, const char* meshName, const char* materialName, int nodeIndex, int mateiralIndex, int vertexSize, int indexCount, const float* pInVex, const float* normals,
			const unsigned int* indices, const float* textures, const float* Tangents, const float* Bitangent, float* worldMat, const float* maxbox, const float* minbox, int meshCount, int materialCount, const char* entityParams) = 0;//从模型读取mesh

		virtual void writeMesh(int index, char* meshName, std::string& materialName, int& mateiralIndex, float* pInVex, float* normals,
			unsigned int* indices, float* textures, float* Tangents, float* Bitangents, float* maxbox, float* minbox, std::string& entityparams) = 0;//从optix读取mesh

		virtual void getMeshVertexIndexCount(int i, int& indexCount, int& vertexCount, int& meshNameLen) = 0;//获得mesh的索引顶点和名字的长度

		virtual void getMeshNormalAndTexture(int index, float* normals, float* textures) = 0; //获得mesh的法线和索引

		virtual void getVertexAndIndexOfMesh(int index, float* vertex, unsigned int* indices) = 0;//获取mesh的顶点和索引数据

		virtual int getMeshCount() = 0;//获得mesh数目
		virtual void setMeshCount(int count) = 0;
		virtual void updateWorldMat(float* mat, int instanceIndex, int nodeType) = 0;//更新mesh的数据矩阵
		virtual void setCenter(float*, int, int) = 0;
		virtual void setCenter(int, float, float, float) = 0;;
		virtual void Init() = 0;//重置场景中的物体

		virtual void deleteObjBase(int key) = 0;//mesh管理器清除对应mesh

		virtual void ShowUnSelectMesh(int* index, int size, bool bShow) = 0;
		virtual void HideSelectMesh(int index, bool bHide, int type) = 0;
		virtual void HideSelectLight(int index, bool bShow) = 0;

		//surround操作
		virtual void removeSurroundChild(int surroundid)=0;//删除optix中对应的mesh数据
		virtual int getSurroundInstanceCount()=0;//获取surround的mesh数量
		virtual void setBackScene(int index,bool isShow)=0;
		virtual void setAllBackScene(bool isShow)=0;
		virtual bool processSurroundCamera()=0;
		virtual void HideSurroundLight(int surroundid, bool bShow) = 0;
		virtual void HideSurroundMesh(int surroundid, bool bHide) = 0;
		virtual void deleteSurroundLight(int surroundid) = 0;

		//opengl调用		
		//获取输出buf大小
		virtual size_t GetOutBufSize() = 0;

		//创建输出buf
		virtual void CreateOutPutBuf(unsigned int/*glvboid*/,
			unsigned int/*width*/, unsigned int/*height*/) = 0;

		//重置输出buf大小
		virtual void SetOutputBufReSize(unsigned int/*width*/, unsigned int/*height*/, void* hwnd =nullptr) = 0;
		virtual void SetMatBallOutputBufSize(unsigned int/*width*/, unsigned int/*height*/) = 0;

		//解注册GLBUF
		virtual void UnregisterGLBuffer() = 0;

		//获取GLID
		virtual unsigned int/*glvboid*/GetGLBOId() = 0;

		//获取大小
		virtual unsigned int GetElementSize() = 0;

		//注册GLBUF
		virtual void RegisterGLBuffer() = 0;

		virtual void* OutPutMap() = 0;
		virtual void OutPutUnMap() = 0;
		virtual void* OutPutMatBallMap() = 0;
		virtual void* OutPutLayerBuffer(unsigned int layerType, unsigned int quality = 100) = 0;
		virtual void AddLayer(unsigned int layerType) = 0;
		virtual void RemoveLayer(unsigned int layerType) = 0;

		//CameraInfo摄像头操作
		//get pickout ID
		virtual bool GetPickOutID(int x, int y, int& nOutID) = 0;
		virtual void GetHitPos(float& x, float& y, float& z) = 0;
		virtual void GetWorldRay(float* srcRay, float* rayDir) = 0;
		//htc handle
		virtual void GetHandleRayPoint(float* HTCHandleViveInfo) = 0;
		virtual std::map<int, float> GetHTCHandleButtonState() = 0;
		virtual void GetPickOutIDByHTCHandle(Vector3D srcPos, Vector3D dir, bool bNetLeader, Vector3D& t_hitPos, int& nOutID) = 0;
		virtual void SetOperateHandleShow(bool isShow) = 0;
		virtual void SetHTCHandle(Vector3D pos, Vector3D up, Vector3D dir, bool isRight) = 0;
		virtual void SetHTCHandle(Vector3D Lpos, Vector3D Lup, Vector3D Ldir, Vector3D Rpos, Vector3D Rup, Vector3D Rdir) = 0;
		virtual void SetHTCHandle(bool isShow, bool MetaHandIsShow) =0;
		virtual void SetHTCHandlePosOffset(float x, float y, float z,bool isMeta) = 0;
		virtual void SetHTCCameraOffset(float x, float y, float z,bool isMeta) = 0;
		virtual void SetHTCCameraScale(float t_scale) = 0;
		virtual void GetHtcHandleViveMat(float* mat) = 0;
		virtual void SetHTCVive(bool b,bool isMeta) = 0;
		virtual void SetIsSplitScreen(bool b) = 0;
		virtual void GetHTCCameraInfo(int t_eye, float& t_fov, float& eye_x, float& eye_y, float& eye_z,
			float& up_x, float& up_y, float& up_z,
			float& dir_x, float& dir_y, float& dir_z, float& t_modulatedHalfIpd, float& aspectRatio, float& transfAsymH, float& transfAsymV) = 0;
		virtual void SetHTCSelectFeedback() = 0;
		virtual void SetHTCNetPeoplePos(int SelfClientID, HTC_PosMessage* t_mess, bool Allshow) = 0;
		virtual void SetHTCLeftHandleRayMat(Vector3D t_pos) = 0;
		//htc end

		//获取相机信息
		virtual void getCameraInfo(Vector3D& eye, Vector3D& lookat, Vector3D& up, float& fov) = 0;
		virtual	void UpdatePinholeCamera(Vector3D eye, Vector3D lookat, Vector3D up, float HFOV, float VFOV) = 0;
		virtual void GetOrthographicWith(float &with) = 0;
		virtual	void SetOrthographicWith(float with) = 0;
		virtual	void SetCameraType(CameraType camtype) = 0;
		virtual void UpdateBindBox(Vector3D vMin, Vector3D vMax) = 0;
		virtual RenderAABBV3& GetLocalBBOX(int index) = 0;
		virtual RenderAABBV3& GetWorldBBOX(int index) = 0;
		virtual RenderAABBV3& GetSceneBBOX() = 0;

		virtual void updateAABB() = 0;
		virtual void updateAABB(RenderAABBV3 aabb, int index) = 0;
		virtual void GetRootworld(float* rootworld) = 0;
		virtual int GetMeshNodeIndex(int meshIndex) = 0;
		virtual void SetStereo(bool b) = 0;
		virtual void SetActiveStereo(bool b) = 0;
		virtual void SetEyeDist(float dist) = 0;
		virtual void SetScreenDist(float dist) = 0;
		virtual bool GetStereo() = 0;
		virtual float GetEyeDist() = 0;
		virtual float GetScreenDist() = 0;
		virtual bool GetVRMode() = 0;
		virtual void SetFarPlane(float t_far) = 0;
		virtual void SetNearPlane(float t_near) = 0;
		virtual void setGroundPositionMin() = 0;
		virtual void GetSuperSampling(int& t_supersampling) = 0;

		//计算mesh包围盒的各个轴向的最小值来设置地面（暂时只用来设置地面，后续可能有别的用处）
		virtual bool calWorldMinXYZ(float& minX, float& minY, float& minZ) = 0;
		virtual bool calWorldMaxXYZ(float& maxX, float& maxY, float& maxZ) = 0;
		virtual bool getAABBLength(float& Length) = 0;

		virtual void setMeshIndex(int meshIndex) = 0;
		//获取新的可用meshIndex
		virtual int GetNewMeshIndex() = 0;
		virtual void setMeshChangeCallBack(NotifyMeshIndexChange func) { this->meshIndexChange = func;}

		virtual void processClipPlane(int nNodeIndex, float* pMat, bool bLight) = 0;
		virtual void ChangeClipLight(int nInstanceIndex, bool bLight) = 0;
		virtual bool GetClipLight(int nInstanceIndex) = 0;


		virtual void SetRenderLevel(int nLevel) = 0;//0-2默认是0
		virtual void  SetRayDepthReflect(int type, int value) = 0;
		virtual int GetRenderLevel() = 0;//0-2默认是0
		virtual void SetTexturePackagePath(std::string path) = 0;
		virtual void SetTexturePackage(int saveMode) = 0;
		//HsessionView

		virtual	std::string GetHsessionViewOptionsBuff() = 0;
		virtual	bool GetHsessionViewOptionsBuff(VARIANT *v) = 0;
		virtual bool SetHsessionViewOption(const char* propertyName, VARIANT* v) = 0;
		virtual void SetAllHsessionViewOption(std::string jsonbuff) = 0;

		virtual bool ExportCurrentRenderLayer(const std::string& fileName, const unsigned int layerType = 0, unsigned int quality = 100) = 0;

		//Environment
		virtual	std::string GetEnvironmentOptionsBuff() = 0;
		virtual bool GetEnvironmentOptionsBuff(VARIANT *v) = 0;
		virtual bool SetEnvironmentType(const char* EnvType) = 0;
		virtual bool SetEnvironmentOption(const char* propertyName, VARIANT* v) = 0;
		virtual void SetAllEnvironmentOption(std::string jsonbuff) = 0;


		virtual	std::string GetEnvFunctionBuff() = 0;
		virtual bool GetEnvFunctionBuff(VARIANT *v) = 0;
		virtual bool SetEnvFunction(const char* propertyName, VARIANT* v) = 0;
		virtual bool SetEnvFunctionType(const char* type) = 0;
		virtual void SetAllEnvFunOption(std::string jsonbuff) = 0;

		//set baclpate
		virtual	std::string GetBackplateOptionsBuff() = 0;
		virtual bool GetBackplateOptionsBuff(VARIANT *v) = 0;
		virtual void SetBackplateType(const char* EnvType) = 0;
		virtual bool SetBackplateOption(const char* propertyName, VARIANT* v) = 0;
		virtual void SetAllBackplateOption(std::string jsonbuff) = 0;


		virtual void ExportScene() = 0;		//set ToneMapper

		virtual	std::string GetToneMapperOptionsBuff() = 0;
		virtual bool GetToneMapperOptionsBuff(VARIANT *v) = 0;
		virtual bool SetToneMapperOption(const char* propertyName, VARIANT* v) = 0;
		virtual void SetToneMapperType(std::string m_ToneMapper) = 0;
		virtual void SetAllToneMapperOption(std::string jsonbuff) = 0;
		virtual void TestInterface() = 0;

#pragma region **********<Material>**********
		/////材质相关

		virtual void CreateMaterial(int matindex, OPENNP::Material* pMat) = 0;

		/**
		* 创建材质库中的material;
		* 第二个参数需要明确含义；
		* matlibIndex          材质库索引
		* matlibItemIndex      新创建的材质在材质库中的索引
		*
		* 返回材质唯一ID
		*/
		virtual int CreateMaterial(unsigned int matlibIndex, int *matlibItemIndex) = 0;

		virtual int GetMatIndex(int meshIndex) = 0;
		virtual bool ChangeMatType(int matIndex, const char* typeName) = 0;
		virtual int GetSceneMatCount() = 0; //获取材质数量
		virtual void destroySceneMaterial() = 0; //释放场景中的材质
		virtual void SetMatToBall(int matIndex) = 0;
		virtual bool SetMatProperty(int meshIndex, const char* propName, VARIANT* v, int count) = 0;
		virtual bool GetMatProperty(int meshIndex, const char* propName, VARIANT* v, int count) = 0;
		virtual bool GetMatProperty(int matIndex, const char* propName, float* pv) = 0;
		virtual int GetMatOffset() = 0;
		virtual void setMaterialRes(std::string jsonbuffs, std::map<std::string, int>& fileGuidIndexMap, std::map<std::string, int>&allGuidIndexMap) = 0;
		virtual std::string GetMatTypes() = 0;
		virtual std::string GetMatType(int matIndex) = 0;
		virtual std::string GetMatBuff(int matIndex) = 0;
		virtual std::string GetAllMaterialBuf() = 0;
		virtual std::string GetSceneMatList() = 0;

		virtual std::string MakeUniqMatName(std::string srcName) = 0;
		virtual bool SetMatDefinitionName(int matIndex, const char*  matName) = 0;
		virtual std::string GetMatDefinitionName(int matIndex) = 0;
		virtual std::string GetMatUniqueName(int matIndex) = 0;

		virtual std::string GetMatLibrary(int matlibIndex) = 0;
		virtual int OpenMatLibrary(const char* path, bool isNew) = 0;
		virtual bool SaveMatLibrary(const char* path, int matllibIndex) = 0;
		virtual bool CloseMatLibrary(int matllibIndex) = 0;

		virtual bool AssignlookToSelection(std::vector<int> meshes, int matlibIndex, int matIndex, int assignMode, int& NewMatIndex) = 0;
		virtual bool CreateMatForVariant(int mat, int oldMatId, int& newMatIndex) = 0;
		virtual bool AssignlookToSingleMesh(int meshindex, int matindex) = 0;
		virtual bool AssignlookToMat(int oldmatindex, int newmatindex, int& outmatindex, bool beCopy) = 0;

		virtual	std::string GetMatLibItem(int matlibIndex, int matitemIndex) = 0;
		virtual	bool SetUserLevel(int matindex, int level) = 0;
		virtual bool DeleteMatItem(int tabindex, int itemindex) = 0;
		virtual bool CopyMatItem(int srcMat, int desMatlib, int& newItemIndex) = 0;
		virtual int GetMatLibIndex(int matindex) = 0;

#pragma region MaterialCollections
		virtual bool AssignCollection(int matlibindex, int* matitems, int num, const char* collection, bool isadd) = 0;
		virtual bool ModifyCollection(int matlibindex, const char* oldname, const char* newname) = 0;
		virtual bool RemoveCollection(int matlibindex, const char* name) = 0;
		virtual bool AddCollection(int matlibindex, const char* name) = 0;
#pragma endregion MaterialCollections
#pragma endregion  **********<Material>**********

		virtual void CancelRender() = 0;
		virtual void SetViewMatCallBack(std::function<void(float[3], float[3], float[3], float)>) = 0;
		virtual void UnLockBuffer() = 0;
		virtual bool RenderBegin(void* hwnd = nullptr) = 0;
		virtual bool Render(void* hwnd = nullptr) = 0;
		virtual bool RenderEnd(void* hwnd = nullptr) = 0;
		virtual bool GetViewPort(void*hwnd, int&x, int&y, int&width, int&hight) = 0;
		virtual void SetNodeParams(int meshindex, NodeParams nodepar) = 0;
		virtual void GetNodeParams(int meshindex, NodeParams& nodepar) = 0;

		virtual unsigned char * GetImageBuf() = 0;
		virtual bool ChangeMatShader(int matindex, const char* propName, int type) = 0;
		virtual bool GetMatShader(int matindex, const char* propName, int& shaderType) = 0;

		virtual bool SetTextureSpaceType(int meshIndex, std::string tsm) = 0;
		virtual bool SetTextureSpaceProp(int meshIndex, const char* propName, VARIANT* v) = 0;
		virtual std::string GetMatTextureSpaceBuff(int meshIndex) = 0;
		virtual bool ClearSceneUnusedMats(std::vector<int> usedmatindexs) = 0;


		// for uv editor
		virtual bool GetUVMatImagePath(std::string &imagepath, int meshindex) = 0;
		virtual void * GetD3DDevice() = 0;
		virtual void * GetFactory() = 0;
		virtual void * GetD3DDeviceContext() = 0;
		virtual void EngineOnIdle() = 0;

		virtual bool TestConsoleVar_RT(const char* param, int value) = 0;
		virtual bool GetConsoleVar_RT(const char* param, VARIANT& value) = 0;
		virtual std::string GetConsoleData() = 0;
		virtual void SetConsoleVal(std::string bug) = 0;

		virtual void SetEnvProperty(const char* propName, VARIANT* v, int argCount, EnviromentType t) = 0;
		virtual void GetEnvProperty(const char* propname, VARIANT* v, int count, EnviromentType t) = 0;
		virtual void GetEnvironmentJsonData(VARIANT* v, EnviromentType t) = 0;
		virtual float GetTerrainSize() = 0;

        virtual void SetVideoTextureFilePath(std::string filePath) = 0;
        virtual void BegineEncodeVideoTexture() = 0;
        virtual void EndEncodeVideoTexture() = 0;

		//Vegetation start
		virtual void LoadVegetationFile(bool isSurround, const char* meshName, const char* materialName, int nodeIndex, int mateiralIndex, int vertexSize, int indexCount, const float* pInVex, const float* normals,
			const unsigned int* indices, const float* textures, const float* Tangents, const float* Bitangent, float* worldMat, const float* maxbox, const float* minbox, int meshCount, int materialCount, int nodeid) = 0;
		virtual void SetVegetationToolMode(int toolmode) = 0;
		virtual int GetVegetationToolMode() = 0;
		virtual std::shared_ptr<InstanceValueMap> VegetationButtonDown(Vector3D worldPoint, size_t instanceId, size_t nodeindex) = 0;
		virtual Vector3D VeiwToWorldTerrain(int x, int y) = 0;
		virtual void GetVegetationNodeBufferByNodeMeshid(int nodeid, int & indexCount, int & vertexCount, float ** vertex, unsigned int ** index, float ** normals, float ** textrues, int meshindex) = 0;
		virtual void MoveSelectedVegetation(const Vector3D & offset, bool bFollowTerrain) = 0;
		virtual void RotateSelectedVegetation(const Vector3D & rot) = 0;
		virtual void ScaleSelected(float fScale) = 0;
		virtual void SetSelectInstance(std::set<size_t> & selectedInstance) = 0;
		virtual void SetVegetationPosCallBack(void * callBack) = 0;
		virtual void SetVegetationRoteCallBack(void * callBack) = 0;
		virtual void SetVegetationScaleCallBack(void * callBack) = 0;
		virtual void SetVegetationRemoveCallBack(void * callBack) = 0;
		virtual float GetVegetationBrushRadius() = 0;
		virtual int GetMaterialIndexByNodeIndex(int nodeindex) = 0;
		virtual void FinishProcessVegetationProcess(int nodeId) = 0;
		virtual void RemoveNode(size_t nodeid) = 0;
		virtual void * GetVegetationValueChangerByNodeId(size_t nodeid) = 0;
		virtual void SetVegetationBrushRadius(float radius) = 0;
		virtual EngineBlock * SaveVegetation(size_t & EngineBlockLength) = 0;
		virtual void LoadVegetation(std::vector<int> & nodeIndex, EngineBlock * block, size_t EngineBlockLength) = 0;
        virtual void BeginRecordVegetationEvent(int eventType) = 0;
        virtual void EndRecordVegetationEvent() = 0;
        virtual void VegetationUnDo() = 0;
        virtual void VegetationReDo() = 0;
        virtual void SetVegetationAddCallBack(void * callBack) = 0;
		virtual void SetVegetationHiddenByNodeId(size_t nodeid, bool hidden) = 0;
        virtual void DeleteSelectVegetationInstance() = 0;
		virtual std::string MatIndex2Id(std::string oldid) = 0;
		//Vegetation end
		virtual void SetNormalSaveSign(bool bSave) = 0;
		virtual bool RenderComplete() = 0;
		//临时
		virtual bool GetIsPkFromMaterial(const char* buf) { return false; }
		protected:
			NotifyMeshIndexChange meshIndexChange = nullptr;

	};
}

typedef RenderMax::IBaseProcess* (*PCREATEENGINEPROCESSINTERFACE)();
#endif
