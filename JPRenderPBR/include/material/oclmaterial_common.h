#pragma once

#include <vector>
#include <map>
#include "json.h"
#include "core\oclcore.h"

OCL_NAMESPACE_START

struct SMaterialDescription{
	oclID ID;//材质的全局唯一ID，引擎约束不能重复，所以该值交由对接层管理(rmprocessor.cpp)，PBR不做ID的维护；
	oclInt itemIndex=-1;//材质在材质库中的索引，该值由具体的材质库赋值，用户不必关心赋值，使用即可；
	oclInt libID=-1;//材质库ID，该值由具体的材质库赋值，用户不必关心。
	oclStr definitionName="NewMaterial";
	oclStr type="";
	oclStr jsonString="";
	oclStr texturePath="./texture/";
	oclStr thumbnailPath="";
	oclStr group="";
   std::map<oclStr,oclStr> textureMap;//1.第一个元素为原始纹理名称，第二个为修改后得纹理名称
};

struct SMaterialLibraryDescription{
	oclInt ID;
	oclStr libName="NewLibrary";
	oclStr filePath="";//材质库外部文件路径，空的话表示创建一个全新的材质库。
	std::vector<SMaterialDescription> vecMaterial;
};


/**
* Returns oclStr witch is combined by type names of all material types.
* Used for combo box of UI.
*/
oclStr GetTypeNames();
const Json::Value& GetTypeJson(oclStr type);
const void LoadMaterialConfigFile(const oclStr& URL);
void DestroyDefaultTextures();

OCL_NAMESPACE_END
