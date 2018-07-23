
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

#include "SetCameraDlg.h"

#include "SonyCamera_Library.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// #define _IMAGECALLBACK_

#ifdef _IMAGECALLBACK_
extern "C" VOID CALLBACK ImageDataRcv(HCAMERA, XCCAM_IMAGE*, pXCCAM_IMAGEDATAINFO, PVOID);
#endif

extern "C" VOID CALLBACK SystemFunc(STATUS_SYSTEMCODE Status, PVOID Countext);

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
	ON_BN_CLICKED(IDC_BUTTON_SETCAMERA, &CSonyCamera_MFCDlg::OnBnClickedButtonSetcamera)
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
	cameraStage = CAMERA_SHUTDOWN;
	cameraState = CAMERA_CLOSED;
	isBooting = FALSE;

	XCCAM_SetCallBack(this, SystemFunc);

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
	CSonyCamera_MFCDlg *pMp = (CSonyCamera_MFCDlg *)Countext;
	switch (Status)
	{
	case STATUSXCCAM_DEVICECHANGE:				// Processing of Device Change
		// AfxMessageBox(_T("Device Change Event"));
		if (pMp->cameraStage == CAMERA_SHUTDOWN){
			AfxMessageBox(_T("����������"));
			pMp->cameraStage = CAMERA_BOOTED;
			pMp->isBooting = FALSE;
		}
		else{
			AfxMessageBox(_T("����ػ����"));
			pMp->cameraStage = CAMERA_SHUTDOWN;
		}
		break;

	case STATUSXCCAM_POWERUP:					// Processing of PowerUP
		AfxMessageBox(_T("PowerUP Event"));
		break;
	}
}


void CSonyCamera_MFCDlg::OnBnClickedButtonOpencam()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������

	// XCCAM_SetStructVersion(XCCAM_LIBRARY_STRUCT_VERSION);
	
	bool ret;
	ret = OpenCamera();
	if (!ret){
		MessageBox(_T("Open Camera Failed!"));
		return;
	}

	Sony_Camera_Handle sonyHandle = GetCameraHandle();

	hCamera = sonyHandle->hCamera;
	hFeature = sonyHandle->hFeature;

	cameraStage = CAMERA_BOOTED;
	cameraState = CAMERA_OPENED;

	XCCAM_SetCallBack(this, SystemFunc);

	MessageBox(_T("Open Camera Successfully!"));

	return;
}



// �����ȡͼ����̵߳ĺ���
//UINT AFX_CDECL ImageAcquisition(LPVOID Countext)
//{
//	CSonyCamera_MFCDlg *pMp = (CSonyCamera_MFCDlg *)Countext;
//
//	// ����������ʾͼ��Ĵ���
//	cv::namedWindow("figure");
//
//	LARGE_INTEGER li;
//	LONGLONG start, end, freq;
//
//	QueryPerformanceFrequency(&li);
//	freq = li.QuadPart;
//
//	while (1){
//		// �����˳��ɼ�ͼ���¼�����������󵽣����˳��ɼ�����
//		if (WaitForSingleObject(pMp->m_EndEvent, 0) == WAIT_OBJECT_0){
//			cv::destroyWindow("figure");
//			ResetEvent(pMp->m_EndEvent);
//			break;
//		}
//
//		QueryPerformanceCounter(&li);
//		start = li.QuadPart;
//
//		XCCAM_ImageReq(pMp->hCamera, pMp->pImage);						// ��ȡͼ��
//		XCCAM_ImageComplete(pMp->hCamera, pMp->pImage, -1, NULL);		// �ȴ�ͼ���ȡ��ɣ���ʱ��������Ϊ-1�����޵ȴ�
//		
//		QueryPerformanceCounter(&li);
//		end = li.QuadPart;
//		int useTime = (int)((end - start) * 1000 / freq);
//		std::cout << "acqu time: " << useTime << "ms" << std::endl;
//
//		XCCAM_ConvExec(pMp->hCamera, pMp->pImage, pMp->m_pImage);		// ����ȡ����ͼ��ת����������ʾ�ĸ�ʽ
//		cv::imshow("figure", pMp->cv_image);
//		cv::waitKey(20);
//	}
//
//	// ���òɼ���ֹ�¼�����ʾ�ɼ�������ֹ
//	SetEvent(pMp->m_RcvTermEvent);
//	return 0;
//}

UINT AFX_CDECL ImageShow(LPVOID Countext)
{
	CSonyCamera_MFCDlg *pMp = (CSonyCamera_MFCDlg *)Countext;

	cv::Mat img;

	cv::namedWindow("Image", 0);
	cvResizeWindow("Image", 1224, 1024);

	while (1){

		if (WaitForSingleObject(pMp->endShowEvent, 0) == WAIT_OBJECT_0){
			cv::destroyWindow("Image");
			break;
		}

		img = GetImage();
		if (img.empty())
			continue;

		cv::imshow("Image", img);
		cv::waitKey(5);
	}

	SetEvent(pMp->showEndEvent);
	return 0;
}

void CSonyCamera_MFCDlg::OnBnClickedButtonShowimg()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	if (cameraState != CAMERA_OPENED){
		MessageBox(_T("Open Camera First!"));
		return;
	}

	if (isShowingImage == TRUE){
		MessageBox(_T("Image is being showed!"));
		return;
	}

	bool ret = StartImageAcquisition();
	if (!ret){
		MessageBox(_T("Fail to start image acqusition!"));
		return;
	}

	// ������ʾ�̺߳�ͬ����
	endShowEvent = CreateEvent(NULL, true, false, NULL);
	showEndEvent = CreateEvent(NULL, true, false, NULL);
	AfxBeginThread(ImageShow, this);

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

	bool ret;

	SetEvent(endShowEvent);
	WaitForSingleObject(showEndEvent, INFINITE);
	CloseHandle(showEndEvent);
	CloseHandle(endShowEvent);


	ret = StopImageAcquisition();

	isShowingImage = FALSE;
	return;
}


void CSonyCamera_MFCDlg::OnBnClickedButtonClosecam()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	if (isShowingImage == TRUE){
		// ���������ʾͼ����ֹͣ��ʾ
		MessageBox(_T("Please Stop Image Show First!"));
		return;
	}

	// �ر����
	if (cameraState == CAMERA_OPENED)
		CloseCamera();
	cameraState = CAMERA_CLOSED;
	MessageBox(_T("Close Camera Successfully!"));
	return;
}


void CSonyCamera_MFCDlg::OnBnClickedButtonTrigger()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������

	bool ret = TriggerShooting();
	if (!ret){
		MessageBox(_T("Fail to Trigger Shooting!"));
	}

	return;
}



void CSonyCamera_MFCDlg::OnBnClickedButtonSetcamera()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	// CSetCameraDlg setCameraDlg;

	// ������δ�򿪣����ȴ����
	if (cameraState == CAMERA_CLOSED){
		MessageBox(_T("Please Open Camera First"));
		return;
	}

	// ������ɼ�ͼ����ֹͣ�ɼ�
	if (isShowingImage == TRUE){
		MessageBox(_T("Please Stop Acqusition First"));
		return;
	}

	//  �����������ĳЩ����֮ǰ��Ҫ���ͷ������Դ
	XCCAM_ResourceRelease(hCamera);

	if (setCameraDlg.DoModal() == IDOK){

	}

	// ������ɺ��������������Դ
	XCCAM_ResourceAlloc(hCamera);
}
