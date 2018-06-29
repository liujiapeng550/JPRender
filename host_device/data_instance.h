#pragma once

/**
* 注意：
* 不要在data文件中包含任意头文件
* 即便有错误也不要理会；
*/

/****************************************************************************************************/
// 定义所有材质属性的宏，避免输入有误；
/*****************************************************************************************************/
// geometry
#define M_MPROP_GE_BUMPTEXTUREPATH GE_BumpTexturePath
#define M_MPROP_GE_BUMPSCALE GE_BumpScale
#define M_MPROP_GE_ROTATION GE_Rotation
#define M_MPROP_GE_UVREPEAT GE_UVRepeat
#define M_MPROP_GE_REPEAT GE_Repeat
#define M_MPROP_GE_INDENSITY GE_Indensity
#define M_MPROP_GE_ITERATORTIMES GE_IteratorTimes
#define M_MPROP_GE_LAYERSCOUNT GE_LayersCount
#define M_MPROP_GE_USEBUMPMAPTS GE_UseBumpMapTS


// clearcoat
#define M_MPROP_CC_ORANGEPEELTEXTUREPATH CC_OrangePeelTexturePath
#define M_MPROP_CC_COLOR CC_Color
#define M_MPROP_CC_EXPONENT CC_Exponent
#define M_MPROP_CC_FMAX CC_Fmax
#define M_MPROP_CC_FMIN CC_Fmin
#define M_MPROP_CC_USEORANGEPEELNORMALMAPTS CC_UseOrangePeelNormalMapTS
#define M_MPROP_CC_BUMPSCALE CC_BumpScale
#define M_MPROP_CC_ROTATION CC_Rotation
#define M_MPROP_CC_UVREPEAT CC_UVRepeat
#define M_MPROP_CC_WEIGHT CC_Weight


// flake
#define M_MPROP_CF_COLOR CF_Color
#define M_MPROP_CF_IRREGULARITY CF_Irregularity
#define M_MPROP_CF_REFLECTIVITY CF_Reflectivity
#define M_MPROP_CF_DENSITY CF_Density
#define M_MPROP_CF_ROUGHNESS CF_Roughness
#define M_MPROP_CF_SIZE CF_Size


// base
#define M_MPROP_BS_CHECKBOARDSIZE BS_CheckBoardSize
#define M_MPROP_BS_EXPONENT BS_Exponent
#define M_MPROP_BS_FMIN BS_Fmin
#define M_MPROP_BS_FMAX BS_Fmax
#define M_MPROP_BS_DIFFUSECOLOR BS_DiffuseColor 
#define M_MPROP_BS_GLOSSYCOLOR BS_GlossyColor   
#define M_MPROP_BS_REFLECTCOLOR BS_ReflectColor 
#define M_MPROP_BS_UVREPEAT BS_UVRepeat
#define M_MPROP_BS_IOR BS_IOR
#define M_MPROP_BS_GAMMA BS_Gamma
#define M_MPROP_BS_AOINTENSITY BS_AOIntensity
#define M_MPROP_BS_INTENSITY BS_Intensity
#define M_MPROP_BS_SIGMA BS_Sigma
#define M_MPROP_BS_ROTATION BS_Rotation
#define M_MPROP_BS_ROUGHNESS BS_Roughness
#define M_MPROP_BS_CENTER_X BS_CenterX
#define M_MPROP_BS_CENTER_Y BS_CenterY
#define M_MPROP_BS_CENTER_Z BS_CenterZ
#define M_MPROP_BS_DIFFUSEREFLECTANCE BS_DiffuseReflectance
#define M_MPROP_BS_SPECULARREFLECTANCE BS_SpecularReflectance
#define M_MPROP_BS_USEDIFFUSEMAPTS BS_UseDiffuseMapTS
#define M_MPROP_BS_USEGLOSSYMAPTS BS_UseGlossyMapTS
#define M_MPROP_BS_ANISOTROPYU BS_AnisotropyU
#define M_MPROP_BS_ANISOTROPYV BS_AnisotropyV
#define M_MPROP_BS_DIFFUSETEXTUREPATH BS_DiffuseTexturePath
#define M_MPROP_BS_GLOSSYTEXTUREPATH BS_GlossyTexturePath

#define M_MPROP_SHADOW_ATTENUATION ShadowAttenuation
#define M_MPROP_INDEX material_index;


struct SData_Texture{
	optix::float4 color;

	optix::float2 UVrepeat;
	float rotation;
	float bumpscale;

	float invGamma;
	float place_holder;
	int TSID;
	int enable;
};//48 bytes

struct SData_Fresnel{
	float exponent;
	float fmin;
	float fmax;
	float place_holder;
};// 16 bytes;

struct SData_Material{
	//float GE_indensity;
	//int GE_iteratortimes;
	//int GE_layerscount;
	//float BS_sigma;              //sigma ON模型的微调参数，其值为0会退化为Lamberian模型；
	//float BS_absorption;         //材质吸收率


	SData_Texture CC_texture;
	SData_Texture BS_textureDiffuse;
	SData_Texture BS_textureGlossy;
	SData_Texture GE_texture;
	SData_Fresnel CC_fresnel;
	SData_Fresnel BS_fresnel;
	union{
		optix::float4 BS_colorReflect;
		optix::float4 CF_color;
	};
	optix::float4 BS_center;

	float CC_weight;
	float CF_irregularity;
	float CF_reflectivity;
	float CF_roughness;

	float CF_size;
	float CF_density;
	float BS_ior;
	float BS_roughness;

	float BS_reflectanceSpecular;//specular反射比
	float BS_reflectanceDiffuse; //diffuse反射比
	float BS_anisotropyU;
	float BS_anisotropyV;

	float BS_AOIntensity;
	float BS_checkboardSize;
	float GE_shadowAttenuation;
	int materialIndex;
};

/*
这种结构不符合我们的模型模块。
struct SData_Vertex{
	optix::float3 pos;
	optix::float3 normal;
	optix::float3 tangent;
	optix::float3 bitangent;
	optix::float2 uv;
};
*/

struct SData_Instance{
	/// mesh
	int vertexBufferID;//float3
	int norBufferID;//float3
	int tanBufferID;//float3，可能会没有值；
	int biTanBufferID;//float3，可能会没有值；
	int uvBufferID;//float2
	int indexBufferID;//mesh三角面的索引缓冲区id，元素是个int3，这个buffer存几个元素，就意味着有几个三角面；
  int tanBufferSize;
	int biTanBufferSize;

	/// material
	int materialBufferID;//所有SData_Material缓冲区id，元素是个自定义结构体，里面包含材质需要的所有数据；
	int materialBufferOffset;//该mesh具体使用的材质数据在缓冲区中的偏移；

	/// instance
	int nodeIndex;//mesh在模型层级结构中的节点id，用于拾取；
	int worldMatrixBufferID;//用于区域光源计算mesh各顶点的世界坐标；

};
