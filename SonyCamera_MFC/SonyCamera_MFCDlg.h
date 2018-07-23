
// SonyCamera_MFCDlg.h : ͷ�ļ�
//

#pragma once

#include <XCCamAPI.h>

#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>

#include "SetCameraDlg.h"

#include "SonyCamera_Library.h"

enum CAMERA_STATE{
	CAMERA_CLOSED,
	CAMERA_OPENED
};

enum CAMERA_STAGE{
	CAMERA_SHUTDOWN,
	CAMERA_BOOTED
};

// CSonyCamera_MFCDlg �Ի���
class CSonyCamera_MFCDlg : public CDialogEx
{
// ����
public:
	CSonyCamera_MFCDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_SONYCAMERA_MFC_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	
public:
	HANDLE endShowEvent;		// �˳���ʾ�¼�
	HANDLE showEndEvent;		// ��ʾ�˳��¼�

	HCAMERA hCamera;			// ������
	XCCAM_IMAGE* pImage;
	cv::Mat cv_image;
	PBITMAPINFO m_pBitInfo;
	PUCHAR m_pImage;
	HFEATURE* hFeature;

	BOOL isShowingImage;

	CAMERA_STATE cameraState;
	CAMERA_STAGE cameraStage;
	BOOL isBooting;

	CSetCameraDlg setCameraDlg;

public:
	afx_msg void OnBnClickedButtonOpencam();
	afx_msg void OnBnClickedButtonShowimg();
	afx_msg void OnBnClickedButtonStopshowimg();
	afx_msg void OnBnClickedButtonClosecam();
	afx_msg void OnBnClickedButtonTrigger();
	afx_msg void OnBnClickedButtonShowedge();
	afx_msg void OnBnClickedButton12bitmode();
	afx_msg void OnBnClickedButtonSetcamera();
};
