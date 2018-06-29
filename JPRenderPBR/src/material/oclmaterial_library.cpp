#include "material/oclmaterial_library.h"
#include "core\oclbuffer_manager.h"
#include "material\oclmaterial.h"

using namespace std;

OCL_NAMESPACE_START

oclMaterialLibrary::oclMaterialLibrary(RTcontext ctx)
	:m_ctx(ctx){
	F_DBG_INFO("[CONSTRUCT] -> oclMaterialLibrary");
	m_pBufMaterialData=oclBufferManager::S()->Create(GetUniqueBufferKey("materialDatas"),RT_BUFFER_INPUT,RT_FORMAT_USER);
	m_pBufMaterialData->SetElementSize(sizeof(SData_Material));
	m_pBufMaterialData->SetSize(0);
	m_nBufID=m_pBufMaterialData->GetID();
}

oclMaterialLibrary::~oclMaterialLibrary(){
	DestroyMaterials();
	oclBufferManager::S()->Destroy(m_pBufMaterialData);
	m_pBufMaterialData=nullptr;
	m_ctx=nullptr;
}

oclMaterial* oclMaterialLibrary::CreateMaterial(SMaterialDescription d){
	/// 1. 材质表述体中这两项由材质库指定；
	d.libID=m_nLibID;
	d.itemIndex=m_nNextMaterialDataBufOffset;

	/// 2. 创建材质
	oclMaterial* m=new oclMaterial(m_ctx,m_nBufID,m_nNextMaterialDataBufOffset,d.ID);
	++m_nNextMaterialDataBufOffset;
	m->Reset(d);
	m_mapMaterials[d.ID]=m;
	return m;
}

oclBool oclMaterialLibrary::Init(const SMaterialLibraryDescription & d){
	m_sName=d.libName;
	m_sLibFilePath=d.filePath;
	m_nLibID=d.ID;
	for(auto& md:d.vecMaterial){
		CreateMaterial(md);
	}
	return true;
}

oclBool oclMaterialLibrary::DestroyMaterials(const vector<oclID>& vecIds){
	for(oclID id:vecIds){
		DestroyMaterial(id);
	}
  return true;
}

//oclBool oclMaterialLibrary::DestroyUnusedMaterials(){
//	vector<oclID> vecDel;
//  for(auto var:m_mapMaterials){
//    if(!var.second->GetMaterialInUse()){
//			vecDel.push_back(var.second->GetID());
//    }
//  }
//	return DestroyMaterials(vecDel);
//}

oclMaterial* oclMaterialLibrary::GetMaterial(oclID ID){
	map<oclID,oclMaterial*>::iterator it=m_mapMaterials.find(ID);
	return it==m_mapMaterials.end()?nullptr:it->second;
}

const oclMaterial * oclMaterialLibrary::GetMaterial(oclID ID) const{
	map<oclID,oclMaterial*>::const_iterator it=m_mapMaterials.find(ID);
	return it==m_mapMaterials.end()?nullptr:it->second;
}

oclMaterial * oclMaterialLibrary::GetMaterialByIndex(oclInt index){
	for(auto& it:m_mapMaterials){
		if(it.second->GetIndex()==index)return it.second;
	}
	return nullptr;
}

oclBool oclMaterialLibrary::DestroyMaterial(oclID ID){
	oclMaterial* m=GetMaterial(ID);
	if(m){
		m_mapMaterials.erase(ID);
		delete m;
		return true;
	}
  return false;
}

SMaterialLibraryDescription oclMaterialLibrary::GetDescription()const{
	SMaterialLibraryDescription d;
	d.filePath=m_sLibFilePath;
	d.ID=m_nLibID;
	d.libName=m_sName;
	for(const auto& it:m_mapMaterials){
		const oclMaterial* m=it.second;
		d.vecMaterial.emplace_back(m->GetDescription());
	}
	return d;
}

void oclMaterialLibrary::FetchTextures(std::vector<const oclTexture*>* out)const{
	for(auto v:m_mapMaterials){
		oclMaterial* m=v.second;
		m->FetchTextures(out);
	}
}

void oclMaterialLibrary::Upload(){
	//TODO:pay attention to performence!
	// JC:这里可以尝试优化成只提交对应材质数据；
	const oclInt SIZE=m_mapMaterials.size();
	std::vector<SData_Material> m_vecMaterialData(SIZE);
	for(auto& it:m_mapMaterials){
		oclInt offset=it.second->GetBufferOffset();
		m_vecMaterialData[offset]=it.second->GetMaterialData();
	}
	oclInt bufSize;
	m_pBufMaterialData->GetSize(&bufSize);
	if(bufSize!=SIZE) m_pBufMaterialData->SetSize(SIZE);
	m_pBufMaterialData->SetData<SData_Material>(&m_vecMaterialData[0],SIZE);
}

void oclMaterialLibrary::GetMaterialIDs(std::vector<oclID>* v)const{
  for(const auto &i:m_mapMaterials){
    v->push_back(i.first);
  }
}

void oclMaterialLibrary::DestroyMaterials(){
	for(auto& it:m_mapMaterials){
		oclMaterial* material=it.second;
		delete material;
	}
	m_mapMaterials.clear();
	m_nNextMaterialDataBufOffset=0u;
	m_pBufMaterialData->SetSize(0);
}

OCL_NAMESPACE_END
