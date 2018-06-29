#pragma once  

enum class LIGHT_TYPE {
	POINT = 0,
	AREA,
	SPOT,
	DISTANT,
	ENVIRONMENTPROBE,
	NO_LIGHT,
};

struct SData_Light {
	/*通用属性*/
	unsigned int lightFuncIndex;
	Pos pos;                               //位置
	Color4 diffuse;                           //光源颜色
	Dir dir;                               //光源方向，平行光，聚光灯有用，must be normalized
	float intensity;                  //光源颜色放大倍数；
	float radius;                             //光照半径，在此半径以外的物体不被照亮；点光源与聚光灯使用；
  float totalTheta;                         //聚光灯张角；
  float fullLightTheta;                     //聚光灯完全照亮角度,注：totalTheta>fullLightTheta

	/*自发光源（区域光）相关专属属性*/
	float meshAreaIntegration;
	float sumArea;
	int nodeIndex;
	int meshAreaCDFBufferID;
	int meshAreaBufferID;
	int meshCount;
	int bufferID_index;
	int bufferID_vertex;
	int bufferID_worldMatrix;// 用于区域光的采样与HDR的旋转；
	int bufferID_materials;
	int bufferOffset_materials;

	/*环境光相关属性*/
	optix::uint2 HDR_size;
	float energyBalance;                       
	float HDR_inv_gamma;                       
	float HDR_exposure;                       // 环境光曝光率         
	float HDR_Intensity;	                    // 高光曝光率                     
	float HDR_contrast;                     
	float HDR_saturation;                  
  int HDR_textureSamplerID;
	int HDR_LuminanceBufferID;           
	int HDR_CDFBufferID;                      //二维缓冲区，这个buffer的宽度比image宽度大1个像素；高度与image一样
	int HDR_MarginalCDFBufferID;              //一维缓冲区，这个buffer的宽度比image高度大一个像素；
	int HDR_IntegrationBufferID;              //一维缓冲区，这个buffer的宽度比image高度大一个像素；正常的宽度记录原始图像每一行的积分值，多出来的一位记录上面积分值的积分：；
	int SH_lmax;                              //球谐函数最大的band，此参数暂时无用；
	int SH_cBufferID;		                      //球谐函数的系数buffer


};

