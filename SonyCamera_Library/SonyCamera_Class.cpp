#include "stdafx.h"
#include "tchar.h"
#include "SequencePool.h"
#include "SonyCamera_Class.h"

#include <iostream>
#include <string>
#include <process.h>

using namespace std;

// #define _IMAGECALLBACK_ 
//---------------------------------------------------------------------------
// System Call back
// When bus reset occurred, when the numbers of camera that I can open changed
// on a bus, when system abnormality occurred, it is called.
extern "C" VOID CALLBACK SystemFunc(STATUS_SYSTEMCODE Status, PVOID Countext)
{
	switch (Status)
	{
	case STATUSXCCAM_DEVICECHANGE:				// Processing of Device Change
		break;

	case STATUSXCCAM_POWERUP:					// Processing of PowerUP
		break;
	}
}

// 定义图像回调函数
#ifdef _IMAGECALLBACK_
extern "C"
VOID  CALLBACK ImageDataRcv(HCAMERA hCamera,
							XCCAM_IMAGE* pImage,
							pXCCAM_IMAGEDATAINFO pImageInfo,
							PVOID Context)
{
	Sony_Camera* pMp = (Sony_Camera*)Context;

	WaitForSingleObject(pMp->hMutex, INFINITE);

	if (pMp->dataType == BayerRG8){
		Image_Pool_8Bit *poolHandle = (Image_Pool_8Bit *)(pMp->imgBufPoolHandle);
		UCHAR *buffer = poolHandle->ReqBuffer();
		// 使用相机API转为24bit的BMP彩图
		XCCAM_ConvExec(pMp->hCamera, pImage, buffer);
		poolHandle->PushBack(buffer);
	}
	else if (pMp->dataType == Mono8){
		Image_Pool_8Bit *poolHandle = (Image_Pool_8Bit *)(pMp->imgBufPoolHandle);
		UCHAR *buffer = poolHandle->ReqBuffer();
		// 直接拷贝内存
		memcpy(buffer, pImage->pBuffer, pImage->Length);
		poolHandle->PushBack(buffer);
	}
	else if (pMp->dataType == BayerRG12Packed){
		Image_Pool_16Bit *poolHandle = (Image_Pool_16Bit *)(pMp->imgBufPoolHandle);
		USHORT *buffer = poolHandle->ReqBuffer();
		// 将12位的bayer转为12位三通道的rgb图像
		// memcpy(buffer, pImage->pBuffer, pImage->Length);
		// TODO
		poolHandle->PushBack(buffer);
	}
	else if (pMp->dataType == Mono12Packed){
		Image_Pool_16Bit *poolHandle = (Image_Pool_16Bit *)(pMp->imgBufPoolHandle);
		USHORT *buffer = poolHandle->ReqBuffer();
		// 直接拷贝内存
		memcpy(buffer, pImage->pBuffer, pImage->Length);
		poolHandle->PushBack(buffer);
	}
	else{
	}

	ReleaseMutex(pMp->hMutex);
}
#endif


#ifndef _IMAGECALLBACK_
// 定义图像采集线程函数
unsigned int __stdcall ImageAcquThread(LPVOID Countext)
{
	Sony_Camera *pMp = (Sony_Camera *)Countext;

	//LARGE_INTEGER li;
	//LONGLONG start, end, freq;

	//QueryPerformanceFrequency(&li);
	//freq = li.QuadPart;

	while (1){
		// 请求退出采集图像事件句柄，若请求到，则退出采集过程
		if (WaitForSingleObject(pMp->endEvent, 0) == WAIT_OBJECT_0){
			ResetEvent(pMp->endEvent);
			break;
		}

		//QueryPerformanceCounter(&li);
		//start = li.QuadPart;

		XCCAM_ImageReq(pMp->hCamera, pMp->pImage);				
		XCCAM_ImageComplete(pMp->hCamera, pMp->pImage, -1, NULL);

		//QueryPerformanceCounter(&li);
		//end = li.QuadPart;
		//int useTime = (int)((end - start) * 1000 / freq);
		//std::cout << "acqu time: " << useTime << "ms" << std::endl;

		WaitForSingleObject(pMp->hMutex, INFINITE);

		if (pMp->dataType == BayerRG8){
			Image_Pool_8Bit *poolHandle = (Image_Pool_8Bit *)(pMp->imgBufPoolHandle);
			UCHAR *buffer = poolHandle->ReqBuffer();
			// 使用相机API转为24bit的BMP彩图
			XCCAM_ConvExec(pMp->hCamera, pMp->pImage, buffer);	
			poolHandle->PushBack(buffer);
		}
		else if (pMp->dataType == Mono8){
			Image_Pool_8Bit *poolHandle = (Image_Pool_8Bit *)(pMp->imgBufPoolHandle);
			UCHAR *buffer = poolHandle->ReqBuffer();
			// 直接拷贝内存
			memcpy(buffer, pMp->pImage->pBuffer, pMp->pImage->Length);
			poolHandle->PushBack(buffer);
		}
		else if (pMp->dataType == BayerRG12Packed){
			Image_Pool_16Bit *poolHandle = (Image_Pool_16Bit *)(pMp->imgBufPoolHandle);
			USHORT *buffer = poolHandle->ReqBuffer();
			// 将12位的bayer转为12位三通道的rgb图像
			// memcpy(buffer, pMp->pImage->pBuffer, pMp->pImage->Length);
			// TODO
			poolHandle->PushBack(buffer);
		}
		else if (pMp->dataType == Mono12Packed){
			Image_Pool_16Bit *poolHandle = (Image_Pool_16Bit *)(pMp->imgBufPoolHandle);
			USHORT *buffer = poolHandle->ReqBuffer();
			// 直接拷贝内存
			memcpy(buffer, pMp->pImage->pBuffer, pMp->pImage->Length);
			poolHandle->PushBack(buffer);
		}
		else{
			// TODO
		}

		ReleaseMutex(pMp->hMutex);
	}

	// 设置采集终止事件，表示采集进程终止
	// SetEvent(pMp->m_RcvTermEvent);

	_endthreadex(0);
	return 0;
}
#endif


bool Sony_Camera::_openCam()
{
	endEvent = CreateEvent(NULL, true, false, NULL);
	rcvTermEvent = CreateEvent(NULL, true, false, NULL);

	XCCAM_SetStructVersion(XCCAM_LIBRARY_STRUCT_VERSION);
	XCCAM_SetCallBack(NULL, SystemFunc);

	// 打开相机
	if (!XCCAM_Open(NULL, &hCamera))
	{
		CloseHandle(endEvent);
		CloseHandle(rcvTermEvent);
		return false;
	}

	// 申请相机的资源？
	if (!XCCAM_ResourceAlloc(hCamera))
	{
		CloseHandle(endEvent);
		CloseHandle(rcvTermEvent);
		XCCAM_Close(hCamera);
		hCamera = 0;
		return false;
	}

	// 为获取的图像申请内存空间
	XCCAM_ImageAlloc(hCamera, &pImage);

	// 获取相机特性句柄
	if (!XCCAM_GetFeatureHandle(hCamera, (HNodeMap*)&hFeature))
	{
		CloseHandle(endEvent);
		CloseHandle(rcvTermEvent);
		XCCAM_Close(hCamera);
		hCamera = 0;
		return false;
	}

	// 获取相机输出的数据类型
	char buffer[100];
	XCCAM_GetFeatureEnumeration(hFeature, "PixelFormat", buffer, 100, FALSE);
	if (0 == strcmp(buffer, "Mono8")){
		m_channels = 1;
		m_bitPerPixel = 8;
		dataType = Mono8;
	}
	else if (0 == strcmp(buffer, "BayerRG8")){
		m_channels = 3;
		m_bitPerPixel = 24;
		dataType = BayerRG8;
	}
	else if (0 == strcmp(buffer, "Mono12Packed")){
		m_channels = 1;
		m_bitPerPixel = 16;
		dataType = Mono12Packed;
	}
	else if (0 == strcmp(buffer, "BayerRG12Packed")){
		m_channels = 3;
		m_bitPerPixel = 48;
		dataType = BayerRG12Packed;
	}

	// 获取图像的宽度和高度
	XCCAM_GetFeatureInteger(hFeature, "Height", &m_height, FALSE);
	XCCAM_GetFeatureInteger(hFeature, "Width", &m_width, FALSE);


	if (dataType == BayerRG8){
		// 设置颜色转换模式
		XCCAM_COLORCONVMODE Mode = {};
		Mode.DIBMode = XCCAM_DIB24;
		Mode.ShiftID = XCCAM_SFTAUTO;
		Mode.Parallel_Thread = 4;
		if (!XCCAM_SetConvMode(hCamera, &Mode, NULL))
		{
			CloseHandle(endEvent);
			CloseHandle(rcvTermEvent);
			XCCAM_Close(hCamera);
			hCamera = 0;
			return false;
		}

		// 为内存池申请空间，用于存放转换后的图像
		imgBufPoolHandle = new Image_Pool_8Bit((int)(m_height*m_width*m_channels));
	}
	else if (dataType == Mono8){
		imgBufPoolHandle = new Image_Pool_8Bit((int)(m_height*m_width*m_channels));
	}
	else if (dataType == BayerRG12Packed){
		imgBufPoolHandle = new Image_Pool_16Bit((int)(m_height*m_width*m_channels));
	}
	else if (dataType == Mono12Packed){
		imgBufPoolHandle = new Image_Pool_16Bit((int)(m_height*m_width*m_channels));
	}
	else{
		return false;
	}

	hMutex = CreateMutex(NULL, FALSE, _T("ImgBufPoolMutex"));

	isStarted = false;
	isOpened = true;

	return true;
}

bool Sony_Camera::_closeCam()
{
	if (isOpened = false){
		return false;
	}

	if (isStarted == true){
#ifdef _IMAGECALLBACK_
		XCCAM_ImageStop(hCamera);
		XCCAM_SetImageCallBack(hCamera, NULL, NULL, 0, FALSE);
#else
		SetEvent(endEvent);
		XCCAM_ImageReqAbortAll(hCamera);
		// WaitForSingleObject(m_RcvTermEvent, INFINITE);
		WaitForSingleObject(hThread, INFINITE);
		// ResetEvent(rcvTermEvent);
		XCCAM_ImageStop(hCamera);
		CloseHandle(hThread);
#endif
	}

	// 释放相关资源
	XCCAM_ResourceRelease(hCamera);
	XCCAM_ImageFreeAll(hCamera);
	XCCAM_Close(hCamera);
	CloseHandle(endEvent);
	CloseHandle(rcvTermEvent);

	// 释放内存池
	if (dataType == BayerRG12Packed || dataType == Mono12Packed){
		Image_Pool_16Bit *temp = (Image_Pool_16Bit *)imgBufPoolHandle;
		delete temp;
	}
	else{
		Image_Pool_8Bit *temp = (Image_Pool_8Bit *)imgBufPoolHandle;
		delete temp;
	}

	return true;
}

bool Sony_Camera::_startAcquisition()
{
	if (isOpened == false){
		cout << "Need open camera first!" << endl;
		return false;
	}
	if (isStarted == true){
		cout << "Acquisition has already been started!" << endl;
		return false;
	}

#ifndef _IMAGECALLBACK_
	XCCAM_ImageStart(hCamera);
	hThread = (HANDLE)_beginthreadex(NULL, 0, &ImageAcquThread, this, CREATE_SUSPENDED, &threadID);
	// SetThreadPriority(hThread, THREAD_PRIORITY_BELOW_NORMAL);
	ResumeThread(hThread);
	Sleep(2);
#else
	XCCAM_SetImageCallBack(hCamera, this, ImageDataRcv, 1, FALSE);
	XCCAM_ImageStart(hCamera);
#endif

	isStarted = true;
	return true;
}


bool Sony_Camera::_getImgBuf(UCHAR **pBuffer, int *pHeight, int *pWidth, int *pChannels)
{
	Image_Pool_8Bit *poolHandle = (Image_Pool_8Bit *)imgBufPoolHandle;

	while (0 == poolHandle->QueueSize()){
		ReleaseMutex(hMutex);
		Sleep(10);
		WaitForSingleObject(hMutex, INFINITE);
	}

	*pBuffer = poolHandle->PopFront();
	*pHeight = m_height;
	*pWidth = m_width;
	*pChannels = m_channels;

	return true;
}

bool Sony_Camera::_getImgBuf(USHORT **pBuffer, int *pHeight, int *pWidth, int *pChannels)
{
	Image_Pool_16Bit *poolHandle = (Image_Pool_16Bit *)imgBufPoolHandle;

	while (0 == poolHandle->QueueSize()){
		ReleaseMutex(hMutex);
		Sleep(10);
		WaitForSingleObject(hMutex, INFINITE);
	}

	*pBuffer = poolHandle->PopFront();
	*pHeight = m_height;
	*pWidth = m_width;
	*pChannels = m_channels;

	return true;
}

bool Sony_Camera::_getImgInfo(int *pHeight, int *pWidth, int *pBitPerPixel){
	*pHeight = m_height;
	*pWidth = m_width;
	*pBitPerPixel = m_bitPerPixel;

	return true;
}
