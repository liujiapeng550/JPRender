#ifndef __VEGETATIONVALUECHANGER_H__
#define __VEGETATIONVALUECHANGER_H__
//#ifdef VEGDLL
//#define VEGLL_API _declspec(dllexport) 
//#else
//#define VEGDLL_API _declspec(dllimport) 
//#endif

#include <functional>
#include <map>
#include <string>
#include <vector>
#include <memory>

typedef std::function<void(size_t, std::string)> UIAddGroupFunction;
typedef std::function<void(size_t, size_t, std::string)> UIAddNodeFunction;
typedef std::function<void(size_t)> ValueChangerFun;
typedef std::map<std::pair<int, int>, std::string> VegUIInitMap;

//误轻易修改
enum
{
    VegetationValue_Size = 0,
    VegetationValue_SizeVar,
    VegetationValue_RandomRotation,
    VegetationValue_RotationRangeToTerrainNormal,
    VegetationValue_AlignToTerrainNormal,
    VegetationValue_UseTerrainColor,
    VegetationValue_AllowIndoor,
    VegetationValue_Bending,
    VegetationValue_Hideable,
    VegetationValue_PlayerHidealbe,
    VegetationValue_GrowOnBrushes,
    VegetationValue_GrowOnTerrain,
    VegetationValue_AutoMerged,
    VegetationValue_Stiffness,
    VegetationValue_Damping,
    VegetationValue_Variance,
    VegetationValue_AirResisance,
    VegetationValue_Pickable,
    VegetationValue_AiRadius,
    VegetationValue_Density,
    VegetationValue_ElevationMin,
    VegetationValue_ElevationMax,
    VegetationValue_SlopeMin,
    VegetationValue_SlopeMax,
    VegetationValue_CastShadowMinSpec,
    VegetationValue_DynamicDistanceShadows,
    VegetationValue_GlobalIllumination,
    VegetationValue_SpriteDistRatio,
    VegetationValue_LodDistRatio,
    VegetationValue_MaxViewDistRatio,
    VegetationValue_UseSprites,
    VegetationValue_MinSpec,
    VegetationValue_LayerFrozen,
    VegetationValue_LayerWet,
    VegetationValue_EngineSomeThingAndTerrainDefault

};

struct VegetationValueChanger
{
    void * ValuePoint[35];
    std::function<void(void *)> OnValueChange;
};

struct VegetationInstanceValue
{
    /*float pos_x, pos_y, pos_z;
    float scale;
    float angle_x, angle_y, angle_z;
    VegetationInstanceValue(float _pos_x, float _pos_y, float _pos_z, float _scale, float _angle_x, float _angle_y, float _angle_z)
    : pos_x(_pos_x), pos_y(_pos_y), pos_z(_pos_z), scale(_scale), angle_x(_angle_x), angle_y(_angle_y), angle_z(_angle_z) { }*/
    float worldtm[16];
    VegetationInstanceValue(float * worldTm) { memcpy(worldtm, worldTm, sizeof(float) * 16); }
};

typedef std::vector<std::pair<size_t, VegetationInstanceValue> > InstanceValueMap;

struct ArchiveBlock
{
    size_t BlockFlag;
    size_t GroupId;
    size_t NodeId;
    size_t InstanceId;
    int BlockSize;
    unsigned char * BlockMemory;
    ArchiveBlock * next;
    ArchiveBlock()
    {
        BlockMemory = nullptr;
        BlockSize = 0;
        next = nullptr;
    }
    ~ArchiveBlock()
    {
        delete[] BlockMemory;
    }
};

extern ArchiveBlock * block_head;
extern ArchiveBlock * block_now;
extern int ArchiveRef;
extern UIAddGroupFunction * vegUIAddGroupFunction;
extern UIAddNodeFunction * vegUIAddNodeFunction;

class VegetationArchive
{
public:
    VegetationArchive()
    {
        if (ArchiveRef == 0) {
            block_head = new ArchiveBlock;
            block_now = block_head;
        }
        ArchiveRef++;
    }

    VegetationArchive(const VegetationArchive & that) {
        ArchiveRef++;
    }

    virtual ~VegetationArchive() {
        ArchiveRef--;
        if (ArchiveRef == 0) {
            cleanArchiveBlock();
            block_head = nullptr;
            block_now = nullptr;
        }
    }
    virtual void cleanArchiveBlock() {
        while (block_head) {
            ArchiveBlock * temp = block_head->next;
            delete block_head;
            block_head = temp;
        }
    }
    virtual bool FillArchiveBlock() = 0;
    virtual bool ReadArchiveBlock(void * block) = 0;
};

#pragma pack(push, 1)

struct ModelBlock
{
    size_t GroupId;
    size_t NodeId;
    int NameLength;
    char * fileName;
    ~ModelBlock() {
        delete[] fileName;
    }
};

struct GroupBlock
{
    size_t m_nGroupId;
    int NameLength;
    char * m_GroupName;

    ~GroupBlock() {
        delete[] m_GroupName;
    }
};

struct NodeBlock
{
    size_t m_nNodeId;

    int NameLength;
    char * m_NodeName;

    float m_worldMat[16];
    float m_localCoord[3];
    float m_localAxes[3];

    ~NodeBlock() {
        delete[] m_NodeName;
    }

};

struct EngineBlock
{
    int DataLength;
    unsigned char * EngineXmlData;
    int ArraySize;
    unsigned char * InstanceData; // 在此释放

    EngineBlock(std::string xmldata, unsigned char * data, int size) {
        DataLength = xmldata.length() + 1;
        EngineXmlData = new unsigned char[DataLength];
        memcpy(EngineXmlData, xmldata.c_str(), DataLength - 1);
        EngineXmlData[DataLength - 1] = 0;
        InstanceData = data;
        ArraySize = size;
    }

    EngineBlock(int dataLength, unsigned char * engineData) {
        if (dataLength == 0) {
            return;
        }
        int nowStep = 0;
        memcpy(&DataLength, engineData, sizeof(int));
        nowStep += sizeof(int);
        EngineXmlData = new unsigned char[DataLength];
        memcpy(EngineXmlData, engineData + nowStep, DataLength);
        nowStep += DataLength;
        memcpy(&ArraySize, engineData + nowStep, sizeof(int));
        nowStep += sizeof(int);
        InstanceData = new unsigned char[ArraySize];
        memcpy(InstanceData, engineData + nowStep, ArraySize);
    }

    void Serialize(int maxLength, unsigned char * engineData) {
        if (maxLength == 0) {
            return;
        }
        int nowStep = 0;
        memcpy(engineData, &DataLength, sizeof(int));
        nowStep += sizeof(int);
        memcpy(engineData + nowStep, EngineXmlData, DataLength);
        nowStep += DataLength;
        memcpy(engineData + nowStep, &ArraySize, sizeof(int));
        nowStep += sizeof(int);
        memcpy(engineData + nowStep, InstanceData, ArraySize);
    }

    ~EngineBlock() {
        delete[] EngineXmlData;
        delete[] InstanceData;
    }
};

struct InstanceBlock
{
    float vMin[3], vMax[3];
    float AABBox[6];
    size_t InstanceId;

    float mworldMat[16];//保存相对于父矩阵的位移信息
    float localAxes[16];//局部坐标轴，局部旋转，缩放，平移以此为基准
    float localMat[16];//模型相对于localAxes的位置
                       //mrelativeMat=mworldMat*localAxes*localMat
    float mrelativeMat[16]; // 保存相对于父矩阵的本地矩阵	
    float realWorldMat[16];//真正的世界矩阵
    float rotationMat[9]; //旋转矩阵
    float scalaMat[3]; //缩放矩阵
    float translationMat[3];//本地坐标
    float m_LocalEulerAngle[3];
    float m_OrigineScale[3];
};

struct VegetationBlock
{
    size_t ModelDataLength;
    size_t NodeNumber;
    ModelBlock * NodeModelInfo;
    size_t EngineDataLength;
    EngineBlock * EnginedData;
    size_t NodeTreeDataLength;
    unsigned char * NodeTreeData;

    VegetationBlock() {
        ModelDataLength = 0;
        NodeNumber = 0;
        NodeModelInfo = nullptr;
        EngineDataLength = 0;
        EnginedData = nullptr;
        NodeTreeDataLength = 0;
        NodeTreeData = nullptr;
    }

    ~VegetationBlock() {
        delete[] NodeModelInfo;
        delete EnginedData;
        delete[] NodeTreeData;
    }
private:
    VegetationBlock(const VegetationBlock & that) {}
    VegetationBlock & operator=(const VegetationBlock) { return *this; }
};

#pragma pack(pop)

#define VEGETATION_BLOCK_LENGTH(block) (sizeof(size_t) * 4 + (block).ModelDataLength + (block).EngineDataLength + (block).NodeTreeDataLength)
#define NODE_BLOCK_LENGTH(block) (sizeof(size_t) + sizeof(int) + sizeof(float) * 22 + (block).NameLength)
#define INSTANCE_BLOCK_LENGTH (sizeof(InstanceBlock))
#define GROUP_BLOCK_LENGTH(block) (sizeof(int) + sizeof(size_t) + (block).NameLength)
#define ENGINE_BLOCK_LENGTH(block) ((block).DataLength + (block).ArraySize + sizeof(int) * 2)
#define MODEL_BLOCK_LENGTH(block) (sizeof(size_t) * 2 + sizeof(int) + (block).NameLength)

#define BLOCK_FLAG_INSTANCEBLOCK 0x1
#define BLOCK_FLAG_NODEBLOCK 0x2
#define BLOCK_FLAG_GROUPBLOCK 0x3

#define EVENT_TYPE_MOVE 0x1
#define EVENT_TYPE_SCALE 0x2
#define EVENT_TYPE_ROTATION 0x3
#define EVENT_TYPE_DELETE 0x4
#define EVENT_TYPE_PRINT 0x5

#endif
