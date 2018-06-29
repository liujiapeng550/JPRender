#pragma once

//==========================================================================================
// constant define
#define M_MAX_UINT 0x7FFFFFFF
#define M_MAX_FLOAT 1000000.f
#define M_MIN_FLOAT 0.000001f
#define M_RAND_MAX 0x7fff
#define M_MAX_FLAKE_SIZE 0.025f

#define M_MIN_IMPORTANCE 0.001f
#define M_MIN_PDF 0.001f
#define M_MIN_RADIANCE 0.000001f

#define M_EPSILON 0.000001f
#define M_1_MINUS_EPSILON (1.f-M_EPSILON)


#define M_PIf 3.14159265358979323846f
#define M_2PIf (6.283185307179586476925286766559f)
#define M_1_2PIf (0.15915494309189533576888376337251f)
#define M_4PIf (12.566370614359172953850573533118f)
#define M_8PIf (25.132741228718345907701147066236f)
#define M_1_4PIf (0.07957747154594766788444188168626f)
#define M_RADIAN_PER_DEGREE (0.01745329251994329576923690768489f)
#define M_DEGREE_PER_RADIAN (57.295779513082320876798154814105f)
#define F_RADIAN(degree) (M_RADIAN_PER_DEGREE*(degree))
#define F_DEGREE(radian) (M_DEGREE_PER_RADIAN*(radian))
#define M_255f 255.0f
#define M_1_255f (0.0039215686274509803921568627451f)

//==========================================================================================
// Spherical Harmonics Declarations
// 左手坐标系，z轴向上
// 下面做了简化，将Y1_1 Y11 Y2_1 Y21的负号省去，因为在通过SH函数与系数计算radiance时候负负得正。
#define M_MATERIAL_ID_NONE -9999
#define SH_L_MAX 2u
#define SHTerms 9u //#define SHTerms(lmax) ((lmax + 1) * (lmax + 1))
#define SHIndex(l, m) ( l*l + l + m)
#define Y00 0.282095f
#define Y1_1(y)  (0.488603f*y)
#define Y10(z)   (0.488603f*z)
#define Y11(x)   (0.488603f*x)
#define Y2_2(x,y) (1.092548f*x*y)
#define Y2_1(y,z) (1.092548f*y*z)
#define Y20(z)   (0.315392f*(3*z*z-1.f))
#define Y21(x,z) (1.092548f*x*z)
#define Y22(x,y) (0.546274f*(x*x-y*y))
#define Y3_3(x,y) (0.590043589927f*y*(3*x*x-y*y))
#define Y3_2(x,y,z) (2.89061144264f*x*y*z)
#define Y3_1(y,z) (0.457045799464f*y*(5*z*z-1.f)
#define Y30(z) (0.37317633259f*z*(5.f*z*z-3.f)
#define Y31(x,z) (0.457045799464f*x*(5.f*z*z-1.f)
#define Y32(x,y,z) (1.44530572132f*z*(x*x-y*y))
#define Y33(x,y) (0.590043589927f*x*(x*x-3.f*y*y))
#define A0 3.141593f
#define A1 2.094395f
#define A2 0.785398f
#define A3 0.f
#define A4 -0.130900f
#define A5 0.f
#define A6 0.049087f


//==========================================================================================
// The following Macros defines that all variables declared with both C and G side.
/// 这些名字不能改变，是optix的內建变量名；
#define V_C_TONEMAPPED_INPUT_BUFFER input_buffer
#define V_C_TONEMAPPED_OUTPUT_BUFFER output_buffer
#define V_C_TONEMAPPED_EXPOSURE exposure
#define V_C_TONEMAPPED_GAMMA gamma
#define V_C_DENOISER_INPUT_BUFFER input_buffer
#define V_C_DENOISER_OUTPUT_BUFFER output_buffer
#define V_C_DENOISER_NORMAL_BUFFER input_normal_buffer
#define V_C_DENOISER_ALBEDO_BUFFER input_albedo_buffer
#define V_C_DENOISER_BLEND blend
#define NOACCEL "NoAccel"
#define TRBVH "Trbvh"
#define BVH "Bvh"
#define SBVH "Sbvh"

///这些名字可以改变；
#define CF_GETTRIPLANENORMAL GetTriplanNormal
#define CF_GETTEXTURE_DIFFUSE GetTexture_diffuse
#define CF_GETTEXTURE_GLOSSY GetTexture_glossy
#define CF_GETTEXTURE_NORMAL GetTexture_normal
#define CF_GETSKYBOXRADIANCE GetTextur_Dir
#define CF_MERGETEXTURE MergeTexture
#define CF_GETRADIANCEFROMSH GetRadianceFromSH
#define CF_GETRANDOMSEED GetRandomSeed
#define CF_GETSAMPLE1 GetSample1
#define CF_GETSAMPLE2 GetSample2
#define CF_UNIFORMSAMPLETRIANGLE UniformSampleTriangle
#define CF_COSINESAMPLEHEMISPHERE_QUICKER CosineSampleHemisphere_quicker
#define CF_GETENVRADIANCE GetEnvRadiance
#define CF_CONCENTRICSAMPLEDISK ConcentricSampleDisk
#define CF_CHECKERBOARD3 CheckerBoard3

#define CF_DISTRIBUTION1D_SAMPLEDISCRETE Distribution1D_SampleDiscrete
#define CF_DISTRIBUTION2D_SAMPLECONTINUOUS Distribution2D_SampleContinuous
#define CF_DISTRIBUTION2D_PDF Distribution2D_Pdf


#define CFINDEX_LIGHT_POINT 0
#define CFINDEX_LIGHT_SPOT 1
#define CFINDEX_LIGHT_DISTANT 2
#define CFINDEX_LIGHT_ENVIRONMENT 3
#define CFINDEX_LIGHT_AREA 4


#define CFINDEX_LM_LAMBERTIAN 0
#define CFINDEX_LM_TORRANCESPARROW 1
#define CFINDEX_LM_SPECULAR_REFLECT 2
#define CFINDEX_LM_SPECULAR_REFRACT 3
#define CFINDEX_LM_ASHIKHMIN_SHIRLEY 4
#define CFINDEX_LM_NULL 255

#define V_C_TOP_OBJECT topObject
#define V_C_DATA_CONTEXT dataContext
#define V_C_DATA_RENDERER dataRenderer
#define V_C_DATA_INSTANCE dataInstance
#define L_POINT "Point"
#define L_DISTSNT "Distant"
#define L_SPOT "Spot"
#define L_ENVIRONMENT "EnvironmentProbe"
#define L_AREA_RECTANGLE "Area Rectangle"

#define MATERIAL_NONE_INDEX -1
#define MATERIAL_DEFAULT_INDEX 0
#define MATERIAL_SHADOW_INDEX 1
#define MATERIAL_METAL_INDEX 2
#define MATERIAL_GLASS_INDEX 3
#define MATERIAL_WOOD_INDEX 4
#define MATERIAL_LEATHER_INDEX 5
#define MATERIAL_RUBBER_INDEX 6
#define MATERIAL_CARPAINT_UNIQUE_CLEAR_INDEX 7
#define MATERIAL_CARPAINT_UNIQUE_INDEX 8
#define MATERIAL_CARPAINT_METALLIC_INDEX 9
#define MATERIAL_MATTE_INDEX 10
#define MATERIAL_METAL_BRUSHED_INDEX 11
#define MATERIAL_EMISSIVE_INDEX 12
#define MATERIAL_UV_INDEX 13

enum OptixRayTypes{
  PICKUP=0,
  RADIANCE,
  SHADOW,
  DEPTH,
  AO,
  NORMAL,
  TOTAL,
};

enum EntryPoint{
  E_ENTRYPOINT_PICK=0,
  E_ENTRYPOINT_RADIANCE,
  E_ENTRYPOINT_DEPTH,
  E_ENTRYPOINT_AO,
  E_ENTRYPOINT_NORMAL,
  //e_entryPoint_materialBall,
  E_ENTRYPOINT_TOTAL,
};

