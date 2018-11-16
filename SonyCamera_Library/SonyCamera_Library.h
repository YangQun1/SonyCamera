/*
DLL的对外接口
*/

#pragma once 


#define _C_CPP_INTERFACE_

// 定义这个宏，使能额外的C++接口，用于访问Sony_Camera类
#define _C_CPP_ADDITIONAL_
// 定义这个宏，使能python接口
#define _PYTHON_INTERFACE_


#ifdef _C_CPP_INTERFACE_

#include <opencv2\core\core.hpp>
#define EXPORT __declspec(dllexport)

EXPORT bool OpenCamera();
EXPORT bool CloseCamera();
EXPORT bool StartImageAcquisition();
EXPORT bool StopImageAcquisition();
EXPORT bool TriggerShooting();
EXPORT cv::Mat GetImage(signed long timeOut = 250);

#ifdef _C_CPP_ADDITIONAL_
#include "SonyCamera_Class.h"

EXPORT Sony_Camera_Handle GetCameraHandle();

#endif /* _C_CPP_ADDITIONAL_ */

#endif /* _C_CPP_INTERFACE_ */

#ifdef _PYTHON_INTERFACE_

#endif