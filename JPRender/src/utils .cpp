#pragma once

#include "utils.h"

namespace DUCK{

void SetOpenGLPerspectiveMatrix(float l,float r,float b,float t,float n,float f){
  float rMl=r-l;
  float rAl=r+l;
  float fMn=f-n;
  float fAn=f+n;
  float tMb=t-b;
  float tAb=t+b;
  float pmatrix[16]={
    2*n/rMl,0.0f,rAl/rMl,0.0f,
    0.0f,2*n/tMb,-tAb/tMb,0.0f,
    0.0f,0.0f,-fAn/fAn,-2*n*f/fMn,
    0.0f,0.0f,-1.0f,0.0f
  };
  glMatrixMode(GL_PROJECTION);
  glLoadTransposeMatrixf(pmatrix);
}

bool SaveData(const char* matName,std::string dataStr){
  std::ofstream os;
  char c_path[100]="";
  const char *s=strstr(matName,".dat");
  if(s!=NULL){
    strcat(c_path,"MatLibraryOptix//");
    strcat(c_path,matName);
  } else{
    strcat(c_path,"materials//");
    strcat(c_path,matName);
    strcat(c_path,".mat");
  }
  os.open(c_path);
  os<<dataStr;
  os.close();
  return true;
}

}
