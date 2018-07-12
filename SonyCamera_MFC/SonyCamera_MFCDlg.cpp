
// SonyCamera_MFCDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "SonyCamera_MFC.h"
#include "SonyCamera_MFCDlg.h"
#include "afxdialogex.h"

#include <iostream>
#include <XCCamAPI.h>
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// #define _IMAGECALLBACK_

#ifdef _IMAGECALLBACK_
extern "C" VOID CALLBACK ImageDataRcv(HCAMERA, XCCAM_IMAGE*, pXCCAM_IMAGEDATAINFO, PVOID);
#endif

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CSonyCamera_MFCDlg �Ի���



CSonyCamera_MFCDlg::CSonyCamera_MFCDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSonyCamera_MFCDlg::IDD, pParent),
	isShowingImage(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSonyCamera_MFCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSonyCamera_MFCDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_OPENCAM, &CSonyCamera_MFCDlg::OnBnClickedButtonOpencam)
	ON_BN_CLICKED(IDC_BUTTON_SHOWIMG, &CSonyCamera_MFCDlg::OnBnClickedButtonShowimg)
	ON_BN_CLICKED(IDC_BUTTON_STOPSHOWIMG, &CSonyCamera_MFCDlg::OnBnClickedButtonStopshowimg)
	ON_BN_CLICKED(IDC_BUTTON_CLOSECAM, &CSonyCamera_MFCDlg::OnBnClickedButtonClosecam)
	ON_BN_CLICKED(IDC_BUTTON_TRIGGER, &CSonyCamera_MFCDlg::OnBnClickedButtonTrigger)
END_MESSAGE_MAP()


// CSonyCamera_MFCDlg ��Ϣ�������

BOOL CSonyCamera_MFCDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO:  �ڴ���Ӷ���ĳ�ʼ������

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CSonyCamera_MFCDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CSonyCamera_MFCDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CSonyCamera_MFCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}




//---------------------------------------------------------------------------
// System Call back
// When bus reset occurred, when the numbers of camera that I can open changed
// on a bus, when system abnormality occurred, it is called.
extern "C" VOID CALLBACK SystemFunc(STATUS_SYSTEMCODE Status, PVOID Countext)
{
	switch (Status)
	{
	case STATUSXCCAM_DEVICECHANGE:				// Processing of Device Change
		AfxMessageBox(_T("Device Change Event"));
		break;

	case STATUSXCCAM_POWERUP:					// Processing of PowerUP
		AfxMessageBox(_T("PowerUP Event"));
		break;
	}
}


void CSonyCamera_MFCDlg::OnBnClickedButtonOpencam()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	m_EndEvent = CreateEvent(NULL, true, false, NULL);
	m_RcvTermEvent = CreateEvent(NULL, true, false, NULL);

	XCCAM_SetStructVersion(XCCAM_LIBRARY_STRUCT_VERSION);
	XCCAM_SetCallBack(NULL, SystemFunc);

	// �����
	if (!XCCAM_Open(NULL, &hCamera))
	{
		MessageBox(_T("Camera Not found"));
		CloseHandle(m_EndEvent);
		CloseHandle(m_RcvTermEvent);
		return;
	}

	// ��ȡ�����Ϣ
	// Get the camera model name
	XCCAM_DEVINFO		cinfo;
	if (!XCCAM_CameraInfo(hCamera, &cinfo))
	{
		MessageBox(_T("Get Camera Info Error"));
		CloseHandle(m_EndEvent);
		CloseHandle(m_RcvTermEvent);
		XCCAM_Close(hCamera);
		hCamera = 0;
		return;
	}
	// CString		string;
	switch (cinfo.DeviceType)
	{
	case XCCAM_GIGECAMERA:
		cout << "Free Run Sample   Model=" << cinfo.u.GigEDev.ModelName << cinfo.UID << endl;
		// string.Format(_T("Free Run Sample   Model=%S 0x%016I64X"), cinfo.u.GigEDev.ModelName, cinfo.UID);
		break;

	case XCCAM_USBCAMERA:
		cout << "Free Run Sample   Model=" << cinfo.u.UsbDev.ModelName << cinfo.UID << endl;
		// string.Format(_T("Free Run Sample   Model=%S 0x%016I64X"), cinfo.u.UsbDev.ModelName, cinfo.UID);
		break;
	}

	// ��ȡ������Ծ��
	
	if (!XCCAM_GetFeatureHandle(hCamera, (HNodeMap*)&m_hFeature))
	{
		MessageBox(_T("Get FeatureHandle Error"));
		CloseHandle(m_EndEvent);
		CloseHandle(m_RcvTermEvent);
		XCCAM_Close(hCamera);
		hCamera = 0;
		return;
	}

	// ��������ɼ�ģʽ
	if (!XCCAM_SetFeatureEnumeration(m_hFeature, "AcquisitionMode", "Continuous"))
	{
		MessageBox(_T("Set Feature Enumeration Error"));
		CloseHandle(m_EndEvent);
		CloseHandle(m_RcvTermEvent);
		XCCAM_Close(hCamera);
		hCamera = 0;
		return;
	}

	BOOL ret = FALSE;
	//char buffer[100];

	//XCCAM_FEATUREINFO TriggerModeInfo, TriggerSourceInfo, TriggerSoftwareInfo, \
	//	TriggerDelayInfo, GainAutoInfo, ExposureAutoInfo, ExposureTimeInfo, GainRawInfo;

	//ret = XCCAM_FeatureInfo(m_hFeature, "TriggerMode", &TriggerModeInfo);
	//ret = XCCAM_FeatureInfo(m_hFeature, "TriggerSource", &TriggerSourceInfo);
	//ret = XCCAM_FeatureInfo(m_hFeature, "TriggerSoftware", &TriggerSoftwareInfo);
	//ret = XCCAM_FeatureInfo(m_hFeature, "TriggerDelay", &TriggerDelayInfo);
	//ret = XCCAM_FeatureInfo(m_hFeature, "GainAuto", &GainAutoInfo);
	//ret = XCCAM_FeatureInfo(m_hFeature, "ExposureAuto", &ExposureAutoInfo);
	//ret = XCCAM_FeatureInfo(m_hFeature, "ExposureTime", &ExposureTimeInfo);
	//ret = XCCAM_FeatureInfo(m_hFeature, "GainRaw", &GainRawInfo);


	//// ��������Ĵ���ģʽ�ʹ����ӳ�
	//ret = XCCAM_SetFeatureEnumeration(m_hFeature, "TriggerMode", "On");
	//ret = XCCAM_SetFeatureEnumeration(m_hFeature, "TriggerSource", "Software");
	//ret = XCCAM_SetFeatureFloat(m_hFeature, "TriggerDelay", 0);	// 2s

	//// ����������ع�
	//ret = XCCAM_SetFeatureEnumeration(m_hFeature, "GainAuto", "Off");
	//ret = XCCAM_SetFeatureInteger(m_hFeature, "GainRaw", 100);	// -38 ~ 442
	//ret = XCCAM_SetFeatureEnumeration(m_hFeature, "ExposureAuto", "Off");
	//ret = XCCAM_SetFeatureFloat(m_hFeature, "ExposureTime", 60000.0);

	//// �����������������
	//INT64 WidthSize, HeightSize, OffSetX, OffSetY;

	//WidthSize = 640;
	//HeightSize = 480;
	//OffSetX = 0;
	//OffSetY = 0;

	//XCCAM_SetFeatureInteger(m_hFeature, "OffsetX", OffSetX);
	//XCCAM_SetFeatureInteger(m_hFeature, "OffsetY", OffSetY);

	//XCCAM_FEATUREINFO WInfo, HInfo, OXInfo, OYInfo;

	//XCCAM_FeatureInfo(m_hFeature, "Width", &WInfo);
	//XCCAM_FeatureInfo(m_hFeature, "Height", &HInfo);
	//XCCAM_FeatureInfo(m_hFeature, "OffsetX", &OXInfo);
	//XCCAM_FeatureInfo(m_hFeature, "OffsetY", &OYInfo);

	//if (WInfo.u.IntReg.MaxValue < WidthSize)
	//	WidthSize = WInfo.u.IntReg.MaxValue;
	//OffSetX = (WInfo.u.IntReg.MaxValue - WidthSize) / 2;
	//if (HInfo.u.IntReg.MaxValue < HeightSize)
	//	HeightSize = HInfo.u.IntReg.MaxValue;
	//OffSetY = (HInfo.u.IntReg.MaxValue - HeightSize) / 2;

	//XCCAM_SetFeatureInteger(m_hFeature, "Width", WidthSize);
	//XCCAM_SetFeatureInteger(m_hFeature, "Height", HeightSize);
	//XCCAM_SetFeatureInteger(m_hFeature, "OffsetX", OffSetX);
	//XCCAM_SetFeatureInteger(m_hFeature, "OffsetY", OffSetY);

	// ������ɫת��ģʽ?
	XCCAM_COLORCONVMODE Mode = {};
	Mode.StoreMode = XCCAM_MEMmode;
	Mode.DIBMode = XCCAM_DIB32;
	Mode.ShiftID = XCCAM_SFTAUTO;
	Mode.Parallel_Thread = 4;
	Mode.BayerRevision_G = FALSE;
	if (!XCCAM_SetConvMode(hCamera, &Mode, NULL))
	{
		MessageBox(_T("Conver Mode Set Error"));
		CloseHandle(m_EndEvent);
		CloseHandle(m_RcvTermEvent);
		XCCAM_Close(hCamera);
		hCamera = 0;
		return;
	}

	// �����������Դ��
	if (!XCCAM_ResourceAlloc(hCamera))
	{
		MessageBox(_T("Resource Alloc Error"));
		CloseHandle(m_EndEvent);
		CloseHandle(m_RcvTermEvent);
		::XCCAM_Close(hCamera);
		hCamera = 0;
		return;
	}

	// Ϊ��ȡ��ͼ�������ڴ�ռ�
	XCCAM_ImageAlloc(hCamera, &pImage);

	// ��ȡBMPͼ����Ϣ
	ULONG Len = 0;
	if (!XCCAM_GetBMPINFO(hCamera, NULL, &Len, false))
	{
		MessageBox(_T("Get BMPINFO Error"));
		XCCAM_ResourceRelease(hCamera);
		XCCAM_ImageFreeAll(hCamera);
		XCCAM_Close(hCamera);
		CloseHandle(m_EndEvent);
		CloseHandle(m_RcvTermEvent);
		hCamera = 0;
		return;
	}
	m_pBitInfo = (PBITMAPINFO)new BYTE[Len];
	if (!XCCAM_GetBMPINFO(hCamera, m_pBitInfo, &Len, false))
	{
		MessageBox(_T("Get BMPINFO Error"));
		XCCAM_ResourceRelease(hCamera);
		XCCAM_ImageFreeAll(hCamera);
		XCCAM_Close(hCamera);
		CloseHandle(m_EndEvent);
		CloseHandle(m_RcvTermEvent);
		hCamera = 0;
		return;
	}

	// Ϊת�����ͼ�������ڴ�ռ�
	int height = m_pBitInfo->bmiHeader.biHeight;
	int width = m_pBitInfo->bmiHeader.biWidth;
	int channels = m_pBitInfo->bmiHeader.biBitCount / 8;
	cv_image.create(height, width, CV_8UC4);
	m_pImage = NULL;
	if (cv_image.isContinuous()) {
		m_pImage = cv_image.data;
	}
	else {
		MessageBox(_T("opencv mat is not continuous!"));
		XCCAM_ResourceRelease(hCamera);
		XCCAM_ImageFreeAll(hCamera);
		delete[] m_pBitInfo;
		XCCAM_Close(hCamera);
		CloseHandle(m_EndEvent);
		CloseHandle(m_RcvTermEvent);
		hCamera = 0;
		return;
	}

	MessageBox(_T("Camera Open Successfully!"));

	return;
}


// ����ͼ��ص�����
#ifdef _IMAGECALLBACK_
extern "C"
VOID  CALLBACK ImageDataRcv(HCAMERA hCamera, 
							XCCAM_IMAGE* pImage, 
							pXCCAM_IMAGEDATAINFO pImageInfo,
							PVOID Context )
{
	CSonyCamera_MFCDlg* pMp = (CSonyCamera_MFCDlg*)Context;
	/* Color-converts the acquired image data with the color conversion API 
	and convert it to data for display. */
	XCCAM_ConvExec(hCamera, pImage, pMp->m_pImage);
	cv::imshow("figure", pMp->cv_image);
	cv::waitKey(20);
}
#endif

// �����ȡͼ����̵߳ĺ���
UINT AFX_CDECL ImageAcquisition(LPVOID Countext)
{
	CSonyCamera_MFCDlg *pMp = (CSonyCamera_MFCDlg *)Countext;

	// ����������ʾͼ��Ĵ���
	cv::namedWindow("figure");

	LARGE_INTEGER li;
	LONGLONG start, end, freq;

	QueryPerformanceFrequency(&li);
	freq = li.QuadPart;

	while (1){
		// �����˳��ɼ�ͼ���¼�����������󵽣����˳��ɼ�����
		if (WaitForSingleObject(pMp->m_EndEvent, 0) == WAIT_OBJECT_0){
			cv::destroyWindow("figure");
			ResetEvent(pMp->m_EndEvent);
			break;
		}

		QueryPerformanceCounter(&li);
		start = li.QuadPart;

		XCCAM_ImageReq(pMp->hCamera, pMp->pImage);						// ��ȡͼ��
		XCCAM_ImageComplete(pMp->hCamera, pMp->pImage, -1, NULL);		// �ȴ�ͼ���ȡ��ɣ���ʱ��������Ϊ-1�����޵ȴ�
		
		QueryPerformanceCounter(&li);
		end = li.QuadPart;
		int useTime = (int)((end - start) * 1000 / freq);
		std::cout << "acqu time: " << useTime << "ms" << std::endl;

		XCCAM_ConvExec(pMp->hCamera, pMp->pImage, pMp->m_pImage);		// ����ȡ����ͼ��ת����������ʾ�ĸ�ʽ
		cv::imshow("figure", pMp->cv_image);
		cv::waitKey(20);
	}

	// ���òɼ���ֹ�¼�����ʾ�ɼ�������ֹ
	SetEvent(pMp->m_RcvTermEvent);
	return 0;
}

void CSonyCamera_MFCDlg::OnBnClickedButtonShowimg()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	if (isShowingImage == TRUE){
		MessageBox(_T("Image is being showed!"));
		return;
	}

#ifndef _IMAGECALLBACK_
	// ��ʼͼ��ɼ�
	XCCAM_ImageStart(hCamera);

	// �����ɼ���ʾ�߳�
	AfxBeginThread(ImageAcquisition, this);

#else
	cv::namedWindow("figure");
	XCCAM_SetImageCallBack(hCamera, this, ImageDataRcv, 1, FALSE);
	XCCAM_ImageStart(hCamera);
#endif

	isShowingImage = TRUE;
	return;
}


void CSonyCamera_MFCDlg::OnBnClickedButtonStopshowimg()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	if (isShowingImage == FALSE){
		MessageBox(_T("Image is not being showed!"));
		return;
	}

#ifdef _IMAGECALLBACK_
	XCCAM_ImageStop(hCamera);
	cv::destroyWindow("figure");
	XCCAM_SetImageCallBack(hCamera, NULL, NULL, 0, FALSE);
#else
	SetEvent(m_EndEvent);
	XCCAM_ImageReqAbortAll(hCamera);
	WaitForSingleObject(m_RcvTermEvent, INFINITE);
	ResetEvent(m_RcvTermEvent);
	XCCAM_ImageStop(hCamera);
#endif

	isShowingImage = FALSE;
	return;
}


void CSonyCamera_MFCDlg::OnBnClickedButtonClosecam()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	if (isShowingImage == TRUE){
		// ���������ʾͼ����ֹͣ��ʾ
#ifdef _IMAGECALLBACK_
		XCCAM_ImageStop(hCamera);
		cv::destroyWindow("figure");
		XCCAM_SetImageCallBack(hCamera, NULL, NULL, 0, FALSE);
#else
		SetEvent(m_EndEvent);
		XCCAM_ImageReqAbortAll(hCamera);
		WaitForSingleObject(m_RcvTermEvent, INFINITE);
		ResetEvent(m_RcvTermEvent);
		XCCAM_ImageStop(hCamera);
#endif
	}

	// �ͷ������Դ
	XCCAM_ResourceRelease(hCamera);
	XCCAM_ImageFreeAll(hCamera);
	delete[] m_pBitInfo;
	XCCAM_Close(hCamera);
	CloseHandle(m_EndEvent);
	CloseHandle(m_RcvTermEvent);

	isShowingImage = FALSE;
	return;
}


void CSonyCamera_MFCDlg::OnBnClickedButtonTrigger()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������

	XCCAM_FeatureCommand(m_hFeature, "TriggerSoftware");

	return;
}
