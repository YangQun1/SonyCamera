/*
DLL�Ķ���ӿ�
*/

#pragma once 

#include <opencv2\core\core.hpp>

#define EXPORT __declspec(dllexport)

EXPORT bool OpenCamera();
EXPORT bool StartImageAcquisition();
EXPORT cv::Mat GetImage();
EXPORT bool CloseCamera();