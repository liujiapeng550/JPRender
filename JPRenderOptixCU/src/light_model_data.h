#pragma once
#include "cu_common.h"

struct LMD{
	Nor sn_g;
	Nor wo_g;
	Nor wi_g;
	Pos hitPos_g;
	float PDF,BRDF;
	Radiance4 L;
	float albedo;
	union{
		float roughness;
		float IOR;
	};

	// tmp use
	float2 anisotropicUV;
	float2 reflatance;// x:diffuse,y:specular
	Dir tangent_g,bitangent_g;



	//debug info
	uint2 index;
	uint2 dim;
	/*
	*/

	RT_FUNC_F void NormalizeWi(){ wi_g=normalize(wi_g); }
	RT_FUNC_F bool WoWiAtSamehemisphere()const{
		return dot(wo_g,sn_g)*dot(wi_g,sn_g)>0.f;
	}

	RT_FUNC_F Nor GetWh_g()const{
		return normalize(wi_g+wo_g);
	}

	RT_FUNC_F float WiDotN()const{
		return dot(wi_g,sn_g);
	}
	RT_FUNC_F float WoDotN()const{
		return dot(wo_g,sn_g);
	}

};