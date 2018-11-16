#pragma once

// #include "stdafx.h"
#include <XCCamAPI.h>

#include "SequencePool.h"

typedef void *HANDLE;

// 相机类型和输出数据格式
enum DATA_TYPE{
	Mono8,
	BayerRG8,
	Mono12Packed,
	BayerRG12Packed
};

class Sony_Camera
{
public:
	Sony_Camera(){};
	~Sony_Camera(){};

	HCAMERA			hCamera;			// 相机句柄
	XCCAM_IMAGE		*pImage;			// 相机图像句柄
	HFEATURE		*hFeature;			// 相机特性句柄
	PBITMAPINFO		m_pBitInfo;			// BMP图像属性，相机的API做图像转换时使用

private:
	HANDLE			endEvent;			// 开始退出采集过程事件
	DATA_TYPE		dataType;
	HANDLE			hThread;
	unsigned int	threadID;

	// 图像缓冲区
	// void *imgBufPoolHandle;
	Sequence_Pool<XCCAM_IMAGE> *imgBufPoolHandle;

	HANDLE hMutex;
	HANDLE hSemImgValid;
	CRITICAL_SECTION hCriticalSection;

private:
	// 图像的属性
	INT64 m_height;
	INT64 m_width;
	int m_channels;
	int m_bitPerPixel;
	
	bool isStarted;
	bool isOpened;

	

public:
	friend unsigned int __stdcall ImageAcquThread(LPVOID Countext);

	bool	_openCam();
	bool	_closeCam();
	bool	_startAcquisition();
	bool	_stopAcquisition();
	bool	_getImgBuf(UCHAR *pBuffer, signed long timeOut = 250);
	bool	_getImgInfo(int *pHeight, int *pWidth, int *pBitPerPixel);
	bool	_triggerShooting();
};

typedef Sony_Camera* Sony_Camera_Handle;

