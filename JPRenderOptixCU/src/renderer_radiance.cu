#include <optix_device.h>
#include <optixu\optixu_math_namespace.h>
#include <optixu\optixu_matrix_namespace.h>
#include "cu_common.h"
#include "per_ray_data.h"
#include "../../host_device/data_context.h"
#include "../../host_device/data_renderer.h"

using namespace optix;

#pragma region declare

rtDeclareVariable(Ray,dRay,rtCurrentRay,);
rtDeclareVariable(uint2,dLd,rtLaunchDim,);
rtDeclareVariable(uint2,dLi,rtLaunchIndex,);
rtDeclareVariable(rtObject,V_C_TOP_OBJECT,,);
rtDeclareVariable(SData_Renderer,V_C_DATA_RENDERER,,);


rtDeclareVariable(rtCallableProgramId<uint(uint,uint)>,CF_GETRANDOMSEED,,);
rtDeclareVariable(rtCallableProgramId<RandomFloat2(uint&)>,CF_GETSAMPLE2,,);
rtDeclareVariable(rtCallableProgramId<Dir(const RandomFloat2&,float,float)>,CF_COSINESAMPLEHEMISPHERE_QUICKER,,);
rtDeclareVariable(rtCallableProgramId<float(int,int,const Nor&,float,Color4&)>,CF_GETENVRADIANCE,,);
rtDeclareVariable(rtCallableProgramId<void(RandomFloat1,RandomFloat1,RandomVariable1*,RandomVariable1*)>,CF_CONCENTRICSAMPLEDISK,,);

#pragma endregion

#define __ONLY_FUNC__
#include "common_AO_launch.h"
#include "prepare_launch.h"
#undef __ONLY_FUNC__


RT_FUNC_F float luminanceCIE(const float4& rgba){
	const float4 ntsc_luminance={0.30f, 0.59f, 0.11f,0.f};
	return dot(rgba,ntsc_luminance);
}

RT_PROGRAM void PgmLaunch_radiance(){

	/// 1. 通用计算；
	int previousMaterialIndex=MATERIAL_NONE_INDEX;
	Nor rayDirection; uint seed;
	PrepareLaunch(rayDirection,seed);
	PRD prd;


	/// 2. 计算景深
	if(V_C_DATA_RENDERER.camera_enableDOF){
		const float dirLen=length(rayDirection);
		const float wLen=length(V_C_DATA_RENDERER.camera_W);
		const float t=dirLen*V_C_DATA_RENDERER.camera_focalLength/wLen;
		rayDirection=normalize(rayDirection);
		const Pos p1=V_C_DATA_RENDERER.camera_pos+t*rayDirection;

		// lens sample;
		RandomFloat2 rf2=CF_GETSAMPLE2(seed);
		RandomVariable2 out;
		CF_CONCENTRICSAMPLEDISK(rf2.x,rf2.y,&out.x,&out.y);
		out*=V_C_DATA_RENDERER.camera_lensRadius;
		prd.hp_g=V_C_DATA_RENDERER.camera_pos+(out.x*V_C_DATA_RENDERER.camera_U+out.y*V_C_DATA_RENDERER.camera_V);

		// Ray direction;
		rayDirection=p1-prd.hp_g;
	} else{
		prd.hp_g=V_C_DATA_RENDERER.camera_pos;
	}
	rayDirection=normalize(rayDirection);


	/// 4. 迭代发射
	float4 historySum=make_float4(1.f);
	bool firstFlag=true;
	Radiance4 L_final=make_float4(0.f,0.f,0.f,1.f);
	float4 nor_final=make_float4(0.f);
	Albedo4 albedo_final=make_float4(1.f);
	prd.missed=false;
	prd.uDepth=0u;
	prd.lastMaterialEnvBlend=10.f;
	prd.seed=seed;
	prd.materialIndex=MATERIAL_NONE_INDEX;
	Ray ray=make_Ray(prd.hp_g,rayDirection,OptixRayTypes::RADIANCE,V_C_DATA_RENDERER.ray_clamp_min,V_C_DATA_RENDERER.ray_clamp_max);

	while(prd.uDepth<=V_C_DATA_RENDERER.ray_iterateDepth){
		/// 4.0 发射射线；
		rtTrace(V_C_TOP_OBJECT,ray,prd);

#if 0
		//test
		if(uint(dLd.x*.5f)==dLi.x && uint(dLd.y*.5f)==dLi.y){
			rtPrintf("wo_g[%f,%f,%f],hit[%f,%f,%f],nor[%f,%f,%f],",
				-rayDirection.x,-rayDirection.y,-rayDirection.z,
				prd.hp_g.x,prd.hp_g.y,prd.hp_g.z,
				prd.n_g.x,prd.n_g.y,prd.n_g.z
			);
			rtPrintf("wi_g[%f,%f,%f],color_out[%f,%f,%f]\n",
				prd.wi_g.x,prd.wi_g.y,prd.wi_g.z,
				prd.color_out.x,prd.color_out.y,prd.color_out.z
			);
			rtPrintf("history[%f,%f,%f],lumi[%f],BRDF[%f],PDF[%f],depth[%d],seed[%d]\n",
				historySum.x,historySum.y,historySum.z,
				prd.luminance_out,
				prd.BRDF,
				prd.PDF,
				prd.uDepth,
				prd.seed
			);
		}
#endif


		if(dot(historySum,prd.color_out)<=M_EPSILON)break;

		prd.color_out*=2.f/(1.f+luminanceCIE(prd.color_out));

		/// 4.2 累积计算结果；
		L_final+=prd.color_out*prd.luminance_out*historySum;
		if(firstFlag){
			firstFlag=false;
			if(V_C_DATA_RENDERER.enableAIdenoise){
				Nor4 normal_eyespace=make_float4((length(prd.n_g)>0.f)?normalize(V_C_DATA_RENDERER.matrix_WorldtoCamera * prd.n_g):make_float3(0.f,0.f,1.f),1.f);
				if(V_C_DATA_RENDERER.frame_current>1u){
					const float weight=1.f/(float)V_C_DATA_RENDERER.frame_current;

					float4 tmp;
					tmp=GET_F4_2D(V_C_DATA_RENDERER.output_stageNormalBufID,dLi);
					normal_eyespace=lerp(tmp,normal_eyespace,weight);

					tmp=GET_F4_2D(V_C_DATA_RENDERER.output_stageAlbedoBufID,dLi);
					prd.albedo=lerp(tmp,prd.albedo,weight);
				}
				SET_F4_2D(V_C_DATA_RENDERER.output_stageNormalBufID,dLi,normal_eyespace);
				SET_F4_2D(V_C_DATA_RENDERER.output_stageAlbedoBufID,dLi,prd.albedo);
			}
			historySum*=prd.color_out;
		} else{
			if(previousMaterialIndex==MATERIAL_GLASS_INDEX){
				historySum*=prd.color_out;
			}
		}


		if(prd.missed)break;


		/// 4.3 累乘系数，用于迭代；
		historySum*=prd.BRDF;
		historySum*=fabsf(dot(prd.n_g,prd.wi_g));
		historySum/=prd.PDF;
		if(luminanceCIE(historySum)<V_C_DATA_RENDERER.radiance_importanceThreshold)break;

		rayDirection=prd.wi_g;
		ray.origin=prd.hp_g;
		ray.direction=rayDirection;
		previousMaterialIndex=prd.materialIndex;
		++prd.uDepth;
	}// end while


	/// x. 辉度补偿；
	if(prd.uDepth>V_C_DATA_RENDERER.ray_iterateDepth){
		Color4 color;
		float L=CF_GETENVRADIANCE(V_C_DATA_RENDERER.skybox_TSID_show,V_C_DATA_RENDERER.bufferID_lights,rayDirection,prd.lastMaterialEnvBlend,color);
		L_final+=color*L*historySum;
#if 0
		if(uint(dLd.x*.5f)==dLi.x && uint(dLd.y*.5f)==dLi.y){
			rtPrintf("amended L[%f,%f,%f]\n",L*color.x,L*color.y,L*color.z);
		}
#endif
	}

#if 0
	if(uint(dLd.x*.5f)==dLi.x && uint(dLd.y*.5f)==dLi.y){
		rtPrintf("L_final[%f,%f,%f]\n",L_final.x,L_final.y,L_final.z);
	}
#endif



	/// 5. 更新辉度数据；
	const float weight=1.f/(float)V_C_DATA_RENDERER.frame_current;
	if(V_C_DATA_RENDERER.frame_current>1u){
		Radiance4 tmp=GET_F4_2D(V_C_DATA_RENDERER.output_FinalRadianceBufID,dLi);
		L_final=lerp(tmp,L_final,weight);
	}
	SET_F4_2D(V_C_DATA_RENDERER.output_FinalRadianceBufID,dLi,L_final);


	/// 6. 计算AO系数
	float AO_final=1.f;
	if(V_C_DATA_RENDERER.enableAO){
		AO_final=AOCommonLaunch(seed);

		if(V_C_DATA_RENDERER.frame_current>1u){
			float preSum=GET_F_2D(V_C_DATA_RENDERER.output_AODesityBufID,dLi);
			AO_final=lerp(preSum,AO_final,weight);
		}
		SET_F_2D(V_C_DATA_RENDERER.output_AODesityBufID,dLi,AO_final);
	}


	/// 7. 调制辉度与AO
	SET_F4_2D(V_C_DATA_RENDERER.output_launchBufID,dLi,L_final*AO_final);


#if 0
	if(uint(dLd.x*.5f)==dLi.x && uint(dLd.y*.5f)==dLi.y){
		rtPrintf("AO_final[%f]\n",AO_final);
	}
#endif

}

RT_PROGRAM void PgmException(){
	/*
	const int index=dLi.y*dLd.x+dLi.x;
	F_SET_FLOAT4_TO_BUFFER(V_C_DATA_RENDERER.output_launchBufID,index,V_C_DATA_RENDERER.color_bad);
	*/
	SET_F4_2D(V_C_DATA_RENDERER.output_launchBufID,dLi,V_C_DATA_RENDERER.color_bad);
#if 1
	//test
	//result=make_float3(GetEnvironmentTextureColor4(dRay.rayDirection));
	const uint code=rtGetExceptionCode();
	if(code!=RT_EXCEPTION_STACK_OVERFLOW){
		if(uint(dLd.x*.5f)==dLi.x && uint(dLd.y*.5f)==dLi.y){
			rtPrintExceptionDetails();
		}
		//rtPrintf("Caught exception 0x%X at launch index (%d,%d)\n",code,dLi.x,dLi.y);
	}
#endif
}
