
// SonyCamera_MFCDlg.h : 头文件
//

#pragma once

#include <XCCamAPI.h>

#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>

// CSonyCamera_MFCDlg 对话框
class CSonyCamera_MFCDlg : public CDialogEx
{
// 构造
public:
	CSonyCamera_MFCDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_SONYCAMERA_MFC_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	// 操作相机相关的函数
	
public:
	HANDLE m_EndEvent;			// 退出采集事件句柄
	HANDLE m_RcvTermEvent;		// 采集线程终止句柄

	HCAMERA hCamera;			// 相机句柄
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
