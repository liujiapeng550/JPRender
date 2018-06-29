#pragma once

#include "oclcore.h"

OCL_NAMESPACE_START

inline oclFloat VanDerCorput(oclUint n, oclUint scramble) {
	// Reverse bits of _n_
	n = (n << 16) | (n >> 16);
	n = ((n & 0x00ff00ff) << 8) | ((n & 0xff00ff00) >> 8);
	n = ((n & 0x0f0f0f0f) << 4) | ((n & 0xf0f0f0f0) >> 4);
	n = ((n & 0x33333333) << 2) | ((n & 0xcccccccc) >> 2);
	n = ((n & 0x55555555) << 1) | ((n & 0xaaaaaaaa) >> 1);
	n ^= scramble;
	return fminf(((n >> 8) & 0xffffff) / oclFloat(1 << 24), M_1_MINUS_EPSILON);
}


inline oclFloat Sobol2(oclUint n, oclUint scramble) {
	for (oclUint v = 1 << 31; n != 0; n >>= 1, v ^= v >> 1)
		if (n & 0x1) scramble ^= v;
	return fminf(((scramble >> 8) & 0xffffff) / oclFloat(1 << 24), M_1_MINUS_EPSILON);
}


// Sampling Inline Functions
inline void Sample02(oclUint n, const oclUint scramble[2],
	oclFloat sample[2]) {
	sample[0] = VanDerCorput(n, scramble[0]);
	sample[1] = Sobol2(n, scramble[1]);
}


OCL_NAMESPACE_END