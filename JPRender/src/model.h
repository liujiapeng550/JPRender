#ifndef _MODEL_H_
#define _MODEL_H_

#include <map>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "mesh.h"


/*
* 代表一个模型 模型可以包含一个或多个Mesh
*/
class Model{
public:
  bool loadModel(const std::string& filePath){
    Assimp::Importer importer;
    if(filePath.empty()){
      std::cerr<<"Error:Model::loadModel, empty model file path."<<std::endl;
      return false;
    }
    const aiScene* sceneObjPtr=importer.ReadFile(filePath,
                                                 aiProcess_Triangulate|
                                                 aiProcess_FlipUVs);
    if(!sceneObjPtr
       ||sceneObjPtr->mFlags==AI_SCENE_FLAGS_INCOMPLETE
       ||!sceneObjPtr->mRootNode){
      std::cerr<<"Error:Model::loadModel, description: "
        <<importer.GetErrorString()<<std::endl;
      return false;
    }
    this->modelFileDir=filePath.substr(0,filePath.find_last_of('/'));
    if(!this->processNode(sceneObjPtr->mRootNode,sceneObjPtr)){
      std::cerr<<"Error:Model::loadModel, process node failed."<<std::endl;
      return false;
    }
    return true;
  }
  ~Model(){
    for(std::vector<Mesh>::const_iterator it=this->meshes.begin(); this->meshes.end()!=it; ++it){
      it->final();
    }
  }

  /*
  * 递归处理模型的结点
  */
  bool processNode(const aiNode* node,const aiScene* sceneObjPtr){
    if(!node||!sceneObjPtr){
      return false;
    }
    // 先处理自身结点
    for(size_t i=0; i<node->mNumMeshes; ++i){
      // 注意node中的mesh是对sceneObject中mesh的索引
      const aiMesh* meshPtr=sceneObjPtr->mMeshes[node->mMeshes[i]];
      if(meshPtr){
        Mesh meshObj;
        if(this->processMesh(meshPtr,sceneObjPtr,meshObj)){
          this->meshes.push_back(meshObj);
        }
      }
    }
    // 处理孩子结点
    for(size_t i=0; i<node->mNumChildren; ++i){
      this->processNode(node->mChildren[i],sceneObjPtr);
    }
    return true;
  }
  bool processMesh(const aiMesh* meshPtr,const aiScene* sceneObjPtr,Mesh& meshObj){
    if(!meshPtr||!sceneObjPtr){
      return false;
    }
    std::vector<Vertex> vertData;
    std::vector<Texture> textures;
    std::vector<GLuint> indices;
    // 从Mesh得到顶点数据、法向量、纹理数据
    for(size_t i=0; i<meshPtr->mNumVertices; ++i){
      Vertex vertex;
      // 获取顶点位置
      if(meshPtr->HasPositions()){
        vertex.position.x=meshPtr->mVertices[i].x;
        vertex.position.y=meshPtr->mVertices[i].y;
        vertex.position.z=meshPtr->mVertices[i].z;
      }
      // 获取纹理数据 目前只处理0号纹理
      if(meshPtr->HasTextureCoords(0)){
        vertex.texCoords.x=meshPtr->mTextureCoords[0][i].x;
        vertex.texCoords.y=meshPtr->mTextureCoords[0][i].y;
      } else{
        vertex.texCoords=glm::vec2(0.0f,0.0f);
      }
      // 获取法向量数据
      if(meshPtr->HasNormals()){
        vertex.normal.x=meshPtr->mNormals[i].x;
        vertex.normal.y=meshPtr->mNormals[i].y;
        vertex.normal.z=meshPtr->mNormals[i].z;
      }
      vertData.push_back(vertex);
    }
     //获取索引数据
    for(size_t i=0; i<meshPtr->mNumFaces; ++i){
      aiFace face=meshPtr->mFaces[i];
      if(face.mNumIndices!=3){
        return false;
      }
      for(size_t j=0; j<face.mNumIndices; ++j){
        indices.push_back(face.mIndices[j]);
      }
    }
    meshObj.setData(vertData,textures,indices);
    return true;
  }
  std::vector<Mesh> GetMeshVector(){ return meshes; }
public:
  std::vector<Mesh> meshes; // 保存Mesh
private:
  std::string modelFileDir; // 保存模型文件的文件夹路径
  typedef std::map<std::string,Texture> LoadedTextMapType; // key = texture file path
  LoadedTextMapType loadedTextureMap; // 保存已经加载的纹理
  float* m_pv;
  float* m_puv;
  float* m_pn;
  unsigned int* m_pi;
  int poSize=0;
  int txSize=0;
  int noSize=0;
  int inSize=0;
};



#endif