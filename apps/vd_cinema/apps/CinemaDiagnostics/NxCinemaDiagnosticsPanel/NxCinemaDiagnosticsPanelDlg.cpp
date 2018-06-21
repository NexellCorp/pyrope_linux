
// NxCinemaDiagnosticsPanelDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "NxCinemaDiagnosticsPanel.h"
#include "NxCinemaDiagnosticsPanelDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
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


// CNxCinemaDiagnosticsPanelDlg 대화 상자

CNxCinemaDiagnosticsPanelDlg::CNxCinemaDiagnosticsPanelDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CNxCinemaDiagnosticsPanelDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CNxCinemaDiagnosticsPanelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_LOG, m_EditLog);
	DDX_Control(pDX, IDC_BTN_TEST, m_BtnTest);
	DDX_Control(pDX, IDC_BTN_RUN, m_BtnRun);
	DDX_Control(pDX, IDC_BTN_WORKING_OPEN, m_BtnWorkingOpen);
	DDX_Control(pDX, IDC_BTN_WORKING_SELECT, m_BtnWorkingSelect);
	DDX_Control(pDX, IDC_EDIT_STATUS_SAP, m_EditStatusSap);
	DDX_Control(pDX, IDC_EDIT_SERIAL, m_EditSerial);
	DDX_Control(pDX, IDC_EDIT_PRODUCT, m_EditProduct);
	DDX_Control(pDX, IDC_EDIT_CERTIFICATE, m_EditCertificate);
	DDX_Control(pDX, IDC_EDIT_CERTIFICATE_TEXT, m_EditCertificateText);
	DDX_Control(pDX, IDC_EDIT_STATUS_NAP, m_EditStatusNap);
	DDX_Control(pDX, IDC_EDIT_TCON_LEFT, m_EditTconLeft);
	DDX_Control(pDX, IDC_EDIT_TCON_RIGHT, m_EditTconRight);
	DDX_Control(pDX, IDC_EDIT_PFPGA, m_EditPfpga);
	DDX_Control(pDX, IDC_EDIT_EEPROM, m_EditEEPRom);
	DDX_Control(pDX, IDC_EDIT_MARRIAGE_TAMPER, m_EditMarriageTamper);
	DDX_Control(pDX, IDC_EDIT_DOOR_TAMPER, m_EditDoorTamper);
	DDX_Control(pDX, IDC_EDIT_NETWORK, m_EditNetwork);
	DDX_Control(pDX, IDC_IPADDRESS, m_IpAddr);
	DDX_Control(pDX, IDC_EDIT_WORKING, m_EditWorking);
	DDX_Control(pDX, IDC_EDIT_TCON_REPEAT, m_EditTconRepeat);
	DDX_Control(pDX, IDC_EDIT_PFPGA_REPEAT, m_EditPfpgaRepeat);
	DDX_Control(pDX, IDC_EDIT_TAMPER_REPEAT, m_EditTamperRepeat);
	DDX_Control(pDX, IDC_EDIT_NETWORK_REPEAT, m_EditNetworkRepeat);
	DDX_Control(pDX, IDC_EDIT_VERSION, m_EditVersion);
	DDX_Control(pDX, IDC_SPIN_TCON_REPEAT, m_SpinTconRepeat);
	DDX_Control(pDX, IDC_SPIN_PFPGA_REPEAT, m_SpinPfpgaRepeat);
	DDX_Control(pDX, IDC_SPIN_TAMPER_REPEAT, m_SpinTamperRepeat);
	DDX_Control(pDX, IDC_SPIN_NETWORK_REPEAT, m_SpinNetworkRepeat);
	DDX_Control(pDX, IDC_BTN_REBOOT, m_BtnReboot);
	DDX_Control(pDX, IDC_BTN_RESTART_APP, m_BtnRestartApp);
}

BEGIN_MESSAGE_MAP(CNxCinemaDiagnosticsPanelDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BTN_TEST, &CNxCinemaDiagnosticsPanelDlg::OnBnClickedBtnTest)
	ON_BN_CLICKED(IDC_BTN_WORKING_SELECT, &CNxCinemaDiagnosticsPanelDlg::OnBnClickedBtnWorkingSelect)
	ON_BN_CLICKED(IDC_BTN_WORKING_OPEN, &CNxCinemaDiagnosticsPanelDlg::OnBnClickedBtnWorkingOpen)
	ON_BN_CLICKED(IDC_BTN_RUN, &CNxCinemaDiagnosticsPanelDlg::OnBnClickedBtnRun)
	ON_BN_CLICKED(IDC_BTN_RESTART_APP, &CNxCinemaDiagnosticsPanelDlg::OnBnClickedBtnRestartApp)
	ON_BN_CLICKED(IDC_BTN_REBOOT, &CNxCinemaDiagnosticsPanelDlg::OnBnClickedBtnReboot)
END_MESSAGE_MAP()


// CNxCinemaDiagnosticsPanelDlg 메시지 처리기

#ifdef _DEBUG
#include <locale.h>
#endif

//
//	Version Information
//	* Version 0.1.0 at 2018.06.12
//		- First Draft.
//

#define NX_DIAG_VERSION		L"v0.1.0"

BOOL CNxCinemaDiagnosticsPanelDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
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

	// 이 대화 상자의 아이콘을 설정합니다. 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	CString szTitle;
	this->GetWindowTextW( szTitle );
	
	szTitle += L" ";
	szTitle += NX_DIAG_VERSION;
	this->SetWindowText( szTitle);

	m_ToolTip.Create( this );

#if 0
	CFont font;
	LOGFONT logfont;
	
	ZeroMemory( &logfont, sizeof(logfont) );
	logfont.lfHeight = 15;
	logfont.lfWeight = FW_BOLD;
	lstrcpy( logfont.lfFaceName, (LPCWSTR)"Tahoma" );
	
	font.CreateFontIndirect( &logfont );
	m_EditLog.SetFont( &font );
	font.Detach();
#endif

#ifdef _DEBUG
	_tsetlocale( LC_ALL, L"korean" );
#endif

	m_BtnTest.ShowWindow( FALSE );

	LoadConfig();

	UDACCEL udAccels[] = {{0, -1}};
	m_SpinTconRepeat.SetRange( MAX_REPEAT_NUM, MIN_REPEAT_NUM );
	m_SpinTconRepeat.SetAccel( 1, udAccels );
	m_SpinPfpgaRepeat.SetRange( MAX_REPEAT_NUM, MIN_REPEAT_NUM );
	m_SpinPfpgaRepeat.SetAccel( 1, udAccels );
	m_SpinTamperRepeat.SetRange( MAX_REPEAT_NUM, MIN_REPEAT_NUM );
	m_SpinTamperRepeat.SetAccel( 1, udAccels );
	m_SpinNetworkRepeat.SetRange( MAX_REPEAT_NUM, MIN_REPEAT_NUM );
	m_SpinNetworkRepeat.SetAccel( 1, udAccels );

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CNxCinemaDiagnosticsPanelDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다. 문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CNxCinemaDiagnosticsPanelDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CNxCinemaDiagnosticsPanelDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL CNxCinemaDiagnosticsPanelDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	m_ToolTip.RelayEvent( pMsg );

	if( pMsg->message == WM_KEYDOWN )
	{
		if( pMsg->wParam == VK_ESCAPE )
		{
			return TRUE;
		}

		if( pMsg->wParam == VK_RETURN )
		{
			OnBnClickedBtnRun();
			return TRUE;
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

void CNxCinemaDiagnosticsPanelDlg::OnClose()
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	SaveConfig();

	CDialogEx::OnClose();
}

void CNxCinemaDiagnosticsPanelDlg::OnBnClickedBtnTest()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}

#include "BrowseFolderDialog.h"

void CNxCinemaDiagnosticsPanelDlg::OnBnClickedBtnWorkingSelect()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString szWorking;
	m_EditWorking.GetWindowTextW( szWorking );
	
	CBrowseFolderDialog dlg;
	if( dlg.DoModal(szWorking) == IDOK )
	{
		dlg.GetSelectStr( szWorking );
		m_EditWorking.SetWindowTextW( szWorking );
		
		m_EditWorking.SetSel( 0, -1 );
		m_EditWorking.SetSel( -1, -1 );
		m_ToolTip.AddTool( GetDlgItem(IDC_EDIT_WORKING), szWorking );
	}
}

void CNxCinemaDiagnosticsPanelDlg::OnBnClickedBtnWorkingOpen()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString szWorking;
	m_EditWorking.GetWindowTextW( szWorking );

	ShellExecute( NULL, L"open", L"explorer.exe",  szWorking, NULL, SW_SHOW ); 
}

void CNxCinemaDiagnosticsPanelDlg::OnBnClickedBtnRun()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	ClearEditControl();
	Run( &RunDiagnostics, this );
}

void CNxCinemaDiagnosticsPanelDlg::OnBnClickedBtnRestartApp()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	ClearEditControl();
	Run( &RunRestartApplication, this );
}

void CNxCinemaDiagnosticsPanelDlg::OnBnClickedBtnReboot()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	ClearEditControl();
	Run( &RunReboot, this );
}

BOOL CNxCinemaDiagnosticsPanelDlg::Execute( CString szCommand, CString *pResult )
{
	SECURITY_ATTRIBUTES securityAttr;
	ZeroMemory( &securityAttr, sizeof(SECURITY_ATTRIBUTES) );
	securityAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	securityAttr.bInheritHandle = TRUE;
	securityAttr.lpSecurityDescriptor = NULL;

	HANDLE hReadPipe, hWritePipe;
	CreatePipe( &hReadPipe, &hWritePipe, &securityAttr, 0 );

	STARTUPINFO startupInfo;
	PROCESS_INFORMATION processInfo;
	ZeroMemory( &startupInfo, sizeof(STARTUPINFO) );
	ZeroMemory( &processInfo, sizeof(PROCESS_INFORMATION) );

	startupInfo.cb = sizeof(STARTUPINFO);
	startupInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	startupInfo.wShowWindow = SW_HIDE;

	startupInfo.hStdOutput = hWritePipe;
	startupInfo.hStdError = hWritePipe;

	TCHAR tcPath[MAX_PATH];
	GetModuleFileName( NULL, tcPath, MAX_PATH );

	CString szFullCommand;
	szFullCommand.Format( L"%s", tcPath );
	szFullCommand = szFullCommand.Left( szFullCommand.ReverseFind('\\') );
	szFullCommand += L"\\bin\\";
	szFullCommand += szCommand;

	int iRet = CreateProcess( NULL, (TCHAR*)(LPCTSTR)szFullCommand, NULL, NULL, TRUE, 0, NULL, NULL, &startupInfo, &processInfo );
	if( FALSE == iRet )
	{
		return FALSE;
	}

	do {
		CHAR result[1024];
		DWORD dwTotalByteAvail = 0, dwRead = 0, dwLeftThisMessage = 0;

		BOOL bSuccess = PeekNamedPipe( hReadPipe, result, 1, &dwRead, &dwTotalByteAvail, &dwLeftThisMessage);
		if( TRUE == bSuccess )
		{
			if( 0 < dwRead )
			{
				bSuccess = ReadFile( hReadPipe, result, sizeof(result) - 1, &dwRead, NULL );
				if( TRUE == bSuccess )
				{
					result[dwRead] = NULL;
					CString szTemp(result);

					if( pResult != NULL ) 
						*pResult += szTemp;
				}
			}
		}
	} while( WAIT_OBJECT_0 != WaitForSingleObject(processInfo.hProcess, 0) );

	CloseHandle( processInfo.hThread );
	CloseHandle( processInfo.hProcess );

	CloseHandle( hReadPipe );
	CloseHandle( hWritePipe );

	return TRUE;
}

BOOL CNxCinemaDiagnosticsPanelDlg::IsConnected( CString szProduct )
{
	for( int i = 0; i < m_InfoNum; i++ )
	{
		if( !m_Info[i].szProduct.Compare( szProduct ) && !m_Info[i].szStatus.Compare( L"device" ) )
		{
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CNxCinemaDiagnosticsPanelDlg::IsPass( CString szResult )
{
	szResult.Trim();
	szResult.MakeUpper();
	return ( 0 <= szResult.Find( L"PASS" )) ? TRUE : FALSE;
}

BOOL CNxCinemaDiagnosticsPanelDlg::IsFail( CString szResult )
{
	szResult.Trim();
	szResult.MakeUpper();
	return ( 0 <= szResult.Find( L"FAIL" )) ? TRUE : FALSE;
}

void CNxCinemaDiagnosticsPanelDlg::EnableControl( BOOL bEnable )
{
	m_BtnRun.EnableWindow( bEnable );
	m_BtnRestartApp.EnableWindow( bEnable );
	m_BtnReboot.EnableWindow( bEnable );
	m_BtnWorkingSelect.EnableWindow( bEnable );
	m_BtnWorkingOpen.EnableWindow( bEnable );

	m_EditTconRepeat.EnableWindow( bEnable );
	m_SpinTconRepeat.EnableWindow( bEnable );
	m_EditPfpgaRepeat.EnableWindow( bEnable );
	m_SpinPfpgaRepeat.EnableWindow( bEnable );
	GetDlgItem(IDC_RADIO_EEPROM_ALL)->EnableWindow( bEnable );
	GetDlgItem(IDC_RADIO_EEPROM_VERSION)->EnableWindow( bEnable );
	m_EditTamperRepeat.EnableWindow( bEnable );
	m_SpinTamperRepeat.EnableWindow( bEnable );
	m_IpAddr.EnableWindow( bEnable );
	m_EditNetworkRepeat.EnableWindow( bEnable );
	m_SpinNetworkRepeat.EnableWindow( bEnable );
}

void CNxCinemaDiagnosticsPanelDlg::ClearEditControl()
{
	m_EditStatusSap.SetWindowTextW(L"");
	m_EditProduct.SetWindowTextW(L"");
	m_EditSerial.SetWindowTextW(L"");
	m_EditCertificateText.SetWindowTextW(L"");
	m_EditCertificate.SetWindowTextW(L"");

	m_EditStatusNap.SetWindowTextW(L"");
	m_EditTconLeft.SetWindowTextW(L"");
	m_EditTconRight.SetWindowTextW(L"");
	m_EditPfpga.SetWindowTextW(L"");
	m_EditEEPRom.SetWindowTextW(L"");
	m_EditDoorTamper.SetWindowTextW(L"");
	m_EditMarriageTamper.SetWindowTextW(L"");
	m_EditNetwork.SetWindowTextW(L"");
}

void CNxCinemaDiagnosticsPanelDlg::LogPrint( const CString& szMsg, ... )
{
	CString szLog;
	va_list valist;
	va_start( valist, szMsg );
	szLog.Format( szMsg, valist );
	va_end(valist);

	int nLength = m_EditLog.GetWindowTextLengthW();
	m_EditLog.SetSel( nLength, nLength );
	m_EditLog.ReplaceSel( szLog + L"\r\n" );
	m_EditLog.LineScroll( m_EditLog.GetLineCount() );
}

void CNxCinemaDiagnosticsPanelDlg::LogClear()
{
	m_EditLog.SetReadOnly( FALSE );
	m_EditLog.SetSel( 0, -1 );
	m_EditLog.Clear();
	m_EditLog.SetReadOnly( TRUE );
}

void CNxCinemaDiagnosticsPanelDlg::LoadConfig()
{
	TCHAR tcApp[MAX_PATH];
	GetModuleFileName( NULL, tcApp, MAX_PATH );

	CString szConfig;
	szConfig.Format( L"%s", tcApp );
	szConfig = szConfig.Left( szConfig.ReverseFind('.') );
	szConfig += L".ini";

	TCHAR tcValue[MAX_PATH];

	CString szDefaultPath;
	szDefaultPath.Format( L"%s", tcApp );
	szDefaultPath = szDefaultPath.Left( szDefaultPath.ReverseFind('\\') );

	GetPrivateProfileString( L"app",	L"working",			szDefaultPath,		tcValue,	MAX_PATH,	szConfig );
	m_EditWorking.SetWindowTextW( (LPCTSTR)tcValue );
	m_EditWorking.SetSel( 0, -1 );
	m_EditWorking.SetSel( -1, -1 );
	m_ToolTip.AddTool( GetDlgItem(IDC_EDIT_WORKING), (LPCTSTR)tcValue );

	GetPrivateProfileString( L"app",	L"tcon_repeat",		L"16",				tcValue,	MAX_PATH,	szConfig );
	m_EditTconRepeat.SetWindowTextW( (LPCTSTR)tcValue );

	GetPrivateProfileString( L"app",	L"pfpga_repeat",	L"16",				tcValue,	MAX_PATH,	szConfig );
	m_EditPfpgaRepeat.SetWindowTextW( (LPCTSTR)tcValue );

	GetPrivateProfileString( L"app",	L"eeprom_access",	L"1",				tcValue,	MAX_PATH,	szConfig );
	int iValue = _wtoi( tcValue );
	CheckRadioButton( IDC_RADIO_EEPROM_ALL, IDC_RADIO_EEPROM_VERSION, (iValue == 0) ? IDC_RADIO_EEPROM_ALL : IDC_RADIO_EEPROM_VERSION );

	GetPrivateProfileString( L"app",	L"tamper_repeat",	L"8",				tcValue,	MAX_PATH,	szConfig );
	m_EditTamperRepeat.SetWindowTextW( (LPCTSTR)tcValue );

	GetPrivateProfileString( L"app",	L"target_ip",		L"192.168.1.108",	tcValue,	MAX_PATH,	szConfig );
	CString szField, szField0, szField1, szField2, szField3;
	BYTE nField0, nField1, nField2, nField3;

	szField.Format( L"%s", tcValue );

	AfxExtractSubString( szField0, szField, 0, '.' );
	AfxExtractSubString( szField1, szField, 1, '.' );
	AfxExtractSubString( szField2, szField, 2, '.' );
	AfxExtractSubString( szField3, szField, 3, '.' );
	
	nField0 = _wtoi( szField0 );
	nField1 = _wtoi( szField1 );
	nField2 = _wtoi( szField2 );
	nField3 = _wtoi( szField3 );

	m_IpAddr.SetAddress( nField0, nField1, nField2, nField3 );

	GetPrivateProfileString( L"app",	L"network_repeat",	L"8",				tcValue,	MAX_PATH,	szConfig );
	m_EditNetworkRepeat.SetWindowTextW( (LPCTSTR)tcValue );
}

void CNxCinemaDiagnosticsPanelDlg::SaveConfig()
{
	TCHAR tcPath[MAX_PATH];
	GetModuleFileName( NULL, tcPath, MAX_PATH );

	CString szConfig;
	szConfig.Format( L"%s", tcPath );
	szConfig = szConfig.Left( szConfig.ReverseFind('.') );
	szConfig += L".ini";

	CString szValue;

	m_EditWorking.GetWindowTextW( szValue );
	WritePrivateProfileStringW( L"app",		L"working",			szValue,	szConfig );

	m_EditTconRepeat.GetWindowTextW( szValue );
	WritePrivateProfileStringW( L"app",		L"tcon_repeat",		szValue,	szConfig );

	m_EditPfpgaRepeat.GetWindowTextW( szValue );
	WritePrivateProfileStringW( L"app",		L"pfpga_repeat",	szValue,	szConfig );

	int iChecked = GetCheckedRadioButton( IDC_RADIO_EEPROM_ALL, IDC_RADIO_EEPROM_VERSION );
	szValue.Format( L"%d", (iChecked == IDC_RADIO_EEPROM_ALL) ? 0 : 1 );
	WritePrivateProfileStringW( L"app",		L"eeprom_access",	szValue,	szConfig );

	m_EditTamperRepeat.GetWindowTextW( szValue );
	WritePrivateProfileStringW( L"app",		L"tamper_repeat",	szValue,	szConfig );

	BYTE nField0, nField1, nField2, nField3;
	m_IpAddr.GetAddress( nField0, nField1, nField2, nField3 );
	szValue.Format( L"%d.%d.%d.%d", nField0, nField1, nField2, nField3 );
	WritePrivateProfileStringW( L"app",		L"target_ip",		szValue,	szConfig );

	m_EditNetworkRepeat.GetWindowTextW( szValue );
	WritePrivateProfileStringW( L"app",		L"network_repeat",	szValue,	szConfig );
}

void CNxCinemaDiagnosticsPanelDlg::Run( void (*cbFunc)(void *pObj), void *pObj )
{
	m_ProcessCallbackFunc = cbFunc;
	m_pObj = pObj;
		
	m_pThread = AfxBeginThread( this->ThreadStub, this, NULL, NULL, NULL );
}

UINT CNxCinemaDiagnosticsPanelDlg::ThreadStub( LPVOID lpParam )
{
	CNxCinemaDiagnosticsPanelDlg *pDlg = (CNxCinemaDiagnosticsPanelDlg*)lpParam;
	if( pDlg )
		pDlg->ThreadProc();

	return 0xDEADDEAD;
}

void CNxCinemaDiagnosticsPanelDlg::ThreadProc( void )
{
	EnableControl( FALSE );

	if( m_ProcessCallbackFunc )
	{
		m_ProcessCallbackFunc( m_pObj );
	}

	EnableControl( TRUE );
}

void CNxCinemaDiagnosticsPanelDlg::RunDiagnostics( void *pObj )
{
	CNxCinemaDiagnosticsPanelDlg *pDlg = (CNxCinemaDiagnosticsPanelDlg*)pObj;
	
	Init( pObj );
	StopControlPanel( pObj );

	GetProduct( pObj );
	GetSerial( pObj );
	GetLeafCertificate( pObj );
	GetLeafCertificateText( pObj );

	TestTconLeft( pObj );
	TestTconRight( pObj );
	TestPfpga( pObj );
	TestEEPRom( pObj );
	
	TestMarriageTamper( pObj );
	TestDoorTamper( pObj );

	TestNetwork( pObj );

	StartControlPanel( pObj );
	Deinit( pObj );
}

void CNxCinemaDiagnosticsPanelDlg::RunRestartApplication( void *pObj )
{
	CNxCinemaDiagnosticsPanelDlg *pDlg = (CNxCinemaDiagnosticsPanelDlg*)pObj;

	Init( pObj );
	StopControlPanel( pObj );
	Sleep( 1000 );
	StartControlPanel( pObj );
	Deinit( pObj );

	pDlg->ClearEditControl();
}

void CNxCinemaDiagnosticsPanelDlg::RunReboot( void *pObj )
{
	CNxCinemaDiagnosticsPanelDlg *pDlg = (CNxCinemaDiagnosticsPanelDlg*)pObj;

	Init( pObj );

	if( pDlg->IsConnected( L"NAP" ) )
		pDlg->Execute( L"adb -s NAP shell reboot" );

	if( pDlg->IsConnected( L"SAP" ) )
		pDlg->Execute( L"adb -s SAP shell reboot" );

	Deinit( pObj );

	pDlg->ClearEditControl();
}

void CNxCinemaDiagnosticsPanelDlg::Init( void *pObj )
{
	CNxCinemaDiagnosticsPanelDlg *pDlg = (CNxCinemaDiagnosticsPanelDlg*)pObj;

	//pDlg->LogPrint( L"Start ADB." );

	CString szResult;
	pDlg->Execute( L"adb start-server" );
	pDlg->Execute( L"adb devices", &szResult ); 

	pDlg->m_InfoNum = 0;

	int iLIneCount = 0;
	do {
		CString szLine;
		if( !AfxExtractSubString( szLine, szResult, iLIneCount++, '\n' ) )
			break;
	
		szLine.Trim();

		if( !szLine.Compare( L"" ) || 0 <= szLine.Find( L"List of devices attached" ) )
			continue;

		int iTokenCount = 0;
		do {
			CString szTemp;
			if( !AfxExtractSubString( szTemp, szLine, iTokenCount, '\t' ) )
				break;

			szTemp.Trim();

			if( iTokenCount == 0 ) pDlg->m_Info[pDlg->m_InfoNum].szProduct = szTemp;
			if( iTokenCount == 1 ) pDlg->m_Info[pDlg->m_InfoNum].szStatus = szTemp;

			iTokenCount++;
		} while( TRUE );

		pDlg->m_InfoNum++;
	} while( TRUE );

	if( pDlg->IsConnected( L"NAP" ) )
	{
		szResult = L"";
		pDlg->m_EditStatusNap.SetWindowTextW( L"Connected" );
		pDlg->Execute( L"adb -s NAP root", &szResult );

		szResult.Trim();

		if( 0 <= szResult.Find( L"restarting adbd as root" ) )
		{
			// Change root permission at First.
			Sleep( 2000 );
		}
		
		TCHAR tcPath[MAX_PATH];
		GetModuleFileName( NULL, tcPath, MAX_PATH );

		CString szBin;
		szBin.Format( L"%s", tcPath );
		szBin = szBin.Left( szBin.ReverseFind('\\') );
		szBin += L"\\bin\\NxCinemaDiagnostics";
		
		CString szCommand;
		szCommand.Format( L"adb -s NAP push %s /system/bin/", szBin );
		pDlg->Execute( szCommand );
		pDlg->Execute( L"adb -s NAP shell sync" );

		szResult = L"";
		pDlg->Execute( L"adb -s NAP shell NxCinemaDiagnostics -v", &szResult );
		szResult.Trim();
		pDlg->m_EditVersion.SetWindowTextW( szResult );
	}
	else
	{
		pDlg->m_EditStatusNap.SetWindowTextW( L"Disconnected" );
	}

	if( pDlg->IsConnected( L"SAP" ) )
	{
		pDlg->m_EditStatusSap.SetWindowTextW( L"Connected" );
	}
	else
	{
		pDlg->m_EditStatusSap.SetWindowTextW( L"Disconnected" );
	}
}

void CNxCinemaDiagnosticsPanelDlg::Deinit( void *pObj )
{
	CNxCinemaDiagnosticsPanelDlg *pDlg = (CNxCinemaDiagnosticsPanelDlg*)pObj;

	pDlg->m_InfoNum = 0;

	pDlg->Execute( L"adb kill-server" );
	//pDlg->LogPrint( L"Stop ADB." );
}

void CNxCinemaDiagnosticsPanelDlg::StartControlPanel( void *pObj )
{
	CNxCinemaDiagnosticsPanelDlg *pDlg = (CNxCinemaDiagnosticsPanelDlg*)pObj;

	pDlg->Execute( L"adb -s NAP shell pm enable com.samsung.vd.cinemacontrolpanel" );
	pDlg->Execute( L"adb -s NAP shell am start -a android.intent.action.MAIN -n com.samsung.vd.cinemacontrolpanel/com.samsung.vd.cinemacontrolpanel.InitialActivity" );
	//pDlg->LogPrint( L"Start CinemaControlPanel." );
}

void CNxCinemaDiagnosticsPanelDlg::StopControlPanel( void *pObj )
{
	CNxCinemaDiagnosticsPanelDlg *pDlg = (CNxCinemaDiagnosticsPanelDlg*)pObj;

	pDlg->Execute( L"adb -s NAP shell pm disable com.samsung.vd.cinemacontrolpanel" );
	//pDlg->LogPrint( L"Stop CinemaControlPanel." );
}

BOOL CNxCinemaDiagnosticsPanelDlg::CheckFile( CString szFile, void *pObj )
{
	CNxCinemaDiagnosticsPanelDlg *pDlg = (CNxCinemaDiagnosticsPanelDlg*)pObj;

	CString szCommand, szResult;
	
	CString szPath, szName;
	szPath = szFile.Left( szFile.ReverseFind(L'/') );
	szName = szFile.Right( szFile.GetLength() - szFile.ReverseFind(L'/') - 1 );
	szCommand.Format( L"adb -s NAP shell busybox find %s -name %s",szPath.GetLength() ? szPath : L".", szName );

	pDlg->Execute( szCommand, &szResult );

	szResult.Trim();

	if( !szResult.Compare( szFile ) )
	{
		return TRUE;
	}
	
	return FALSE;
}

BOOL CNxCinemaDiagnosticsPanelDlg::CheckProcess( CString szProcess, void *pObj )
{
	CNxCinemaDiagnosticsPanelDlg *pDlg = (CNxCinemaDiagnosticsPanelDlg*)pObj;

	CString szCommand, szResult;

	//szCommand.Format( L"adb -s NAP shell ps | grep com.samsung.vd.cinemadiagnostics" );
	szCommand.Format( L"adb -s NAP shell ps | grep %s", szProcess );	// com.samsung.vd.cinemadiagnostics

	pDlg->Execute( szCommand, &szResult );

	szResult.Trim();
	if( 0 < szResult.GetLength() )
	{
		return TRUE;
	}

	return FALSE;
}

void CNxCinemaDiagnosticsPanelDlg::GetProduct( void *pObj )
{
	CNxCinemaDiagnosticsPanelDlg *pDlg = (CNxCinemaDiagnosticsPanelDlg*)pObj;

	if( !pDlg->IsConnected( L"SAP" ) )
	{
		pDlg->m_EditProduct.SetWindowTextW( L"FAIL" );
		return;
	}
	
	CString szCommand, szResult;
	szCommand.Format( L"adb -s SAP shell openssl x509 -text -in /mnt/mmc/bin/cert/leaf.signed.pem" );

	pDlg->Execute( szCommand, &szResult );

	int iStart, iEnd;
	iStart = szResult.Find( L"CN=PR SPB." );
	if( 0 <= iStart )
	{
		iEnd = szResult.Find( L".LED.PRODUCT", iStart);

		CString szTemp = szResult.Mid( iStart + 10, iEnd - (iStart + 10) );
		CString szProduct;
		AfxExtractSubString( szProduct, szTemp, 1, '.' );
		pDlg->m_EditProduct.SetWindowTextW( szProduct );
		return ;
	}

	pDlg->m_EditProduct.SetWindowTextW( L"FAIL" );
}

void CNxCinemaDiagnosticsPanelDlg::GetSerial( void *pObj )
{
	CNxCinemaDiagnosticsPanelDlg *pDlg = (CNxCinemaDiagnosticsPanelDlg*)pObj;
	
	if( !pDlg->IsConnected( L"SAP" ) )
	{
		pDlg->m_EditSerial.SetWindowTextW( L"FAIL" );
		return;
	}

	CString szCommand, szResult;
	szCommand.Format( L"adb -s SAP shell openssl x509 -text -in /mnt/mmc/bin/cert/leaf.signed.pem" );

	pDlg->Execute( szCommand, &szResult );

	int iStart, iEnd;
	iStart = szResult.Find( L"CN=PR SPB." );
	if( 0 <= iStart )
	{
		iEnd = szResult.Find( L".LED.PRODUCT", iStart);

		CString szTemp = szResult.Mid( iStart + 10, iEnd - (iStart + 10) );
		CString szSerial;
		AfxExtractSubString( szSerial, szTemp, 0, '.' );
		pDlg->m_EditSerial.SetWindowTextW( szSerial );
		return ;
	}

	pDlg->m_EditSerial.SetWindowTextW( L"FAIL" );
}

void CNxCinemaDiagnosticsPanelDlg::GetLeafCertificate( void *pObj )
{
	CNxCinemaDiagnosticsPanelDlg *pDlg = (CNxCinemaDiagnosticsPanelDlg*)pObj;

	if( !pDlg->IsConnected( L"SAP" ) )
	{
		pDlg->m_EditCertificate.SetWindowTextW( L"FAIL" );
		return;
	}

	CString szSerial;
	pDlg->m_EditSerial.GetWindowText( szSerial );

	if( !szSerial.Trim().Compare( L"" ) )
	{
		pDlg->m_EditCertificate.SetWindowTextW( L"FAIL" );
		return ;
	}

	CString szWorking;
	pDlg->m_EditWorking.GetWindowTextW( szWorking );

	szWorking += L"\\";
	szWorking += szSerial;
	szWorking += L"\\";
	
	CreateDirectory( szWorking, NULL );

	CString szCommand, szResult;
	szCommand.Format( L"adb -s SAP pull /mnt/mmc/bin/cert/leaf.signed.pem %sleaf.signed.pem", szWorking );

	pDlg->Execute( szCommand, &szResult );

	if( 0 <= szResult.Find( L"does not exist" ) )
	{
		pDlg->m_EditCertificate.SetWindowTextW( L"FAIL" );
	}

	pDlg->m_EditCertificate.SetWindowTextW( L"DONE" );
}

void CNxCinemaDiagnosticsPanelDlg::GetLeafCertificateText( void *pObj )
{
	CNxCinemaDiagnosticsPanelDlg *pDlg = (CNxCinemaDiagnosticsPanelDlg*)pObj;

	if( !pDlg->IsConnected( L"SAP" ) )
	{
		pDlg->m_EditCertificateText.SetWindowTextW( L"FAIL" );
		return;
	}

	CString szSerial;
	pDlg->m_EditSerial.GetWindowText( szSerial );

	if( !szSerial.Trim().Compare( L"" ) )
	{
		pDlg->m_EditCertificateText.SetWindowTextW( L"FAIL" );
		return ;
	}

	CString szWorking;
	pDlg->m_EditWorking.GetWindowTextW( szWorking );

	szWorking += L"\\";
	szWorking += szSerial;
	szWorking += L"\\";
	
	CreateDirectory( szWorking, NULL );

	CString szCommand;
	CString szResult;
	szCommand.Format( L"adb -s SAP shell openssl x509 -text -in /mnt/mmc/bin/cert/leaf.signed.pem" );

	pDlg->Execute( szCommand, &szResult );

	if( 0 <= szResult.Find( L"unable to load certificate" ) )
	{
		pDlg->m_EditCertificateText.SetWindowTextW( L"FAIL" );
		return;
	}

	szWorking += L"leaf.signed.txt";
	
	CFile file;
	file.Open( szWorking, CFile::modeCreate | CFile::modeWrite, NULL );
	file.Write( szResult, szResult.GetLength()*sizeof(TCHAR));
	file.Close();

	pDlg->m_EditCertificateText.SetWindowTextW( L"DONE" );
}

void CNxCinemaDiagnosticsPanelDlg::TestTconLeft( void *pObj )
{
	CNxCinemaDiagnosticsPanelDlg *pDlg = (CNxCinemaDiagnosticsPanelDlg*)pObj;

	if( !pDlg->IsConnected( L"NAP" ) )
	{
		pDlg->m_EditTconLeft.SetWindowText( L"FAIL" );
		return;
	}

	CString szRepeat;
	pDlg->m_EditTconRepeat.GetWindowTextW( szRepeat );

	for( int i = 0; i < _wtoi( szRepeat ); i++ )
	{
		CString szLog;
		szLog.Format( L"check: %d", i+1 );
		pDlg->m_EditTconLeft.SetWindowText( szLog );

		CString szResult;
		pDlg->Execute( L"adb -s NAP shell NxCinemaDiagnostics -c 0", &szResult );
		
		if( pDlg->IsFail( szResult ) )
		{
			pDlg->m_EditTconLeft.SetWindowText( L"FAIL" );
			return;
		}
	}

	pDlg->m_EditTconLeft.SetWindowText( L"PASS" );
}

void CNxCinemaDiagnosticsPanelDlg::TestTconRight( void *pObj )
{
	CNxCinemaDiagnosticsPanelDlg *pDlg = (CNxCinemaDiagnosticsPanelDlg*)pObj;

	if( !pDlg->IsConnected( L"NAP" ) )
	{
		pDlg->m_EditTconRight.SetWindowText( L"FAIL" );
		return;
	}

	CString szRepeat;
	pDlg->m_EditTconRepeat.GetWindowTextW( szRepeat );

	for( int i = 0; i < _wtoi( szRepeat ); i++ )
	{
		CString szLog;
		szLog.Format( L"check: %d", i+1 );
		pDlg->m_EditTconRight.SetWindowText( szLog );

		CString szResult = L"";
		pDlg->Execute( L"adb -s NAP shell NxCinemaDiagnostics -c 1", &szResult );
		
		if( pDlg->IsFail( szResult ) )
		{
			pDlg->m_EditTconRight.SetWindowText( L"FAIL" );
			return;
		}
	}

	pDlg->m_EditTconRight.SetWindowText( L"PASS" );
}

void CNxCinemaDiagnosticsPanelDlg::TestPfpga( void *pObj )
{
	CNxCinemaDiagnosticsPanelDlg *pDlg = (CNxCinemaDiagnosticsPanelDlg*)pObj;

	if( !pDlg->IsConnected( L"NAP" ) )
	{
		pDlg->m_EditPfpga.SetWindowText( L"FAIL" );
		return;
	}

	CString szRepeat;
	pDlg->m_EditPfpgaRepeat.GetWindowTextW( szRepeat );

	for( int i = 0; i < _wtoi( szRepeat ); i++ )
	{
		CString szLog;
		szLog.Format( L"check: %d", i+1 );
		pDlg->m_EditPfpga.SetWindowText( szLog );

		CString szResult = L"";
		pDlg->Execute( L"adb -s NAP shell NxCinemaDiagnostics -c 2", &szResult );
		
		if( pDlg->IsFail( szResult ) )
		{
			pDlg->m_EditPfpga.SetWindowText( L"FAIL" );
			return;
		}
	}

	pDlg->m_EditPfpga.SetWindowText( L"PASS" );
}

void CNxCinemaDiagnosticsPanelDlg::TestEEPRom( void *pObj )
{
	CNxCinemaDiagnosticsPanelDlg *pDlg = (CNxCinemaDiagnosticsPanelDlg*)pObj;

	if( !pDlg->IsConnected( L"NAP" ) )
	{
		pDlg->m_EditEEPRom.SetWindowText( L"FAIL" );
		return;
	}

	int iChecked = pDlg->GetCheckedRadioButton( IDC_RADIO_EEPROM_ALL, IDC_RADIO_EEPROM_VERSION );

	CString szCommand, szResult;
	szCommand.Format( L"adb -s NAP shell NxCinemaDiagnostics -c 3 -t %d", (iChecked == IDC_RADIO_EEPROM_ALL) ? 0 : 1);

	pDlg->Execute( szCommand, &szResult );

	if( pDlg->IsFail( szResult ) )
	{
		pDlg->m_EditEEPRom.SetWindowTextW( L"FAIL" );
		return;
	}

	szResult.Trim();
	szResult = szResult.Mid( szResult.ReverseFind( L'\n' ), szResult.GetLength() - szResult.ReverseFind( L'\n' ) );
	pDlg->m_EditEEPRom.SetWindowTextW( szResult );
}

void CNxCinemaDiagnosticsPanelDlg::TestMarriageTamper( void *pObj )
{
	CNxCinemaDiagnosticsPanelDlg *pDlg = (CNxCinemaDiagnosticsPanelDlg*)pObj;

	if( !pDlg->IsConnected( L"NAP" ) )
	{
		pDlg->m_EditMarriageTamper.SetWindowText( L"FAIL" );
		return;
	}

	CString szRepeat;
	pDlg->m_EditTamperRepeat.GetWindowTextW( szRepeat );

	for( int i = 0; i < _wtoi( szRepeat ); i++ )
	{
		CString szLog;
		szLog.Format( L"check: %d", i+1 );
		pDlg->m_EditMarriageTamper.SetWindowText( szLog );

		CString szResult = L"";
		pDlg->Execute( L"adb -s NAP shell NxCinemaDiagnostics -c 4", &szResult );
		
		if( pDlg->IsPass( szResult ) )
		{
			pDlg->m_EditMarriageTamper.SetWindowText( L"PASS" );
			return;
		}
	}

	pDlg->m_EditMarriageTamper.SetWindowText( L"FAIL" );
}

void CNxCinemaDiagnosticsPanelDlg::TestDoorTamper( void *pObj )
{
	CNxCinemaDiagnosticsPanelDlg *pDlg = (CNxCinemaDiagnosticsPanelDlg*)pObj;

	if( !pDlg->IsConnected( L"NAP" ) )
	{
		pDlg->m_EditDoorTamper.SetWindowText( L"FAIL" );
		return;
	}

	CString szRepeat;
	pDlg->m_EditTamperRepeat.GetWindowTextW( szRepeat );

	for( int i = 0; i < _wtoi( szRepeat ); i++ )
	{
		CString szLog;
		szLog.Format( L"check: %d", i+1 );
		pDlg->m_EditDoorTamper.SetWindowText( szLog );

		CString szResult = L"";
		pDlg->Execute( L"adb -s NAP shell NxCinemaDiagnostics -c 5", &szResult );
		
		szResult.Trim();
		if( pDlg->IsPass( szResult ) )
		{
			pDlg->m_EditDoorTamper.SetWindowText( L"PASS" );
			return;
		}
	}

	pDlg->m_EditDoorTamper.SetWindowText( L"FAIL" );
}

void CNxCinemaDiagnosticsPanelDlg::TestNetwork( void *pObj )
{
	CNxCinemaDiagnosticsPanelDlg *pDlg = (CNxCinemaDiagnosticsPanelDlg*)pObj;

	if( !pDlg->IsConnected( L"NAP" ) )
	{
		pDlg->m_EditNetwork.SetWindowText( L"FAIL" );
		return;
	}

	CString szRepeat;
	pDlg->m_EditNetworkRepeat.GetWindowTextW( szRepeat );

	for( int i = 0; i < _wtoi( szRepeat ); i++ )
	{
		CString szLog;
		szLog.Format( L"check: %d", i+1 );
		pDlg->m_EditNetwork.SetWindowText( szLog );

		CString szResult = L"";
		CString szCommand;

		BYTE nField0, nField1, nField2, nField3;
		pDlg->m_IpAddr.GetAddress( nField0, nField1, nField2, nField3 );

		szCommand.Format( L"adb -s NAP shell NxCinemaDiagnostics -c 6 -i %d.%d.%d.%d", nField0, nField1, nField2, nField3);
		pDlg->Execute( szCommand, &szResult );

		if( pDlg->IsFail( szResult ) )
		{
			pDlg->m_EditNetwork.SetWindowText( L"FAIL" );
			return;
		}

		if( pDlg->IsPass( szResult ) )
		{
			if( 0 <= szResult.Find( L"Unknown ICMP message received" ) )
			{
				pDlg->m_EditNetwork.SetWindowText( L"FAIL" );
				return;
			}
		}
	}

	pDlg->m_EditNetwork.SetWindowText( L"PASS" );
}
