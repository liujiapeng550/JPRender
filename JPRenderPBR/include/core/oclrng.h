#pragma once

#include "oclcore.h"

OCL_NAMESPACE_START

// Random Number Declarations
class oclRNG {
public:
	oclRNG(oclUint seed = 5489UL) {
		mti = N + 1; /* mti==N+1 means mt[N] is not initialized */
		Seed(seed);
	}
	oclRNG(const oclRNG& RNG) = delete;
	void Seed(oclUint seed) const;

/* generates a random number on [0,1)-real-interval */
	inline oclFloat RandomFloat() const;
	inline oclUint RandomUint() const;

private:
	static const oclInt N = 624;
	mutable oclUlong mt[N]; /* the array for the state vector  */
	mutable oclInt mti;

};

OCL_NAMESPACE_END
