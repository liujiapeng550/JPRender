#pragma once

#include <optixu\optixu_math_namespace.h>


// Spherical Harmonics Local Definitions
RT_FUNC static void legendrep(float x, int lmax, float *out) {
#define P(l,m) out[SHIndex(l,m)]
	// Compute $m=0$ Legendre values using recurrence
	P(0, 0) = 1.f;
	P(1, 0) = x;
	for (int l = 2; l <= lmax; ++l) {
		P(l, 0) = ((2 * l - 1)*x*P(l - 1, 0) - (l - 1)*P(l - 2, 0)) / l;
	}

	// Compute $m=l$ edge using Legendre recurrence
	float neg = -1.f;
	float dfact = 1.f;
	float xroot = sqrtf(fmaxf(0.f, 1.f - x*x));
	float xpow = xroot;
	for (int l = 1; l <= lmax; ++l) {
		P(l, l) = neg * dfact * xpow;
		neg *= -1.f;      // neg = (-1)^l
		dfact *= 2 * l + 1; // dfact = (2*l-1)!!
		xpow *= xroot;    // xpow = powf(1.f - x*x, float(l) * 0.5f);
	}

	// Compute $m=l-1$ edge using Legendre recurrence
	for (int l = 2; l <= lmax; ++l) {
		P(l, l - 1) = x * (2 * l - 1) * P(l - 1, l - 1);
	}

	// Compute $m=1, \ldots, l-2$ values using Legendre recurrence
	for (int l = 3; l <= lmax; ++l)
		for (int m = 1; m <= l - 2; ++m) {
			P(l, m) = ((2 * (l - 1) + 1) * x * P(l - 1, m) -
				(l - 1 + m) * P(l - 2, m)) / (l - m);
		}
#if 0
	// wrap up with the negative m ones now
	// P(l,-m)(x) = -1^m (l-m)!/(l+m)! P(l,m)(x)
	for (int l = 1; l <= lmax; ++l) {
		float fa = 1.f, fb = fact(2 * l);
		// fa = fact(l+m), fb = fact(l-m)
		for (int m = -l; m < 0; ++m) {
			float neg = ((-m) & 0x1) ? -1.f : 1.f;
			P(l, m) = neg * fa / fb * P(l, -m);
			fb /= l - m;
			fa *= (l + m + 1) > 1 ? (l + m + 1) : 1.;
		}
	}
#endif
#undef P
}

RT_FUNC static inline float fact(float v);
RT_FUNC static inline float divfact(int a, int b);
RT_FUNC static inline float K(int l, int m) {
	return sqrtf((2.f * l + 1.f) * M_1_4PIf * divfact(l, m));
}

RT_FUNC static inline float divfact(int a, int b) {
	if (b == 0) return 1.f;
	float fa = a, fb = fabsf(b);
	float v = 1.f;
	for (float x = fa - fb + 1.f; x <= fa + fb; x += 1.f)
		v *= x;
	return 1.f / v;
}

// n!! = 1 if n==0 or 1, otherwise n * (n-2)!!
RT_FUNC static float dfact(float v) {
	if (v <= 1.f) return 1.f;
	return v * dfact(v - 2.f);
}


RT_FUNC static inline float fact(float v) {
	if (v <= 1.f) return 1.f;
	return v * fact(v - 1.f);
}

RT_FUNC static void sinCosIndexed(float s, float c, int n,
	float *sout, float *cout) {
	float si = 0, ci = 1;
	for (int i = 0; i < n; ++i) {
		// Compute $\sin{}i\phi$ and $\cos{}i\phi$ using recurrence
		*sout++ = si;
		*cout++ = ci;
		float oldsi = si;
		si = si * c + ci * s;
		ci = ci * c - oldsi * s;
	}
}

// Spherical Harmonics Definitions
RT_FUNC void SHEvaluate_H(const Nor &w_g, float *out) {
	// Compute Legendre polynomial values for $\cos\theta$
	legendrep(w_g.z, SH_L_MAX, out);

	// Compute $K_l^m$ coefficients
	float Klm[SHTerms];
	for (int l = 0; l <= SH_L_MAX; ++l)
		for (int m = -l; m <= l; ++m)
			Klm[SHIndex(l, m)] = K(l, m);

	// Compute $\sin\phi$ and $\cos\phi$ values
	float sins[SH_L_MAX + 1u];
	float coss[SH_L_MAX + 1u];
	float xyLen = sqrtf(fmaxf(0.f, 1.f - w_g.z*w_g.z));
	if (xyLen == 0.f) {
		for (int i = 0; i <= SH_L_MAX; ++i) sins[i] = 0.f;
		for (int i = 0; i <= SH_L_MAX; ++i) coss[i] = 1.f;
	} else
		sinCosIndexed(w_g.y / xyLen, w_g.x / xyLen, SH_L_MAX + 1, sins, coss);

	// Apply SH definitions to compute final $(l,m)$ values
	for (int l = 0; l <= SH_L_MAX; ++l) {
		for (int m = -l; m < 0; ++m) {
			out[SHIndex(l, m)] = M_SQRT2f* Klm[SHIndex(l, m)] * out[SHIndex(l, -m)] * sins[-m];
		}
		out[SHIndex(l, 0)] *= Klm[SHIndex(l, 0)];
		for (int m = 1; m <= l; ++m) {
			out[SHIndex(l, m)] *= M_SQRT2f * Klm[SHIndex(l, m)] * coss[m];
		}
	}
}
