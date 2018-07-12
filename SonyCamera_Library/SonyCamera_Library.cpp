// SonyCamera_Library.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"

#include <iostream>
#include <XCCamAPI.h>
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>

#include "SonyCamera_Class.h"
#include "SonyCamera_Library.h"
/*
�����
����ʵ�����࣬������ص���Դ��
*/

static Sony_Camera_Handle g_CameraHandle;

cv::Mat *p_cvMat;

// ����Ľӿ�C/C++
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

// ��ȡͼ��ӿ�
// ���ڴ���е�ͼ�����ݿ������ӿ�ͼ���ڴ���
// ����������Ҫ��������ֹͼ������̸߳ı��ڴ���е�����
cv::Mat GetImage()
{
	cv::Mat mat;

	// ���󻥳���
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

	// �ͷŻ�����
	ReleaseMutex(g_CameraHandle->hMutex);

	return mat;
}




