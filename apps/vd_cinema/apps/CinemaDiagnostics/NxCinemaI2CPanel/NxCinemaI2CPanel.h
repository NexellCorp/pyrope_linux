
// NxCinemaI2CPanel.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CNxCinemaI2CPanelApp:
// �� Ŭ������ ������ ���ؼ��� NxCinemaI2CPanel.cpp�� �����Ͻʽÿ�.
//

class CNxCinemaI2CPanelApp : public CWinApp
{
public:
	CNxCinemaI2CPanelApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CNxCinemaI2CPanelApp theApp;