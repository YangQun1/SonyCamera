#pragma once

#include "stdafx.h"
#include <XCCamAPI.h>

#include "SequencePool.h"

// ������ͺ�������ݸ�ʽ
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

	HANDLE			endEvent;			// ��ʼ�˳��ɼ������¼�
	// HANDLE			rcvTermEvent;		// �ɼ��߳��Ѿ��˳��¼�
	HCAMERA			hCamera;			// ������
	XCCAM_IMAGE		*pImage;			// ���ͼ����
	HFEATURE		*hFeature;			// ������Ծ��
	PBITMAPINFO		m_pBitInfo;			// BMPͼ�����ԣ������API��ͼ��ת��ʱʹ��
	DATA_TYPE		dataType;
	HANDLE			hThread;
	unsigned int	threadID;

	// ͼ�񻺳���
	// void *imgBufPoolHandle;
	Sequence_Pool<XCCAM_IMAGE> *imgBufPoolHandle;

	HANDLE hMutex;
	HANDLE hSemImgValid;
	CRITICAL_SECTION hCriticalSection;

private:
	// ͼ�������
	INT64 m_height;
	INT64 m_width;
	int m_channels;
	int m_bitPerPixel;
	
	bool isStarted;
	bool isOpened;

public:
	bool	_openCam();
	bool	_closeCam();
	bool	_startAcquisition();
	bool	_stopAcquisition();
	bool	_getImgBuf(UCHAR *pBuffer, signed long timeOut = 250);
	bool	_getImgInfo(int *pHeight, int *pWidth, int *pBitPerPixel);
	bool	_triggerShooting();
};

typedef Sony_Camera* Sony_Camera_Handle;

