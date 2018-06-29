#include "core\ocldistribution.h"

using namespace std;

OCL_NAMESPACE_START

oclDistribution1D::oclDistribution1D(const oclFloat *f,oclInt n){
  // 这里采用幂函数将CDF的分布进一步拉大，已确保重要性采样；
#define POWER 2.f
  count=n;
  value=new oclFloat[n];
  memcpy(value,f,n*sizeof(oclFloat));
  cdf=new oclFloat[n+1];
  cdf[0]=0.f;
  const oclFloat INV_N=1.f/oclFloat(n);
  for(oclInt i=1; i<n+1; ++i) cdf[i]=cdf[i-1]+powf(value[i-1],POWER)*INV_N;

  valueInt=cdf[n];
  if(valueInt==0.f){
    for(oclInt i=1; i<n+1; ++i) cdf[i]=oclFloat(i)*INV_N;
  } else{
    const oclFloat inv_valueInt=1.f/oclFloat(valueInt);
    for(oclInt i=1; i<n+1; ++i) cdf[i]*=inv_valueInt;
  }

  valueInt=0.f;
  for(oclInt i=0; i<n; ++i) valueInt+=value[i];
  valueInt*=INV_N;
#undef POWER
}

void oclDistribution1D::CopyCDF(oclFloat * outData) const{
  memcpy(outData,cdf,sizeof(oclFloat)*(count+1));
}

oclDistribution2D::oclDistribution2D(const oclFloat *value,oclInt nu,oclInt nv){
  pConditionalV.reserve(nv);
  for(oclInt v=0; v<nv; ++v){
    // Compute conditional sampling distribution for $\tilde{v}$
    pConditionalV.push_back(new oclDistribution1D(&value[v*nu],nu));
  }
  // Compute marginal sampling distribution $p[\tilde{v}]$
  vector<oclFloat> marginalFunc;
  marginalFunc.reserve(nv);
  for(oclInt v=0; v<nv; ++v)
    marginalFunc.push_back(pConditionalV[v]->GetIntegration());
  pMarginal=new oclDistribution1D(&marginalFunc[0],nv);
}

oclDistribution2D::~oclDistribution2D(){
  delete pMarginal;
  for(uint32_t i=0; i<pConditionalV.size(); ++i)
    delete pConditionalV[i];
}

oclFloat oclDistribution2D::GetFuncIntAtRow(oclUint row)const{
  return pConditionalV[row]->GetIntegration();
}

void oclDistribution2D::CopyCDFAtRow(oclUint row,oclFloat* pos)const{
  const oclDistribution1D* dis1D=pConditionalV[row];
  dis1D->CopyCDF(pos);
}

void oclDistribution2D::CopyCDFAtMargin(oclFloat* outData)const{
  pMarginal->CopyCDF(outData);
}

OCL_NAMESPACE_END
