#ifndef _MESH_H_
#define _MESH_H_

#include <glew.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/string_cast.hpp>
#include <string>       
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


// 表示一个顶点属性
struct Vertex{
  glm::vec3 position;
  glm::vec2 texCoords;
  glm::vec3 normal;
};

// 表示一个Texture
struct Texture{
  GLuint id;
  aiTextureType type;
  std::string path;
};

// 表示一个用于渲染的最小实体
class Mesh{
public:
 
  Mesh():VAOId(0),VBOId(0),EBOId(0){}
  Mesh(const std::vector<Vertex>& vertData,
       const std::vector<Texture> & textures,
       const std::vector<GLuint>& indices):VAOId(0),VBOId(0),EBOId(0) // 构造一个Mesh
  {
    setData(vertData,textures,indices);
  }
  void setData(const std::vector<Vertex>& vertData,
               const std::vector<Texture> & textures,
               const std::vector<GLuint>& indices){
    this->vertData=vertData;
    this->indices=indices;
    this->textures=textures;
    if(!vertData.empty()&&!indices.empty()){
      this->setupMesh();
    }
  }
  void setDataPtr(){
  
  }
  void final() const{
    glDeleteVertexArrays(1,&this->VAOId);
    glDeleteBuffers(1,&this->VBOId);
    glDeleteBuffers(1,&this->EBOId);
  }
  ~Mesh(){
    // 不要再这里释放VBO等空间 因为Mesh对象传递时 临时对象销毁后这里会清理VBO等空间
  }
  std::vector<Vertex> GetVector(){ return vertData; }
  std::vector<GLuint> GetIndices(){ return indices; }
private:
  std::vector<Vertex> vertData;
  std::vector<GLuint> indices;
  std::vector<Texture> textures;
  GLuint VAOId,VBOId,EBOId;

  void setupMesh()  // 建立VAO,VBO等缓冲区
  {
    glGenVertexArrays(1,&this->VAOId);
    glGenBuffers(1,&this->VBOId);
    glGenBuffers(1,&this->EBOId);

    glBindVertexArray(this->VAOId);
    glBindBuffer(GL_ARRAY_BUFFER,this->VBOId);
    glBufferData(GL_ARRAY_BUFFER,sizeof(Vertex) * this->vertData.size(),
                 &this->vertData[0],GL_STATIC_DRAW);
    // 顶点位置属性
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,
                          sizeof(Vertex),(GLvoid*)0);
    glEnableVertexAttribArray(0);
    // 顶点纹理坐标
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,
                          sizeof(Vertex),(GLvoid*)(3*sizeof(GL_FLOAT)));
    glEnableVertexAttribArray(1);
    // 顶点法向量属性
    glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,
                          sizeof(Vertex),(GLvoid*)(5*sizeof(GL_FLOAT)));
    glEnableVertexAttribArray(2);
    // 索引数据
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,this->EBOId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(GLuint)* this->indices.size(),
                 &this->indices[0],GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindVertexArray(0);
  }
};

#endif 