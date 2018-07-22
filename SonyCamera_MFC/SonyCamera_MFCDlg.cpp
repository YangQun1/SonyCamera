
// SonyCamera_MFCDlg.cpp : 实现文件
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

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CSonyCamera_MFCDlg 对话框



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


// CSonyCamera_MFCDlg 消息处理程序

BOOL CSonyCamera_MFCDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码
	cameraStage = CAMERA_SHUTDOWN;
	cameraState = CAMERA_CLOSED;
	isBooting = FALSE;

	XCCAM_SetCallBack(this, SystemFunc);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CSonyCamera_MFCDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
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
			AfxMessageBox(_T("相机开机完成"));
			pMp->cameraStage = CAMERA_BOOTED;
			pMp->isBooting = FALSE;
		}
		else{
			AfxMessageBox(_T("相机关机完成"));
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
	// TODO:  在此添加控件通知处理程序代码

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



// 定义获取图像的线程的函数
//UINT AFX_CDECL ImageAcquisition(LPVOID Countext)
//{
//	CSonyCamera_MFCDlg *pMp = (CSonyCamera_MFCDlg *)Countext;
//
//	// 创建用于显示图像的窗口
//	cv::namedWindow("figure");
//
//	LARGE_INTEGER li;
//	LONGLONG start, end, freq;
//
//	QueryPerformanceFrequency(&li);
//	freq = li.QuadPart;
//
//	while (1){
//		// 请求退出采集图像事件句柄，若请求到，则退出采集过程
//		if (WaitForSingleObject(pMp->m_EndEvent, 0) == WAIT_OBJECT_0){
//			cv::destroyWindow("figure");
//			ResetEvent(pMp->m_EndEvent);
//			break;
//		}
//
//		QueryPerformanceCounter(&li);
//		start = li.QuadPart;
//
//		XCCAM_ImageReq(pMp->hCamera, pMp->pImage);						// 获取图像
//		XCCAM_ImageComplete(pMp->hCamera, pMp->pImage, -1, NULL);		// 等待图像获取完成，超时参数设置为-1，无限等待
//		
//		QueryPerformanceCounter(&li);
//		end = li.QuadPart;
//		int useTime = (int)((end - start) * 1000 / freq);
//		std::cout << "acqu time: " << useTime << "ms" << std::endl;
//
//		XCCAM_ConvExec(pMp->hCamera, pMp->pImage, pMp->m_pImage);		// 将获取到的图像转换成用于显示的格式
//		cv::imshow("figure", pMp->cv_image);
//		cv::waitKey(20);
//	}
//
//	// 设置采集终止事件，表示采集进程终止
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
	// TODO:  在此添加控件通知处理程序代码
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

	// 创建显示线程和同步量
	endShowEvent = CreateEvent(NULL, true, false, NULL);
	showEndEvent = CreateEvent(NULL, true, false, NULL);
	AfxBeginThread(ImageShow, this);

	isShowingImage = TRUE;
	return;
}


void CSonyCamera_MFCDlg::OnBnClickedButtonStopshowimg()
{
	// TODO:  在此添加控件通知处理程序代码
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
	// TODO:  在此添加控件通知处理程序代码
	if (isShowingImage == TRUE){
		// 如果正在显示图像，先停止显示
		MessageBox(_T("Please Stop Image Show First!"));
		return;
	}

	// 关闭相机
	if (cameraState == CAMERA_OPENED)
		CloseCamera();
	cameraState = CAMERA_CLOSED;
	MessageBox(_T("Close Camera Successfully!"));
	return;
}


void CSonyCamera_MFCDlg::OnBnClickedButtonTrigger()
{
	// TODO:  在此添加控件通知处理程序代码

	bool ret = TriggerShooting();
	if (!ret){
		MessageBox(_T("Fail to Trigger Shooting!"));
	}

	return;
}



void CSonyCamera_MFCDlg::OnBnClickedButtonSetcamera()
{
	// TODO:  在此添加控件通知处理程序代码
	// CSetCameraDlg setCameraDlg;

	// 如果相机未打开，则先打开相机
	if (cameraState == CAMERA_CLOSED){
		MessageBox(_T("Please Open Camera First"));
		return;
	}

	// 如果正采集图像，则停止采集
	if (isShowingImage == TRUE){
		MessageBox(_T("Please Stop Acqusition First"));
		return;
	}

	//  在配置相机的某些参数之前需要先释放相机资源
	XCCAM_ResourceRelease(hCamera);

	if (setCameraDlg.DoModal() == IDOK){

	}

	// 配置完成后，重新申请相机资源
	XCCAM_ResourceAlloc(hCamera);
}
