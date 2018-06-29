#pragma once

#include <optix_device.h>

struct Intersection{
  Nor meshNor;
  Nor geoNor;
  UV2 texCoord;
  //optix::float3 back_hit_point;
  //optix::float3 front_hit_point;
	struct {
		Nor Tangent;
		Nor Bitangent;
		Nor Normal;
	} tbn;
	int nodeIndex;// ∞»°mesh”√µΩ£ª
};