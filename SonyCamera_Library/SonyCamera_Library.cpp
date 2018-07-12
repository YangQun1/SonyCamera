// SonyCamera_Library.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"

#include <iostream>
#include <XCCamAPI.h>
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>

#include "SonyCamera_Class.h"
#include "SonyCamera_Library.h"
/*
打开相机
包括实例化类，申请相关的资源等
*/

static Sony_Camera_Handle g_CameraHandle;

cv::Mat *p_cvMat;

// 对外的接口C/C++
bool OpenCamera()
{
	g_CameraHandle = new Sony_Camera();
	g_CameraHandle->_openCam();
	return true;
}

bool CloseCamera()
{
	g_CameraHandle->_closeCam(); 
	delete g_CameraHandle;
	return true;
}


bool StartImageAcquisition()
{
	g_CameraHandle->_startAcquisition();
	return true;
}

// 获取图像接口
// 将内存池中的图像数据拷贝到接口图像内存中
// 拷贝过程需要加锁，防止图像接收线程改变内存池中的数据
cv::Mat GetImage()
{
	cv::Mat mat;

	// 请求互斥锁
	WaitForSingleObject(g_CameraHandle->hMutex, INFINITE);

	if (g_CameraHandle->dataType == Mono8 || \
		g_CameraHandle->dataType == BayerRG8){
		UCHAR *imgBuf = NULL;
		int height, width, channels;

		g_CameraHandle->_getImgBuf(&imgBuf, &height, &width, &channels);

		if (channels == 1){
			mat.create(height, width, CV_8UC1);
		}
		else{
			mat.create(height, width, CV_8UC3);
		}

		if (mat.isContinuous()){
			memcpy(mat.data, imgBuf, height*width*channels);
		}
	}
	else if (g_CameraHandle->dataType == Mono12Packed || \
			g_CameraHandle->dataType == BayerRG12Packed){
		USHORT *imgBuf = NULL;
		int height, width, channels;

		g_CameraHandle->_getImgBuf(&imgBuf, &height, &width, &channels);

		if (channels == 1){
			mat.create(height, width, CV_16UC1);
		}
		else{
			mat.create(height, width, CV_16UC3);
		}

		if (mat.isContinuous()){
			memcpy(mat.data, imgBuf, height*width*channels*sizeof(USHORT));
		}
	}
	else{
		//TODO
	}

	// 释放互斥锁
	ReleaseMutex(g_CameraHandle->hMutex);

	return mat;
}




