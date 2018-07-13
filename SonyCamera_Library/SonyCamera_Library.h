/*
DLL的对外接口
*/

#pragma once 


#define _C_CPP_INTERFACE_
#define _PYTHON_INTERFACE_

#ifdef _C_CPP_INTERFACE_

#include <opencv2\core\core.hpp>

#define EXPORT __declspec(dllexport)

EXPORT bool OpenCamera();
EXPORT bool CloseCamera();
EXPORT bool StartImageAcquisition();
EXPORT cv::Mat GetImage();

#endif

#ifdef _PYTHON_INTERFACE_

#endif