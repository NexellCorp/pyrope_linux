
// NxCinemaCommand.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CNxCinemaCommandApp:
// �� Ŭ������ ������ ���ؼ��� NxCinemaCommand.cpp�� �����Ͻʽÿ�.
//

class CNxCinemaCommandApp : public CWinApp
{
public:
	CNxCinemaCommandApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CNxCinemaCommandApp theApp;