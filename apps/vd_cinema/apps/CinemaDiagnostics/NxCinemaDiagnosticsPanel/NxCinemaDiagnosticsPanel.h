
// NxCinemaDiagnosticsPanel.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CNxCinemaDiagnosticsPanelApp:
// �� Ŭ������ ������ ���ؼ��� NxCinemaDiagnosticsPanel.cpp�� �����Ͻʽÿ�.
//

class CNxCinemaDiagnosticsPanelApp : public CWinApp
{
public:
	CNxCinemaDiagnosticsPanelApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CNxCinemaDiagnosticsPanelApp theApp;