/*
DLL�Ķ���ӿ�
*/

#pragma once 


#define _C_CPP_INTERFACE_

// ��������꣬ʹ�ܶ����C++�ӿڣ����ڷ���Sony_Camera��
#define _C_CPP_ADDITIONAL_
// ��������꣬ʹ��python�ӿ�
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