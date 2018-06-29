#include "ocllight_sh.h"
#include <optixu\optixu_math_namespace.h>
#include "../../host_device/SH.h"
#include "ocllight_source.h"

OCL_NAMESPACE_START

static inline oclFloat lambda(oclFloat l) {
	return sqrtf((M_4PIf) / (2.f * l + 1.));
}

void SHEvaluate(const oclFloat3 & v, oclFloat * out) {
	SHEvaluate_H(v, out);
}

/*
void SHProjectIncidentDirectRadiance(
	const oclLightSource* light,
	oclFloat3 *c_d)
{
	// Loop over light sources and sum their SH coefficients
	oclFloat3 *c = new oclFloat3[SHTerms];

	light->SHProject(c);
	for (oclInt j = 0; j < SHTerms; ++j) c_d[j] += c[j];

	SHReduceRinging(c_d);
	delete[] c;
}
*/

void SHReduceRinging(oclFloat3 *c, oclFloat lambda) {
	for (oclInt l = 0; l <= SH_L_MAX; ++l) {
		oclFloat scale = 1.f / (1.f + lambda * l * l * (l + 1) * (l + 1));
		for (oclInt m = -l; m <= l; ++m) c[SHIndex(l, m)] *= scale;
	}
}


/*
void SHComputeDiffuseTransfer(const oclFloat3 &p, const Normal &n,
	oclFloat rayEpsilon, const Scene *scene, RNG &rng, oclInt nSamples,
	oclInt lmax, oclFloat *c_transfer) {
	for (oclInt i = 0; i < SHTerms; ++i)
		c_transfer[i] = 0.f;
	oclInt scramble[2] = { rng.RandomUInt(), rng.RandomUInt() };
	oclFloat *Ylm = ALLOCA(oclFloat, SHTerms);

	// 我觉得下一行的nSamples应该写成2*nSamples，因为for循环里面是舍选采样，有一半的采样点是被过滤掉的。
	for (oclInt i = 0; i < nSamples; ++i) {
		// Sample _i_th direction and compute estimate for transfer coefficients
		oclFloat u[2];
		Sample02(i, scramble, u);
		oclFloat3 w = UniformSampleSphere(u[0], u[1]);
		oclFloat pdf = UniformSpherePdf();
		if (Dot(w, n) > 0.f && !scene->IntersectP(Ray(p, w, rayEpsilon))) {
			// Accumulate contribution of direction $\w{}$ to transfer coefficients
			SHEvaluate(w, lmax, Ylm);
			for (oclInt j = 0; j < SHTerms; ++j)
				c_transfer[j] += (Ylm[j] * AbsDot(w, n)) / (pdf * nSamples);
		}
	}
}


void SHComputeTransferMatrix(const oclFloat3 &p, oclFloat rayEpsilon,
	const Scene *scene, RNG &rng, oclInt nSamples, oclInt lmax,
	oclFloat *T) {
	for (oclInt i = 0; i < SHTerms*SHTerms; ++i)
		T[i] = 0.f;
	oclInt scramble[2] = { rng.RandomUInt(), rng.RandomUInt() };
	oclFloat *Ylm = ALLOCA(oclFloat, SHTerms);
	for (oclInt i = 0; i < nSamples; ++i) {
		// Compute Monte Carlo estimate of $i$th sample for transfer matrix
		oclFloat u[2];
		Sample02(i, scramble, u);
		oclFloat3 w = UniformSampleSphere(u[0], u[1]);
		oclFloat pdf = UniformSpherePdf();
		if (!scene->IntersectP(Ray(p, w, rayEpsilon))) {
			// Update transfer matrix for unoccluded direction
			SHEvaluate(w, lmax, Ylm);
			for (oclInt j = 0; j < SHTerms; ++j)
				for (oclInt k = 0; k < SHTerms; ++k)
					T[j*SHTerms + k] += (Ylm[j] * Ylm[k]) / (pdf * nSamples);
		}
	}
}
*/

void SHMatrixoclFloat3Multiply(const oclFloat *M, const oclFloat *v,
	oclFloat *vout, oclInt lmax) {
	for (oclInt i = 0; i < SHTerms; ++i) {
		vout[i] = 0.f;
		for (oclInt j = 0; j < SHTerms; ++j)
			vout[i] += M[SHTerms * i + j] * v[j];
	}
}


OCL_NAMESPACE_END
