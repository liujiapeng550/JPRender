#pragma once
#include <vector>
#include <OAIDL.h>

using namespace std;

struct SProperty{
  VARIANT var;
  VARIANT oldvar;
  string name;
};

class GUI final{
public:
  GUI();
  ~GUI();
  void Render();
  void PreRender();
  void PostRender();

public:
  //properties;
  SProperty samples_per_pixel;
  SProperty bad_color[3];
private:
  float bad_color_[3];

private:
  char m_cAccount[255];
  char m_cGitExeURL[255];

  bool m_bShowConfigWnd=false;
  bool m_bShowMainWnd=true;
  bool m_bShowConfigWndWarningText=false;
  bool m_bShowAboutWnd=false;

  
};