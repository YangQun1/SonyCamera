
// SonyCamera_MFC.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CSonyCamera_MFCApp: 
// �йش����ʵ�֣������ SonyCamera_MFC.cpp
//

class CSonyCamera_MFCApp : public CWinApp
{
public:
	CSonyCamera_MFCApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CSonyCamera_MFCApp theApp;