#include "stdafx.h"
#include "tchar.h"
#include "SequencePool.h"
#include "SonyCamera_Class.h"

#include <iostream>
#include <string>
#include <queue>
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

		// 将Mono12Packed格式（一个像素占用1.5个字节）的图像
		// 转换成Mono格式（一个像素占用两个字节），方便后期处理
		unsigned long length = pMp->pImage->Length;
		unsigned char *packedImgBuf = pMp->pImage->pBuffer;
		for (int i = 0, j = 0; i < length; i += 3, j += 2){
			buffer[j] = packedImgBuf[i] << 8 | ((packedImgBuf[i + 1] << 4) | 0xF0);
			buffer[j + 1] = packedImgBuf[i + 2] << 8 | (packedImgBuf[i + 1] | 0xF0);
		}
		
		poolHandle->PushBack(buffer);
	}
	else{
	}

	ReleaseMutex(pMp->hMutex);
}
#endif


#ifndef _IMAGECALLBACK_
// 定义图像采集线程函数
//unsigned int __stdcall ImageAcquThread(LPVOID Countext)
//{
//	Sony_Camera *pMp = (Sony_Camera *)Countext;
//
//	XCCAM_IMAGE *pImage;
//
//	//LARGE_INTEGER li;
//	//LONGLONG start, end, freq;
//
//	//QueryPerformanceFrequency(&li);
//	//freq = li.QuadPart;
//
//	while (1){
//		// 请求退出采集图像事件句柄，若请求到，则退出采集过程
//		if (WaitForSingleObject(pMp->endEvent, 0) == WAIT_OBJECT_0){
//			ResetEvent(pMp->endEvent);
//			break;
//		}
//
//		//QueryPerformanceCounter(&li);
//		//start = li.QuadPart;
//
//		EnterCriticalSection(&(pMp->hCriticalSection));
//		pImage = pMp->imgBufPoolHandle->ReqBuffer();
//		LeaveCriticalSection(&(pMp->hCriticalSection));
//
//		//XCCAM_ImageReq(pMp->hCamera, pMp->pImage);				
//		//XCCAM_ImageComplete(pMp->hCamera, pMp->pImage, -1, NULL);
//
//		XCCAM_ImageReq(pMp->hCamera, pImage);
//		XCCAM_ImageComplete(pMp->hCamera, pImage, -1, NULL);
//
//		EnterCriticalSection(&(pMp->hCriticalSection));
//		pMp->imgBufPoolHandle->PushBack(pImage);
//		LeaveCriticalSection(&(pMp->hCriticalSection));
//
//		//QueryPerformanceCounter(&li);
//		//end = li.QuadPart;
//		//int useTime = (int)((end - start) * 1000 / freq);
//		//std::cout << "acqu time: " << useTime << "ms" << std::endl;
//	}
//
//	_endthreadex(0);
//	return 0;
//}

// unsigned int __stdcall ImageAcquThread(LPVOID Countext)
unsigned int __stdcall ImageAcquThread(LPVOID Countext)
{
	Sony_Camera *pMp = (Sony_Camera *)Countext;

	XCCAM_IMAGE *pImage_idle;
	XCCAM_IMAGE *pImage_acqusiting;

	std::queue<XCCAM_IMAGE *> acqusitQueue;

	//LARGE_INTEGER li;
	//LONGLONG start, end, freq;
	//QueryPerformanceFrequency(&li);
	//freq = li.QuadPart;

	// 将所有空闲内存段都启动图像采集
	EnterCriticalSection(&(pMp->hCriticalSection));
	for (int i = 0; i < Max_Buffer; i++){
		pImage_idle = pMp->imgBufPoolHandle->ReqBuffer();
		acqusitQueue.push(pImage_idle);
		XCCAM_ImageReq(pMp->hCamera, pImage_idle);
	}
	LeaveCriticalSection(&(pMp->hCriticalSection));

	int i = 0;
	while (1){
		// 请求退出采集图像事件句柄，若请求到，则退出采集过程
		if (WaitForSingleObject(pMp->endEvent, 0) == WAIT_OBJECT_0){
			ResetEvent(pMp->endEvent);
			break;
		}

		//QueryPerformanceCounter(&li);
		//start = li.QuadPart;

		// 申请一块空闲内存段
		EnterCriticalSection(&(pMp->hCriticalSection));
		pImage_idle = pMp->imgBufPoolHandle->ReqBuffer();
		LeaveCriticalSection(&(pMp->hCriticalSection));

		// 取出最先开始采集过程的内存段
		if (acqusitQueue.empty()){
			pImage_acqusiting = NULL;
		}
		else{
			pImage_acqusiting = acqusitQueue.front();
			acqusitQueue.pop();
		}

		if (NULL == pImage_idle && NULL == pImage_acqusiting){
			//Sleep(2);
			continue;
		}

		if (NULL != pImage_idle){
			acqusitQueue.push(pImage_idle);
			XCCAM_ImageReq(pMp->hCamera, pImage_idle);
		}

		if (NULL != pImage_acqusiting){
			// 等待采集完成
			BOOL status;
			status = XCCAM_ImageComplete(pMp->hCamera, pImage_acqusiting, -1, NULL);
			if (TRUE == status){
				// 放到采集完成队列中，供处理线程使用
				EnterCriticalSection(&(pMp->hCriticalSection));
				pMp->imgBufPoolHandle->PushBack(pImage_acqusiting);
				LeaveCriticalSection(&(pMp->hCriticalSection));
				ReleaseSemaphore(pMp->hSemImgValid, 1, NULL);	// 信号量加1
			}
			else{
				// 采集图像失败，将这段内存放到空闲队列中，以备下次使用
				EnterCriticalSection(&(pMp->hCriticalSection));
				pMp->imgBufPoolHandle->RetBuffer(pImage_acqusiting);
				LeaveCriticalSection(&(pMp->hCriticalSection));
			}


			//QueryPerformanceCounter(&li);
			//end = li.QuadPart;
			//int useTime = (int)((end - start) * 1000 / freq);
			//std::cout << "acqu time: " << useTime << "ms" << std::endl;
		}

	}

	_endthreadex(0);
	return 0;
}
#endif


bool Sony_Camera::_openCam()
{
	// endEvent = CreateEvent(NULL, true, false, NULL);
	// rcvTermEvent = CreateEvent(NULL, true, false, NULL);

	XCCAM_SetStructVersion(XCCAM_LIBRARY_STRUCT_VERSION);
	XCCAM_SetCallBack(NULL, SystemFunc);

	// 打开相机
	if (!XCCAM_Open(NULL, &hCamera))
	{
		//CloseHandle(endEvent);
		// CloseHandle(rcvTermEvent);
		return false;
	}

	// 申请相机的资源？
	if (!XCCAM_ResourceAlloc(hCamera))
	{
		//CloseHandle(endEvent);
		// CloseHandle(rcvTermEvent);
		XCCAM_Close(hCamera);
		hCamera = 0;
		return false;
	}

	// 获取相机特性句柄
	if (!XCCAM_GetFeatureHandle(hCamera, (HNodeMap*)&hFeature))
	{
		//CloseHandle(endEvent);
		// CloseHandle(rcvTermEvent);
		XCCAM_Close(hCamera);
		hCamera = 0;
		return false;
	}

	isStarted = false;
	isOpened = true;

	return true;
}

bool Sony_Camera::_closeCam()
{
	if (isOpened == false){
		return false;
	}

	if (isStarted == true){
		_stopAcquisition();
		isStarted = false;
	}

	XCCAM_ResourceRelease(hCamera);
	XCCAM_Close(hCamera);
	isOpened = false;

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

	// 获取相机的图像相关参数
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
			XCCAM_Close(hCamera);
			hCamera = 0;
			return false;
		}
	}

	// 为内存池申请空间，用于存放转换后的图像
	imgBufPoolHandle = new Sequence_Pool<XCCAM_IMAGE>(hCamera);


	endEvent = CreateEvent(NULL, true, false, NULL);
	hSemImgValid = CreateSemaphore(NULL, 0, Max_Buffer, _T("SemImgValid"));
	InitializeCriticalSection(&hCriticalSection);

#ifndef _IMAGECALLBACK_
	XCCAM_ImageStart(hCamera);
	hThread = (HANDLE)_beginthreadex(NULL, 0, &ImageAcquThread, this, CREATE_SUSPENDED, &threadID);
	ResumeThread(hThread);
	Sleep(2);
#else
	XCCAM_SetImageCallBack(hCamera, this, ImageDataRcv, 1, FALSE);
	XCCAM_ImageStart(hCamera);
#endif

	isStarted = true;
	return true;
}

bool Sony_Camera::_stopAcquisition()
{

	if (isStarted == true){
#ifdef _IMAGECALLBACK_
		XCCAM_SetImageCallBack(hCamera, NULL, NULL, 0, FALSE);
		XCCAM_ImageStop(hCamera);
#else
		SetEvent(endEvent);
		XCCAM_ImageReqAbortAll(hCamera);
		WaitForSingleObject(hThread, INFINITE);
		XCCAM_ImageStop(hCamera);
		CloseHandle(hThread);
#endif

		delete imgBufPoolHandle;
		CloseHandle(endEvent);
		CloseHandle(hSemImgValid);
		DeleteCriticalSection(&hCriticalSection);
	}

	isStarted = false;

	return true;
}


/*
 * 函数功能：	从采集完成的图像缓存队列中取出一段缓存，
 *				将缓存的内容拷贝到传入的地址中，然后将缓存放回到空闲队列（空闲内存池）中
 * 
 * 参数：		pBuffer	已经申请好的一段内存，用于存放从缓存中拷贝出的数据，供后续的处理使用
 *				timeOut 超时参数，达到时间后，若仍没有获取到有效的图像，则认为获取图像失败
 *						非负数表示以ms为单位的等待时间，-1表示永远等待
 *				注意：	Windows中，超时参数的类型位unsigned long，其中无穷超时INFINITE对应的
 *						二进制为0xFFFFFFFF,对应signed long的-1。因此，此处直接使用了signed long类型
 *						的超时参数。
 *
 * 返回：		true	获取成功
 *				false	获取失败
 */
bool Sony_Camera::_getImgBuf(UCHAR *pBuffer, signed long timeOut)
{
	XCCAM_IMAGE *pImage = NULL;

	// 等待图像有效
	if (WaitForSingleObject(hSemImgValid, timeOut) != WAIT_OBJECT_0){
		return false;
	}

	// 获取有效的图像缓存
	EnterCriticalSection(&(hCriticalSection));
	pImage = imgBufPoolHandle->PopFront();
	if (pImage == NULL) {
		return false;
	}
	LeaveCriticalSection(&(hCriticalSection));

	// 转换并填充到用户图像内存
	if (dataType == BayerRG8){
		XCCAM_ConvExec(hCamera, pImage, pBuffer);
	}
	else if ( dataType == Mono8){
		memcpy(pBuffer, pImage->pBuffer, pImage->Length);
	}
	else if ( dataType == BayerRG12Packed){
		// TODO:将BayerRG12Packed转成BGR图像
	}
	else if ( dataType == Mono12Packed){
		// 将Mono12Packed格式（一个像素占用1.5个字节）的图像
		// 转换成Mono12格式（一个像素占用两个字节），方便后期处理
		unsigned long length =  pImage->Length;
		unsigned char *packedImgBuf =  pImage->pBuffer;
		USHORT *buffer = (USHORT *)pBuffer;
		for (int i = 0, j = 0; i < length; i += 3, j += 2){
			buffer[j] = packedImgBuf[i] << 8 | ((packedImgBuf[i + 1] << 4) | 0xF0);
			buffer[j + 1] = packedImgBuf[i + 2] << 8 | (packedImgBuf[i + 1] | 0xF0);
		}
	}
	else{
		return false;
	}

	// 将缓冲内存归还给内存池
	EnterCriticalSection(&hCriticalSection);
	imgBufPoolHandle->RetBuffer(pImage);
	LeaveCriticalSection(&hCriticalSection);

	return true;
}

bool Sony_Camera::_getImgInfo(int *pHeight, int *pWidth, int *pBitPerPixel){
	*pHeight = m_height;
	*pWidth = m_width;
	*pBitPerPixel = m_bitPerPixel;

	return true;
}

/*
 * 函数功能：	仅当相机被设置为软件触发模式时，使用该函数触发相机拍照
 *
 */
bool Sony_Camera::_triggerShooting()
{
	if (!isOpened){
		cout << "Error:Camera is not opened!" << endl;
		return false;
	}

	if (!isStarted){
		cout << "Warning: Acqusition is not started!" << endl;
		return false;
	}

	BOOL ret = XCCAM_FeatureCommand(hFeature, "TriggerSoftware");
	if (FALSE == ret){
		cout << "Error:Trigger failed!" << endl;
		return false;
	}

	return true;
}