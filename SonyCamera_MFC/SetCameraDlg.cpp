// SetCameraDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SonyCamera_MFC.h"
#include "SetCameraDlg.h"
#include "afxdialogex.h"

#include "SonyCamera_MFCDlg.h"

#include "XCCamAPI.h"

#include "resource.h"


// CSetCameraDlg 对话框

IMPLEMENT_DYNAMIC(CSetCameraDlg, CDialogEx)

CSetCameraDlg::CSetCameraDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSetCameraDlg::IDD, pParent)
{

}

CSetCameraDlg::~CSetCameraDlg()
{
}

void CSetCameraDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSetCameraDlg, CDialogEx)
	ON_CBN_SELCHANGE(IDC_COMBO_DRIVE_MODE, &CSetCameraDlg::OnCbnSelchangeComboDriveMode)
	ON_CBN_SELCHANGE(IDC_COMBO_PIXEL_FORMAT, &CSetCameraDlg::OnCbnSelchangeComboPixelFormat)
	ON_BN_CLICKED(IDC_CHECK_AGC, &CSetCameraDlg::OnBnClickedCheckAgc)
	ON_BN_CLICKED(IDC_CHECK_AEC, &CSetCameraDlg::OnBnClickedCheckAec)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_AGC_VALUE, &CSetCameraDlg::OnNMCustomdrawSliderAgcValue)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_AEC_VALUE, &CSetCameraDlg::OnNMCustomdrawSliderAecValue)
	ON_EN_CHANGE(IDC_EDIT_AGC_VALUE, &CSetCameraDlg::OnEnChangeEditAgcValue)
	ON_EN_CHANGE(IDC_EDIT_AEC_VALUE, &CSetCameraDlg::OnEnChangeEditAecValue)
	ON_EN_CHANGE(IDC_EDIT_WIDTH, &CSetCameraDlg::OnEnChangeEditWidth)
	ON_EN_CHANGE(IDC_EDIT_OFFSETX, &CSetCameraDlg::OnEnChangeEditOffsetx)
	ON_EN_CHANGE(IDC_EDIT_HEIGHT, &CSetCameraDlg::OnEnChangeEditHeight)
	ON_EN_CHANGE(IDC_EDIT_OFFSETY, &CSetCameraDlg::OnEnChangeEditOffsety)
	ON_CBN_SELCHANGE(IDC_COMBO_TRIGGER_MODE, &CSetCameraDlg::OnCbnSelchangeComboTriggerMode)
	ON_CBN_SELCHANGE(IDC_COMBO_TRIGGER_SOURCE, &CSetCameraDlg::OnCbnSelchangeComboTriggerSource)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_TRIGGER_DELAY, &CSetCameraDlg::OnNMCustomdrawSliderTriggerDelay)
	ON_EN_CHANGE(IDC_EDIT_TRIGGER_DELAY, &CSetCameraDlg::OnEnChangeEditTriggerDelay)
	ON_CBN_SELCHANGE(IDC_COMBO_DEFAULT_SELECTOR, &CSetCameraDlg::OnCbnSelchangeComboDefaultSelector)
	ON_CBN_SELCHANGE(IDC_COMBO_SAVE_SELECTOR, &CSetCameraDlg::OnCbnSelchangeComboSaveSelector)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CSetCameraDlg::OnBnClickedButtonSave)
END_MESSAGE_MAP()


// CSetCameraDlg 消息处理程序


BOOL CSetCameraDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	isInitingDialog = TRUE;

	CSonyCamera_MFCDlg *pMp = (CSonyCamera_MFCDlg *)GetParent();
	char bufferGet[100];

	// 获取相机信息
	XCCAM_DEVINFO		cinfo;
	XCCAM_CameraInfo(pMp->hCamera, &cinfo);
	switch (cinfo.DeviceType)
	{
	case XCCAM_GIGECAMERA:
		strcpy_s(modelName, cinfo.u.GigEDev.ModelName);
		break;

	case XCCAM_USBCAMERA:
		strcpy_s(modelName, cinfo.u.UsbDev.ModelName);
		break;
	}

	CString str(modelName);
	// str.Format(_T("%s"), modelName);
	SetDlgItemText(IDC_EDIT_CAMERA_NAME, str);

	// 获取相机的特征句柄
	// XCCAM_GetFeatureHandle(pMp->hCamera, (HNodeMap*)&pMp->hFeature);


	// 读取相机模式和像素格式并显示
	cb_DriveMode = (CComboBox *)GetDlgItem(IDC_COMBO_DRIVE_MODE);
	cb_PixelFormat = (CComboBox *)GetDlgItem(IDC_COMBO_PIXEL_FORMAT);
	cb_DriveMode->AddString(_T("Mode0"));
	cb_DriveMode->AddString(_T("Mode1"));
	XCCAM_GetFeatureEnumeration(pMp->hFeature, "DriveMode", bufferGet, 100, FALSE);
	if (strcmp(bufferGet, "Mode0") == 0){
		cb_DriveMode->SetCurSel(0);
		if (0 == strcmp(modelName, "XCG-CG510")){
			cb_PixelFormat->AddString(_T("Mono8"));
		}
		else if (0 == strcmp(modelName, "XCG-CG510C")){
			cb_PixelFormat->AddString(_T("BayerRG8"));
		}
	}
	else if (strcmp(bufferGet, "Mode1") == 0){
		cb_DriveMode->SetCurSel(1);
		if (0 == strcmp(modelName, "XCG-CG510")){
			cb_PixelFormat->AddString(_T("Mono8"));
			cb_PixelFormat->AddString(_T("Mono12Packed"));
		}
		else if (0 == strcmp(modelName, "XCG-CG510C")){
			cb_PixelFormat->AddString(_T("BayerRG8"));
			cb_PixelFormat->AddString(_T("BayerRG12Packed"));
		}
	}
	XCCAM_GetFeatureEnumeration(pMp->hFeature, "PixelFormat", bufferGet, 100, FALSE);
	if (strcmp(bufferGet, "Mono8") == 0 || \
		strcmp(bufferGet, "BayerRG8") == 0){
		cb_PixelFormat->SetCurSel(0);
	}
	else if (strcmp(bufferGet, "Mono12Packed") == 0 || \
		strcmp(bufferGet, "BayerRG12Packed") == 0){
		cb_PixelFormat->SetCurSel(1);
	}

	// 读取增益控制参数并显示
	sd_AgcValue = (CSliderCtrl *)GetDlgItem(IDC_SLIDER_AGC_VALUE);
	sd_AgcValue->SetRange(-38, 442, 0);
	XCCAM_GetFeatureEnumeration(pMp->hFeature, "GainAuto", bufferGet, 100, FALSE);
	if (strcmp(bufferGet, "Off") == 0){
		((CButton *)GetDlgItem(IDC_CHECK_AGC))->SetCheck(0);
		INT64 gainValue;
		XCCAM_GetFeatureInteger(pMp->hFeature, "GainRaw", &gainValue, FALSE);
		SetDlgItemInt(IDC_EDIT_AGC_VALUE, gainValue, 1);
		sd_AgcValue->SetPos(gainValue);
	}
	else{
		((CButton *)GetDlgItem(IDC_CHECK_AGC))->SetCheck(1);
	}

	// 读取曝光控制参数并显示
	sd_AecValue = (CSliderCtrl *)GetDlgItem(IDC_SLIDER_AEC_VALUE);
	sd_AecValue->SetRange(10, 60000000, 6000);
	XCCAM_GetFeatureEnumeration(pMp->hFeature, "ExposureAuto", bufferGet, 100, FALSE);
	if (strcmp(bufferGet, "Off") == 0){
		((CButton *)GetDlgItem(IDC_CHECK_AEC))->SetCheck(0);
		double exposureValue;
		XCCAM_GetFeatureFloat(pMp->hFeature, "ExposureTime", &exposureValue, FALSE);
		SetDlgItemInt(IDC_EDIT_AEC_VALUE, (UINT)exposureValue, 1);
		sd_AecValue->SetPos((int)exposureValue);
	}
	else{
		((CButton *)GetDlgItem(IDC_CHECK_AEC))->SetCheck(1);
	}

	// 读取画面参数并显示
	INT64 WidthSize, HeightSize, OffSetX, OffSetY;
	XCCAM_GetFeatureInteger(pMp->hFeature, "Width", &WidthSize, FALSE);
	XCCAM_GetFeatureInteger(pMp->hFeature, "Height", &HeightSize, FALSE);
	XCCAM_GetFeatureInteger(pMp->hFeature, "OffsetX", &OffSetX, FALSE);
	XCCAM_GetFeatureInteger(pMp->hFeature, "OffsetY", &OffSetY, FALSE);
	SetDlgItemInt(IDC_EDIT_WIDTH, WidthSize, 1);
	SetDlgItemInt(IDC_EDIT_HEIGHT, HeightSize, 1);
	SetDlgItemInt(IDC_EDIT_OFFSETX, OffSetX, 1);
	SetDlgItemInt(IDC_EDIT_OFFSETY, OffSetY, 1);

	// 读取触发参数并显示
	cb_TriggerMode = (CComboBox *)GetDlgItem(IDC_COMBO_TRIGGER_MODE);
	cb_TriggerMode->AddString(_T("On"));
	cb_TriggerMode->AddString(_T("Off"));

	cb_TriggerSource = (CComboBox *)GetDlgItem(IDC_COMBO_TRIGGER_SOURCE);
	cb_TriggerSource->AddString(_T("Line1"));
	cb_TriggerSource->AddString(_T("Line2"));
	cb_TriggerSource->AddString(_T("Line3"));
	cb_TriggerSource->AddString(_T("Software"));

	sd_TriggerDelay = (CSliderCtrl *)GetDlgItem(IDC_SLIDER_TRIGGER_DELAY);
	sd_TriggerDelay->SetRange(0, 4000000, 0);

	XCCAM_GetFeatureEnumeration(pMp->hFeature, "TriggerMode", bufferGet, 100, FALSE);
	if (strcmp(bufferGet, "On") == 0){
		cb_TriggerMode->SetCurSel(0);
		XCCAM_GetFeatureEnumeration(pMp->hFeature, "TriggerSource", bufferGet, 100, FALSE);
		if (strcmp(bufferGet, "Line1") == 0){
			cb_TriggerSource->SetCurSel(0);
		}
		else if (strcmp(bufferGet, "Line2") == 0){
			cb_TriggerSource->SetCurSel(1);
		}
		else if (strcmp(bufferGet, "Line3") == 0){
			cb_TriggerSource->SetCurSel(2);
		}
		else if (strcmp(bufferGet, "Software") == 0){
			cb_TriggerSource->SetCurSel(3);
		}
		double triggerDelay;
		XCCAM_GetFeatureFloat(pMp->hFeature, "TriggerDelay", &triggerDelay, FALSE);
		sd_TriggerDelay->SetPos((int)triggerDelay);
		SetDlgItemInt(IDC_EDIT_TRIGGER_DELAY, (UINT)triggerDelay, 1);
	}
	else{
		cb_TriggerMode->SetCurSel(1);
		cb_TriggerSource->EnableWindow(FALSE);
		sd_TriggerDelay->EnableWindow(FALSE);
		((CEdit *)GetDlgItem(IDC_EDIT_TRIGGER_DELAY))->EnableWindow(FALSE);
	}


	// 读取默认启动选择器参数并显示
	BOOL ret;
	//XCCAM_FEATUREINFO UserSetDefaultInfo;
	//ret = XCCAM_FeatureInfo(pMp->hFeature, "UserSetDefaultSelector", &UserSetDefaultInfo);
	cb_DefaultSelector = (CComboBox *)GetDlgItem(IDC_COMBO_DEFAULT_SELECTOR);
	cb_DefaultSelector->AddString(_T("Default"));
	cb_DefaultSelector->AddString(_T("UserSet1"));
	cb_DefaultSelector->AddString(_T("UserSet2"));
	ret = XCCAM_GetFeatureEnumeration(pMp->hFeature, "UserSetDefaultSelector", bufferGet, 100, FALSE);
	if (strcmp(bufferGet, "Default") == 0){
		cb_DefaultSelector->SetCurSel(0);
	}
	else if (strcmp(bufferGet, "UserSet1") == 0){
		cb_DefaultSelector->SetCurSel(1);
	}
	else if (strcmp(bufferGet, "UserSet2") == 0){
		cb_DefaultSelector->SetCurSel(2);
	}
	else{
		// TODO
		cb_DefaultSelector->SetCurSel(-1);
	}


	// 读取保存参数选择器参数并显示
	cb_SaveSelector = (CComboBox *)GetDlgItem(IDC_COMBO_SAVE_SELECTOR);
	cb_SaveSelector->AddString(_T("Default"));
	cb_SaveSelector->AddString(_T("UserSet1"));
	cb_SaveSelector->AddString(_T("UserSet2"));
	ret = XCCAM_GetFeatureEnumeration(pMp->hFeature, "UserSetSelector", bufferGet, 100, FALSE);
	if (strcmp(bufferGet, "Default") == 0){
		cb_SaveSelector->SetCurSel(0);
	}
	else if (strcmp(bufferGet, "UserSet1") == 0){
		cb_SaveSelector->SetCurSel(1);
	}
	else if (strcmp(bufferGet, "UserSet2") == 0){
		cb_SaveSelector->SetCurSel(2);
	}
	else{
		// TODO
		cb_SaveSelector->SetCurSel(-1);
	}

	isInitingDialog = FALSE;

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}


extern "C" VOID CALLBACK SystemFunc(STATUS_SYSTEMCODE Status, PVOID Countext);
void CSetCameraDlg::OnCbnSelchangeComboDriveMode()
{
	// TODO:  在此添加控件通知处理程序代码
	if (TRUE == isInitingDialog)
		return;

	MessageBox(_T("此过程需要重启相机，点击\"确定\"开始自动重启过程"));
	BOOL ret = FALSE;
	char bufferGet[100];

	CSonyCamera_MFCDlg *pMp = (CSonyCamera_MFCDlg *)GetParent();
	HFEATURE* hFeature = pMp->hFeature;
	// HCAMERA hCamera = pMp->hCamera;

	// 设置相机模式
	int sel = cb_DriveMode->GetCurSel();
	if (0 == sel){
		ret = XCCAM_SetFeatureEnumeration(hFeature, "DriveMode", "Mode0");
		cb_PixelFormat->SetCurSel(0);		// 只有Mono8或BayerRG8
		cb_PixelFormat->DeleteString(1);
	}
	else{
		ret = XCCAM_SetFeatureEnumeration(hFeature, "DriveMode", "Mode1");
		if (0 == strcmp(modelName, "XCG-CG510")){
			cb_PixelFormat->AddString(_T("Mono12Packed"));
		}
		else if (0 == strcmp(modelName, "XCG-CG510C")){
			cb_PixelFormat->AddString(_T("BayerRG12Packed"));
		}

		// 更改模式后的默认设置肯定是8bit的
		cb_PixelFormat->SetCurSel(0);
	}
	
	// 重启相机
	ret = XCCAM_FeatureCommand(hFeature, "CameraReboot");
	XCCAM_Close(pMp->hCamera);
	pMp->cameraState = CAMERA_CLOSED;
	pMp->isBooting = TRUE;

	// 等待相机重启完成
	this->EnableWindow(FALSE);
	while (pMp->isBooting == TRUE);
	this->EnableWindow(TRUE);

	// 重新打开相机并获取特征句柄
	XCCAM_SetStructVersion(XCCAM_LIBRARY_STRUCT_VERSION);
	ret = XCCAM_Open(NULL, &(pMp->hCamera));
	XCCAM_GetFeatureHandle(pMp->hCamera, (HNodeMap*)&pMp->hFeature);
	Sony_Camera_Handle sonyHandle = GetCameraHandle();
	sonyHandle->hCamera = pMp->hCamera;
	sonyHandle->hFeature = pMp->hFeature;

	pMp->cameraState = CAMERA_OPENED;

	XCCAM_SetCallBack(pMp, SystemFunc);

}


void CSetCameraDlg::OnCbnSelchangeComboPixelFormat()
{
	// TODO:  在此添加控件通知处理程序代码
	if (TRUE == isInitingDialog)
		return;

	CSonyCamera_MFCDlg *pMp = (CSonyCamera_MFCDlg *)GetParent();

	BOOL ret;
	int sel = cb_PixelFormat->GetCurSel();

	if (sel == 0){
		if (0 == strcmp(modelName, "XCG-CG510")){
			ret = XCCAM_SetFeatureEnumeration(pMp->hFeature, "PixelFormat", "Mono8");
		}
		else{
			ret = XCCAM_SetFeatureEnumeration(pMp->hFeature, "PixelFormat", "BayerRG8");
		}
	}
	else{
		if (0 == strcmp(modelName, "XCG-CG510")){
			ret = XCCAM_SetFeatureEnumeration(pMp->hFeature, "PixelFormat", "Mono12Packed");
		}
		else{
			ret = XCCAM_SetFeatureEnumeration(pMp->hFeature, "PixelFormat", "BayerRG12Packed");
		}
	}

	if (ret == FALSE){
		MessageBox(_T("Set Failed"));
	}

	return;
}


void CSetCameraDlg::OnBnClickedCheckAgc()
{
	// TODO:  在此添加控件通知处理程序代码
	if (TRUE == isInitingDialog)
		return;

	CSonyCamera_MFCDlg *pMp = (CSonyCamera_MFCDlg *)GetParent();

	int check = ((CButton *)GetDlgItem(IDC_CHECK_AGC))->GetCheck();
	if (0 == check){
		XCCAM_SetFeatureEnumeration(pMp->hFeature, "GainAuto", "Off");

		int agcValue = sd_AgcValue->GetPos();
		XCCAM_SetFeatureInteger(pMp->hFeature, "GainRaw", agcValue);
	}
	else{
		XCCAM_SetFeatureEnumeration(pMp->hFeature, "GainAuto", "Continuous");
	}
}


void CSetCameraDlg::OnBnClickedCheckAec()
{
	// TODO:  在此添加控件通知处理程序代码
	if (TRUE == isInitingDialog)
		return;

	CSonyCamera_MFCDlg *pMp = (CSonyCamera_MFCDlg *)GetParent();

	int check = ((CButton *)GetDlgItem(IDC_CHECK_AEC))->GetCheck();
	if (0 == check){
		XCCAM_SetFeatureEnumeration(pMp->hFeature, "ExposureAuto", "Off");

		int aecValue = sd_AecValue->GetPos();
		XCCAM_SetFeatureFloat(pMp->hFeature, "ExposureTime", aecValue);
	}
	else{
		XCCAM_SetFeatureEnumeration(pMp->hFeature, "ExposureAuto", "Continuous");
	}
}


void CSetCameraDlg::OnNMCustomdrawSliderAgcValue(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO:  在此添加控件通知处理程序代码
	if (TRUE == isInitingDialog)
		return;

	CSonyCamera_MFCDlg *pMp = (CSonyCamera_MFCDlg *)GetParent();

	CEdit * pEdit = (CEdit *)GetDlgItem(IDC_EDIT_AGC_VALUE);

	int agcValue = sd_AgcValue->GetPos();
	SetDlgItemInt(IDC_EDIT_AGC_VALUE, agcValue, 1);

	// XCCAM_SetFeatureInteger(pMp->hFeature, "GainRaw", agcValue);

	*pResult = 0;
}


void CSetCameraDlg::OnNMCustomdrawSliderAecValue(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO:  在此添加控件通知处理程序代码
	if (TRUE == isInitingDialog)
		return;

	CSonyCamera_MFCDlg *pMp = (CSonyCamera_MFCDlg *)GetParent();

	CEdit * pEdit = (CEdit *)GetDlgItem(IDC_EDIT_AEC_VALUE);

	int aecValue = sd_AecValue->GetPos();
	SetDlgItemInt(IDC_EDIT_AEC_VALUE, aecValue, 1);

	// XCCAM_SetFeatureFloat(pMp->hFeature, "ExposureTime", (double)aecValue);

	*pResult = 0;
}


void CSetCameraDlg::OnEnChangeEditAgcValue()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	if (TRUE == isInitingDialog)
		return;

	CSonyCamera_MFCDlg *pMp = (CSonyCamera_MFCDlg *)GetParent();
	
	int agcValue = GetDlgItemInt(IDC_EDIT_AGC_VALUE);

	sd_AgcValue->SetPos(agcValue);

	XCCAM_SetFeatureInteger(pMp->hFeature, "GainRaw", agcValue);
}


void CSetCameraDlg::OnEnChangeEditAecValue()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	if (TRUE == isInitingDialog)
		return;

	CSonyCamera_MFCDlg *pMp = (CSonyCamera_MFCDlg *)GetParent();

	int aecValue = GetDlgItemInt(IDC_EDIT_AEC_VALUE);

	sd_AecValue->SetPos(aecValue);

	XCCAM_SetFeatureFloat(pMp->hFeature, "ExposureTime", (double)aecValue);
}


void CSetCameraDlg::OnEnChangeEditWidth()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	if (TRUE == isInitingDialog)
		return;

	CSonyCamera_MFCDlg *pMp = (CSonyCamera_MFCDlg *)GetParent();

	int width = GetDlgItemInt(IDC_EDIT_WIDTH);
	int offsetX = GetDlgItemInt(IDC_EDIT_OFFSETX);

	XCCAM_FEATUREINFO WInfo, OXInfo;
	XCCAM_FeatureInfo(pMp->hFeature, "Width", &WInfo);
	XCCAM_FeatureInfo(pMp->hFeature, "OffsetX", &OXInfo);

	if (WInfo.u.IntReg.MaxValue < width){
		width = WInfo.u.IntReg.MaxValue;
		SetDlgItemInt(IDC_EDIT_WIDTH, width);
		return;
	}
	if (width + offsetX > WInfo.u.IntReg.MaxValue){
		offsetX = WInfo.u.IntReg.MaxValue - width;
		SetDlgItemInt(IDC_EDIT_OFFSETX, offsetX);
		return;
	}

	XCCAM_SetFeatureInteger(pMp->hFeature, "Width", width);
	XCCAM_SetFeatureInteger(pMp->hFeature, "OffsetX", offsetX);
}


void CSetCameraDlg::OnEnChangeEditHeight()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	if (TRUE == isInitingDialog)
		return;

	CSonyCamera_MFCDlg *pMp = (CSonyCamera_MFCDlg *)GetParent();

	int height = GetDlgItemInt(IDC_EDIT_HEIGHT);
	int offsetY = GetDlgItemInt(IDC_EDIT_OFFSETY);

	XCCAM_FEATUREINFO HInfo, OYInfo;
	XCCAM_FeatureInfo(pMp->hFeature, "Height", &HInfo);
	XCCAM_FeatureInfo(pMp->hFeature, "OffsetY", &OYInfo);


	if (HInfo.u.IntReg.MaxValue < height){
		height = HInfo.u.IntReg.MaxValue;
		SetDlgItemInt(IDC_EDIT_HEIGHT, height);
		return;
	}
	if (height + offsetY > HInfo.u.IntReg.MaxValue){
		offsetY = HInfo.u.IntReg.MaxValue - height;
		SetDlgItemInt(IDC_EDIT_OFFSETY, offsetY);
		return;
	}

	XCCAM_SetFeatureInteger(pMp->hFeature, "Height", height);
	XCCAM_SetFeatureInteger(pMp->hFeature, "OffsetY", offsetY);
}

void CSetCameraDlg::OnEnChangeEditOffsetx()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	if (TRUE == isInitingDialog)
		return;

	CSonyCamera_MFCDlg *pMp = (CSonyCamera_MFCDlg *)GetParent();

	int width = GetDlgItemInt(IDC_EDIT_WIDTH);
	int offsetX = GetDlgItemInt(IDC_EDIT_OFFSETX);

	XCCAM_FEATUREINFO WInfo, OXInfo;
	XCCAM_FeatureInfo(pMp->hFeature, "Width", &WInfo);
	XCCAM_FeatureInfo(pMp->hFeature, "OffsetX", &OXInfo);
 
	if (WInfo.u.IntReg.MaxValue < width){
		width = WInfo.u.IntReg.MaxValue;
		SetDlgItemInt(IDC_EDIT_WIDTH, width);
		return;
	}
	if (width + offsetX > WInfo.u.IntReg.MaxValue){
		offsetX = WInfo.u.IntReg.MaxValue - width;
		SetDlgItemInt(IDC_EDIT_OFFSETX, offsetX);
		return;
	}

	XCCAM_SetFeatureInteger(pMp->hFeature, "Width", width);
	XCCAM_SetFeatureInteger(pMp->hFeature, "OffsetX", offsetX);
}


void CSetCameraDlg::OnEnChangeEditOffsety()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	if (TRUE == isInitingDialog)
		return;

	CSonyCamera_MFCDlg *pMp = (CSonyCamera_MFCDlg *)GetParent();

	int height = GetDlgItemInt(IDC_EDIT_HEIGHT);
	int offsetY = GetDlgItemInt(IDC_EDIT_OFFSETY);

	XCCAM_FEATUREINFO HInfo, OYInfo;
	XCCAM_FeatureInfo(pMp->hFeature, "Height", &HInfo);
	XCCAM_FeatureInfo(pMp->hFeature, "OffsetY", &OYInfo);


	if (HInfo.u.IntReg.MaxValue < height){
		height = HInfo.u.IntReg.MaxValue;
		SetDlgItemInt(IDC_EDIT_HEIGHT, height);
		return;
	}
	if (height + offsetY > HInfo.u.IntReg.MaxValue){
		offsetY = HInfo.u.IntReg.MaxValue - height;
		SetDlgItemInt(IDC_EDIT_OFFSETY, offsetY);
		return;
	}

	XCCAM_SetFeatureInteger(pMp->hFeature, "Height", height);
	XCCAM_SetFeatureInteger(pMp->hFeature, "OffsetY", offsetY);
}




void CSetCameraDlg::OnCbnSelchangeComboTriggerMode()
{
	// TODO:  在此添加控件通知处理程序代码
	if (TRUE == isInitingDialog)
		return;

	CSonyCamera_MFCDlg *pMp = (CSonyCamera_MFCDlg *)GetParent();

	BOOL ret;
	int sel = cb_TriggerMode->GetCurSel();

	if (sel == 0){
		// 设置为触发模式
		ret = XCCAM_SetFeatureEnumeration(pMp->hFeature, "TriggerMode", "On");
		cb_TriggerSource->EnableWindow(TRUE);
		sd_TriggerDelay->EnableWindow(TRUE);
		((CEdit *)GetDlgItem(IDC_EDIT_TRIGGER_DELAY))->EnableWindow(TRUE);

		// 获取并设置参数
		int sel = cb_TriggerSource->GetCurSel();
		if (0 == sel){
			XCCAM_SetFeatureEnumeration(pMp->hFeature, "TriggerSource", "Line1");
		}
		else if (1 == sel){
			XCCAM_SetFeatureEnumeration(pMp->hFeature, "TriggerSource", "Line2");
		}
		else if (2 == sel){
			XCCAM_SetFeatureEnumeration(pMp->hFeature, "TriggerSource", "Line3");
		}
		else{
			XCCAM_SetFeatureEnumeration(pMp->hFeature, "TriggerSource", "Software");
		}

		int delayTime = sd_TriggerDelay->GetPos();
		XCCAM_SetFeatureFloat(pMp->hFeature, "TriggerDelay", delayTime);
	}
	else{
		ret = XCCAM_SetFeatureEnumeration(pMp->hFeature, "TriggerMode", "Off");
		cb_TriggerSource->EnableWindow(FALSE);
		sd_TriggerDelay->EnableWindow(FALSE);
		((CEdit *)GetDlgItem(IDC_EDIT_TRIGGER_DELAY))->EnableWindow(FALSE);
	}

	if (ret == FALSE){
		MessageBox(_T("Set Failed"));
	}
}


void CSetCameraDlg::OnCbnSelchangeComboTriggerSource()
{
	// TODO:  在此添加控件通知处理程序代码
	if (TRUE == isInitingDialog)
		return;

	BOOL ret;

	CSonyCamera_MFCDlg *pMp = (CSonyCamera_MFCDlg *)GetParent();

	int sel = cb_TriggerSource->GetCurSel();
	if (0 == sel){
		ret = XCCAM_SetFeatureEnumeration(pMp->hFeature, "TriggerSource", "Line1");
	}
	else if (1 == sel){
		ret = XCCAM_SetFeatureEnumeration(pMp->hFeature, "TriggerSource", "Line2");
	}
	else if (2 == sel){
		ret = XCCAM_SetFeatureEnumeration(pMp->hFeature, "TriggerSource", "Line3");
	}
	else{
		ret = XCCAM_SetFeatureEnumeration(pMp->hFeature, "TriggerSource", "Software");
	}
}


void CSetCameraDlg::OnNMCustomdrawSliderTriggerDelay(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO:  在此添加控件通知处理程序代码
	if (TRUE == isInitingDialog)
		return;

	CSonyCamera_MFCDlg *pMp = (CSonyCamera_MFCDlg *)GetParent();

	int delayTime = sd_TriggerDelay->GetPos();

	// XCCAM_SetFeatureFloat(pMp->hFeature, "TriggerDelay", delayTime);

	SetDlgItemInt(IDC_EDIT_TRIGGER_DELAY, delayTime);

	*pResult = 0;
}


void CSetCameraDlg::OnEnChangeEditTriggerDelay()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	if (TRUE == isInitingDialog)
		return;

	CSonyCamera_MFCDlg *pMp = (CSonyCamera_MFCDlg *)GetParent();

	int delayTime = GetDlgItemInt(IDC_EDIT_TRIGGER_DELAY);
	
	XCCAM_SetFeatureFloat(pMp->hFeature, "TriggerDelay", delayTime);

	sd_TriggerDelay->SetPos(delayTime);
}


void CSetCameraDlg::OnCbnSelchangeComboDefaultSelector()
{
	// TODO:  在此添加控件通知处理程序代码
	if (TRUE == isInitingDialog)
		return;

	CSonyCamera_MFCDlg *pMp = (CSonyCamera_MFCDlg *)GetParent();

	BOOL ret;
	int sel = cb_DefaultSelector->GetCurSel();
	
	if (0 == sel){
		MessageBox(_T("Dafault Selector is Reserved for Vendor"));
	}
	else if (1 == sel){
		ret = XCCAM_SetFeatureEnumeration(pMp->hFeature, "UserSetDefaultSelector", "UserSet1");
	}
	else if (2 == sel){
		ret = XCCAM_SetFeatureEnumeration(pMp->hFeature, "UserSetDefaultSelector", "UserSet2");
	}

	return;
}


void CSetCameraDlg::OnCbnSelchangeComboSaveSelector()
{
	// TODO:  在此添加控件通知处理程序代码
	// Do Nothing
}


void CSetCameraDlg::OnBnClickedButtonSave()
{
	// TODO:  在此添加控件通知处理程序代码
	if (TRUE == isInitingDialog)
		return;

	CSonyCamera_MFCDlg *pMp = (CSonyCamera_MFCDlg *)GetParent();

	BOOL ret;
	int sel = cb_SaveSelector->GetCurSel();

	if (0 == sel){
		MessageBox(_T("Dafault Selector is Reserved for Vendor"));
	}
	else if (1 == sel){
		ret = XCCAM_SetFeatureEnumeration(pMp->hFeature, "UserSetSelector", "UserSet1");
	}
	else if (2 == sel){
		ret = XCCAM_SetFeatureEnumeration(pMp->hFeature, "UserSetSelector", "UserSet2");
	}

	ret = XCCAM_FeatureCommand(pMp->hFeature, "UserSetSave");

	return;
}
