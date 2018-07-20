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

// ����ͼ��ص�����
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
		// ʹ�����APIתΪ24bit��BMP��ͼ
		XCCAM_ConvExec(pMp->hCamera, pImage, buffer);
		poolHandle->PushBack(buffer);
	}
	else if (pMp->dataType == Mono8){
		Image_Pool_8Bit *poolHandle = (Image_Pool_8Bit *)(pMp->imgBufPoolHandle);
		UCHAR *buffer = poolHandle->ReqBuffer();
		// ֱ�ӿ����ڴ�
		memcpy(buffer, pImage->pBuffer, pImage->Length);
		poolHandle->PushBack(buffer);
	}
	else if (pMp->dataType == BayerRG12Packed){
		Image_Pool_16Bit *poolHandle = (Image_Pool_16Bit *)(pMp->imgBufPoolHandle);
		USHORT *buffer = poolHandle->ReqBuffer();
		// ��12λ��bayerתΪ12λ��ͨ����rgbͼ��
		// memcpy(buffer, pImage->pBuffer, pImage->Length);
		// TODO
		poolHandle->PushBack(buffer);
	}
	else if (pMp->dataType == Mono12Packed){
		Image_Pool_16Bit *poolHandle = (Image_Pool_16Bit *)(pMp->imgBufPoolHandle);
		USHORT *buffer = poolHandle->ReqBuffer();

		// ��Mono12Packed��ʽ��һ������ռ��1.5���ֽڣ���ͼ��
		// ת����Mono��ʽ��һ������ռ�������ֽڣ���������ڴ���
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
// ����ͼ��ɼ��̺߳���
unsigned int __stdcall ImageAcquThread(LPVOID Countext)
{
	Sony_Camera *pMp = (Sony_Camera *)Countext;

	XCCAM_IMAGE *pImage;

	//LARGE_INTEGER li;
	//LONGLONG start, end, freq;

	//QueryPerformanceFrequency(&li);
	//freq = li.QuadPart;

	while (1){
		// �����˳��ɼ�ͼ���¼�����������󵽣����˳��ɼ�����
		if (WaitForSingleObject(pMp->endEvent, 0) == WAIT_OBJECT_0){
			ResetEvent(pMp->endEvent);
			break;
		}

		//QueryPerformanceCounter(&li);
		//start = li.QuadPart;

		EnterCriticalSection(&(pMp->hCriticalSection));
		pImage = pMp->imgBufPoolHandle->ReqBuffer();
		LeaveCriticalSection(&(pMp->hCriticalSection));

		XCCAM_ImageReq(pMp->hCamera, pImage);
		XCCAM_ImageComplete(pMp->hCamera, pImage, -1, NULL);

		EnterCriticalSection(&(pMp->hCriticalSection));
		pMp->imgBufPoolHandle->PushBack(pImage);
		LeaveCriticalSection(&(pMp->hCriticalSection));

		//QueryPerformanceCounter(&li);
		//end = li.QuadPart;
		//int useTime = (int)((end - start) * 1000 / freq);
		//std::cout << "acqu time: " << useTime << "ms" << std::endl;
	}

	_endthreadex(0);
	return 0;
}
#endif


bool Sony_Camera::_openCam()
{
	endEvent = CreateEvent(NULL, true, false, NULL);
	// rcvTermEvent = CreateEvent(NULL, true, false, NULL);

	XCCAM_SetStructVersion(XCCAM_LIBRARY_STRUCT_VERSION);
	XCCAM_SetCallBack(NULL, SystemFunc);

	// �����
	if (!XCCAM_Open(NULL, &hCamera))
	{
		CloseHandle(endEvent);
		// CloseHandle(rcvTermEvent);
		return false;
	}

	// �����������Դ��
	if (!XCCAM_ResourceAlloc(hCamera))
	{
		CloseHandle(endEvent);
		// CloseHandle(rcvTermEvent);
		XCCAM_Close(hCamera);
		hCamera = 0;
		return false;
	}

	// ��ȡ������Ծ��
	if (!XCCAM_GetFeatureHandle(hCamera, (HNodeMap*)&hFeature))
	{
		CloseHandle(endEvent);
		// CloseHandle(rcvTermEvent);
		XCCAM_Close(hCamera);
		hCamera = 0;
		return false;
	}

	// ��ȡ����������������
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

	// ��ȡͼ��Ŀ�Ⱥ͸߶�
	XCCAM_GetFeatureInteger(hFeature, "Height", &m_height, FALSE);
	XCCAM_GetFeatureInteger(hFeature, "Width", &m_width, FALSE);

	if (dataType == BayerRG8){
		// ������ɫת��ģʽ
		XCCAM_COLORCONVMODE Mode = {};
		Mode.DIBMode = XCCAM_DIB24;
		Mode.ShiftID = XCCAM_SFTAUTO;
		Mode.Parallel_Thread = 4;
		if (!XCCAM_SetConvMode(hCamera, &Mode, NULL))
		{
			CloseHandle(endEvent);
			// CloseHandle(rcvTermEvent);
			XCCAM_Close(hCamera);
			hCamera = 0;
			return false;
		}
	}

	// Ϊ�ڴ������ռ䣬���ڴ��ת�����ͼ��
	imgBufPoolHandle = new Sequence_Pool<XCCAM_IMAGE>(hCamera);


	hMutex = CreateMutex(NULL, FALSE, _T("ImgBufPoolMutex"));
	InitializeCriticalSection(&hCriticalSection);

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
#ifdef _IMAGECALLBACK_
		XCCAM_SetImageCallBack(hCamera, NULL, NULL, 0, FALSE);
		XCCAM_ImageStop(hCamera);
#else
		SetEvent(endEvent);
		WaitForSingleObject(hThread, INFINITE);
		XCCAM_ImageReqAbortAll(hCamera);
		XCCAM_ImageStop(hCamera);
		CloseHandle(hThread);
#endif
	}

	// �ͷ������Դ
	delete imgBufPoolHandle;
	XCCAM_ResourceRelease(hCamera);
	XCCAM_Close(hCamera);
	CloseHandle(endEvent);

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
	ResumeThread(hThread);
	Sleep(2);
#else
	XCCAM_SetImageCallBack(hCamera, this, ImageDataRcv, 1, FALSE);
	XCCAM_ImageStart(hCamera);
#endif

	isStarted = true;
	return true;
}


bool Sony_Camera::_getImgBuf(UCHAR *pBuffer)
{
	XCCAM_IMAGE *pImage = NULL;

	// �Ӷ�����ȡ��һ���Ѿ����õĻ����ڴ�
	while (1){
		EnterCriticalSection(&(hCriticalSection));
		if (pImage = imgBufPoolHandle->PopFront()){
			// pImage = imgBufPoolHandle->PopFront();
			LeaveCriticalSection(&(hCriticalSection));
			break;
		}
		LeaveCriticalSection(&(hCriticalSection));
		Sleep(1);
	}

	// ת������䵽�û�ͼ���ڴ�
	if (dataType == BayerRG8){
		XCCAM_ConvExec(hCamera, pImage, pBuffer);
	}
	else if ( dataType == Mono8){
		memcpy(pBuffer, pImage->pBuffer, pImage->Length);
	}
	else if ( dataType == BayerRG12Packed){
		// TODO:��BayerRG12Packedת��BGRͼ��
	}
	else if ( dataType == Mono12Packed){
		// ��Mono12Packed��ʽ��һ������ռ��1.5���ֽڣ���ͼ��
		// ת����Mono12��ʽ��һ������ռ�������ֽڣ���������ڴ���
		unsigned long length =  pImage->Length;
		unsigned char *packedImgBuf =  pImage->pBuffer;
		USHORT *buffer = (USHORT *)pBuffer;
		for (int i = 0, j = 0; i < length; i += 3, j += 2){
			buffer[j] = packedImgBuf[i] << 8 | ((packedImgBuf[i + 1] << 4) | 0xF0);
			buffer[j + 1] = packedImgBuf[i + 2] << 8 | (packedImgBuf[i + 1] | 0xF0);
		}
	}
	else{
		// TODO
	}

	// �������ڴ�黹���ڴ��
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
