#include "material\oclmaterial_manager.h"
#include "core\ocltexture.h"
#include "material\oclmaterial.h"
#include "material\oclmaterial_library.h"
#define BOOST_FILESYSTEM_NO_DEPRECATED
#include "boost\filesystem.hpp"

using namespace std;

OCL_NAMESPACE_START

oclMaterialManager::oclMaterialManager(RTcontext ctx):m_ctx(ctx){
	F_DBG_INFO("[CONSTRUCT] -> oclMaterialManager");
	Init_();
}

oclMaterialManager::~oclMaterialManager(){
	for(auto& it:m_mapLibs){
		oclMaterialLibrary* lib=it.second;
		delete lib;
	}
	m_mapLibs.clear();
  m_ctx=nullptr;
}

oclMaterialLibrary* oclMaterialManager::CreateLibrary(SMaterialLibraryDescription d){
	oclMaterialLibrary* lib=new oclMaterialLibrary(m_ctx);
	lib->Init(d);
	m_mapLibs[d.ID]=lib;
	return lib;
}

oclBool oclMaterialManager::DestroyLibrary(oclMaterialLibrary * lib){
	if(lib){
		m_mapLibs.erase(lib->GetID());
		delete lib;
		return true;
	}
	return false;
}

oclMaterialLibrary * oclMaterialManager::GetLibrary(oclInt libIndex){
  map<oclInt,oclMaterialLibrary*>::iterator it=m_mapLibs.find(libIndex);
	return it==m_mapLibs.end()?nullptr:it->second;
}

const oclMaterialLibrary * oclMaterialManager::GetLibrary(oclInt libIndex) const{
  map<oclInt,oclMaterialLibrary*>::const_iterator it=m_mapLibs.find(libIndex);
	return it==m_mapLibs.end()?nullptr:it->second;
}

oclMaterial * oclMaterialManager::GetMaterial(oclID ID,oclMaterialLibrary** lib){
	oclMaterial* r=nullptr;
	for(auto& it:m_mapLibs){
		r=it.second->GetMaterial(ID);
		if(r){
			*lib=it.second;
			return r;
		}
	}
	return r;
}

oclInt oclMaterialManager::GetLibraryIndex(oclID ID) const{
	//TODO;
	return -1;
}

oclBool oclMaterialManager::Init_(){
	return true;
}

oclBool oclMaterialManager::SaveMaterialEnvTexPath(oclStr topath,oclStr oldEnvPath){
	/*
  //first method
  oclStr filename=oldEnvPath;
  //判断是绝对路径还是相对路径
  int pos1=filename.find_last_of(':');
  if(-1!=pos1){
    //get name & copy
    int pos=filename.find_last_of('\\');
    if(pos!=-1){
      oclStr newname=filename.substr(pos+1);
      oclStr fullname=topath+filename.substr(pos+1);
      oclBool b=CopyFile(charToWChar(filename.c_str()).c_str(),charToWChar(fullname.c_str()).c_str(),true);
    } else{
      pos=filename.find_last_of('/');
      if(pos!=-1){
        oclStr newname=filename.substr(pos+1);
        oclStr fullname=topath+"/"+filename.substr(pos+1);
        oclBool b=CopyFile(charToWChar(filename.c_str()).c_str(),charToWChar(fullname.c_str()).c_str(),true);
      }
    }
  } else{
    WCHAR szFilePath[MAX_PATH+1];
    GetModuleFileName(NULL,szFilePath,MAX_PATH);

    (_tcsrchr(szFilePath,('\\')))[1]=0; //删除文件名，只获得路径
    WCHAR* str_url=szFilePath;    //str_url 中保存的是当前目录

                                  // fix path
    oclStr srcfile=(oclStr)WideCharToChar(str_url)+filename;

    int pos=filename.find_last_of('/');
    if(pos!=-1){
      //get name 
      filename=filename.substr(pos+1);
    }

    oclStr fullname=topath+filename;

    oclBool b=CopyFile(charToWChar(srcfile.c_str()).c_str(),charToWChar(fullname.c_str()).c_str(),true);
  }
	*/

  return true;
}

OCL_NAMESPACE_END
