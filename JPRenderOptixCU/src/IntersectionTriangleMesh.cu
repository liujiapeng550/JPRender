#include <optix_device.h>
#include <optixu\optixu_math_namespace.h>
#include <optixu/optixu_aabb_namespace.h>
#include "cu_common.h"
#include "intersection.h"
#include "../../host_device/data_instance.h"
//#include "components\intersection_refinement.h"

using namespace optix;

rtDeclareVariable(uint2,dLd,rtLaunchDim,);
rtDeclareVariable(uint2,dLi,rtLaunchIndex,);
rtDeclareVariable(Ray,dRay,rtCurrentRay,);
rtDeclareVariable(Intersection,aInset,attribute aInset,);
rtDeclareVariable(SData_Instance,V_C_DATA_INSTANCE,,);

RT_FUNC_F void GetMeshData(int bufferID,const int3& idx,float3& p0,float3& p1,float3& p2){
	p0=GET_F3_1D(bufferID,idx.x);
	p1=GET_F3_1D(bufferID,idx.y);
	p2=GET_F3_1D(bufferID,idx.z);
}

RT_FUNC_F void GetMeshData2(int bufferID,const int3& idx,float2& p0,float2& p1,float2& p2){
	p0=GET_F2_1D(bufferID,idx.x);
	p1=GET_F2_1D(bufferID,idx.y);
	p2=GET_F2_1D(bufferID,idx.z);
}

RT_PROGRAM void PgmIntersect_mesh_triangle(int primIdx){
#if 0
	//testing
	if(rtPotentialIntersection(.5f*primIdx)){
		aNodeIndex=primIdx;
		aInset.back_hit_point=make_float3(1.f);
		aInset.front_hit_point=make_float3(1.f);
		aInset.geoNor=make_float3(1.f);
		aInset.meshNor=make_float3(1.f);
		aInset.texCoord=make_float2(1.f);
		rtReportIntersection(0);
	}
	return;
#endif

	const int3 v_idx=GET_I3_1D(V_C_DATA_INSTANCE.indexBufferID,primIdx);

	Pos p0,p1,p2;
	GetMeshData(V_C_DATA_INSTANCE.vertexBufferID,v_idx,p0,p1,p2);

	// Intersect ray with triangle
	Nor n;
	float t,beta,gamma;
	if(intersect_triangle(dRay,p0,p1,p2,n,t,beta,gamma)){
		if(rtPotentialIntersection(t)){
			aInset.meshNor=normalize(n);
			float3 n0,n1,n2;
			GetMeshData(V_C_DATA_INSTANCE.norBufferID,v_idx,n0,n1,n2);
			aInset.geoNor=normalize(n1*beta+n2*gamma+n0*(1.0f-beta-gamma));

			if(V_C_DATA_INSTANCE.tanBufferSize<=0){
				aInset.tbn.Tangent=normalize(cross(aInset.geoNor,-dRay.direction));
			} else{
				float3 st0,st1,st2;
				GetMeshData(V_C_DATA_INSTANCE.tanBufferID,v_idx,st0,st1,st2);
				float3 shading_tangent=normalize(st1*beta+st2*gamma+st0*(1.0f-beta-gamma));
				aInset.tbn.Tangent=normalize(shading_tangent);
			}
			aInset.tbn.Normal=aInset.geoNor;
			aInset.tbn.Bitangent=normalize(cross(aInset.tbn.Tangent,aInset.tbn.Normal));

			float2 t0,t1,t2;
			GetMeshData2(V_C_DATA_INSTANCE.uvBufferID,v_idx,t0,t1,t2);
			aInset.texCoord=t1*beta+t2*gamma+t0*(1.0f-beta-gamma);

			aInset.nodeIndex=V_C_DATA_INSTANCE.nodeIndex;

			/*
			refine_and_offset_hitpoint(
				dRay.origin+t*dRay.direction,
				dRay.direction,
				aInset.meshNor,
				p0,
				aInset.back_hit_point,
				aInset.front_hit_point);

			aInset.back_hit_point=dRay.origin+t*dRay.direction;
			aInset.front_hit_point=dRay.origin+t*dRay.direction;
				*/
			rtReportIntersection(0);
		}
	}
}

RT_PROGRAM void PgmBounds_mesh_triangle(int primIdx,float result[6]){
	Aabb* aabb=(Aabb*)result;

	rtPrintf("MeshData.indexBufferID is :%d\n",V_C_DATA_INSTANCE.vertexBufferID);
	//rtPrintf("triangle count is :%d\n",V_C_DATA_INSTANCE.triangleCount);

	const int3 v_idx=GET_I3_1D(V_C_DATA_INSTANCE.indexBufferID,primIdx);
	rtPrintf("vertex index is :%d,%d,%d\n",v_idx.x,v_idx.y,v_idx.z);

	Pos p0,p1,p2;
	GetMeshData(V_C_DATA_INSTANCE.vertexBufferID,v_idx,p0,p1,p2);
	rtPrintf("triangle first vertex pos is :%f,%f,%f\n",p0.x,p0.y,p0.z);

	const float area=length(cross(p1-p0,p2-p0));
	if(area>0.0f&&!isinf(area)){
		aabb->m_min=fminf(fminf(p0,p1),p2);
		aabb->m_max=fmaxf(fmaxf(p0,p1),p2);
	} else{
		aabb->invalidate();
	}
}

