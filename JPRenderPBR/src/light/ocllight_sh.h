#pragma once

#include <optixu\optixpp_namespace.h>
#include "core\oclcore.h"
#include "core\oclrng.h"

OCL_NAMESPACE_START

/**
 * v				direction, in world space;
 * lmax			
 * out			output SH function value;
 */
void SHEvaluate(const oclFloat3 &v, oclFloat *out);
void SHReduceRinging(oclFloat3 *c, oclFloat lambda = .005f);

OCL_NAMESPACE_END
