#pragma once


// CSetCameraDlg 对话框

class CSetCameraDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSetCameraDlg)

public:
	CSetCameraDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSetCameraDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_SETCAMERA };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

private:
	BOOL isInitingDialog;

public:
	char modelName[32];

	CComboBox *cb_DriveMode;
	CComboBox *cb_PixelFormat;
	CComboBox *cb_TriggerMode;
	CComboBox *cb_TriggerSource;
	CComboBox *cb_DefaultSelector;
	CComboBox *cb_SaveSelector;

	CSliderCtrl *sd_AgcValue;
	CSliderCtrl *sd_AecValue;
	CSliderCtrl *sd_TriggerDelay;

	CEdit *ed_Width;
	CEdit *ed_Height;
	CEdit *ed_OffsetX;
	CEdit *ed_OffsetY;


	virtual BOOL OnInitDialog();
	afx_msg void OnCbnSelchangeComboDriveMode();
	afx_msg void OnCbnSelchangeComboPixelFormat();
	afx_msg void OnBnClickedCheckAgc();
	afx_msg void OnBnClickedCheckAec();
	afx_msg void OnNMCustomdrawSliderAgcValue(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSliderAecValue(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangeEditAgcValue();
	afx_msg void OnEnChangeEditAecValue();
	afx_msg void OnEnChangeEditWidth();
	afx_msg void OnEnChangeEditOffsetx();
	afx_msg void OnEnChangeEditHeight();
	afx_msg void OnEnChangeEditOffsety();
	afx_msg void OnCbnSelchangeComboTriggerMode();
	afx_msg void OnCbnSelchangeComboTriggerSource();
	afx_msg void OnNMCustomdrawSliderTriggerDelay(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangeEditTriggerDelay();
	afx_msg void OnCbnSelchangeComboDefaultSelector();
	afx_msg void OnCbnSelchangeComboSaveSelector();
	afx_msg void OnBnClickedButtonSave();
};
