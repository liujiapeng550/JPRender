#include "core/JsonConverter.h"
#include "material\oclmaterial.h"
#include "material\oclmaterial_library.h"
#include "material\oclmaterial_manager.h"

#define M_LIBNAME "name"
#define M_MAT_DEF "matdefinitionName"
#define M_TYPE "type"
#define M_GROUP "group"
#define M_THUMBNAIL "thumbnail"
#define M_MAT_DATA "matdata"
#define M_LIST "list"
#define M_INDEX "index"
#define M_MAT_INDEX "matindex"
#define M_ITEM_INDEX "itemindex"
#define M_NORPPT "NorProperty"
#define M_TEXPPT "TexProperty"
#define M_PPT "Prop"
#define M_VALUE "value"

//copy from iray "ParameterType.h"
enum ParamType
{
	TYPE_UNKNOWN = 0,                      //!< Type is not known/recognised.
	TYPE_BOOLEAN,                          //!< bool
	TYPE_CHAR,                             //!< char
	TYPE_UNSIGNED_CHAR,                    //!< unsigned char
	TYPE_SHORT,                            //!< short
	TYPE_UNSIGNED_SHORT,                   //!< unsigned short
	TYPE_INT,                              //!< int
	TYPE_UNSIGNED_INT,                     //!< unsigned int
	TYPE_LONG,                             //!< long
	TYPE_UNSIGNED_LONG,                    //!< unsigned long
	TYPE_FLOAT,                            //!< float
	TYPE_DOUBLE,                           //!< double
	TYPE_VECTOR2_FLOAT,                    //!< lw::maths::Vector2 or lw::maths::Vector2_f.
	TYPE_VECTOR2_DOUBLE,                   //!< lw::maths::Vector2_d
	TYPE_VECTOR3_FLOAT,                    //!< lw::maths::Vector3 or lw::maths::Vector3_f.
	TYPE_VECTOR3_DOUBLE,                   //!< lw::maths::Vector3_d
	TYPE_COLOUR_RGB,                       //!< lw::ColourRGB.
	TYPE_COLOUR_RGBA,                      //!< lw::ColourRGBA.
	TYPE_POINTER,                          //!< Generic pointer type.
	TYPE_ENUMERATION,                      //!< general enum
	TYPE_SCATTER_MODE,                     //!< scatter mode enum
	TYPE_WRAP_MODE,                        //!< texture wrap modes
	TYPE_MONO_MODE,                        //!< texture alpha samping mode
	TYPE_COORDINATE_SYSTEM,                //!< coordinate system enum
	TYPE_AXIS,                             //!< Axis enum
	TYPE_STRING,                           //!< char* string
	TYPE_COLOUR_INTERFACE,                 //!< Pointers to Colour Interfaces
	TYPE_BUMP_MAP_INTERFACE,               //!< Pointers to BumpMap Interfaces
	TYPE_FLOAT_INTERFACE,                  //!< Pointers to Float Interfaces
	TYPE_TEXTURE_SPACE_INTERFACE,          //!< Pointers to Texture Space Interfaces
	TYPE_LIGHT_INTERFACE,                  //!< Pointers to Light Interfaces
	TYPE_BACKPLATE_INTERFACE,              //!< Pointers to Backplate Interfaces
	TYPE_ENVIRONMENT_INTERFACE,            //!< Pointers to Environment Interfaces
	TYPE_ENVIRONMENT_FUNCTION_INTERFACE,   //!< Pointers to Environment Function Interfaces
	TYPE_TONE_MAP_INTERFACE,               //!< Pointers to Tonemap Interfaces
	TYPE_INTENSITY_MODE,                   //!< intensity mode enum
	TYPE_IMAGE,                            //!< Pointers to Image class
	TYPE_EMISSIVE_INTERFACE,               //!< Pointers to Emissive Interfaces
	TYPE_COMBINE_MODE,                     //!< texture space combine mode
	TYPE_MATRIX4_FLOAT,                    //!< lw::maths::Matrix4 or lw::maths::Matrix4_f
	TYPE_MATRIX4_DOUBLE,                   //!< lw::maths::Matrix4_d
	TYPE_TEXTURE_COMPRESSION,              //!< texture compression mode
	TYPE_FILTER_MODE,					   //!< texture filter modes
	TYPE_BRUSH_MAPPING,					   //!< texture filter modes
	//realtime
	TYPE_CASTSHADOWS,					   //!< light prop castshadows
	TYPE_SHADOWMINRESPERENT,			   //!< light prop shadowminresperent
	TYPE_LIGHTTEXTURE,					   //!< light prop light texture
	TYPE_BUTTON,						   //!< prop with button
	TYPE_ETEX_TYPE_ENUM,
	TYPE_FILTER_TYPE_ENUM,
	TYPE_ETexGenType_ENUM,
	TYPE_ETexModRotateType_ENUM,
	TYPE_ETexModMoveType_ENUM,
	TYPE_EDeformType_ENUM,
	TYPE_EWaveForm_ENUM,
	TYPE_LIST,							   //!< prop with list
	//vegetation
	TYPE_VEG_OPTIONS_HIDEABLE,
	TYPE_VEG_OPTIONS_PLAYER_HIDEALBE,
	TYPE_VEG_OPTIONS_CAST_SHADOW_MIN_SPEC,
	TYPE_VEG_OPTIONS_MIN_SPEC,
	//common
	TYPE_INPUT_WITH_PATH_BROWSER,
	//special
	TYPE_SUROUNDING_OPEN_CLEAN,
	TYPE_USE_CAMERA_WITH_DROPBOX,
	TYPE_BOOLEAN_AUTO_CHANGE,
	//NoSlider
	TYPE_FLOAT_NoSlider,				   //!< float without slider

	TYPE_INPUT_BUDDY_CHECKBOX,
	TYPE_VIEWPORT,
	TYPE_GEN_IMAGE,
	TYPE_IMAGE_BK,
	TYPE_VIDEO,
	TYPE_MATT
};

OCL_NAMESPACE_START

namespace jsonconvertor{

static void InitSDataMaterialProperty(const Json::Value& value,const oclStr& ppName,SData_Material* p){
#define GET_JSON_VALUE_INDEXED(propName,index)(value[M_PPT][propName][M_VALUE][index][M_VALUE])
#define GET_JSON_VALUE(propName)(value[M_PPT][propName][M_VALUE])
#define SET_JSON_VALUE_INDEXED(propName,index,v)(value[M_PPT][propName][M_VALUE][index][M_VALUE]=v)
#define SET_JSON_VALUE(propName,v)(value[M_PPT][propName][M_VALUE]=v)
	const oclChar* propName=ppName.c_str();
	// clearcoat
	if(strcmp(propName,F_STR(M_MPROP_CC_BUMPSCALE))==0){
		p->CC_texture.bumpscale=GET_JSON_VALUE(ppName).asFloat();
	} else if(strcmp(propName,F_STR(M_MPROP_CC_COLOR))==0){
		p->CC_texture.color.x=GET_JSON_VALUE_INDEXED(ppName,0).asFloat();
		p->CC_texture.color.y=GET_JSON_VALUE_INDEXED(ppName,1).asFloat();
		p->CC_texture.color.z=GET_JSON_VALUE_INDEXED(ppName,2).asFloat();
	} else if(strcmp(propName,F_STR(M_MPROP_CC_ROTATION))==0){
		p->CC_texture.rotation=GET_JSON_VALUE(ppName).asFloat();
	} else if(strcmp(propName,F_STR(M_MPROP_CC_EXPONENT))==0){
		p->CC_fresnel.exponent=GET_JSON_VALUE(ppName).asFloat();
	} else if(strcmp(propName,F_STR(M_MPROP_CC_FMIN))==0){
		p->CC_fresnel.fmin=GET_JSON_VALUE(ppName).asFloat();
	} else if(strcmp(propName,F_STR(M_MPROP_CC_FMAX))==0){
		p->CC_fresnel.fmax=GET_JSON_VALUE(ppName).asFloat();
	} else if(strcmp(propName,F_STR(M_MPROP_CC_WEIGHT))==0){
		p->CC_weight=GET_JSON_VALUE(ppName).asFloat();
	} else if(strcmp(propName,F_STR(M_MPROP_CC_UVREPEAT))==0){
		p->CC_texture.UVrepeat.x=GET_JSON_VALUE_INDEXED(ppName,0).asFloat();
		p->CC_texture.UVrepeat.y=GET_JSON_VALUE_INDEXED(ppName,1).asFloat();
	} else if(strcmp(propName,F_STR(M_MPROP_CC_USEORANGEPEELNORMALMAPTS))==0){
		p->CC_texture.enable=GET_JSON_VALUE(ppName).asBool();

		//flake
	} else if(strcmp(propName,F_STR(M_MPROP_CF_COLOR))==0){
		p->CF_color.x=GET_JSON_VALUE_INDEXED(ppName,0).asFloat();
		p->CF_color.y=GET_JSON_VALUE_INDEXED(ppName,1).asFloat();
		p->CF_color.z=GET_JSON_VALUE_INDEXED(ppName,2).asFloat();
	} else if(strcmp(propName,F_STR(M_MPROP_CF_DENSITY))==0){
		p->CF_density=GET_JSON_VALUE(ppName).asFloat();
	} else if(strcmp(propName,F_STR(M_MPROP_CF_IRREGULARITY))==0){
		p->CF_irregularity=GET_JSON_VALUE(ppName).asFloat();
	} else if(strcmp(propName,F_STR(M_MPROP_CF_REFLECTIVITY))==0){
		p->CF_reflectivity=GET_JSON_VALUE(ppName).asFloat();
	} else if(strcmp(propName,F_STR(M_MPROP_CF_ROUGHNESS))==0){
		p->CF_roughness=GET_JSON_VALUE(ppName).asFloat();
	} else if(strcmp(propName,F_STR(M_MPROP_CF_SIZE))==0){
		p->CF_size=GET_JSON_VALUE(ppName).asFloat();

		//base
	} else if(strcmp(propName,F_STR(M_MPROP_BS_EXPONENT))==0){
		p->BS_fresnel.exponent=GET_JSON_VALUE(ppName).asFloat();
	} else if(strcmp(propName,F_STR(M_MPROP_BS_FMIN))==0){
		p->BS_fresnel.fmin=GET_JSON_VALUE(ppName).asFloat();
	} else if(strcmp(propName,F_STR(M_MPROP_BS_FMAX))==0){
		p->BS_fresnel.fmax=GET_JSON_VALUE(ppName).asFloat();
	} else if(strcmp(propName,F_STR(M_MPROP_BS_USEDIFFUSEMAPTS))==0){
		p->BS_textureDiffuse.enable=GET_JSON_VALUE(ppName).asBool();
	} else if(strcmp(propName,F_STR(M_MPROP_BS_DIFFUSECOLOR))==0){
		p->BS_textureDiffuse.color.x=GET_JSON_VALUE_INDEXED(ppName,0).asFloat();
		p->BS_textureDiffuse.color.y=GET_JSON_VALUE_INDEXED(ppName,1).asFloat();
		p->BS_textureDiffuse.color.z=GET_JSON_VALUE_INDEXED(ppName,2).asFloat();
	} else if(strcmp(propName,F_STR(M_MPROP_BS_UVREPEAT))==0){
		float x=GET_JSON_VALUE_INDEXED(ppName,0).asFloat();
		float y=GET_JSON_VALUE_INDEXED(ppName,1).asFloat();
		p->BS_textureDiffuse.UVrepeat.x=x;
		p->BS_textureDiffuse.UVrepeat.y=y;
		p->BS_textureGlossy.UVrepeat.x=x;
		p->BS_textureGlossy.UVrepeat.y=y;
	} else if(strcmp(propName,F_STR(M_MPROP_BS_ROTATION))==0){
		p->BS_textureDiffuse.rotation=GET_JSON_VALUE(ppName).asFloat();
		p->BS_textureGlossy.rotation=GET_JSON_VALUE(ppName).asFloat();
	} else if(strcmp(propName,F_STR(M_MPROP_BS_GLOSSYCOLOR))==0){
		p->BS_textureGlossy.color.x=GET_JSON_VALUE_INDEXED(ppName,0).asFloat();
		p->BS_textureGlossy.color.y=GET_JSON_VALUE_INDEXED(ppName,1).asFloat();
		p->BS_textureGlossy.color.z=GET_JSON_VALUE_INDEXED(ppName,2).asFloat();
	} else if(strcmp(propName,F_STR(M_MPROP_BS_REFLECTCOLOR))==0){
		p->BS_colorReflect.x=GET_JSON_VALUE_INDEXED(ppName,0).asFloat();
		p->BS_colorReflect.y=GET_JSON_VALUE_INDEXED(ppName,1).asFloat();
		p->BS_colorReflect.z=GET_JSON_VALUE_INDEXED(ppName,2).asFloat();
	} else if(strcmp(propName,F_STR(M_MPROP_BS_GAMMA))==0){
		p->BS_textureGlossy.invGamma=1.f/GET_JSON_VALUE(ppName).asFloat();
	} else if(strcmp(propName,F_STR(M_MPROP_BS_USEGLOSSYMAPTS))==0){
		p->BS_textureGlossy.enable=GET_JSON_VALUE(ppName).asBool();
	} else if(strcmp(propName,F_STR(M_MPROP_BS_IOR))==0){
		p->BS_ior=GET_JSON_VALUE(ppName).asFloat();
	} else if(strcmp(propName,F_STR(M_MPROP_BS_AOINTENSITY))==0){
		p->BS_AOIntensity=GET_JSON_VALUE(ppName).asFloat();
	} else if(strcmp(propName,F_STR(M_MPROP_BS_SIGMA))==0){
		//p->BS_sigma=GET_JSON_VALUE(ppName).asFloat();
	} else if(strcmp(propName,F_STR(M_MPROP_BS_ROUGHNESS))==0){
		//TODO(JC);fix magic number
		p->BS_roughness=GET_JSON_VALUE(ppName).asFloat()*0.1f;
	} else if(strcmp(propName,F_STR(M_MPROP_BS_INTENSITY))==0){
		p->BS_AOIntensity=GET_JSON_VALUE(ppName).asFloat();
	} else if(strcmp(propName,F_STR(M_MPROP_BS_CENTER_X))==0){
		p->BS_center.x=GET_JSON_VALUE(ppName).asFloat();
	} else if(strcmp(propName,F_STR(M_MPROP_BS_CENTER_Y))==0){
		p->BS_center.y=GET_JSON_VALUE(ppName).asFloat();
	} else if(strcmp(propName,F_STR(M_MPROP_BS_CENTER_Z))==0){
		p->BS_center.z=GET_JSON_VALUE(ppName).asFloat();
	} else if(strcmp(propName,F_STR(M_MPROP_BS_DIFFUSEREFLECTANCE))==0){
		p->BS_reflectanceDiffuse=GET_JSON_VALUE(ppName).asFloat();
	} else if(strcmp(propName,F_STR(M_MPROP_BS_SPECULARREFLECTANCE))==0){
		p->BS_reflectanceDiffuse=GET_JSON_VALUE(ppName).asFloat();
	} else if(strcmp(propName,F_STR(M_MPROP_BS_ANISOTROPYU))==0){
		p->BS_anisotropyU=GET_JSON_VALUE(ppName).asFloat();
	} else if(strcmp(propName,F_STR(M_MPROP_BS_ANISOTROPYV))==0){
		p->BS_anisotropyV=GET_JSON_VALUE(ppName).asFloat();
	} else if(strcmp(propName,F_STR(M_MPROP_BS_CHECKBOARDSIZE))==0){
		p->BS_checkboardSize=GET_JSON_VALUE(ppName).asFloat();
	}



	// geometry
	else if(strcmp(propName,F_STR(M_MPROP_GE_BUMPSCALE))==0){
		p->GE_texture.bumpscale=GET_JSON_VALUE(ppName).asFloat();
	} else if(strcmp(propName,F_STR(M_MPROP_GE_ROTATION))==0){
		p->GE_texture.rotation=GET_JSON_VALUE(ppName).asFloat();
	} else if(strcmp(propName,F_STR(M_MPROP_GE_REPEAT))==0){
		p->GE_texture.UVrepeat.x=GET_JSON_VALUE(ppName).asFloat();
		p->GE_texture.UVrepeat.x=GET_JSON_VALUE(ppName).asFloat();
	} else if(strcmp(propName,F_STR(M_MPROP_GE_UVREPEAT))==0){
		p->GE_texture.UVrepeat.x=GET_JSON_VALUE_INDEXED(ppName,0).asFloat();
		p->GE_texture.UVrepeat.y=GET_JSON_VALUE_INDEXED(ppName,1).asFloat();
	} else if(strcmp(propName,F_STR(M_MPROP_GE_USEBUMPMAPTS))==0){
		p->GE_texture.enable=GET_JSON_VALUE(ppName).asBool();



		// Env
	} else if(strcmp(propName,F_STR(M_MPROP_SHADOW_ATTENUATION))==0){
		p->GE_shadowAttenuation=GET_JSON_VALUE(ppName).asFloat();
	} else{
		oclStr s="不存在的属性:propName:"+oclStr(propName);
		F_DBG_ERROR(s);
	}
#undef GET_JSON_VALUE_INDEXED
#undef GET_JSON_VALUE
#undef SET_JSON_VALUE_INDEXED
#undef SET_JSON_VALUE
}

oclBool ConvertJsonToMaterialLibraryDescription(const oclStr& jsonString,SMaterialLibraryDescription* out){
	Json::Value v;
	Json::Reader r;
	if(r.parse(jsonString,v)){
		out->libName=v[M_LIBNAME].asString();
		out->ID=v[M_INDEX].asInt();
		Json::Value& jl=v[M_LIST];
		const int N=jl.size();
		for(int i=0; i<N;++i){
			Json::Value& j=jl[i];
			SMaterialDescription md;
			ConvertJsonToMaterialDescription(j,&md);
			out->vecMaterial.emplace_back(md);
		}
		return true;
	}
	return false;
}

oclBool ConvertMaterialLibraryDescriptionToJson(const SMaterialLibraryDescription& desp,oclStr* out){
	Json::Value jmat;
	jmat[M_LIBNAME]=desp.libName;
	jmat[M_INDEX]=desp.ID;

	oclInt N=desp.vecMaterial.size();
	for(int i=0;i<N;++i){
		const SMaterialDescription& d=desp.vecMaterial[i];
		ConvertMaterialDescriptionToJson(d,&jmat[M_LIST][i]);
	}

	*out=jmat.toStyledString();
	return true;
}

oclBool ConvertMaterialLibraryDescriptionToJson_scene(const SMaterialLibraryDescription & desp,oclStr * out){
	oclInt N=desp.vecMaterial.size();
	if(N<=0){
		*out="null\n";
	} else{
		Json::Value jmat;

		for(int i=0;i<N;++i){
			const SMaterialDescription& d=desp.vecMaterial[i];
			ConvertMaterialDescriptionToJson(d,&jmat[std::to_string(i)]);
		}

		*out=jmat.toStyledString();
	}
	return true;
}

oclBool ConvertJsonToMaterialDescription(const Json::Value & j,SMaterialDescription * d){
	d->itemIndex=j[M_ITEM_INDEX].asInt();
	d->jsonString=j[M_MAT_DATA].asString();
	d->definitionName=j[M_MAT_DEF].asString();
	d->ID=j[M_MAT_INDEX].asInt();
	d->type=j[M_TYPE].asString();
	d->thumbnailPath=j[M_THUMBNAIL].asString();
	d->group=j[M_GROUP].asString();
	return true;
}

oclStr ConvertMaterialTexURLNameToNewName(const SMaterialDescription desp){
  Json::Value v;
  Json::Reader r;
  r.parse(desp.jsonString,v);
  int size=v[M_TEXPPT].size();
  for(int it=0; it<size; it++){
    auto mem=v[M_TEXPPT][it].getMemberNames();
    for(auto iter=mem.begin();iter!=mem.end();iter++){
      const oclInt N=v[M_TEXPPT][it][*iter].size();
      for(oclInt i=0; i<N; ++i){
        auto mem2=v[M_TEXPPT][it][*iter][i].getMemberNames();
        const oclStr name=*mem2.begin();
        const oclInt type=v[M_PPT][name][M_TYPE].asInt();
        if(type==36){
          oclStr tmp=v[M_PPT][name][M_VALUE].asString();
          v[M_PPT][name][M_VALUE]=desp.textureMap.find(tmp)->second;
        }
      }
    }
  }
  return v.toStyledString();
}

oclBool ConvertMaterialLibraryDescriptionToJson_save(const SMaterialLibraryDescription & desp,oclStr * out){
	Json::Reader r;
	Json::Value j;

	const oclInt N=desp.vecMaterial.size();
	for(int i=0;i<N;++i){
		const SMaterialDescription& d=desp.vecMaterial[i];
       oclStr newStr=ConvertMaterialTexURLNameToNewName(d);
			Json::Value v;
			r.parse(newStr,v);
			j[std::to_string(d.ID)]=v;
	}

	*out=j.toStyledString();
	return true;
}

oclBool ConvertJsonToMaterialLibraryDescription_save(const oclStr & jsonString,SMaterialLibraryDescription * out){
	Json::Value v;
	Json::Reader r;
	if(r.parse(jsonString,v)){
		auto mem=v.getMemberNames();
		for(auto it=mem.begin(); it!=mem.end(); ++it){
			SMaterialDescription md;
			md.ID=std::stoi(*it);
			md.jsonString=v[*it].toStyledString();
			const Json::Value& j=v[*it];
			md.type=j[M_TYPE].asString();
			md.group=j[M_GROUP].asString();
			md.definitionName=j[M_MAT_DEF].asString();
			out->vecMaterial.emplace_back(md);
		}
		return true;
	}
	return false;
}

oclBool ConvertMaterialDescriptionToJson(const SMaterialDescription & d,Json::Value * out){
	Json::Value& j=*out;
	j[M_ITEM_INDEX]=d.itemIndex;
	j[M_MAT_DATA]=d.jsonString;
	j[M_MAT_DEF]=d.definitionName;
	j[M_MAT_INDEX]=d.ID;
	j[M_TYPE]=d.type;
	j[M_THUMBNAIL]=d.thumbnailPath;
	j[M_GROUP]=d.group;
	return true;
}

oclBool ConvertMaterialDescriptionToJson(const SMaterialDescription & desp,oclStr * out){
	Json::Value j;
	ConvertMaterialDescriptionToJson(desp,&j);
	*out=j.toStyledString();
	return true;
}

oclBool ConvertJsonToDataMaterial(const Json::Value& value,SData_Material* p,std::map<oclStr,oclStr>* names){
   int size=value[M_NORPPT].size();
	for(int it=0; it<size; it++){
     auto mem=value[M_NORPPT][it].getMemberNames();
     for(auto iter=mem.begin();iter!=mem.end();iter++){
       const oclInt N=value[M_NORPPT][it][*iter].size();
       for(oclInt i=0; i<N; ++i){
         auto mem2=value[M_NORPPT][it][*iter][i].getMemberNames();
         const oclStr ppName=*mem2.begin();
         InitSDataMaterialProperty(value,ppName,p);
       }
     }
	}
   size=value[M_TEXPPT].size();
   for(int it=0; it<size; it++){
     auto mem=value[M_TEXPPT][it].getMemberNames();
     for(auto iter=mem.begin();iter!=mem.end();iter++){
       const oclInt N=value[M_TEXPPT][it][*iter].size();
       for(oclInt i=0; i<N; ++i){
         auto mem2=value[M_TEXPPT][it][*iter][i].getMemberNames();
         const oclStr ppName=*mem2.begin();
         const oclInt type=value[M_PPT][ppName][M_TYPE].asInt();
         if(type==36){
           (*names)[ppName]=value[M_PPT][ppName][M_VALUE].asString();
         } else{
           InitSDataMaterialProperty(value,ppName,p);
         }
       }
     }
	}
	return true;
}

oclBool GetTypesTextureFileNames(const std::map<oclStr,Json::Value>& mapTypes,std::vector<oclStr>* out){
	for(auto& it:mapTypes){
		const Json::Value& v=it.second;
      
      int size=v[M_TEXPPT].size();
      for(int it=0; it<size; it++){
        auto mem=v[M_TEXPPT][it].getMemberNames();
        for(auto iter=mem.begin();iter!=mem.end();iter++){
          const oclInt N=v[M_TEXPPT][it][*iter].size();
          for(oclInt i=0; i<N; ++i){
            auto mem2=v[M_TEXPPT][it][*iter][i].getMemberNames();
            const oclStr name=*mem2.begin();
            const oclChar* propName=name.c_str();
            if(strcmp(propName,F_STR(M_MPROP_GE_BUMPTEXTUREPATH))==0){
              (*out).push_back(v[M_PPT][name][M_VALUE].asString());
            } else if(strcmp(propName,F_STR(M_MPROP_CC_ORANGEPEELTEXTUREPATH))==0){
              (*out).push_back(v[M_PPT][name][M_VALUE].asString());
            } else if(strcmp(propName,F_STR(M_MPROP_BS_DIFFUSETEXTUREPATH))==0){
              (*out).push_back(v[M_PPT][name][M_VALUE].asString());
            } else if(strcmp(propName,F_STR(M_MPROP_BS_GLOSSYTEXTUREPATH))==0){
              (*out).push_back(v[M_PPT][name][M_VALUE].asString());
            }
          }
        }
      }
	}
	return true;
}

void SetMaterialProperty(const oclStr & propName,const VARIANT & f1,Json::Value * value,SData_Material * p){
	const oclChar* propName_c;
	switch((f1).vt){
	case VT_R4:
	{
		const float v_float=f1.fltVal;
		(*value)[M_PPT][propName][M_VALUE]=v_float;

		propName_c=propName.c_str();
		if(strcmp(propName_c,F_STR(M_MPROP_CC_BUMPSCALE))==0){
			p->CC_texture.bumpscale=v_float;
		} else if(strcmp(propName_c,F_STR(M_MPROP_CC_ROTATION))==0){
			p->CC_texture.rotation=v_float;
		} else if(strcmp(propName_c,F_STR(M_MPROP_CC_FMIN))==0){
			p->CC_fresnel.fmin=v_float;
		} else if(strcmp(propName_c,F_STR(M_MPROP_CC_FMAX))==0){
			p->CC_fresnel.fmax=v_float;
		} else if(strcmp(propName_c,F_STR(M_MPROP_CC_EXPONENT))==0){
			p->CC_fresnel.exponent=v_float;
		} else if(strcmp(propName_c,F_STR(M_MPROP_CC_WEIGHT))==0){
			p->CC_weight=v_float;
		} else if(strcmp(propName_c,F_STR(M_MPROP_CF_IRREGULARITY))==0){
			p->CF_irregularity=v_float;
		} else if(strcmp(propName_c,F_STR(M_MPROP_CF_REFLECTIVITY))==0){
			p->CF_reflectivity=v_float;
		} else if(strcmp(propName_c,F_STR(M_MPROP_CF_DENSITY))==0){
			p->CF_density=v_float;
		} else if(strcmp(propName_c,F_STR(M_MPROP_CF_ROUGHNESS))==0){
			p->CF_roughness=v_float;
		} else if(strcmp(propName_c,F_STR(M_MPROP_CF_SIZE))==0){
			p->CF_size=v_float;
		} else if(strcmp(propName_c,F_STR(M_MPROP_BS_EXPONENT))==0){
			p->BS_fresnel.exponent=v_float;
		} else if(strcmp(propName_c,F_STR(M_MPROP_BS_FMIN))==0){
			p->BS_fresnel.fmin=v_float;
		} else if(strcmp(propName_c,F_STR(M_MPROP_BS_FMAX))==0){
			p->BS_fresnel.fmax=v_float;
		} else if(strcmp(propName_c,F_STR(M_MPROP_BS_IOR))==0){
			p->BS_ior=v_float;
		} else if(strcmp(propName_c,F_STR(M_MPROP_BS_GAMMA))==0){
			p->BS_textureGlossy.invGamma=1.f/v_float;
		} else if(strcmp(propName_c,F_STR(M_MPROP_BS_AOINTENSITY))==0){
			p->BS_AOIntensity=v_float;
		} else if(strcmp(propName_c,F_STR(M_MPROP_BS_SIGMA))==0){
			//p->BS_sigma=v_float;
		} else if(strcmp(propName_c,F_STR(M_MPROP_BS_ROTATION))==0){
			p->BS_textureDiffuse.rotation=v_float;
			p->BS_textureDiffuse.rotation=v_float;
		} else if(strcmp(propName_c,F_STR(M_MPROP_BS_ROUGHNESS))==0){
			// TODO: fix magic number;
			p->BS_roughness=v_float*0.1f;
		} else if(strcmp(propName_c,F_STR(M_MPROP_BS_INTENSITY))==0){
			p->BS_AOIntensity=v_float;
		} else if(strcmp(propName_c,F_STR(M_MPROP_BS_CENTER_X))==0){
			p->BS_center.x=v_float;
		} else if(strcmp(propName_c,F_STR(M_MPROP_BS_CENTER_Y))==0){
			p->BS_center.y=v_float;
		} else if(strcmp(propName_c,F_STR(M_MPROP_BS_CENTER_Z))==0){
			p->BS_center.z=v_float;
		} else if(strcmp(propName_c,F_STR(M_MPROP_BS_DIFFUSEREFLECTANCE))==0){
			p->BS_reflectanceDiffuse=v_float;
		} else if(strcmp(propName_c,F_STR(M_MPROP_BS_SPECULARREFLECTANCE))==0){
			p->BS_reflectanceSpecular=v_float;
		} else if(strcmp(propName_c,F_STR(M_MPROP_BS_ANISOTROPYU))==0){
			p->BS_anisotropyU=v_float;
		} else if(strcmp(propName_c,F_STR(M_MPROP_BS_ANISOTROPYV))==0){
			p->BS_anisotropyV=v_float;
		} else if(strcmp(propName_c,F_STR(M_MPROP_BS_CHECKBOARDSIZE))==0){
			p->BS_checkboardSize=v_float;
		} else if(strcmp(propName_c,F_STR(M_MPROP_GE_BUMPSCALE))==0){
			p->GE_texture.bumpscale=v_float;
		} else if(strcmp(propName_c,F_STR(M_MPROP_GE_ROTATION))==0){
			p->GE_texture.rotation=v_float;
		} else if(strcmp(propName_c,F_STR(M_MPROP_GE_REPEAT))==0){
			p->GE_texture.UVrepeat.x=v_float;
			p->GE_texture.UVrepeat.y=v_float;
		} else if(strcmp(propName_c,F_STR(M_MPROP_SHADOW_ATTENUATION))==0){
			p->GE_shadowAttenuation=v_float;
		}
		break;
	}
	case VT_BOOL:
	{
		const bool v_bool=f1.boolVal;
		(*value)[M_PPT][propName][M_VALUE]=v_bool;

		propName_c=propName.c_str();
		if(strcmp(propName_c,F_STR(M_MPROP_BS_USEGLOSSYMAPTS))==0){
			p->BS_textureGlossy.enable=v_bool;
		} else if(strcmp(propName_c,F_STR(M_MPROP_CC_USEORANGEPEELNORMALMAPTS))==0){
			p->CC_texture.enable=v_bool;
		} else if(strcmp(propName_c,F_STR(M_MPROP_BS_USEDIFFUSEMAPTS))==0){
			p->BS_textureDiffuse.enable=v_bool;
		} else if(strcmp(propName_c,F_STR(M_MPROP_GE_USEBUMPMAPTS))==0){
			p->GE_texture.enable=v_bool;
		}
		break;
	}
	case VT_BSTR:
	{
		oclChar* url=_com_util::ConvertBSTRToString(f1.bstrVal);
		oclStr strUrl=url;
		(*value)[M_PPT][propName][M_VALUE]=strUrl;
		delete[] url;
		break;
	}
	case VT_I4:
	case VT_INT:
	default:
	break;
	}
}

void SetMaterialProperty(const oclStr & propName,const VARIANT & f1,const VARIANT & f2,Json::Value * value,SData_Material * p){
	const oclChar* propName_c=propName.c_str();
	switch(f1.vt){
	case VT_R4:
	{
		float v1=f1.fltVal;
		float v2=f2.fltVal;
		(*value)[M_PPT][propName][M_VALUE][0][M_VALUE]=v1;
		(*value)[M_PPT][propName][M_VALUE][1][M_VALUE]=v2;
		if(strcmp(propName_c,F_STR(M_MPROP_CC_UVREPEAT))==0){
			p->CC_texture.UVrepeat.x=v1;
			p->CC_texture.UVrepeat.y=v2;
		} else if(strcmp(propName_c,F_STR(M_MPROP_BS_UVREPEAT))==0){
			p->BS_textureDiffuse.UVrepeat.x=v1;
			p->BS_textureDiffuse.UVrepeat.y=v2;
			p->BS_textureGlossy.UVrepeat.x=v1;
			p->BS_textureGlossy.UVrepeat.y=v2;
		} else if(strcmp(propName_c,F_STR(M_MPROP_GE_UVREPEAT))==0){
			p->GE_texture.UVrepeat.x=v1;
			p->GE_texture.UVrepeat.y=v2;
		}
	}
	break;
	case VT_I4:
	case VT_BSTR:
	default:
	break;
	}
}

void SetMaterialProperty(const oclStr & propName,const VARIANT & f1,const VARIANT & f2,const VARIANT & f3,Json::Value * value,SData_Material * p){
	switch(f1.vt){
	case VT_R4:
	{
		float v1=f1.fltVal;
		float v2=f2.fltVal;
		float v3=f3.fltVal;
		(*value)[M_PPT][propName][M_VALUE][0][M_VALUE]=v1;
		(*value)[M_PPT][propName][M_VALUE][1][M_VALUE]=v2;
		(*value)[M_PPT][propName][M_VALUE][2][M_VALUE]=v3;
		const oclChar* propName_c=propName.c_str();
		if(strcmp(propName_c,F_STR(M_MPROP_CC_COLOR))==0){
			p->CC_texture.color.x=v1;
			p->CC_texture.color.y=v2;
			p->CC_texture.color.z=v3;
		} else if(strcmp(propName_c,F_STR(M_MPROP_CF_COLOR))==0){
			p->CF_color.x=v1;
			p->CF_color.y=v2;
			p->CF_color.z=v3;
		} else if(strcmp(propName_c,F_STR(M_MPROP_BS_DIFFUSECOLOR))==0){
			p->BS_textureDiffuse.color.x=v1;
			p->BS_textureDiffuse.color.y=v2;
			p->BS_textureDiffuse.color.z=v3;
		} else if(strcmp(propName_c,F_STR(M_MPROP_BS_GLOSSYCOLOR))==0){
			p->BS_textureGlossy.color.x=v1;
			p->BS_textureGlossy.color.y=v2;
			p->BS_textureGlossy.color.z=v3;
		} else if(strcmp(propName_c,F_STR(M_MPROP_BS_REFLECTCOLOR))==0){
			p->BS_colorReflect.x=v1;
			p->BS_colorReflect.y=v2;
			p->BS_colorReflect.z=v3;
		}
		break;
	}
	case VT_I4:
	case VT_BSTR:
	default:
	break;
	}
}

void GetMaterialProperty(const oclStr & propName,const Json::Value & value,VARIANT * f1){
	ParamType type=(ParamType)(value[M_PPT][propName][M_TYPE].asInt());
	switch(type){
	case TYPE_INT:
	(*f1).vt=VT_I4;
	(*f1).intVal=value[M_PPT][propName][M_VALUE].asInt();
	break;
	case TYPE_FLOAT:
	(*f1).vt=VT_R4;
	(*f1).fltVal=value[M_PPT][propName][M_VALUE].asFloat();
	break;
	case TYPE_STRING:
	default:
	//TODO: WTF;
	break;
	}
}

void GetMaterialProperty(const oclStr & propName,const Json::Value & value,VARIANT * f1,VARIANT * f2){
	ParamType type=(ParamType)(value[M_PPT][propName][M_TYPE].asInt());
	switch(type){
	case TYPE_INT:
	(*f1).vt=VT_I4;
	(*f2).vt=VT_I4;
	(*f1).intVal=value[M_PPT][propName][M_VALUE][0][M_VALUE].asInt();
	(*f2).intVal=value[M_PPT][propName][M_VALUE][1][M_VALUE].asInt();
	break;
	case TYPE_FLOAT:
	(*f1).vt=VT_R4;
	(*f2).vt=VT_R4;
	(*f1).fltVal=value[M_PPT][propName][M_VALUE][0][M_VALUE].asFloat();
	(*f2).fltVal=value[M_PPT][propName][M_VALUE][1][M_VALUE].asFloat();
	break;
	case VT_BSTR:
	break;
	default:
	break;
	}
}

void GetMaterialProperty(const oclStr & propName,const Json::Value & value,VARIANT * f1,VARIANT * f2,VARIANT * f3){
	ParamType type=(ParamType)(value[M_PPT][propName][M_TYPE].asInt());
	switch(type){
	case TYPE_INT:
	(*f1).vt=VT_I4;
	(*f2).vt=VT_I4;
	(*f3).vt=VT_I4;
	(*f1).intVal=value[M_PPT][propName][M_VALUE][0][M_VALUE].asInt();
	(*f2).intVal=value[M_PPT][propName][M_VALUE][1][M_VALUE].asInt();
	(*f3).intVal=value[M_PPT][propName][M_VALUE][2][M_VALUE].asInt();
	break;
	case TYPE_COLOUR_RGB:
	(*f1).vt=VT_R4;
	(*f2).vt=VT_R4;
	(*f3).vt=VT_R4;
	(*f1).fltVal=value[M_PPT][propName][M_VALUE][0][M_VALUE].asFloat();
	(*f2).fltVal=value[M_PPT][propName][M_VALUE][1][M_VALUE].asFloat();
	(*f3).fltVal=value[M_PPT][propName][M_VALUE][2][M_VALUE].asFloat();
	break;
	case VT_BSTR:
	break;
	default:
	break;
	}
}

}

OCL_NAMESPACE_END
