#pragma once

struct SData_Renderer{
  /// 共享属性
  float ray_clamp_min_AO;
  float ray_clamp_max_AO;
  float ray_clamp_min;
  float ray_clamp_max;
  unsigned int samples_per_pixel;
  unsigned int samples_per_pixel_row;
  unsigned int samples_per_pixel_col;
  unsigned int frame_current;
  int output_FinalRadianceBufID;
  int output_AODesityBufID;
  int output_launchBufID;
  bool enableAO;
  optix::float3 camera_pos;
  optix::float3 camera_U;
  optix::float3 camera_V;
  optix::float3 camera_W;
	optix::Matrix3x3 matrix_WorldtoCamera;
  optix::float4 color_bad;
  int bufferID_lights;


	/// 常规渲染器专用；
  unsigned int ray_iterateDepth;
  float camera_lensRadius;
  float camera_focalLength;
  bool camera_enableDOF;
  bool enableGI;
  bool enableAIdenoise;
  float radiance_importanceThreshold;
  bool skybox_enableBackground;
  int skybox_TSID_show;
  int output_stageNormalBufID;
  int output_stageAlbedoBufID;


  ///深度渲染器专属属性；
  float DepthRenderer_depth_min;// 深度小于min的全部是白色
  float DepthRenderer_depth_max;// 深度大于max的全部为黑色，min max之间的部分线性过度；


	/// 法线渲染器专属；
	bool enableShowCameraSpaceNormal;

};
