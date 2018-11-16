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
//		// �����˳��ɼ�ͼ���¼�����������󵽣����˳��ɼ�����
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

	// �����п����ڴ�ζ�����ͼ��ɼ�
	EnterCriticalSection(&(pMp->hCriticalSection));
	for (int i = 0; i < Max_Buffer; i++){
		pImage_idle = pMp->imgBufPoolHandle->ReqBuffer();
		acqusitQueue.push(pImage_idle);
		XCCAM_ImageReq(pMp->hCamera, pImage_idle);
	}
	LeaveCriticalSection(&(pMp->hCriticalSection));

	int i = 0;
	while (1){
		// �����˳��ɼ�ͼ���¼�����������󵽣����˳��ɼ�����
		if (WaitForSingleObject(pMp->endEvent, 0) == WAIT_OBJECT_0){
			ResetEvent(pMp->endEvent);
			break;
		}

		//QueryPerformanceCounter(&li);
		//start = li.QuadPart;

		// ����һ������ڴ��
		EnterCriticalSection(&(pMp->hCriticalSection));
		pImage_idle = pMp->imgBufPoolHandle->ReqBuffer();
		LeaveCriticalSection(&(pMp->hCriticalSection));

		// ȡ�����ȿ�ʼ�ɼ����̵��ڴ��
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
			// �ȴ��ɼ����
			BOOL status;
			status = XCCAM_ImageComplete(pMp->hCamera, pImage_acqusiting, -1, NULL);
			if (TRUE == status){
				// �ŵ��ɼ���ɶ����У��������߳�ʹ��
				EnterCriticalSection(&(pMp->hCriticalSection));
				pMp->imgBufPoolHandle->PushBack(pImage_acqusiting);
				LeaveCriticalSection(&(pMp->hCriticalSection));
				ReleaseSemaphore(pMp->hSemImgValid, 1, NULL);	// �ź�����1
			}
			else{
				// �ɼ�ͼ��ʧ�ܣ�������ڴ�ŵ����ж����У��Ա��´�ʹ��
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

	// �����
	if (!XCCAM_Open(NULL, &hCamera))
	{
		//CloseHandle(endEvent);
		// CloseHandle(rcvTermEvent);
		return false;
	}

	// �����������Դ��
	if (!XCCAM_ResourceAlloc(hCamera))
	{
		//CloseHandle(endEvent);
		// CloseHandle(rcvTermEvent);
		XCCAM_Close(hCamera);
		hCamera = 0;
		return false;
	}

	// ��ȡ������Ծ��
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

	// ��ȡ�����ͼ����ز���
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
			XCCAM_Close(hCamera);
			hCamera = 0;
			return false;
		}
	}

	// Ϊ�ڴ������ռ䣬���ڴ��ת�����ͼ��
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
 * �������ܣ�	�Ӳɼ���ɵ�ͼ�񻺴������ȡ��һ�λ��棬
 *				����������ݿ���������ĵ�ַ�У�Ȼ�󽫻���Żص����ж��У������ڴ�أ���
 * 
 * ������		pBuffer	�Ѿ�����õ�һ���ڴ棬���ڴ�Ŵӻ����п����������ݣ��������Ĵ���ʹ��
 *				timeOut ��ʱ�������ﵽʱ�������û�л�ȡ����Ч��ͼ������Ϊ��ȡͼ��ʧ��
 *						�Ǹ�����ʾ��msΪ��λ�ĵȴ�ʱ�䣬-1��ʾ��Զ�ȴ�
 *				ע�⣺	Windows�У���ʱ����������λunsigned long���������ʱINFINITE��Ӧ��
 *						������Ϊ0xFFFFFFFF,��Ӧsigned long��-1����ˣ��˴�ֱ��ʹ����signed long����
 *						�ĳ�ʱ������
 *
 * ���أ�		true	��ȡ�ɹ�
 *				false	��ȡʧ��
 */
bool Sony_Camera::_getImgBuf(UCHAR *pBuffer, signed long timeOut)
{
	XCCAM_IMAGE *pImage = NULL;

	// �ȴ�ͼ����Ч
	if (WaitForSingleObject(hSemImgValid, timeOut) != WAIT_OBJECT_0){
		return false;
	}

	// ��ȡ��Ч��ͼ�񻺴�
	EnterCriticalSection(&(hCriticalSection));
	pImage = imgBufPoolHandle->PopFront();
	if (pImage == NULL) {
		return false;
	}
	LeaveCriticalSection(&(hCriticalSection));

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
		return false;
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

/*
 * �������ܣ�	�������������Ϊ�������ģʽʱ��ʹ�øú��������������
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