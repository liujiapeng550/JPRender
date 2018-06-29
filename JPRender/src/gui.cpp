#include "gui.h"
#include <string>
#include "imgui/imgui.h"

const static ImVec4 IMCOLOR_GRAY(0.6f,0.6f,0.6f,1.0f);
const static ImVec4 IMCOLOR_WHITE(1.f,1.f,1.f,1.f);
const static ImVec4 IMCOLOR_RED(1.0f,0.0f,0.0f,1.0f);

GUI::GUI(){
  /*
  m_renderData.camera_enableDOF=false;
  m_renderData.color_ambient.x=0.f;
  m_renderData.color_ambient.y=0.f;
  m_renderData.color_ambient.z=0.f;
  m_renderData.color_ambient.w=1.f;
  m_renderData.color_bad.x=0.f;
  m_renderData.color_bad.y=0.f;
  m_renderData.color_bad.z=0.f;
  m_renderData.color_bad.w=1.f;
  m_renderData.enableGI=true;
  m_renderData.frame_current=1u;
  //m_renderData.lightsource_bufID=1000;
  //m_renderData.lightSource_bufsize=1u;
  m_renderData.radiance_importanceThreshold=0.0001f;
  m_renderData.ray_clamp_max=10000000000.f;
  m_renderData.ray_clamp_min=0.00001f;
  m_renderData.ray_iterateDepth=3u;
  m_renderData.samples_per_pixel=256u;
  m_renderData.samples_per_pixel_col=16u;
  m_renderData.samples_per_pixel_row=16u;
  m_renderData.skybox_enableBackground=1;
  */
  samples_per_pixel.name="samples_per_pixel";
  samples_per_pixel.var.fltVal=256.f;
  samples_per_pixel.oldvar.fltVal=256.f;

  bad_color[0].name="bad_color";
  bad_color[0].var.fltVal=1.f;
  bad_color[0].oldvar.fltVal=1.f;
  bad_color[1].name="bad_color";
  bad_color[1].var.fltVal=0.f;
  bad_color[1].oldvar.fltVal=0.f;
  bad_color[2].name="bad_color";
  bad_color[2].var.fltVal=0.f;
  bad_color[2].oldvar.fltVal=0.f;
}

GUI::~GUI(){
}

void GUI::Render(){
  if(ImGui::BeginMainMenuBar()){
    if(ImGui::BeginMenu("?")){
      if(ImGui::MenuItem("About")){ m_bShowAboutWnd=true; }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }


  /// 1. main WND
  if(m_bShowMainWnd){
    ImGui::Begin("Properties");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",1000.0f/ImGui::GetIO().Framerate,ImGui::GetIO().Framerate);

    /*
    static bool cb1=false;
    if(ImGui::Checkbox("camera_enableDOF",&cb1)){ m_renderData.camera_enableDOF=1;
    } else{ m_renderData.camera_enableDOF=0; }
    static bool cb2=true;
    if(ImGui::Checkbox("enableGI",&cb2)){ m_renderData.enableGI=1;
    } else{ m_renderData.enableGI=0; }
    static bool cb3=true;
    if(ImGui::Checkbox("skybox_enableBackground",&cb3)){ m_renderData.enableGI=1;
    } else{ m_renderData.enableGI=0; }
    ImGui::ColorEdit4("color_ambient",&m_renderData.color_ambient);
    ImGui::SliderFloat("radiance_importanceThreshold",&m_renderData.radiance_importanceThreshold,0.f,5.f);
    ImGui::SliderFloat("ray_clamp_max",&m_renderData.ray_clamp_max,0.f,5.f);
    ImGui::SliderFloat("ray_clamp_min",&m_renderData.ray_clamp_min,0.f,5.f);

    
    static int i1=3; ImGui::SliderInt("ray_iterateDepth",&i1,0,10);m_renderData.ray_iterateDepth=i1;
    */
    ImGui::SliderFloat("samples_per_pixel",&samples_per_pixel.var.fltVal,0.f,1000.f);
    ImGui::ColorPicker3("color_bad",bad_color_);
    bad_color[0].var.fltVal=bad_color_[0];
    bad_color[1].var.fltVal=bad_color_[1];
    bad_color[2].var.fltVal=bad_color_[2];


    ImGui::End();
  }


  /// 2 config WND
  if(m_bShowConfigWnd){


    ImGui::Begin("Config");
    ImGui::TextWrapped("Config your remote git server account, along with your local git.exe file URL. The default URL should be here: C:/Program Files/Git/bin/git.exe");

    ImGui::PushAllowKeyboardFocus(false);
    ImGui::InputText("Account",m_cAccount,IM_ARRAYSIZE(m_cAccount));
    ImGui::InputText("git.exe URL",m_cGitExeURL,IM_ARRAYSIZE(m_cGitExeURL));
    ImGui::PopAllowKeyboardFocus();

    ImGui::PushStyleColor(ImGuiCol_Button,(ImVec4)ImColor::HSV(0.6f,0.6f,0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,(ImVec4)ImColor::HSV(0.7f,0.7f,0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,(ImVec4)ImColor::HSV(0.8f,0.8f,0.8f));
    /*
    if(ImGui::Button("Cancel")){
      m_bShowConfigWnd=false;
    };
    ImGui::SameLine();
    */
    ImGui::PopStyleColor(3);

    if(m_bShowConfigWndWarningText)
      ImGui::TextWrapped("Your git.exe file seems not exist! OR your account is empty!");
    ImGui::End();
  }

  /// 3 About WND
  if(m_bShowAboutWnd){
    ImGui::Begin("About",&m_bShowAboutWnd);
    ImGui::TextWrapped("This is a testbed to RenderMaxPBR dll functions. MUST putting under bin directory!");
    ImGui::End();
  }
}

void GUI::PreRender(){
}

void GUI::PostRender(){
  samples_per_pixel.oldvar.intVal=samples_per_pixel.var.intVal;
  bad_color[0].oldvar.fltVal=bad_color[0].var.fltVal;
  bad_color[1].oldvar.fltVal=bad_color[1].var.fltVal;
  bad_color[2].oldvar.fltVal=bad_color[2].var.fltVal;
}

