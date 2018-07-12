
// SonyCamera_MFCDlg.h : ͷ�ļ�
//

#pragma once

#include <XCCamAPI.h>

#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>

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

	// ���������صĺ���
	
public:
	HANDLE m_EndEvent;			// �˳��ɼ��¼����
	HANDLE m_RcvTermEvent;		// �ɼ��߳���ֹ���

	HCAMERA hCamera;			// ������
	XCCAM_IMAGE* pImage;
	cv::Mat cv_image;
	PBITMAPINFO m_pBitInfo;
	PUCHAR m_pImage;
	HFEATURE* m_hFeature;

	BOOL isShowingImage;

public:
	afx_msg void OnBnClickedButtonOpencam();
	afx_msg void OnBnClickedButtonShowimg();
	afx_msg void OnBnClickedButtonStopshowimg();
	afx_msg void OnBnClickedButtonClosecam();
	afx_msg void OnBnClickedButtonTrigger();
};
