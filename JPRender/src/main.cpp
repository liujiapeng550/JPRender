#include <vector>
#include <stdio.h>

#include <GL\glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl2.h"
#include "gui.h"
#include "IBaseProcess.h"
#include "EncodingConversion.h"
#include "freeglut_std.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
//model
#include <model.h>
#include <camera.h>
// 键盘回调函数原型声明
void key_callback(GLFWwindow* window,int key,int scancode,int action,int mods);
// 鼠标移动回调函数原型声明
void mouse_callback(GLFWwindow* window,double xpos,double ypos);
// 鼠标滚轮回调函数原型声明
void mouse_scroll_callback(GLFWwindow* window,double xoffset,double yoffset);
void processInput(GLFWwindow *window);
// 场景中移动
void do_movement();

int g_w=600;
int g_h=500;

float deltaTime=0.0f;	// time between current frame and last frame
float lastFrame=0.0f;
// camera
CameraL camera(glm::vec3(0.0f,0.0f,3.0f));
float lastX=g_w/2.0f;
float lastY=g_h/2.0f;
bool firstMouse=true;


std::vector<Vertex> m_vertData;
std::vector<GLuint> m_indices;
RenderMax::IBaseProcess* g_process=0;


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window,double xpos,double ypos){
  if(firstMouse){
    lastX=xpos;
    lastY=ypos;
    firstMouse=false;
  }
  lastX = xpos;
  lastY = ypos;
  float xoffset=xpos-lastX;
  float yoffset=lastY-ypos; // reversed since y-coordinates go from bottom to top

  lastX=xpos;
  lastY=ypos;

  camera.ProcessMouseMovement(xoffset,yoffset);
}

void scroll_callback(GLFWwindow* window,double xoffset,double yoffset){
  camera.ProcessMouseScroll(yoffset);
}


static void error_callback(int error,const char* description){
  fprintf(stderr,"Error %d: %s\n",error,description);
};
static void WndSizeCallBack(GLFWwindow*wnd,int width,int height){
  g_w=width;g_h=height;
  g_process->SetOutputBufReSize(width,height);
}

void processInput(GLFWwindow *window){
  if(glfwGetKey(window,GLFW_KEY_ESCAPE)==GLFW_PRESS)
    glfwSetWindowShouldClose(window,true);

  if(glfwGetKey(window,GLFW_KEY_W)==GLFW_PRESS)
    camera.ProcessKeyboard(FORWARD,deltaTime);
  if(glfwGetKey(window,GLFW_KEY_S)==GLFW_PRESS)
    camera.ProcessKeyboard(BACKWARD,deltaTime);
  if(glfwGetKey(window,GLFW_KEY_A)==GLFW_PRESS)
    camera.ProcessKeyboard(LEFT,deltaTime);
  if(glfwGetKey(window,GLFW_KEY_D)==GLFW_PRESS)
    camera.ProcessKeyboard(RIGHT,deltaTime);
}


void processMesh_(){
  float* pv=new float[12];
  pv[0]=5.f;pv[1]=5.f;pv[2]=0.f;
  pv[3]=5.f;pv[4]=-5.f;pv[5]=0.f;
  pv[6]=-5.f;pv[7]=-5.f;pv[8]=0.f;
  pv[9]=-5.f;pv[10]=5.f;pv[11]=0.f;
  float* pn=new float[12];
  pn[0]=0.f;pn[1]=0.f;pn[2]=1.f;
  pn[3]=0.f;pn[4]=-0.f;pn[5]=1.f;
  pn[6]=-0.f;pn[7]=-0.f;pn[8]=1.f;
  pn[9]=-0.f;pn[10]=0.f;pn[11]=1.f;
  unsigned int* pi=new unsigned int[6];
  pi[0]=0;
  pi[1]=3;
  pi[2]=2;
  pi[3]=2;
  pi[4]=1;
  pi[5]=0;
  float* puv=new float[8];
  puv[0]=0.f;puv[1]=1.f;
  puv[2]=0.f;puv[3]=0.f;
  puv[4]=1.f;puv[5]=0.f;
  puv[6]=1.f;puv[7]=1.f;
  float* pt=nullptr;//tange
  float* pb=nullptr;//bitange
  float* wm=new float[16];
  for(int i=0;i<16;i++){
    wm[i]=0.f;
  }
  wm[0]=wm[5]=wm[10]=wm[15]=1.f;
  float* max=new float[3];
  max[0]=1.f;
  max[1]=1.f;
  max[2]=1.f;

  float* min=new float[3];
  min[0]=-1.f;
  min[1]=-1.f;
  min[2]=-1.f;

  g_process->processMesh(false,1,"meshname","materialName",1,0,4,6,pv,pn,pi,puv,pt,pb,wm,max,min,1,1,"");
}

int main(int,char**){
  // Setup window
  glfwSetErrorCallback(error_callback);
  if(!glfwInit())
    return 1;
  GLFWwindow* window=glfwCreateWindow(g_w,g_h,"JPRender",NULL,NULL);
  glfwMakeContextCurrent(window);
  glfwSetCursorPosCallback(window,mouse_callback);
  glfwSetScrollCallback(window,scroll_callback);

  // tell GLFW to capture our mouse
  glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);

 

  glfwSwapInterval(1); // Enable vsync

  glewInit();
  ImGui::CreateContext();
  ImGuiIO& io=ImGui::GetIO(); (void)io;
  ImGui_ImplGlfwGL2_Init(window,true);
  ImGui::StyleColorsDark();

  glfwSetWindowSizeCallback(window,WndSizeCallBack);
  PCREATEENGINEPROCESSINTERFACE pCreateEngineProcessInterface;
  HMODULE	hdll=LoadLibrary(AsciiToUnicode("JPRenderPBR.dll").c_str());
  if(hdll==NULL){
    printf("LoadLibrary False.");
    return 1;
  }
  pCreateEngineProcessInterface=(PCREATEENGINEPROCESSINTERFACE)::GetProcAddress(hdll,"CreateEngineProcessInterface");
  if(!pCreateEngineProcessInterface){
    printf("CreateEngineProcessInterface False.");
    return 1;
  }


  g_process=pCreateEngineProcessInterface();
  g_process->initObjBase();
  g_process->SetOutputBufReSize(g_w,g_h);
  //g_process->UpdatePinholeCamera( Vector3D(64.f,32.f,32.f), Vector3D(64.f,32.f,31.f), Vector3D(0.f,1.f,0.f), 45.f, 30.f );
  // g_process->UpdatePinholeCamera(Vector3D(cameraPos.x,cameraPos.y,cameraPos.z),Vector3D(cameraFront.x,cameraFront.y,cameraFront.z),Vector3D(cameraUp.x,cameraUp.y,cameraUp.z),45.f,30.f);
  /*int idx=g_process->GetMatOffset();
  g_process->CreateMaterial(idx,nullptr);
  processMesh_();
    g_process->HideSelectMesh(0,true,1);*/

  //load Modle
  Model objModel;
  std::string modelFilePath="model.obj";
  if(!objModel.loadModel(modelFilePath)){
    glfwTerminate();
    std::system("pause");
    return -1;
  }
  std::vector<Mesh> v=objModel.GetMeshVector();
  for(int i=0;i<=v.size();i++){
    int idx=g_process->GetMatOffset();
    g_process->CreateMaterial(idx,nullptr);
  }
  
  int node=0;
  for(auto v_mesh:v){
    std::vector<float> v_pv;
    std::vector<float> v_puv;
    std::vector<float> v_pn;
    std::vector<unsigned int> v_pi;

    for(auto s_vertex:v_mesh.GetVector()){
      v_pv.push_back(s_vertex.position.x);
      v_pv.push_back(s_vertex.position.y);
      v_pv.push_back(s_vertex.position.z);

      v_puv.push_back(s_vertex.texCoords.x);
      v_puv.push_back(s_vertex.texCoords.y);

      v_pn.push_back(s_vertex.normal.x);
      v_pn.push_back(s_vertex.normal.y);
      v_pn.push_back(s_vertex.normal.z);
    }
    for(auto s_Indices:v_mesh.GetIndices()){
      v_pi.push_back(s_Indices);
    }

    float* pt=nullptr;//tange
    float* pb=nullptr;//bitange
    float* wm=new float[16];
    for(int i=0;i<16;i++){
      wm[i]=0.f;
    }
    wm[0]=wm[5]=wm[10]=wm[15]=1.f;

    float* max=new float[3];
    max[0]=1.f;
    max[1]=1.f;
    max[2]=1.f;

    float* min=new float[3];
    min[0]=-1.f;
    min[1]=-1.f;
    min[2]=-1.f;
    g_process->processMesh(false,1,"meshname","materialName",node+2,node+1,v_mesh.GetVector().size(),v_pi.size(),&v_pv[0],&v_pn[0],&v_pi[0],&v_puv[0],pt,pb,wm,max,min,1,1,"");
    g_process->HideSelectMesh(node,true,1);
    ++node;
  }
  float worldMatrix[16]={
    1.f,0.f,0.f,0.f,
    0.f,1.f,0.f,0.f,
    0.f,0.f,1.f,0.f,
    0.f,0.f,0.f,1.f,
  };

  g_process->updateWorldMat(worldMatrix,0,0x00001);



  GUI gui;

  ImVec4 clear_color=ImVec4(0.45f,0.55f,0.60f,1.00f);
  // Main loop
  while(!glfwWindowShouldClose(window)){
    glfwPollEvents();
    // per-frame time logic
    // --------------------
    float currentFrame=glfwGetTime();
    deltaTime=currentFrame-lastFrame;
    lastFrame=currentFrame;
    processInput(window);
    ImGui_ImplGlfwGL2_NewFrame();
    g_process->Render();
    g_process->UpdatePinholeCamera(Vector3D(camera.Position.x,camera.Position.y,camera.Position.z),Vector3D(camera.Front.x,camera.Front.y,camera.Front.z),Vector3D(camera.Up.x,camera.Up.y,camera.Up.z),45.f,30.f);
    // Rendering
    int display_w,display_h;
    glfwGetFramebufferSize(window,&display_w,&display_h);
    glViewport(0,0,display_w,display_h);
    glClearColor(clear_color.x,clear_color.y,clear_color.z,clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    void* imageData=g_process->OutPutMap();
    if(0==imageData){
      fprintf(stderr,"data in buffer is null.\n");
      return 1;
    }
    glDrawPixels(g_w,g_h,GL_RGBA,GL_FLOAT,imageData);
    g_process->OutPutUnMap();
    gui.PreRender();
    gui.Render();

    ImGui::Render();
    ImGui_ImplGlfwGL2_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);


    // change property;
    if(gui.samples_per_pixel.var.fltVal!=gui.samples_per_pixel.oldvar.fltVal){
      g_process->SetEnvProperty(gui.samples_per_pixel.name.c_str(),&gui.samples_per_pixel.var,1,EnviromentType::GI);
    }
    if(gui.bad_color[0].var.fltVal!=gui.bad_color[0].oldvar.fltVal){
      VARIANT v[3]={gui.bad_color[0].var,gui.bad_color[1].var,gui.bad_color[2].var};
      g_process->SetEnvProperty(gui.bad_color[0].name.c_str(),v,1,EnviromentType::GI);
    }

    gui.PostRender();
  }

  // Cleanup
  ImGui_ImplGlfwGL2_Shutdown();
  ImGui::DestroyContext();
  glfwTerminate();

  return 0;
}




