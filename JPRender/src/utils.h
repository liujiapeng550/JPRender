#pragma once

#include <math.h>
#include <string>
#include <cstdlib>
#include <fstream>


namespace DUCK {
	extern void SetOpenGLPerspectiveMatrix(float l,float r,float b,float t,float n,float f);
	extern bool SaveData(const char* matName,std::string dataStr);
}
