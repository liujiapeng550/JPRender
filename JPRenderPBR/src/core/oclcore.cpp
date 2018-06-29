#include "core\oclcore.h"

#if __OCL_INTERNAL_DEBUG__
RTcontext g_ctx;

#include <iostream>
void SetContext(RTcontext ctx) {
  g_ctx = ctx;
}

void reportErrorMessage(const char* message){
  std::cout<<"OptiX Error: '"<<message<<std::endl;
}

void handleError(RTresult code,const char* file,int line){
  const char* message;
  char s[2048];
  rtContextGetErrorString(g_ctx,code,&message);
  std::sprintf(s,"%s\n(%s:%d)",message,file,line);
  reportErrorMessage(s);
}
#endif

OCL_NAMESPACE_START

void ConvertFloat4ToUchar4(const oclFloat4 * source,void** target,oclInt w,oclInt h){
	oclUchar4* t=static_cast<oclUchar4*>(*target);
	const oclFloat F{255.99f};
	oclInt idx=0;
	for(oclInt i=0;i<h;++i){
		for(oclInt j=0;j<w;++j){
			idx=i*w+j;
			t[idx].z=static_cast<unsigned char>(optix::clamp(source[idx].x,0.f,1.f)*F);
			t[idx].y=static_cast<unsigned char>(optix::clamp(source[idx].y,0.f,1.f)*F);
			t[idx].x=static_cast<unsigned char>(optix::clamp(source[idx].z,0.f,1.f)*F);
			t[idx].w=255u;
		}
	}

}

OCL_NAMESPACE_END
