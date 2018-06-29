#include <string>
#pragma once
#define BIT(x)   (1u << (x))
#define INVALID_VARIANT_ID -1
#define CAMEAR_DEFAULT_WIDTH 1024		
#define CAMERA_DEFAULT_HEIGHT 576
enum UsbCameraError
{
	OK,
	UsbCameraError_BufferPointError,
	UsbCameraError_ThreadStopError,
	UsbCameraError_CameraNotFound,
	UsbCameraError_DeviceReleased,
	UsbCameraError_DLLERROR,
	UsbCameraError_ArToolError

};
enum UsbCamera_State
{
	State_UseNone = NULL,
	State_UseColor = BIT(0),
	State_UseImage = BIT(1),
	State_UseCamera = BIT(2),
	State_UseVideo = BIT(3),
	State_UseHtcCamera = BIT(4)
};
enum UsbCamera_DealState
{
	State_DealNone = NULL,
	State_DealARToolKit = BIT(0),
	State_Dealleapmotion = BIT(1),
};
enum AR_VIEWPORT
{
	WH4X3,
	WH16X9
};
enum ControlWidget_Type
{
	ControlWidget_ImageButton,
	ControlWidget_Image,
	ControlWidget_Text
};
enum class VariantScope_Type
{
	TextScope,
	LookScope,
	LookScope_Override
};
enum Variant_Prop_Tag:long long
{
};

enum AutoPropertyDesc
{
	APD_NO_EXTERN = 0,
	APD_LINK_CHILD_PROPERTIES = 0x01,
	APD_LINK_CHILD_INFINITE = APD_LINK_CHILD_PROPERTIES*2,
	APD_RESET_PAGE_ON_CHANGED = APD_LINK_CHILD_INFINITE*2
	
};
typedef enum {
	Angular = BIT(0),
	Smooth_3D = BIT(1),
	Balanced_3D = BIT(2),
	Smooth_2D = BIT(3),
	Balanced_2D = BIT(4),
}KeyInterpolateType;
typedef enum {
	linear_interpolation,
	fast_interpolation,
	slow_interpolation,
	Bezier_interpolation,
	multistep_interpolation,
}InterpolateType;

typedef struct {
	int time;
	float value;
}ControlPointPos;
typedef struct {
	int realTime;
	float realValue;
	int maxTime;
}LeftExtra;
typedef struct {
	int realTime;
	float realValue;
	int maxTime;
	int time2lreal;
}RightExtra;
struct Frame {
	ControlPointPos left;
	ControlPointPos right;
	LeftExtra leftEx;
	RightExtra rightEx;
	float value;
	int time;
	KeyInterpolateType InterTy = Smooth_3D;
	bool uiShow = true;
	void InitEx()
	{
		leftEx.realTime = left.time;
		leftEx.realValue = left.value;
		leftEx.maxTime = left.time;
		rightEx.realTime = right.time;
		rightEx.realValue = right.value;
		rightEx.maxTime = right.time;
		rightEx.time2lreal = 0;
	}
	void InitRightEx()
	{
		rightEx.realTime = right.time;
		rightEx.realValue = right.value;
		rightEx.maxTime = right.time;
		rightEx.time2lreal = 0;
	}
	void InitLeftEx()
	{
		leftEx.realTime = left.time;
		leftEx.realValue = left.value;
		leftEx.maxTime = left.time;
	}
};

enum class ACTION_TYPE
{
	VARIANT,
	ANIMATION,
	INTIATOR
};

enum class TCONDITION_TYPE
{
	KEY,
	VARIANT,
	ANIMATION,
	PICKING,
	AUTO_START,
	HTC_BUTTON1,
	HTC_BUTTON2,
	HTC_BUTTON3,
	HTC_BUTTON4
};

enum class CVARIANT_TYPE
{
	CAMERA,
	MAT,
	GEOMETRY,
	MATCOLOR,
	OBJECT,
	PACKAGE,
	LOGIC,
	SWITCH
};
enum ANIMATION_KEY_VALUE_TYPE
{
	ANI_UNKNOWN=-1,
	ANI_X,
	ANI_Y,
	ANI_Z,
	ANI_W,
	ANI_TIME
};

enum CURVE_MODE
{
	CURVE_LINE,
	CURVE_BEZIER3
};
enum AnimationKeepMode
{
	Animation_keep_frame,
	Animation_keep_time
};
enum AnimationNodeType
{
	Anode_unknown = -1,
	Anode_timeline,
	Anode_Animation,
	//////////////////////////////////////////////////////////////////////////
	//these nodes are child animation property type.
	Anode_atuo_start = 100,
	Anode_positon,
	Anode_rotationEuler,
	Anode_rotationAxis,
	Anode_scale,
	Anode_center,
	Anode_camera_focal_length,
	Anode_camera_position,
	Anode_camera_phr,
	Anode_light_position,
	Anode_light_rotation,
	Anode_light_attenuationbulbsize,
	Anode_light_color,
	Anode_light_diffuseMultipiler,
	Anode_light_specularMultiplier,
	Anode_light_radius,
	Anode_light_projectorfov,
	Anode_light_penumbraangle,
	Anode_light_boxSize,
	Anode_light_Diameter,
	Anode_light_Width,
	Anode_mat_look,
	Anode_mat_opacity,
	Anode_mat_diffusecolor,
	Anode_mat_specularcolor,
	Anode_mat_emissivecolor,
	Anode_mat_emissiveIntensity,
	Anode_mat_smoothness,
	Anode_camera_center,
	Anode_camera_center_to_eye,
	Anode_auto_end
	//////////////////////////////////////////////////////////////////////////


};
enum AnimationType :long long
{
	Atype_unknown = -1,
	Atype_3dNote = 0x1,
	Atype_cameraColorCrrection = Atype_3dNote * 2,
	Atype_cameraDepthOfField = Atype_cameraColorCrrection * 2,
	Atype_cameraDirtyLens = Atype_cameraDepthOfField * 2,
	Atype_cameraGeneral = Atype_cameraDirtyLens * 2,
	Atype_cameraGlare = Atype_cameraGeneral * 2,
	Atype_cameraGlow = Atype_cameraGlare * 2,
	Atype_cameraMotionBlur = Atype_cameraGlow * 2,
	Atype_cameraSensor = Atype_cameraMotionBlur * 2,
	Atype_cameraStereo = Atype_cameraSensor * 2,
	Atype_directionalLight = Atype_cameraStereo * 2,
	Atype_effectsObject = Atype_directionalLight * 2,
	Atype_flyTo = Atype_effectsObject * 2,
	Atype_graphicsOverlay = Atype_flyTo * 2,
	Atype_human = Atype_graphicsOverlay * 2,
	Atype_lensFlareEffectObject = Atype_human * 2,
	Atype_light = Atype_lensFlareEffectObject * 2,
	Atype_localSurroundingObject = Atype_light * 2,
	Atype_look = Atype_localSurroundingObject * 2,
	Atype_lookColor = Atype_look * 2,
	Atype_objectPpMask = Atype_lookColor * 2,
	Atype_objectWithEulerRotation = Atype_objectPpMask * 2,
	Atype_objectWithAnxisAngleRotation = Atype_objectWithEulerRotation * 2,
	Atype_pointLight = Atype_objectWithAnxisAngleRotation * 2,
	Atype_ramsis = Atype_pointLight * 2,
	Atype_soundObject = Atype_ramsis * 2,
	Atype_spotLight = Atype_soundObject * 2,
	Atype_areaLight = Atype_spotLight * 2,
	Atype_sunShaftEffectObject = Atype_areaLight * 2,
	Atype_switchObject = Atype_sunShaftEffectObject * 2,
	Atype_textOverlay = Atype_switchObject * 2,
	Atype_videoTexture = Atype_textOverlay * 2,
	Atype_Last = Atype_videoTexture * 2,

	Atype_Mask = 0x7FFFFFFFFFFFFFFF//2^63-1
};

enum TreeNode_InsertMode
{
	TreeNode_InsertMode_Before,
	TreeNode_InsertMode_In,
	TreeNode_InsertMode_Follow
};
enum BroadCast_message
{
	MSG_ENGINE_NODE_CHECKUNCHECK_VISIBILITY = 0x0400+3000,//wparam:id,  lparam: true-check  false-uncheck
	MSG_ENGINE_NODE_SHOWHIDE,//wparam:id,  lparam: true-show  false-hide
	MSG_ENGINE_REFRESH_LOOKEDITOR,
	MSG_ENGINE_MODIFY_LOOKEDITOR_MAT_TYPE,//wparam: N/A,  lParam:type's name(wchar*)
	MSG_ENGINE_RESOLUTION_CHANGED,//wparam:LOWORD, HIWORD(with, height)
	MSG_ENGINE_LOCK_SNODE,	//wParam:id, lparam: true-lock  false-unlock
	MSG_ENGINE_REFRESH_SCENETREE_KEEP_STATE//wParam:N/A, lparam:N/A
};
enum  DataType
{
	Material=0,
	TreeeNode=1,
	Camera=2
};
enum WarpMode
{
	CLAMP=0,
	BORDER=1,
	REPEAT=2,
	MIRROR=3
};

enum ope_type{ trans, scale, rotate, ope_null};


enum axes_type{ 
	axes_x = 0,
	axes_y = 1,
	axes_z = 2,
	axes_o = 3,
	axes_xt = 4,
	axes_yt = 5,
	axes_zt = 6,
	axes_yz = 7,
	axes_xz = 8,
	axes_xy = 9,
	axes_xyz = 10,
	axes_null=11 
};
enum UV_HelpMode_Type
{
	UV_noType =0,
	UV_plane = 1,
	UV_cube = 2,
	UV_sphere =3,
	UV_cylinder =4
};

enum units {
	millimeter=0,
	centimeter=1,
	inch=2,
	decimeter=3,
	foot=4,
	yard=5,
	meter=6,
	kilometer=7
};

// 界面Selection点击的按钮
enum SelectionMode
{
	SELECTBYFACEORWIRE,
	Selectbyangle,
	SELECTBYOBJECT,
	SELECTBYBODY,
	SELECTBYGROUP,
	SELECTBYLOD,
	SELECTBYLINK,
	SELECTBYLOOK,
	SELECTBYCOLOR,
};

//框选模式
enum BoxSelectionMode {
	AllShapesPartinally,
	OnlyVisible,
	AllShapesCompletelly,
};

enum TreeObject_Type :long long
{
	TreeObject_Polygon = 0x00001,
	TreeObject_Group = 0x00002,
	TreeObject_ObjectEntity = 0x00003,
	TreeObject_ObjectLight = 0x00004,
	TreeObject_ObjectLight_AreaRectangle = 0x00008,
	TreeObject_ObjectLight_Point = 0x00010,
	TreeObject_ObjectLight_AreaSphere = 0x00020,
	TreeObject_ObjectLight_Photometric = 0x00040,
	TreeObject_ObjectLight_AreaCylinder = 0x00080,
	TreeObject_ObjectLight_AreaDisc = 0x00100,
	TreeObject_ObjectLight_Distant = 0x00200,
	TreeObject_ObjectLight_Spot = 0x00400,
	TreeObject_ObjectLight_AreaDiscSpot = 0x00800,
	TreeObject_ObjectLight_AreaRectangleSpot = 0x01000,
	TreeObject_ObjectLight_EnvironmentProbe = 0x02000,
	TreeObject_ClipPlane = TreeObject_ObjectLight_EnvironmentProbe*2,
	TreeObject_Surrounding = TreeObject_ClipPlane*2,
	TreeObject_Camera = TreeObject_Surrounding*2,
	TreeObject_Model = TreeObject_Camera*2,
	TreeObject_Switch = TreeObject_Model*2,
	TreeObject_NurbsBody = TreeObject_Switch*2,
	TreeObject_NurbsShell = TreeObject_NurbsBody*2,
	TreeObject_NurbsFace = TreeObject_NurbsShell*2,
	TREEOBJ_MASK = 0xFFFFFF,
	TreeObject_NULL
};
#pragma region RealTime
//_RealTime
enum  class R_ETEX_Type
{
	//eTT_1D = 0,
	eTT_2D =1,
	//eTT_3D,
	eTT_Cube =3,
	//eTT_CubeArray,
	eTT_Dyn2D =5,
	eTT_User=6,
	eTT_NearestCube=7,

	//eTT_2DArray,
	//eTT_2DMS,

	//eTT_Auto2D,

	//eTT_MaxTexType,   //!< Not used.
};
enum class R_FILTER_TYPE
{
	FILTER_NON_=-1,
	FILTER_POINT_,
	FILTER_LINEAR_,
	FILTER_BILINEAR_,
	FILTER_TRILINEAR_,
	FILTER_ANISO2X_,
	FILTER_ANISO4X_,
	FILTER_ANISO8X_,
	FILTER_ANISO16X_
};
enum class R_ETexGenType
{
	ETG_Stream,
	ETG_World,
	ETG_Camera,
	ETG_Max
};
enum class R_ETexModRotateType
{
	ETMR_NoChange,
	ETMR_Fixed,
	ETMR_Constant,
	ETMR_Oscillated,
	ETMR_Max
};
enum class R_ETexModMoveType
{
	ETMM_NoChange,
	ETMM_Fixed,
	ETMM_Constant,
	ETMM_Jitter,
	ETMM_Pan,
	ETMM_Stretch,
	ETMM_StretchRepeat,
	ETMM_Max
};

//! Deformations/Morphing types.
enum class R_EDeformType
{
	eDT_Unknown = 0,
	eDT_SinWave = 1,
	eDT_SinWaveUsingVtxColor = 2,
	eDT_Bulge = 3,
	eDT_Squeeze = 4,
	eDT_Perlin2D = 5,
	eDT_Perlin3D = 6,
	eDT_FromCenter = 7,
	eDT_Bending = 8,
	eDT_ProcFlare = 9,
	eDT_AutoSprite = 10,
	eDT_Beam = 11,
	eDT_FixedOffset = 12,
};
//! Wave form evaluator flags.
enum class R_EWaveForm
{
	eWF_None,
	eWF_Sin,
	//eWF_HalfSin,
	//eWF_InvHalfSin,
	//eWF_Square,
	//eWF_Triangle,
	//eWF_SawTooth,
	//eWF_InvSawTooth,
	//eWF_Hill,
	//eWF_InvHill,
};
#pragma endregion


//copy from iray "ParameterType.h"
enum ParamType
{
	TYPE_UNKNOWN = 0,                      //!< Type is not known/recognised.
	TYPE_BOOLEAN,                          //!< bool
	TYPE_CHAR,                             //!< char
	TYPE_UNSIGNED_CHAR,                    //!< unsigned char
	TYPE_SHORT,                            //!< short
	TYPE_UNSIGNED_SHORT,                   //!< unsigned short
	TYPE_INT,                              //!< int
	TYPE_UNSIGNED_INT,                     //!< unsigned int
	TYPE_LONG,                             //!< long
	TYPE_UNSIGNED_LONG,                    //!< unsigned long
	TYPE_FLOAT,                            //!< float
	TYPE_DOUBLE,                           //!< double
	TYPE_VECTOR2_FLOAT,                    //!< lw::maths::Vector2 or lw::maths::Vector2_f.
	TYPE_VECTOR2_DOUBLE,                   //!< lw::maths::Vector2_d
	TYPE_VECTOR3_FLOAT,                    //!< lw::maths::Vector3 or lw::maths::Vector3_f.
	TYPE_VECTOR3_DOUBLE,                   //!< lw::maths::Vector3_d
	TYPE_COLOUR_RGB,                       //!< lw::ColourRGB.
	TYPE_COLOUR_RGBA,                      //!< lw::ColourRGBA.
	TYPE_POINTER,                          //!< Generic pointer type.
	TYPE_ENUMERATION,                      //!< general enum
	TYPE_SCATTER_MODE,                     //!< scatter mode enum
	TYPE_WRAP_MODE,                        //!< texture wrap modes
	TYPE_MONO_MODE,                        //!< texture alpha samping mode
	TYPE_COORDINATE_SYSTEM,                //!< coordinate system enum
	TYPE_AXIS,                             //!< Axis enum
	TYPE_STRING,                           //!< char* string
	TYPE_COLOUR_INTERFACE,                 //!< Pointers to Colour Interfaces
	TYPE_BUMP_MAP_INTERFACE,               //!< Pointers to BumpMap Interfaces
	TYPE_FLOAT_INTERFACE,                  //!< Pointers to Float Interfaces
	TYPE_TEXTURE_SPACE_INTERFACE,          //!< Pointers to Texture Space Interfaces
	TYPE_LIGHT_INTERFACE,                  //!< Pointers to Light Interfaces
	TYPE_BACKPLATE_INTERFACE,              //!< Pointers to Backplate Interfaces
	TYPE_ENVIRONMENT_INTERFACE,            //!< Pointers to Environment Interfaces
	TYPE_ENVIRONMENT_FUNCTION_INTERFACE,   //!< Pointers to Environment Function Interfaces
	TYPE_TONE_MAP_INTERFACE,               //!< Pointers to Tonemap Interfaces
	TYPE_INTENSITY_MODE,                   //!< intensity mode enum
	TYPE_IMAGE,                            //!< Pointers to Image class
	TYPE_EMISSIVE_INTERFACE,               //!< Pointers to Emissive Interfaces
	TYPE_COMBINE_MODE,                     //!< texture space combine mode
	TYPE_MATRIX4_FLOAT,                    //!< lw::maths::Matrix4 or lw::maths::Matrix4_f
	TYPE_MATRIX4_DOUBLE,                   //!< lw::maths::Matrix4_d
	TYPE_TEXTURE_COMPRESSION,              //!< texture compression mode
	TYPE_FILTER_MODE,					   //!< texture filter modes
	TYPE_BRUSH_MAPPING,					   //!< texture filter modes
	//realtime
	TYPE_CASTSHADOWS,					   //!< light prop castshadows
	TYPE_SHADOWMINRESPERENT,			   //!< light prop shadowminresperent
	TYPE_LIGHTTEXTURE,					   //!< light prop light texture
	TYPE_BUTTON,						   //!< prop with button
	TYPE_ETEX_TYPE_ENUM,
	TYPE_FILTER_TYPE_ENUM,
	TYPE_ETexGenType_ENUM,
	TYPE_ETexModRotateType_ENUM,
	TYPE_ETexModMoveType_ENUM,
	TYPE_EDeformType_ENUM,
	TYPE_EWaveForm_ENUM,
	TYPE_LIST,							   //!< prop with list
	//vegetation
	TYPE_VEG_OPTIONS_HIDEABLE,
	TYPE_VEG_OPTIONS_PLAYER_HIDEALBE,
	TYPE_VEG_OPTIONS_CAST_SHADOW_MIN_SPEC,
	TYPE_VEG_OPTIONS_MIN_SPEC,
	//common
	TYPE_INPUT_WITH_PATH_BROWSER,
	//special
	TYPE_SUROUNDING_OPEN_CLEAN,
	TYPE_USE_CAMERA_WITH_DROPBOX,
	TYPE_BOOLEAN_AUTO_CHANGE,
	//NoSlider
	TYPE_FLOAT_NoSlider,				   //!< float without slider

	TYPE_INPUT_BUDDY_CHECKBOX,
	TYPE_VIEWPORT,
	TYPE_GEN_IMAGE,
	TYPE_IMAGE_BK,
	TYPE_VIDEO,
	TYPE_MATT
};

//! \brief Identifies the antialiasing filter mode.
//!
//! These are values which can be used with the 'FilterMode' render parameter, which affects
//! the photorealistic and interactive renderers.
#ifndef _RealTime
enum FilterMode
{
	Box = 0,       //!< Box filter, the fastest method, produces relatively good results. 
	Triangles = 1,  //!< Triangle filter, more processor intensive than box, but offers better results. 
	Gauss = 2      //!< Gaussian filter, produces the best results but is the slowest to render.
};
#endif

//! \brief Identifies the layer display mode.
//!
//! These are values which can be used with the 'IrradianceMode' and 'DepthMode' render parameters.
enum LayerDisplayMode
{
	Automatic = 0,    //!< The layer will show the full range of values in the scene.
	UserDefined = 1,  //!< The layer will show the range of values specified by the user.
};

//! \brief Identifies the instancing mode.
//!
//! These are values which can be used with the 'PhotorealInstancing' render parameter, which affects
//! the photorealistic renderer.
enum InstancingMode
{
	Flattened = 0,  //!< Flattened mode, the fastest rendering, but uses more memory. 
	Instanced = 1,  //!< Instanced mode, uses less memory, but at the expense of rendering speed. 
};

//! \brief Identifies the terminator smoothing mode.
//!
//! When rendering curved objects with a low number of facets, shadow edges can sometimes appear
//! blocky. Enabling terminator smoothing will improve the appearance in most circumstances.
enum ShadowTerminatorMode
{
	NoSmoothing = 0,  //!< Smoothing is disabled.
	Smoothing = 1,  //!< Smoothing is enabled. This may cause other visible effects.
};

//typedef enum{
//	Tracks_Position,
//	Tracks_Rotation,
//	Tracks_Scale,
//
//	Track_Eye,
//	Tracks_LookAt,
//	Tracks_Direction,
//	Tracks_Up,
//}Tracks_Type;

 typedef enum 
  {
     iBeautyLayer,           //!< The layer containing the final, result image from the render.
     iAlphaLayer,            //!< The layer containing alpha data.
     iDepthLayer,            //!< The layer containing depth data.
     iDistanceLayer,         //!< The layer containing distance data.
     iNormalLayer,           //!< The layer containing surface normal data.
     iTextureCoordinate0Layer,//!< The layer containing the first texture coordinate channel.
     iObjectIDLayer,         //!< The layer containing the IDs of objects being rendered.
     iMaterialIDLayer,       //!< The layer containing the IDs of materials being rendered.
     iDiffuseLayer,          //!< The layer containing the diffuse contribution to the final image.
     iSpecularLayer,         //!< The layer containing the specular contribution to the final image.
     iGlossyLayer,           //!< The layer containing the glossy contribution to the final image.
     iEmissionLayer,         //!< The layer containing any emission contribution to the final image.
     iIrradianceLayer,       //!< The layer containing irradiance data.
     iAmbientOcclusionLayer, //!< The layer containing ambient occlusion data.
     iIrradianceProbeLayer,  //!< The layer containing irradiance probe data.
     iNUM_RenderLayerTypes,  //!< Internal marker used to count all allowed layers.
     iUnknown                //!< Used to indicate out of range errors.
}IrayLayerType;
	  
//C++ 11 enum class

enum class BumpMapMode
{
	BumpTexture=0,
	NormalTexture,
	PerlinNoise,
	WorleyNoise,
	FlowNoise,
	FlakeNoise
};
enum class ColourShaderMode
{
	Plain = 0,
	TextureLookUp,
	PerlinNoise,
	WorleyNoise,
	FlowNoise,
	FlakeNoise
};
enum class FloatShaderMode
{
	Scalar = 0,
	TextureLookUp,
	PerlinNoise,
	WorleyNoise,
	FlowNoise,
	FlakeNoise
};
enum class TextureSpaceMode
{
	UV = 0,
	Planar,
	Box,
	Spherical,
	Cylindrical,
	Global
};
enum class EmissiveShaderMode
{
	Plain=0,
	TextureLookUp
};

enum class AxesModelType
{
	World,
	Local,
	Screen
};
enum class CameraType
{
	Perspective,
	Orthographic
};

enum class RealTimeMatGroupType
{
	//Shader,
	Opacity,
	Light,
	Advance,
	Texture,
	ShaderParams,
	ShaderGen,
	VertexDeform
};


enum class EnviromentType
{
	GI,
	TOD
};

struct NodeParams
{
	int viewDistance = 100;
	bool castShadow = true;
};
struct VertexData {
	float pos[3];
	float normal[3];
	float uv[2];
	unsigned int color = 0;
	VertexData operator + (const VertexData& other)
	{
		VertexData res = *this;

		res.pos[0] += other.pos[0];
		res.pos[1] += other.pos[1];
		res.pos[2] += other.pos[2];

		res.normal[0] += other.normal[0];
		res.normal[1] += other.normal[1];
		res.normal[2] += other.normal[2];

		res.uv[0] += other.uv[0];
		res.uv[1] += other.uv[1];

		return res;
	}
	VertexData operator * (float f)
	{
		VertexData res = *this;

		res.pos[0] *= f;
		res.pos[1] *= f;
		res.pos[2] *= f;

		res.normal[0] *= f;
		res.normal[1] *= f;
		res.normal[2] *= f;

		res.uv[0] *= f;
		res.uv[1] *= f;
		return res;
	}

};
struct VertexBuffer {
	int vertexCount = 0;
	int indexCount = 0;
	int normalCount = 0;
	int textureCount = 0;
	float * textures = nullptr;
	float * normals = nullptr;
	float* vertexList = nullptr;
	unsigned int* indexList = nullptr;
	
	VertexBuffer() {}
	VertexBuffer(VertexBuffer** ppData, int count)
	{
		int vertexCount = 0;
		int mindexCount = 0;
		for (int i = 0; i < count; ++i)
		{
			if (ppData[i])
			{
				vertexCount += ppData[i]->getVertexSize();
				mindexCount += ppData[i]->indexCount;
			}

		}
		this->Resize(vertexCount, mindexCount);
		int vertexStart = 0;
		int uvStart = 0;
		int normalStart = 0;
		int indexStart = 0;
		for (int i = 0; i < count; ++i)
		{
			if (ppData[i] != nullptr)
			{
				if (ppData[i]->indexList)
				{
					if (vertexStart > 0)
					{
						int add = vertexStart / 3;
						for (int j = 0; j < ppData[i]->indexCount; ++j)
							this->indexList[j + indexStart] = ppData[i]->indexList[j] + add;
					}
					else
						memcpy(this->indexList + indexStart, ppData[i]->indexList, sizeof(unsigned int)*ppData[i]->indexCount);
					indexStart += ppData[i]->indexCount;
				}
				if (ppData[i]->vertexList)
				{
					memcpy(this->vertexList + vertexStart, ppData[i]->vertexList, sizeof(float)*ppData[i]->vertexCount);
					vertexStart += ppData[i]->vertexCount;
				}
				if (ppData[i]->normals)
				{
					memcpy(this->normals + normalStart, ppData[i]->normals, sizeof(float)*ppData[i]->normalCount);
					normalStart += ppData[i]->normalCount;
				}
				if (ppData[i]->textures)
				{
					memcpy(this->textures + uvStart, ppData[i]->textures, sizeof(float)*ppData[i]->textureCount);
					uvStart += ppData[i]->textureCount;
				}

			}

		}
	}
	~VertexBuffer() {
		Release();
	}
	void Release()
	{
		if (vertexCount > 0) {
			delete[]vertexList;
			vertexList = nullptr;
			vertexCount = 0;
		}
		if (textureCount > 0) {
			delete[]textures;
			textures = nullptr;
			textureCount = 0;
		}
		if (normalCount > 0) {
			delete[]normals;
			normals = nullptr;
			normalCount = 0;
		}
		if (indexCount > 0) {
			delete[]indexList;
			indexList = nullptr;
			indexCount = 0;
		}
	}
	bool Resize(int vertexCount, int indexCount)
	{
		float * textures0 = nullptr;
		float * normals0 = nullptr;
		float* vertexList0 = nullptr;
		unsigned int* indexList0 = nullptr;
		int vertexCount0 = 0;
		int indexCount0 = 0;
		int normalCount0 = 0;
		int textureCount0 = 0;
		if (vertexCount > 0)
		{
			vertexList0 = new float[3 * vertexCount];
			if (!vertexList0)
			{
				return false;
			}
			vertexCount0 = vertexCount * 3;
			memcpy(vertexList0, this->vertexList, sizeof(float)*(this->vertexCount < vertexCount0 ? this->vertexCount : vertexCount0));
			if (this->vertexList)
				delete[] this->vertexList;
			this->vertexCount = 0;
			normals0 = new float[3 * vertexCount];
			if (!normals0)
			{
				vertexCount0 = 0;
				delete[] vertexList0;
				return false;
			}
			normalCount0 = vertexCount * 3;
			memcpy(normals0, this->normals, sizeof(float)*(this->normalCount < normalCount0 ? this->normalCount : normalCount0));
			if (this->normals)
				delete[] this->normals;
			this->normalCount = 0;
			textures0 = new float[2 * vertexCount];
			if (!textures0)
			{
				vertexCount0 = 0;
				normalCount0 = 0;
				delete[] vertexList0;
				delete[] normals0;
				return false;
			}
			textureCount0 = vertexCount * 2;
			memcpy(textures0, this->textures, sizeof(float)* (this->textureCount < textureCount0 ? this->textureCount : textureCount0));
			if (this->textures)
				delete[] this->textures;
			this->textureCount = 0;
		}
		if (indexCount > 0)
		{
			indexList0 = new unsigned int[indexCount];
			if (!indexList0)
			{
				vertexCount0 = 0;
				normalCount0 = 0;
				textureCount0 = 0;
				delete[] vertexList0;
				delete[] normals0;
				delete[] textures0;
				return false;
			}
			indexCount0 = indexCount;
			memcpy(indexList0, this->textures, sizeof(unsigned int)*(this->indexCount < indexCount0 ? this->indexCount : indexCount0));
			if (this->indexList)
				delete[] this->indexList;
			this->indexCount = 0;
		}
		//Release();
		this->textures = textures0;
		this->normals = normals0;
		this->vertexList = vertexList0;
		this->indexList = indexList0;
		this->vertexCount = vertexCount0;
		this->indexCount = indexCount0;
		this->normalCount = normalCount0;
		this->textureCount = textureCount0;
		return true;
	}
	int getVertexSize() const
	{
		int res = vertexCount / 3;
		if (res == normalCount / 3 && res == textureCount / 2)
			return res;
		else
			return -1;
	}
	VertexBuffer& operator += (const VertexBuffer& other)
	{
		int mvsize = getVertexSize();
		int ovsize = other.getVertexSize();
		int mindexsize = indexCount;
		Resize(mvsize + ovsize, indexCount + other.indexCount);
		memcpy(vertexList + 3 * mvsize, other.vertexList, 3 * ovsize * sizeof(float));
		memcpy(normals + 3 * mvsize, other.normals, 3 * ovsize * sizeof(float));
		memcpy(textures + 2 * mvsize, other.textures, 2 * ovsize * sizeof(float));
		for (int i = 0; i < other.indexCount; ++i)
		{
			indexList[mindexsize + i] = other.indexList[i] + mvsize;
		}
		return *this;
	}
	VertexData getVertexData(int index) const
	{
		VertexData res;
		memcpy(res.pos, vertexList + 3 * index, 3 * sizeof(float));
		memcpy(res.normal, normals + 3 * index, 3 * sizeof(float));
		memcpy(res.uv, textures + 2 * index, 2 * sizeof(float));
		/*memcpy(res.pos, &vertexList[3 * index], 3 * sizeof(float));
		memcpy(res.normal, &normals[3 * index], 3 * sizeof(float));
		memcpy(res.uv, &textures[2 * index], 2 * sizeof(float));*/
		return res;
	}
	bool setVertexData(const VertexData& data, int index)
	{
		memcpy(vertexList + 3 * index, data.pos, 3 * sizeof(float));
		memcpy(normals + 3 * index, data.normal, 3 * sizeof(float));
		memcpy(textures + 2 * index, data.uv, 2 * sizeof(float));
		/*memcpy(&vertexList[3 * index], data.pos, 3 * sizeof(float));
		memcpy(&normals[3 * index], data.normal, 3 * sizeof(float));
		memcpy(&textures[2 * index], data.uv, 2 * sizeof(float));*/
		return true;
	}
};


//UI mode name
#define LOOK_EDITOR "look_editor"
#define LIGHT_EDITOR "light_editor"
#define OBJ_EDITOR "object_editor"
#define TESS_EDITOR "tessellate"
#define ENV_SETTING "env_settings"
#define MOD_VEGETATION "veg_props"
#define GLOBAL_ILL "global_ill"

struct MeshData {
	int vertexCount;
	int indicesCount;
	std::string materialName;
	int meshIndex;
	float maxVertex[3];
	float minVertex[3];
	float worldMat[16];
	float* m_pPosList;
	float* m_Normals;
	unsigned int* m_indices;
	float* m_TexCoords;
	float* m_Tangents;
	float* m_Bitangent;
	
};

enum openFileFlags
{
	RM_FILE_FLAG_NEW = BIT(0),
	RM_FILE_FLAG_INSERT = BIT(1),
	RM_FILE_FLAG_SAVED_FILE=BIT(2),
	RM_FILE_FLAG_ADD_MAT_TO_SCENE=BIT(3),
};

typedef struct _RM_FileInfo
{
	std::string m_absolutePath;
	std::string m_dir;
	std::string m_filename;
	std::string m_ext;
	int m_saveMode = 0;

	_RM_FileInfo(std::string abpath)
	{
		FileDescription(abpath);
	}
	_RM_FileInfo(std::string abpath, int saveMode)
	{
		FileDescription(abpath);
		m_saveMode = saveMode;
	}
private:
	void FileDescription(std::string abpath)
	{
		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];
		char filename[_MAX_FNAME];
		char ext[_MAX_EXT];
		_splitpath_s(abpath.c_str(), drive, dir, filename, ext);

		m_absolutePath = abpath;
		m_dir = (std::string)drive + (std::string)dir + (std::string)filename + "\\";
		m_filename = filename;
		m_ext = ext;
	}
}RM_FileInfo;

struct T_OpenfileInfo
{
	int iFileflag;
	int iInsertPosition;
	const RM_FileInfo * rFileinfo;
private:
	std::string sFile;
public:
	T_OpenfileInfo(std::string file)
	{
		rFileinfo = new RM_FileInfo(file);
	}
	T_OpenfileInfo(std::string file, bool package)
	{
		rFileinfo = new RM_FileInfo(file,package);
	}
};

enum renderMaxNodeInfo
{
	RM_NODE_FLAG_CAN_HIDE = BIT(0),
	RM_NODE_FLAG_CAN_DELET = BIT(1),
	RM_NODE_FLAG_CAN_DRAG = BIT(2),
	RM_NODE_FLAG_CAN_DROP = BIT(3),
	RM_NODE_FLAG_VISIBLE1 = BIT(4),		//UI上自身是否显示状态（勾上）
	RM_NODE_FLAG_VISIBLE2 = BIT(5),		//UI上父节点是否显示状态（逐级上查）	  visible12 结合使用。
	RM_NODE_FLAG_HIDEFORDELETE=BIT(6), //隐藏以删除
	RM_NODE_FLAG_ALL = RM_NODE_FLAG_CAN_HIDE | RM_NODE_FLAG_CAN_DELET
										| RM_NODE_FLAG_CAN_DRAG | RM_NODE_FLAG_CAN_DROP	
										| RM_NODE_FLAG_VISIBLE1| RM_NODE_FLAG_VISIBLE2
										|~RM_NODE_FLAG_HIDEFORDELETE
};

#define INVALID_SCNENE_ITEMID  -1000
#define INVALID_ANIMATION_NODEID -1000
#define INVALID_VEGETAION_NODEID -1000

struct MatBallSceneData {
	float m_camEyes[3];
	float m_camLookat[3];
	float m_camUp[3];
	float m_lightRadio;
	float m_lightMat[16];
	float m_ProbeMat[16];
	float maxVertex[3];
	float minVertex[3];
};

///////////nurbsInfo
struct Nurbs_Face {
	int vertexCount = 0;
	int indicescount = 0;
	float* vertices;
	unsigned int* indices;
};

//htc handle button type
enum HTCHandleButton_type {
	L_TouchPadPress = 0,
	L_TouchPadTouch = 1,
	L_TriggerVal = 2,
	L_TriggerGesture = 3,
	L_MenuPress = 4,
	L_GrippedPress = 5,
	L_TouchValX = 6,
	L_TouchValY = 7,
	R_TouchPadPress = 8,
	R_TouchPadTouch = 9,
	R_TriggerVal = 10,
	R_TriggerGesture = 11,
	R_MenuPress = 12,
	R_GrippedPress = 13,
	R_TouchValX = 14,
	R_TouchValY = 15,
};

 struct HTC_PosMessage {
	bool m_bShow;
	float m_PeoplePos[3];
	float m_quat[4];
	float m_LeftHandlePos[3];
	float m_LeftHandlequat[4];
	float m_RightHandlePos[3];
	float m_RightHandlequat[4];
	size_t m_ClientId;
	bool m_isLeader;
};

struct VERTEX_P3F_CU_T2F
{
	float pos[3];
	unsigned int color;
	float texcood[2];
};

struct CutVertexBuffer {
	VERTEX_P3F_CU_T2F* pVertex = nullptr;
	float* pNormal = nullptr;
	unsigned int* pIndex = nullptr;
	unsigned int vertexCount = 0;
	unsigned int indexCount = 0;
	~CutVertexBuffer() {
		Release();
	}
	void Release() {
		if (vertexCount > 0)
		{
			if (pVertex)
			{
				delete[] pVertex;
				pVertex = nullptr;
			}
				
			if (pNormal)
			{
				delete[] pNormal;
				pNormal = nullptr;
			}
				
			vertexCount = 0;
		}
		if (indexCount > 0)
		{
			if (pIndex)
			{
				delete[] pIndex;
				pIndex = nullptr;
			}
			indexCount = 0;
		}
	}
	void Resize(unsigned int vCount, unsigned int iCount) {
		VERTEX_P3F_CU_T2F* pNewVertex = new VERTEX_P3F_CU_T2F[vCount];
		float* pNewNormal = new float[3 * vCount];
		unsigned int* pNewIndex = new unsigned int[iCount];
		unsigned count = 0;
		if (vertexCount > 0)
		{
			count = vCount < vertexCount ? vCount : vertexCount;
			memcpy(pNewVertex, pVertex, count * sizeof(VERTEX_P3F_CU_T2F));
			memcpy(pNewNormal, pNormal, count * 3 * sizeof(float));
		}
		
		if (indexCount > 0)
		{
			count = iCount < indexCount ? iCount : indexCount;
			memcpy(pNewIndex, pIndex, count * sizeof(unsigned int));
		}
		
		
		Release();
		pVertex = pNewVertex;
		pNormal = pNewNormal;
		pIndex = pNewIndex;
		vertexCount = vCount;
		indexCount = iCount;
	}
	VertexData getVertexData(int index) const
	{
		VertexData res;
		memcpy(res.pos, pVertex[index].pos, 3 * sizeof(float));
		memcpy(res.uv, pVertex[index].texcood, 2 * sizeof(float));
		res.color = pVertex[index].color;
		memcpy(res.normal, pNormal + 3 * index, 3 * sizeof(float));
		return res;
	}
	bool setVertexData(const VertexData& data, int index)
	{
		/*if (index >= vertexCount)
			return false;*/
		memcpy(pVertex[index].pos, data.pos, 3 * sizeof(float));
		memcpy(pVertex[index].texcood, data.uv, 2 * sizeof(float));
		pVertex[index].color = data.color;
		memcpy(pNormal + 3 * index, data.normal, 3 * sizeof(float));
		return true;
	}
	int getVertexSize() { return vertexCount; }
};