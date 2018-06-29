#pragma once

#include <vector>
#include "oclcore.h"

OCL_NAMESPACE_START

struct oclDistribution1D {
  oclDistribution1D(const oclFloat *f, oclInt n);

  ~oclDistribution1D() {
    delete[] value;
    delete[] cdf;
  }

  void CopyCDF(oclFloat* outData)const;
  oclFloat GetIntegration()const { return valueInt; }
  oclInt GetCount()const { return count; }

private:
  oclFloat *value, *cdf;
  oclFloat valueInt;
  oclInt count;
};



struct oclDistribution2D {
  oclDistribution2D(const oclFloat *data, oclInt nu, oclInt nv);
  ~oclDistribution2D();

  oclFloat GetFuncIntAtRow(oclUint row)const;
  void CopyCDFAtRow(oclUint row, oclFloat* pos)const;
  oclFloat GetMarginalIntegrator()const { return pMarginal->GetIntegration(); }
  void CopyCDFAtMargin(oclFloat* outData)const;

private:
  std::vector<oclDistribution1D*> pConditionalV;
  oclDistribution1D *pMarginal;
};

OCL_NAMESPACE_END
