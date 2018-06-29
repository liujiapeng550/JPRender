#pragma once

#ifndef __ONLY_FUNC__
#include <optix_device.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu\optixu_matrix_namespace.h>
#include "cu_common.h"
#include "intersection.h"
#include "../../host_device/data_instance.h"

using namespace optix;

rtDeclareVariable(SData_Instance,V_C_DATA_INSTANCE,,);
rtDeclareVariable(Intersection,aInset,attribute aInset,);

rtDeclareVariable(rtCallableProgramId<Nor(int,int,const Intersection&)>,CF_GETTEXTURE_NORMAL,,);
rtDeclareVariable(rtCallableProgramId<Color4(int,int,UV2)>,CF_GETTEXTURE_DIFFUSE,,);
rtDeclareVariable(rtCallableProgramId<Nor(int,const Pos&,const Nor&)>,CF_GETTRIPLANENORMAL,,);
rtDeclareVariable(rtCallableProgramId<RandomFloat1(uint&)>,CF_GETSAMPLE1,,);

#endif




RT_FUNC_S void GetNormal_triplanar(const SData_Texture& data,const Pos& pos_g,Nor& nor){
	const Nor n_g=aInset.geoNor;
	nor=normalize(rtTransformNormal(RT_OBJECT_TO_WORLD,n_g));
	if(data.enable){
		Nor nor2=CF_GETTRIPLANENORMAL(data.TSID,pos_g*data.UVrepeat.x,nor);
		nor=lerp(nor2,nor,data.bumpscale+1.f);
	} 
	nor=normalize(nor);
}

/*
RT_FUNC_S void GetNormal_triplanar_flake(const SData_Texture& data,const Pos& pos_g,Nor& nor){
	uint seed=uint(length(pos_g)*9999997u);
	//rtPrintf("seed[%d],len[%f],",seed,length(pos_g));
	Pos newPos=pos_g*CF_GETSAMPLE1(seed);
	GetNormal_triplanar(data,newPos,nor);
}
*/
