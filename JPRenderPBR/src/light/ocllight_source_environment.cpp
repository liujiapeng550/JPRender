#include "light/ocllight_source_environment.h"
#include "RenderMath.h"
#include "core/ocltexture.h"
#include "core\ocldistribution.h"
#include "core\oclimage_loader.h"
#include "core\oclbuffer_manager.h"

using namespace optix;

OCL_NAMESPACE_START

oclLightSourceEnvironment::oclLightSourceEnvironment(oclInt id,RTcontext ctx)
	:oclLightSource(id,LIGHT_TYPE::ENVIRONMENTPROBE,1u)
	,m_texture(ctx){
	m_lightData.lightFuncIndex=CFINDEX_LIGHT_ENVIRONMENT;
	m_buffer_worldMatrix=oclBufferManager::S()->Create(GetUniqueBufferKey("hdr_light_"),RT_BUFFER_INPUT,RT_FORMAT_FLOAT);
	const oclUint F{16u};
	m_buffer_worldMatrix->SetSize(F);
	// 单位阵
	oclFloat r[F];
	for(oclInt i=0; i<F; i++){
		r[i]=0.0f;
	}
	r[0]=1.0f;
	r[5]=1.0f;
	r[10]=1.0f;
	r[15]=1.0f;
	m_buffer_worldMatrix->SetData(r,F);

	m_lightData.bufferID_worldMatrix=m_buffer_worldMatrix->GetID();
	m_lightData.HDR_inv_gamma=1.f;
	m_lightData.HDR_exposure=1.f;
	m_lightData.HDR_Intensity=0.003f;
	m_lightData.HDR_contrast=0.f;
	m_lightData.HDR_saturation=0.0f;
	m_lightData.energyBalance=0.2f;
	m_lightData.radius=M_MAX_FLOAT;

	m_axis.x=m_axis.y=m_axis.z=0.f;
}

oclLightSourceEnvironment::~oclLightSourceEnvironment(){
	oclBufferManager* bufMgr=oclBufferManager::S();
	bufMgr->Destroy(m_CDFOB);
	bufMgr->Destroy(m_MarginalCDFOB);
	bufMgr->Destroy(m_LumiOB);
	bufMgr->Destroy(m_IntegrationOB);
	bufMgr->Destroy(m_SHco);
	bufMgr->Destroy(m_buffer_worldMatrix);

	m_CDFOB=nullptr;
	m_MarginalCDFOB=nullptr;
	m_LumiOB=nullptr;
	m_IntegrationOB=nullptr;
	m_SHco=nullptr;
	m_buffer_worldMatrix=nullptr;
}

oclBool oclLightSourceEnvironment::Init(){
	Json::Value Property;
	Property["Base"]["HDR_exposure"]=CreateParams_("HDR_exposure",m_lightData.HDR_exposure,LIGHT_VALUE_TYPE::FLOAT,0,10);
	Property["Base"]["HDR_Intensity"]=CreateParams_("HDR_Intensity",m_lightData.HDR_Intensity,LIGHT_VALUE_TYPE::FLOAT,0,10);
	Property["Base"]["HDR_contrast"]=CreateParams_("HDR_contrast",m_lightData.HDR_contrast,LIGHT_VALUE_TYPE::FLOAT,-1,1);
	Property["Base"]["HDR_saturation"]=CreateParams_("HDR_saturation",m_lightData.HDR_saturation,LIGHT_VALUE_TYPE::FLOAT,-1,1);
	Property["Base"]["HDR_gamma"]=CreateParams_("HDR_gamma",1.0f,LIGHT_VALUE_TYPE::FLOAT,0,10);
	Property["Base"]["EnergyBalance"]=CreateParams_("EnergyBalance",m_lightData.energyBalance,LIGHT_VALUE_TYPE::FLOAT,0,2);
	Property["Base"]["HDR_texture"]=CreateParams_2_("HDR_texture","");
	Property["Base"]["Rotation_x"]=CreateParams_("Rotation_x",0,LIGHT_VALUE_TYPE::INT,0,360);
	Property["Base"]["Rotation_y"]=CreateParams_("Rotation_y",0,LIGHT_VALUE_TYPE::INT,0,360);
	Property["Base"]["Rotation_z"]=CreateParams_("Rotation_z",0,LIGHT_VALUE_TYPE::INT,0,360);
	m_props["Property"]=Property;
	return true;
}

void oclLightSourceEnvironment::SHProject(Spectrum * coeffs,const oclFloat4* imgData) const{
	for(oclInt i=0; i<SHTerms; ++i){
		coeffs[i]=make_float3(0.f);
	}

#if 0
	coeffs[0]=make_float3(.79f,.44f,.54f);
	coeffs[1]=make_float3(.39f,.35f,.60f);
	coeffs[2]=make_float3(-.34f,-.18f,-.27f);
	coeffs[3]=make_float3(-.29f,-.06f,.01f);
	coeffs[4]=make_float3(-.11f,-.05f,-.12f);
	coeffs[5]=make_float3(-.26f,-.22f,-.47f);
	coeffs[6]=make_float3(-.16f,-.09f,-.15f);
	coeffs[7]=make_float3(.56f,.21f,.14f);
	coeffs[8]=make_float3(.21f,-.05f,-.30f);

	return;
#endif



	const oclInt ntheta=m_lightData.HDR_size.y;
	const oclInt nphi=m_lightData.HDR_size.x;
	if(min(ntheta,nphi)>50u){
		/// Project _InfiniteAreaLight_ to SH from lat-long representation

		// Precompute $\theta$ and $\phi$ values for lat-long map projection
		oclFloat *buf=new oclFloat[2*ntheta+2*nphi];
		oclFloat *bufp=buf;
		oclFloat *sintheta=bufp;  bufp+=ntheta;
		oclFloat *costheta=bufp;  bufp+=ntheta;
		oclFloat *sinphi=bufp;    bufp+=nphi;
		oclFloat *cosphi=bufp;
		for(oclInt theta=0; theta<ntheta; ++theta){
			sintheta[theta]=sinf((theta+.5f)/ntheta * M_PIf);
			costheta[theta]=cosf((theta+.5f)/ntheta * M_PIf);
		}
		for(oclInt phi=0; phi<nphi; ++phi){
			sinphi[phi]=sinf((phi+.5f)/nphi *  M_2PIf);
			cosphi[phi]=cosf((phi+.5f)/nphi *  M_2PIf);
		}

		// SH calculation;
		// TODO:拷贝过程可以优化
		const float F{M_2PIf/ntheta*M_2PIf/nphi};
		for(oclInt theta=0; theta<ntheta; ++theta){
			for(oclInt phi=0; phi<nphi; ++phi){
				// Add _InfiniteAreaLight_ texel's contribution to SH coefficients
				// 左手坐标系，z轴向上；
				oclFloat3 w{sintheta[theta]*cosphi[phi], sintheta[theta]*sinphi[phi], costheta[theta]};
				/******************************************************************************/
				// 上句 w在构建的时候就是单位化的，所以注释掉下面的；
				//  w = Normalize(LightToWorld(w));
				/******************************************************************************/
				const Spectrum Le=make_float3(imgData[theta*nphi+phi]);

				coeffs[SHIndex(0,0)]+=Le*Y00*sintheta[theta]*F;
				coeffs[SHIndex(1,-1)]+=Le*Y1_1(w.y)*sintheta[theta]*F;
				coeffs[SHIndex(1,0)]+=Le*Y10(w.z)*sintheta[theta]*F;
				coeffs[SHIndex(1,1)]+=Le*Y11(w.x)*sintheta[theta]*F;
				coeffs[SHIndex(2,-2)]+=Le*Y2_2(w.x,w.y)*sintheta[theta]*F;
				coeffs[SHIndex(2,-1)]+=Le*Y2_1(w.y,w.z)*sintheta[theta]*F;
				coeffs[SHIndex(2,0)]+=Le*Y20(w.z)*sintheta[theta]*F;
				coeffs[SHIndex(2,1)]+=Le*Y21(w.x,w.z)*sintheta[theta]*F;
				coeffs[SHIndex(2,2)]+=Le*Y22(w.x,w.y)*sintheta[theta]*F;
			}
		}
		/*
		oclFloat N=1.f/(ntheta*nphi);
		coeffs[SHIndex(0,0)]*=N;
		coeffs[SHIndex(1,-1)]*=N;
		coeffs[SHIndex(1,0)]*=N;
		coeffs[SHIndex(1,1)]*=N;
		coeffs[SHIndex(2,-2)]*=N;
		coeffs[SHIndex(2,-1)]*=N;
		coeffs[SHIndex(2,0)]*=N;
		coeffs[SHIndex(2,1)]*=N;
		coeffs[SHIndex(2,2)]*=N;
		*/
		coeffs[SHIndex(0,0)]*=A0;
		coeffs[SHIndex(1,-1)]*=A1;
		coeffs[SHIndex(1,0)]*=A1;
		coeffs[SHIndex(1,1)]*=A1;
		coeffs[SHIndex(2,-2)]*=A2;
		coeffs[SHIndex(2,-1)]*=A2;
		coeffs[SHIndex(2,0)]*=A2;
		coeffs[SHIndex(2,1)]*=A2;
		coeffs[SHIndex(2,2)]*=A2;

		// Free memory used for lat-long theta and phi values
		delete[] buf;
	}
}

void oclLightSourceEnvironment::FetchTextures(std::vector<const oclTexture*>* out) const{
	out->push_back(&m_texture);
}

void oclLightSourceEnvironment::SetProperty(const oclChar * propName,VARIANT * v){
#if __OCL_INTERNAL_DEBUG__
	oclStr testString=m_props.toStyledString();
#endif
	SetPropertyToData_(propName,v);
	Json::Value::Members mem=m_props["Property"].getMemberNames();
	for(auto iter=mem.begin(); iter!=mem.end(); iter++){
		auto nameMem=m_props["Property"][*iter].getMemberNames();
		for(auto niter=nameMem.begin(); niter!=nameMem.end(); niter++){
			if((*niter).compare(propName)==0){
				Json::Value& tmp=m_props["Property"][*iter][*niter];
				switch(tmp["valueType"].asInt()){
				case LIGHT_VALUE_TYPE::BOOL:
				tmp["realValue"]=v->boolVal;
				break;
				case LIGHT_VALUE_TYPE::COLOR:
				for(oclInt i=0; i<3; i++){
					tmp["value"][i]["realValue"]=v[i].fltVal;
				}
				break;
				case LIGHT_VALUE_TYPE::FLOAT:
				tmp["realValue"]=v->fltVal;
				break;
				case LIGHT_VALUE_TYPE::INT:
				tmp["realValue"]=v->intVal;
				break;
				case LIGHT_VALUE_TYPE::ENUM:
				case LIGHT_VALUE_TYPE::STRING:
				tmp["realValue"]=m_texture.GetFileNewName();
				default:
				break;
				}
			}
		}
	}
}

oclBool oclLightSourceEnvironment::NeedUpload() const{
	oclInt w{-1},h{-1};
	if(m_LumiOB){
		m_LumiOB->GetSize(&w,&h);
	}
	return w>0&&h>0&&m_texture.GetURL()!=""&& m_bActived;
}

void oclLightSourceEnvironment::SetPropertyToData_(const oclChar* propName,VARIANT* v){
	oclBool flag=false;
	if(strcmp("Rotation_x",propName)==0){
		// v中的值是角度；
		// 简单的表示沿着世界x，y、z轴各自旋转了多少角度；
		m_axis.x=v->intVal;
		flag=true;
	} else if(strcmp("Rotation_y",propName)==0){
		m_axis.y=v->intVal;
		flag=true;
	} else if(strcmp("Rotation_z",propName)==0){
		m_axis.z=v->intVal;
		flag=true;


		//TODO:jc:实现渲染矩阵的计算；
	} else if(strcmp("HDR_exposure",propName)==0){
		m_lightData.HDR_exposure=v->fltVal;
	} else if(strcmp("HDR_Intensity",propName)==0){
		m_lightData.HDR_Intensity=v->fltVal;
	} else if(strcmp("HDR_contrast",propName)==0){
		m_lightData.HDR_contrast=v->fltVal;
	} else if(strcmp("HDR_saturation",propName)==0){
		m_lightData.HDR_saturation=v->fltVal;
	} else if(strcmp("HDR_gamma",propName)==0){
		m_lightData.HDR_inv_gamma=1.f/(v->fltVal);
	} else if(strcmp("EnergyBalance",propName)==0){
		m_lightData.energyBalance=v->fltVal;




	} else if(strcmp("HDR_texture",propName)==0){
		oclChar* url=_com_util::ConvertBSTRToString((*v).bstrVal);
		if(strcmp(url,"")!=0&&strcmp(url,m_texture.GetURL().c_str())!=0){
			/// 1. 尝试获取资源;
			STextureDescription param;
			param.fileURL=oclStr(url);
			param.format=RT_FORMAT_FLOAT4;
			param.type=RT_BUFFER_INPUT;
			param.wrapModeX=RT_WRAP_REPEAT;
			param.wrapModeY=RT_WRAP_REPEAT;
			param.wrapModeZ=RT_WRAP_REPEAT;
			param.filterModeX=RT_FILTER_LINEAR;
			param.filterModeY=RT_FILTER_LINEAR;
			param.filterModeZ=RT_FILTER_LINEAR;
			param.mipmapCount=1;
			if(m_texture.Reset(param)){


				/// 1.1 分析图像资源
				oclBuffer* buf=oclBufferManager::S()->Get(param.fileURL);
				F_DBG_ASSERT(buf);
				oclInt w,h;
				buf->GetSize(&w,&h);
				m_lightData.HDR_size.x=w;
				m_lightData.HDR_size.y=h;
				if(!AssembleoclBuffer_(param.fileURL)){
					oclFloat4* imgData=buf->Map<oclFloat4>();
					HandleDistribution_(w,h,imgData);

					float3* shData=m_SHco->Map<float3>();
					SHProject(shData,imgData);
					SHReduceRinging_(shData);
					m_SHco->Unmap();
					buf->Unmap();
				}
				oclBufferManager::S()->Destroy(buf);


				/// 1.2 保存信息；
				m_lightData.HDR_textureSamplerID=m_texture.GetTextureSamplerID();
				m_lightData.HDR_LuminanceBufferID=m_LumiOB->GetID();
				m_lightData.HDR_IntegrationBufferID=m_IntegrationOB->GetID();
				m_lightData.HDR_CDFBufferID=m_CDFOB->GetID();
				m_lightData.HDR_MarginalCDFBufferID=m_MarginalCDFOB->GetID();
				m_lightData.SH_cBufferID=m_SHco->GetID();

			} else{
				F_DBG_ERROR("Resource %s does NOT load successfully\n",param.fileURL);
			}
		}
		delete[] url;
	}
	if(flag){
		Matrix4x4D out;
		out.FromEulerAnglesXYZ(m_axis.x,m_axis.y,m_axis.z);
		oclFloat data[16];
		out.fromMatrixToArray(data);
		m_buffer_worldMatrix->SetData(data,16);
	}
}

void oclLightSourceEnvironment::SHReduceRinging_(oclFloat3 *c,oclFloat lambda){
	for(oclInt l=0; l<=SH_L_MAX; ++l){
		oclFloat scale=1.f/(1.f+lambda * l * l * (l+1) * (l+1));
		for(oclInt m=-l; m<=l; ++m) c[SHIndex(l,m)]*=scale;
	}
}

oclBool oclLightSourceEnvironment::HandleDistribution_(oclInt w,oclInt h,const oclFloat4* imgData){
#define GETLUMI_CIE(c) (.2126f*c.x+.7152f*c.y+.0722f*c.z)

	oclFloat *img=new oclFloat[w*h];
	const oclFloat h_inv=1.f/oclFloat(h);
	for(oclInt v=0; v<h; ++v){
		oclFloat sinTheta=sinf(M_PIf * oclFloat(v+.5f) *h_inv);
		for(oclInt u=0; u<w; ++u){
			oclUint index=u+v*w;
			img[index]=GETLUMI_CIE(imgData[index])*sinTheta;
		}
	}

	oclDistribution2D dis2D(img,w,h);

	//TODO:下面的copy逻辑可以优化；
	// store luminance to buffer;
	m_LumiOB->SetSize(w,h);
	m_LumiOB->SetData(img,w*h);
	delete[] img;

	// store integration to buffer;
	m_IntegrationOB->SetSize(h+1);
	oclFloat* integration=new oclFloat[h+1];
	oclInt v=0;
	for(; v<h; ++v){
		integration[v]=dis2D.GetFuncIntAtRow(v);
	}
	integration[v]=dis2D.GetMarginalIntegrator();
	m_IntegrationOB->SetData(integration,h+1);
	delete[] integration;

	// store CDF to buffer;
	m_CDFOB->SetSize(w+1,h);
	oclFloat* CDF=new oclFloat[(w+1)*h];
	for(oclInt v=0; v<h; ++v){
		dis2D.CopyCDFAtRow(v,CDF+v*(w+1));
	}
	m_CDFOB->SetData(CDF,(w+1)*h);
	delete[] CDF;

	// stroe margin CDF to buffer;
	m_MarginalCDFOB->SetSize(h+1);
	oclFloat* marginalCDF=new oclFloat[h+1];
	dis2D.CopyCDFAtMargin(marginalCDF);
	m_MarginalCDFOB->SetData(marginalCDF,h+1);
	delete[] marginalCDF;

#undef GETLUMI_CIE
	return true;
}

oclBool oclLightSourceEnvironment::AssembleoclBuffer_(oclStr hdrFileURL){
	oclBufferManager* bufMgr=oclBufferManager::S();

	/// 用一个缓冲区Key检测buffer存在性；
	if(bufMgr->Exist(hdrFileURL+"_lumi")){
		/// 已经存在，共享缓冲区；
		m_LumiOB=bufMgr->Get(hdrFileURL+"_lumi");
		m_MarginalCDFOB=bufMgr->Get(hdrFileURL+"_marginal");
		m_CDFOB=bufMgr->Get(hdrFileURL+"_CDF");
		m_IntegrationOB=bufMgr->Get(hdrFileURL+"_Inte");
		m_SHco=bufMgr->Get(hdrFileURL+"SHc");
		return true;
	} else{
		bufMgr->Destroy(m_LumiOB); m_LumiOB=bufMgr->Create(hdrFileURL+"_lumi",RT_BUFFER_INPUT,RT_FORMAT_FLOAT);
		bufMgr->Destroy(m_MarginalCDFOB); m_MarginalCDFOB=bufMgr->Create(hdrFileURL+"_marginal",RT_BUFFER_INPUT,RT_FORMAT_FLOAT);
		bufMgr->Destroy(m_CDFOB); m_CDFOB=bufMgr->Create(hdrFileURL+"_CDF",RT_BUFFER_INPUT,RT_FORMAT_FLOAT);
		bufMgr->Destroy(m_IntegrationOB); m_IntegrationOB=bufMgr->Create(hdrFileURL+"_Inte",RT_BUFFER_INPUT,RT_FORMAT_FLOAT);
		bufMgr->Destroy(m_SHco); m_SHco=bufMgr->Create(hdrFileURL+"SHc",RT_BUFFER_INPUT,RT_FORMAT_FLOAT3);
		m_SHco->SetSize(SHTerms);
		return false;
	}
}

OCL_NAMESPACE_END