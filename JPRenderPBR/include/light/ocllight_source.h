#pragma once  

#include <comutil.h>
#include <optixu\optixpp_namespace.h>
#include <optixu\optixu_matrix_namespace.h>
#include "json.h"
#include "core\oclcore.h"
#include "../../host_device/data_light.h"

OCL_NAMESPACE_START

/*
#define M_DIFFUSE diffuse
#define M_INTENSITY intensity
#define M_EXPOSURE HDR_exposure
#define M_CONTRAST HDR_contrast
#define M_SATURATION HDR_saturation
*/

class oclLightSource{

public:
	enum LIGHT_VALUE_TYPE{
		INT,
		FLOAT,
		BOOL,
		ENUM,
		COLOR,
		STRING
	};

	oclLightSource(oclInt id,LIGHT_TYPE type,oclUint ns);
	virtual ~oclLightSource();

	virtual oclBool Init()=0;

	virtual oclBool SetCenter(oclFloat* center)=0;

	void SetEulerAngle(oclFloat x,oclFloat y,oclFloat z);

	void GetWorldMatrix(oclFloat * tm)const;

	void SetWorldMatrix(oclFloat* tm);

	void SetVisibility(oclBool bShow){ m_bActived=bShow; }

	virtual void SetProperty(const oclChar* propName,VARIANT* v);

	oclBool GetProperty(const oclChar* propName,oclFloat* v)const;

	void InitWithJson(Json::Value props){ m_props=props; };

	const SData_Light* GetLightData()const{ return &m_lightData; }

	LIGHT_TYPE GetLightType()const{ return m_type; }

	Json::Value GetJsonValue()const{ return m_props; }

	oclStr GetJsonString()const{ return m_props.toStyledString(); }

	oclBool GetPreDelete()const{ return !m_bActived; }

	virtual oclBool UpdateWorldMatrix(oclFloat* matrix)=0;

	/**
	 * 在每次提交数据到现存时候，光源管理器需要向每个具体光源通过该接口查询是否需要提交
	 * 他们的信息到显存；
	 * 常规的实现是（并存）：
	 *	1 如果光源有能量；
	 *	2 如果激活了光源；
	 * 就返回true；具体光源具体对待；
	*/
	virtual oclBool NeedUpload()const=0;

	/**
	 * 该函数用来将光源在SH上进行投影，函数具有默认实现，但依然需要具体光源实现其具体逻辑
	 * 目前只对环境光进行SH投影：
	*/
	virtual void SHProject(Spectrum *coeffs,const oclFloat4* imgData) const=0;

protected:
	float luminanceCIE_(const optix::float4& rgba)const{
		const optix::float4 ntsc_luminance={0.30f, 0.59f, 0.11f,0.f};
		return  optix::dot(rgba,ntsc_luminance);
	}

	virtual void SetPropertyToData_(const oclChar* propName,VARIANT* v)=0;

	void CreateTrans3P_(const oclChar* showName,const oclChar* realName,oclFloat val_x,oclFloat val_y,oclFloat val_z);

	void CreateTrans3F_(const oclChar* showName,const oclChar* realName,oclFloat val_x,oclFloat val_y,oclFloat val_z);

	void CreateTransBt_(const oclChar* showName,const oclChar* realName); 

	Json::Value CreateParams_(const oclChar* realName,oclFloat realValue,LIGHT_VALUE_TYPE valueType,oclFloat min,oclFloat max);

	Json::Value CreateParams_2_(const oclChar* realName,const oclChar* realValue,LIGHT_VALUE_TYPE valueType=LIGHT_VALUE_TYPE::STRING);

	Json::Value CreateTransVal_(oclFloat max,oclFloat min,oclFloat step,oclFloat val);

private:
	oclStr FindName_(const oclChar* name,oclFloat val);

	oclStr FindName_(const oclChar* name,const oclChar* val);

protected:
	const LIGHT_TYPE m_type;
	const oclUint m_nSamples;									//球谐投影用来近似积分的MonteCarlo采样数量，只用在C端，必须在光源初始化期间赋值；
	Json::Value m_props;
	SData_Light m_lightData;
	oclBool m_bActived=true;

private:
	oclFloat* m_currentMatrix=nullptr;

};

OCL_NAMESPACE_END

