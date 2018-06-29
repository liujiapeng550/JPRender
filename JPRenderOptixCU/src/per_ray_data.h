#pragma once

struct PRD{
  unsigned int seed;
  unsigned int uDepth;//从相机开始，历史上追踪的深度，从相机发出的射线，其深度为0，首次相交后的第一条射线，深度为1；
  Pos hp_g;
  Nor n_g;
	
	// radiance use only
	Nor wi_g;
	Color4 color_out;
	Albedo4 albedo;
	float lastMaterialEnvBlend;
	Radiance4 luminance_out;
	int materialIndex;
	float BRDF;
	float PDF;
  bool missed;

	// AO use only
	float AO_intensity;
	bool AO_nextIteration;
	float AO_distancePercent;
};