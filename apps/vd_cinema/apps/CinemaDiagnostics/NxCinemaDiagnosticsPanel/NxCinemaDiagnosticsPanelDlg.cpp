
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
	DDX_Control(pDX, IDC_BTN_CLEAR, m_BtnClear);
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
	ON_BN_CLICKED(IDC_BTN_WORKING_SELECT, &CNxCinemaDiagnosticsPanelDlg::OnBnClickedBtnWorkingSelect)
	ON_BN_CLICKED(IDC_BTN_WORKING_OPEN, &CNxCinemaDiagnosticsPanelDlg::OnBnClickedBtnWorkingOpen)
	ON_BN_CLICKED(IDC_BTN_TEST, &CNxCinemaDiagnosticsPanelDlg::OnBnClickedBtnTest)
	ON_BN_CLICKED(IDC_BTN_RUN, &CNxCinemaDiagnosticsPanelDlg::OnBnClickedBtnRun)
	ON_BN_CLICKED(IDC_BTN_RESTART_APP, &CNxCinemaDiagnosticsPanelDlg::OnBnClickedBtnRestartApp)
	ON_BN_CLICKED(IDC_BTN_REBOOT, &CNxCinemaDiagnosticsPanelDlg::OnBnClickedBtnReboot)
	ON_BN_CLICKED(IDC_BTN_CLEAR, &CNxCinemaDiagnosticsPanelDlg::OnBnClickedBtnClear)
END_MESSAGE_MAP()


// CNxCinemaDiagnosticsPanelDlg 메시지 처리기

#ifdef _DEBUG
#include <locale.h>
#endif

//
//	Version Information
//	* Version 0.3.0 at 2018.09.28
//	  -. expand number of log text. 
//	  -. add log rotation function. ( disabled )
//
//	* Version 0.2.0 at 2018.07.03
//	  -. fixed bug related working directory. ( the working directory is default directory when directory is not exist. )
//	  -. add error case. ( device not found )
//    -. add debug options.
//	  -. add to skip test. ( repeat number is zero, select none menu )
//
//	* Version 0.1.0 at 2018.06.12
//	  -. First Draft.
//

#define NX_DIAG_VERSION		L"v0.2.0"

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
	this->GetText( this, szTitle );
	this->SetText( this, L"%s %s", szTitle, NX_DIAG_VERSION );

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

	m_EditLog.SetLimitText( UINT_MAX );    
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

#include "BrowseFolderDialog.h"

void CNxCinemaDiagnosticsPanelDlg::OnBnClickedBtnWorkingSelect()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString szWorking;
	GetText( &m_EditWorking, szWorking );

	CBrowseFolderDialog dlg;
	if( dlg.DoModal(szWorking) == IDOK )
	{
		dlg.GetSelectStr( szWorking );
		SetText( &m_EditWorking, szWorking );
		
		m_EditWorking.SetSel( 0, -1 );
		m_EditWorking.SetSel( -1, -1 );
		m_ToolTip.AddTool( GetDlgItem(IDC_EDIT_WORKING), szWorking );
	}
}

void CNxCinemaDiagnosticsPanelDlg::OnBnClickedBtnWorkingOpen()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString szWorking;
	GetText( &m_EditWorking, szWorking );

	ShellExecute( NULL, L"open", L"explorer.exe",  szWorking, NULL, SW_SHOW ); 
}

void CNxCinemaDiagnosticsPanelDlg::OnBnClickedBtnTest()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	ClearEditControl();
	LogClear();

	Run( &RunTest, this );
}

void CNxCinemaDiagnosticsPanelDlg::OnBnClickedBtnRun()    
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	ClearEditControl();
	LogClear();

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

void CNxCinemaDiagnosticsPanelDlg::OnBnClickedBtnClear()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	LogClear();
}

BOOL CNxCinemaDiagnosticsPanelDlg::Execute( LPCTSTR szFormat, ... )
{
	CString szTemp, szResult;
	va_list args;
	va_start( args, szFormat );
	szTemp.FormatV( szFormat, args );
	va_end( args );

	return Execute( szResult, szTemp );
}

BOOL CNxCinemaDiagnosticsPanelDlg::Execute( CString &szResult, LPCTSTR szFormat, ... )
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

	CString szTemp;
	va_list args;
	va_start( args, szFormat );
	szTemp.FormatV( szFormat, args );
	va_end( args );

	TCHAR tcPath[MAX_PATH];
	GetModuleFileName( NULL, tcPath, MAX_PATH );

	CString szCommand;
	szCommand.Format( L"%s", tcPath );
	szCommand = szCommand.Left( szCommand.ReverseFind('\\') );
	szCommand += L"\\bin\\";
	szCommand += szTemp;

	int iRet = CreateProcess( NULL, (TCHAR*)(LPCTSTR)szCommand, NULL, NULL, TRUE, 0, NULL, NULL, &startupInfo, &processInfo );
	if( FALSE == iRet )
	{
		return FALSE;
	}

	szResult = L"";
	
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
					szResult += szTemp;
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
	return ( 0 <= szResult.Find( NX_RET_PASS )) ? TRUE : FALSE;
}

BOOL CNxCinemaDiagnosticsPanelDlg::IsFail( CString szResult )
{
	return ( 0 <= szResult.Find( NX_RET_FAIL )) ? TRUE : FALSE;
}

BOOL CNxCinemaDiagnosticsPanelDlg::IsError( CString szResult )
{
	return ( 0 <= szResult.Find( NX_RET_ERROR )) ? TRUE : FALSE;
}

BOOL CNxCinemaDiagnosticsPanelDlg::IsDebug()
{
	return (GetCheckedRadioButton( IDC_RADIO_DEBUG_ON, IDC_RADIO_DEBUG_OFF ) == IDC_RADIO_DEBUG_ON);
}

void CNxCinemaDiagnosticsPanelDlg::EnableControl( BOOL bEnable )
{
	m_BtnRun.EnableWindow( bEnable );
	m_BtnClear.EnableWindow( bEnable );
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
	GetDlgItem(IDC_RADIO_EEPROM_NONE)->EnableWindow( bEnable );
	GetDlgItem(IDC_RADIO_DEBUG_ON)->EnableWindow( bEnable );
	GetDlgItem(IDC_RADIO_DEBUG_OFF)->EnableWindow( bEnable );
	m_EditTamperRepeat.EnableWindow( bEnable );
	m_SpinTamperRepeat.EnableWindow( bEnable );
	m_IpAddr.EnableWindow( bEnable );
	m_EditNetworkRepeat.EnableWindow( bEnable );
	m_SpinNetworkRepeat.EnableWindow( bEnable );
}

void CNxCinemaDiagnosticsPanelDlg::ClearEditControl()
{
	SetText( &m_EditStatusSap,		L"" );
	SetText( &m_EditProduct,		L"" );
	SetText( &m_EditSerial,			L"" );
	SetText( &m_EditCertificateText,L"" );
	SetText( &m_EditCertificate,	L"" );

	SetText( &m_EditStatusNap,		L"" );
	SetText( &m_EditTconLeft,		L"" );
	SetText( &m_EditTconRight,		L"" );
	SetText( &m_EditPfpga,			L"" );
	SetText( &m_EditEEPRom,			L"" );
	SetText( &m_EditDoorTamper,		L"" );
	SetText( &m_EditMarriageTamper, L"" );
	SetText( &m_EditNetwork,		L"" );
}

void CNxCinemaDiagnosticsPanelDlg::SetText( CWnd *pWnd, LPCTSTR szFormat, ... )
{
	CString szText;
	va_list args;
	va_start( args, szFormat );
	szText.FormatV( szFormat, args );
	va_end( args );

	if( pWnd ) pWnd->SetWindowTextW( szText );
}

void CNxCinemaDiagnosticsPanelDlg::GetText( CWnd *pWnd, CString &szResult )
{
	if( pWnd ) pWnd->GetWindowTextW( szResult );
}

void CNxCinemaDiagnosticsPanelDlg::LogPrint( LPCTSTR szFormat, ... )
{
	CString szText;

	va_list args;
	va_start( args, szFormat );
	szText.FormatV( szFormat, args );
	va_end( args );

#if NX_ENABLE_LOG_ROTATION
	while( m_EditLog.GetWindowTextLengthW() > MAX_LOG_TEXT || m_EditLog.GetLineCount() > MAX_LOG_LINE )
	{
		CString szTemp;
		m_EditLog.GetWindowTextW( szTemp );
		
		int nLineEnd = szTemp.Find( L"\n", 0 );

		m_EditLog.SetSel( 0, nLineEnd+1, TRUE );
		m_EditLog.ReplaceSel( L"" );
	}
#endif

	int nLength = m_EditLog.GetWindowTextLengthW();
	m_EditLog.SetSel( nLength, nLength );
	m_EditLog.ReplaceSel( szText );
	m_EditLog.LineScroll( m_EditLog.GetLineCount() );
}

void CNxCinemaDiagnosticsPanelDlg::LogPrint( BOOL bEnable, LPCTSTR szFormat, ... )
{
	CString szText;

	va_list args;
	va_start( args, szFormat );
	szText.FormatV( szFormat, args );
	va_end( args );

	if( bEnable ) LogPrint( szText );
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

	int iValue;
	TCHAR tcValue[MAX_PATH];

	CString szDefaultPath;
	szDefaultPath.Format( L"%s", tcApp );
	szDefaultPath = szDefaultPath.Left( szDefaultPath.ReverseFind('\\') );

	GetPrivateProfileString( L"app",	L"working",			szDefaultPath,		tcValue,	MAX_PATH,	szConfig );
	SetText( &m_EditWorking, (INVALID_FILE_ATTRIBUTES == GetFileAttributes( (LPCTSTR)tcValue ))  ? szDefaultPath : (LPCTSTR)tcValue );
	m_ToolTip.AddTool( GetDlgItem(IDC_EDIT_WORKING), (INVALID_FILE_ATTRIBUTES == GetFileAttributes( (LPCTSTR)tcValue ))  ? szDefaultPath : (LPCTSTR)tcValue );
	m_EditWorking.SetSel( 0, -1 );
	m_EditWorking.SetSel( -1, -1 );

	GetPrivateProfileString( L"app",	L"tcon_repeat",		L"16",				tcValue,	MAX_PATH,	szConfig );
	SetText( &m_EditTconRepeat, (LPCTSTR)tcValue );

	GetPrivateProfileString( L"app",	L"pfpga_repeat",	L"16",				tcValue,	MAX_PATH,	szConfig );
	SetText( &m_EditPfpgaRepeat, (LPCTSTR)tcValue );

	GetPrivateProfileString( L"app",	L"eeprom_access",	L"1",				tcValue,	MAX_PATH,	szConfig );
	iValue = _wtoi( tcValue );
	CheckRadioButton( IDC_RADIO_EEPROM_ALL, IDC_RADIO_EEPROM_NONE, (iValue == 0) ? IDC_RADIO_EEPROM_ALL : ((iValue == 1) ? IDC_RADIO_EEPROM_VERSION : IDC_RADIO_EEPROM_NONE));

	GetPrivateProfileString( L"app",	L"tamper_repeat",	L"8",				tcValue,	MAX_PATH,	szConfig );
	SetText( &m_EditTamperRepeat, (LPCTSTR)tcValue );

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
	SetText( &m_EditNetworkRepeat, (LPCTSTR)tcValue );

	GetPrivateProfileString( L"app",	L"debug",			L"0",				tcValue,	MAX_PATH,	szConfig );
	iValue = _wtoi( tcValue );
	CheckRadioButton( IDC_RADIO_DEBUG_ON, IDC_RADIO_DEBUG_OFF, (iValue == 0) ? IDC_RADIO_DEBUG_OFF : IDC_RADIO_DEBUG_ON );
}

void CNxCinemaDiagnosticsPanelDlg::SaveConfig()
{
	TCHAR tcPath[MAX_PATH];
	GetModuleFileName( NULL, tcPath, MAX_PATH );

	CString szConfig;
	szConfig.Format( L"%s", tcPath );
	szConfig = szConfig.Left( szConfig.ReverseFind('.') );
	szConfig += L".ini";

	int iChecked;
	CString szValue;

	GetText( &m_EditWorking, szValue );
	WritePrivateProfileStringW( L"app",		L"working",			szValue,	szConfig );

	GetText( &m_EditTconRepeat, szValue );
	WritePrivateProfileStringW( L"app",		L"tcon_repeat",		szValue,	szConfig );

	GetText( &m_EditPfpgaRepeat, szValue );
	WritePrivateProfileStringW( L"app",		L"pfpga_repeat",	szValue,	szConfig );

	iChecked = GetCheckedRadioButton( IDC_RADIO_EEPROM_ALL, IDC_RADIO_EEPROM_NONE );
	szValue.Format( L"%d", (iChecked == IDC_RADIO_EEPROM_ALL) ? 0 : ((iChecked == IDC_RADIO_EEPROM_VERSION) ? 1 : 2) );
	WritePrivateProfileStringW( L"app",		L"eeprom_access",	szValue,	szConfig );

	GetText( &m_EditTamperRepeat, szValue );
	WritePrivateProfileStringW( L"app",		L"tamper_repeat",	szValue,	szConfig );

	BYTE nField0, nField1, nField2, nField3;
	m_IpAddr.GetAddress( nField0, nField1, nField2, nField3 );
	szValue.Format( L"%d.%d.%d.%d", nField0, nField1, nField2, nField3 );
	WritePrivateProfileStringW( L"app",		L"target_ip",		szValue,	szConfig );

	GetText( &m_EditNetworkRepeat, szValue );
	WritePrivateProfileStringW( L"app",		L"network_repeat",	szValue,	szConfig );

	iChecked = GetCheckedRadioButton( IDC_RADIO_DEBUG_ON, IDC_RADIO_DEBUG_OFF );
	szValue.Format( L"%d", (iChecked == IDC_RADIO_DEBUG_OFF) ? 0 : 1 );
	WritePrivateProfileStringW( L"app",		L"debug",			szValue,	szConfig );
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

void CNxCinemaDiagnosticsPanelDlg::RunTest( void *pObj )
{
	CNxCinemaDiagnosticsPanelDlg *pDlg = (CNxCinemaDiagnosticsPanelDlg*)pObj;

	Init( pObj );
	StopControlPanel( pObj );

	for( int i = 0; i < 100; i++ )
	{
		TestTconLeft( pObj );
		TestTconRight( pObj );
	}

	StartControlPanel( pObj );
	Deinit( pObj );
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
	pDlg->m_InfoNum = 0;

	pDlg->LogPrint( pDlg->IsDebug(), NX_PERFORATED_LINE );
	pDlg->LogPrint( pDlg->IsDebug(), L"Start ADB.\r\n" );

	CString szResult;
	pDlg->Execute( L"adb start-server" );
	pDlg->Execute( szResult, L"adb devices" ); 

	szResult.Trim();
	pDlg->LogPrint( pDlg->IsDebug(), szResult + L"\r\n" );

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
		pDlg->SetText( &pDlg->m_EditStatusNap, L"Connected" );
		pDlg->Execute( szResult, L"adb -s NAP root" );

		szResult.Trim();
		pDlg->LogPrint( pDlg->IsDebug(), NX_PERFORATED_LINE );
		pDlg->LogPrint( pDlg->IsDebug(), L"Change Root.\r\n" );

		if( 0 <= szResult.Find( L"restarting adbd as root" ) )
		{
			// Change root permission at First.
			int iWaitCount = 0;
			do {
				iWaitCount++;
				pDlg->LogPrint( pDlg->IsDebug(), L"wait for changing root permission. ( %d sec / %d sec )\r\n", iWaitCount, MAX_WAIT_TIME );

				Sleep( 1000 );
			} while(iWaitCount < MAX_WAIT_TIME );
		}
		else
		{
			pDlg->LogPrint( pDlg->IsDebug(), L"already root permission.\r\n" );
		}
		
		TCHAR tcPath[MAX_PATH];
		GetModuleFileName( NULL, tcPath, MAX_PATH );

		CString szBin;
		szBin.Format( L"%s", tcPath );
		szBin = szBin.Left( szBin.ReverseFind('\\') );
		szBin += L"\\bin\\NxCinemaDiagnostics";
		
		pDlg->Execute( szResult, L"adb -s NAP push %s /system/bin/", szBin );
		pDlg->Execute( L"adb -s NAP shell sync" );
		
		szResult.Trim();
		pDlg->LogPrint( pDlg->IsDebug(), NX_PERFORATED_LINE );
		pDlg->LogPrint( pDlg->IsDebug(), L"Push Diagnostics.\r\n" );
		pDlg->LogPrint( pDlg->IsDebug(), szResult + L"\r\n" );

		pDlg->Execute( szResult, L"adb -s NAP shell NxCinemaDiagnostics -v" );
		
		szResult.Trim();
		pDlg->SetText( &pDlg->m_EditVersion, szResult );
	}
	else
	{
		pDlg->SetText( &pDlg->m_EditStatusNap, L"Disconnected" );
	}

	if( pDlg->IsConnected( L"SAP" ) )
	{
		pDlg->SetText( &pDlg->m_EditStatusSap, L"Connected" );
	}
	else
	{
		pDlg->SetText( &pDlg->m_EditStatusSap, L"Disconnected" );
	}
}

void CNxCinemaDiagnosticsPanelDlg::Deinit( void *pObj )
{
	CNxCinemaDiagnosticsPanelDlg *pDlg = (CNxCinemaDiagnosticsPanelDlg*)pObj;

	pDlg->m_InfoNum = 0;
	pDlg->Execute( L"adb kill-server" );

	pDlg->LogPrint( pDlg->IsDebug(), NX_PERFORATED_LINE );
	pDlg->LogPrint( pDlg->IsDebug(), L"Stop ADB.\r\n" );
}

void CNxCinemaDiagnosticsPanelDlg::StartControlPanel( void *pObj )
{
	CNxCinemaDiagnosticsPanelDlg *pDlg = (CNxCinemaDiagnosticsPanelDlg*)pObj;

	pDlg->Execute( L"adb -s NAP shell pm enable com.samsung.vd.cinemacontrolpanel" );
	pDlg->Execute( L"adb -s NAP shell am start -a android.intent.action.MAIN -n com.samsung.vd.cinemacontrolpanel/com.samsung.vd.cinemacontrolpanel.InitialActivity" );

	pDlg->LogPrint( pDlg->IsDebug(), NX_PERFORATED_LINE );
	pDlg->LogPrint( pDlg->IsDebug(), L"Start CinemaControlPanel.\r\n" );
}

void CNxCinemaDiagnosticsPanelDlg::StopControlPanel( void *pObj )
{
	CNxCinemaDiagnosticsPanelDlg *pDlg = (CNxCinemaDiagnosticsPanelDlg*)pObj;

	pDlg->Execute( L"adb -s NAP shell pm disable com.samsung.vd.cinemacontrolpanel" );

	pDlg->LogPrint( pDlg->IsDebug(), NX_PERFORATED_LINE );
	pDlg->LogPrint( pDlg->IsDebug(), L"Stop CinemaControlPanel.\r\n" );
}

BOOL CNxCinemaDiagnosticsPanelDlg::CheckFile( CString szFile, void *pObj )
{
	CNxCinemaDiagnosticsPanelDlg *pDlg = (CNxCinemaDiagnosticsPanelDlg*)pObj;

	CString szPath, szName;
	szPath = szFile.Left( szFile.ReverseFind(L'/') );
	szName = szFile.Right( szFile.GetLength() - szFile.ReverseFind(L'/') - 1 );

	CString szResult;
	pDlg->Execute( szResult, L"adb -s NAP shell busybox find %s -name %s",szPath.GetLength() ? szPath : L".", szName );

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

	CString szResult;
	//pDlg->Execute( szResult, L"adb -s NAP shell ps | grep com.samsung.vd.cinemadiagnostics" );
	pDlg->Execute( szResult, L"adb -s NAP shell ps | grep %s", szProcess );	// com.samsung.vd.cinemadiagnostics

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
		pDlg->SetText( &pDlg->m_EditProduct, NX_RET_FAIL );
		return;
	}
	
	CString szResult;
	pDlg->Execute( szResult, L"adb -s SAP shell openssl x509 -text -in /mnt/mmc/bin/cert/leaf.signed.pem" );

	int iStart, iEnd;
	iStart = szResult.Find( L"CN=PR SPB." );
	if( 0 <= iStart )
	{
		iEnd = szResult.Find( L".LED.PRODUCT", iStart);

		CString szTemp = szResult.Mid( iStart + 10, iEnd - (iStart + 10) );
		CString szProduct;
		AfxExtractSubString( szProduct, szTemp, 1, '.' );
		pDlg->SetText( &pDlg->m_EditProduct, szProduct );
		return ;
	}

	pDlg->SetText( &pDlg->m_EditProduct, NX_RET_FAIL );
}

void CNxCinemaDiagnosticsPanelDlg::GetSerial( void *pObj )
{
	CNxCinemaDiagnosticsPanelDlg *pDlg = (CNxCinemaDiagnosticsPanelDlg*)pObj;
	
	if( !pDlg->IsConnected( L"SAP" ) )
	{
		pDlg->SetText( &pDlg->m_EditSerial, NX_RET_FAIL );
		return;
	}

	CString szResult;
	pDlg->Execute( szResult, L"adb -s SAP shell openssl x509 -text -in /mnt/mmc/bin/cert/leaf.signed.pem" );

	int iStart, iEnd;
	iStart = szResult.Find( L"CN=PR SPB." );
	if( 0 <= iStart )
	{
		iEnd = szResult.Find( L".LED.PRODUCT", iStart);

		CString szTemp = szResult.Mid( iStart + 10, iEnd - (iStart + 10) );
		CString szSerial;
		AfxExtractSubString( szSerial, szTemp, 0, '.' );
		pDlg->SetText( &pDlg->m_EditSerial, szSerial );
		return ;
	}

	pDlg->SetText( &pDlg->m_EditSerial, NX_RET_FAIL );
}

void CNxCinemaDiagnosticsPanelDlg::GetLeafCertificate( void *pObj )
{
	CNxCinemaDiagnosticsPanelDlg *pDlg = (CNxCinemaDiagnosticsPanelDlg*)pObj;

	if( !pDlg->IsConnected( L"SAP" ) )
	{
		pDlg->SetText( &pDlg->m_EditCertificate, NX_RET_FAIL );
		return;
	}

	CString szSerial;
	pDlg->GetText( &pDlg->m_EditSerial, szSerial );

	if( !szSerial.Trim().Compare( L"" ) )
	{
		pDlg->SetText( &pDlg->m_EditCertificate, NX_RET_FAIL );
		return ;
	}

	CString szWorking;
	pDlg->GetText( &pDlg->m_EditWorking, szWorking );

	szWorking += L"\\";
	szWorking += szSerial;
	szWorking += L"\\";
	
	CreateDirectory( szWorking, NULL );

	CString szResult;
	pDlg->Execute( szResult,  L"adb -s SAP pull /mnt/mmc/bin/cert/leaf.signed.pem %sleaf.signed.pem", szWorking );

	if( 0 <= szResult.Find( L"does not exist" ) )
	{
		pDlg->SetText( &pDlg->m_EditCertificate, NX_RET_FAIL );
		return;
	}

	pDlg->SetText( &pDlg->m_EditCertificate, L"DONE" );
}

void CNxCinemaDiagnosticsPanelDlg::GetLeafCertificateText( void *pObj )
{
	CNxCinemaDiagnosticsPanelDlg *pDlg = (CNxCinemaDiagnosticsPanelDlg*)pObj;

	if( !pDlg->IsConnected( L"SAP" ) )
	{
		pDlg->SetText( &pDlg->m_EditCertificateText, NX_RET_FAIL );
		return;
	}

	CString szSerial;
	pDlg->GetText( &pDlg->m_EditSerial, szSerial );

	if( !szSerial.Trim().Compare( L"" ) )
	{
		pDlg->SetText( &pDlg->m_EditCertificateText, NX_RET_FAIL );
		return ;
	}

	CString szWorking;
	pDlg->GetText( &pDlg->m_EditWorking, szWorking );

	szWorking += L"\\";
	szWorking += szSerial;
	szWorking += L"\\";
	
	CreateDirectory( szWorking, NULL );

	CString szResult;
	pDlg->Execute( szResult, L"adb -s SAP shell openssl x509 -text -in /mnt/mmc/bin/cert/leaf.signed.pem" );

	if( 0 <= szResult.Find( L"unable to load certificate" ) )
	{
		pDlg->SetText( &pDlg->m_EditCertificateText, NX_RET_FAIL );
		return;
	}

	CFile file;
	szWorking += L"leaf.signed.txt";
	file.Open( szWorking, CFile::modeCreate | CFile::modeWrite, NULL );
	file.Write( szResult, szResult.GetLength()*sizeof(TCHAR));
	file.Close();

	pDlg->SetText( &pDlg->m_EditCertificateText, L"DONE" );
}

void CNxCinemaDiagnosticsPanelDlg::TestTconLeft( void *pObj )
{
	CNxCinemaDiagnosticsPanelDlg *pDlg = (CNxCinemaDiagnosticsPanelDlg*)pObj;

	if( !pDlg->IsConnected( L"NAP" ) )
	{
		pDlg->SetText( &pDlg->m_EditTconLeft, NX_RET_FAIL );
		return;
	}
	
	CString szResult, szRepeat;
	int iRepeat = 0;

	pDlg->GetText( &pDlg->m_EditTconRepeat, szRepeat );
	iRepeat = _wtoi( szRepeat );

	for( int i = 0; i < iRepeat; i++ )
	{
		pDlg->SetText( &pDlg->m_EditTconLeft, L"check: %d", i+1 );
		pDlg->Execute( szResult, L"adb -s NAP shell NxCinemaDiagnostics -c 0 -d %d", pDlg->IsDebug() ? 0 : 1 );

		szResult.Trim();

		pDlg->LogPrint( pDlg->IsDebug(), NX_PERFORATED_LINE );
		pDlg->LogPrint( pDlg->IsDebug(), L"Check TCON Left( %d / %d )\r\n", i+1, iRepeat );
		pDlg->LogPrint( pDlg->IsDebug(), L"%s\r\n", szResult );
		
		if( pDlg->IsError( szResult ) )
		{
			pDlg->SetText( &pDlg->m_EditTconLeft, NX_RET_ERROR );
			return;
		}

		if( pDlg->IsFail( szResult ) )
		{
			pDlg->SetText( &pDlg->m_EditTconLeft, NX_RET_FAIL );
			return;
		}
	}

	pDlg->SetText( &pDlg->m_EditTconLeft, (0 < iRepeat) ? NX_RET_PASS : NX_RET_NONE );
}

void CNxCinemaDiagnosticsPanelDlg::TestTconRight( void *pObj )
{
	CNxCinemaDiagnosticsPanelDlg *pDlg = (CNxCinemaDiagnosticsPanelDlg*)pObj;

	if( !pDlg->IsConnected( L"NAP" ) )
	{
		pDlg->SetText( &pDlg->m_EditTconRight, NX_RET_FAIL );
		return;
	}

	CString szResult, szRepeat;
	int iRepeat = 0;

	pDlg->GetText( &pDlg->m_EditTconRepeat, szRepeat );
	iRepeat = _wtoi( szRepeat );

	for( int i = 0; i < iRepeat; i++ )
	{
		pDlg->SetText( &pDlg->m_EditTconRight, L"check: %d", i+1 );
		pDlg->Execute( szResult, L"adb -s NAP shell NxCinemaDiagnostics -c 1 -d %d", pDlg->IsDebug() ? 0 : 1 );
		
		szResult.Trim();

		pDlg->LogPrint( pDlg->IsDebug(), NX_PERFORATED_LINE );
		pDlg->LogPrint( pDlg->IsDebug(), L"Check TCON Right( %d / %d )\r\n", i+1, iRepeat );
		pDlg->LogPrint( pDlg->IsDebug(), L"%s\r\n", szResult);

		if( pDlg->IsError( szResult ) )
		{
			pDlg->SetText( &pDlg->m_EditTconRight, NX_RET_ERROR );
			return;
		}

		if( pDlg->IsFail( szResult ) )
		{
			pDlg->SetText( &pDlg->m_EditTconRight, NX_RET_FAIL );
			return;
		}
	}

	pDlg->SetText( &pDlg->m_EditTconRight, (0 < iRepeat) ? NX_RET_PASS : NX_RET_NONE );
}

void CNxCinemaDiagnosticsPanelDlg::TestPfpga( void *pObj )
{
	CNxCinemaDiagnosticsPanelDlg *pDlg = (CNxCinemaDiagnosticsPanelDlg*)pObj;

	if( !pDlg->IsConnected( L"NAP" ) )
	{
		pDlg->SetText( &pDlg->m_EditPfpga, NX_RET_FAIL );
		return;
	}

	CString szResult, szRepeat;
	int iRepeat = 0;

	pDlg->GetText( &pDlg->m_EditPfpgaRepeat, szRepeat );
	iRepeat = _wtoi( szRepeat );

	for( int i = 0; i < iRepeat; i++ )
	{
		pDlg->SetText( &pDlg->m_EditPfpga, L"check: %d", i+1 );
		pDlg->Execute( szResult, L"adb -s NAP shell NxCinemaDiagnostics -c 2 -d %d", pDlg->IsDebug() ? 0 : 1 );

		szResult.Trim();

		pDlg->LogPrint( pDlg->IsDebug(), NX_PERFORATED_LINE );
		pDlg->LogPrint( pDlg->IsDebug(), L"Check PFPGA( %d / %d )\r\n", i+1, iRepeat );
		pDlg->LogPrint( pDlg->IsDebug(), L"%s\r\n", szResult );
		
		if( pDlg->IsError( szResult ) )
		{
			pDlg->SetText( &pDlg->m_EditPfpga, NX_RET_ERROR );
			return;
		}

		if( pDlg->IsFail( szResult ) )
		{
			pDlg->SetText( &pDlg->m_EditPfpga, NX_RET_FAIL );
			return;
		}
	}

	pDlg->SetText( &pDlg->m_EditPfpga, (0 < iRepeat) ? NX_RET_PASS : NX_RET_NONE );
}

void CNxCinemaDiagnosticsPanelDlg::TestEEPRom( void *pObj )
{
	CNxCinemaDiagnosticsPanelDlg *pDlg = (CNxCinemaDiagnosticsPanelDlg*)pObj;

	if( !pDlg->IsConnected( L"NAP" ) )
	{
		pDlg->SetText( &pDlg->m_EditEEPRom, NX_RET_FAIL );
		return;
	}

	int iChecked = pDlg->GetCheckedRadioButton( IDC_RADIO_EEPROM_ALL, IDC_RADIO_EEPROM_NONE );
	if( iChecked == IDC_RADIO_EEPROM_NONE )
	{
		pDlg->SetText( &pDlg->m_EditEEPRom, NX_RET_NONE );
		return;
	}

	CString szResult;
	pDlg->Execute( szResult, L"adb -s NAP shell NxCinemaDiagnostics -c 3 -t %d -d %d", (iChecked == IDC_RADIO_EEPROM_ALL) ? 0 : 1, pDlg->IsDebug() ? 0 : 1);

	szResult.Trim();

	pDlg->LogPrint( pDlg->IsDebug(), NX_PERFORATED_LINE );
	pDlg->LogPrint( pDlg->IsDebug(), L"Check EEPRom\r\n" );
	pDlg->LogPrint( pDlg->IsDebug(), szResult + L"\r\n" );
	
    
	if( pDlg->IsFail( szResult ) )
	{
		pDlg->SetText( &pDlg->m_EditEEPRom, NX_RET_FAIL );
		return;
	}

	pDlg->SetText( &pDlg->m_EditEEPRom, szResult.Mid( szResult.ReverseFind( L'\n' ), szResult.GetLength() - szResult.ReverseFind( L'\n' ) ) );
}

void CNxCinemaDiagnosticsPanelDlg::TestMarriageTamper( void *pObj )
{
	CNxCinemaDiagnosticsPanelDlg *pDlg = (CNxCinemaDiagnosticsPanelDlg*)pObj;

	if( !pDlg->IsConnected( L"NAP" ) )
	{
		pDlg->SetText( &pDlg->m_EditMarriageTamper, NX_RET_FAIL );
		return;
	}

	CString szResult, szRepeat;
	int iRepeat = 0;

	pDlg->GetText( &pDlg->m_EditTamperRepeat, szRepeat );
	iRepeat = _wtoi( szRepeat );

	for( int i = 0; i < iRepeat; i++ )
	{
		pDlg->SetText( &pDlg->m_EditMarriageTamper, L"check: %d", i+1 );
		pDlg->Execute( szResult, L"adb -s NAP shell NxCinemaDiagnostics -c 4 -d %d", pDlg->IsDebug() ? 0 : 1 );

		szResult.Trim();

		pDlg->LogPrint( pDlg->IsDebug(), NX_PERFORATED_LINE );
		pDlg->LogPrint( pDlg->IsDebug(), L"Check Marriage Tamper( %d / %d )\r\n", i+1, iRepeat ); 
		pDlg->LogPrint( pDlg->IsDebug(), szResult + L"\r\n" );

		if( pDlg->IsPass( szResult ) )
		{
			pDlg->SetText( &pDlg->m_EditMarriageTamper, NX_RET_PASS );
			return;
		}
	}

	pDlg->SetText( &pDlg->m_EditMarriageTamper, (0 < iRepeat) ? NX_RET_FAIL : NX_RET_NONE );
}

void CNxCinemaDiagnosticsPanelDlg::TestDoorTamper( void *pObj )
{
	CNxCinemaDiagnosticsPanelDlg *pDlg = (CNxCinemaDiagnosticsPanelDlg*)pObj;

	if( !pDlg->IsConnected( L"NAP" ) )
	{
		pDlg->SetText( &pDlg->m_EditDoorTamper, NX_RET_FAIL );
		return;
	}

	CString szResult, szRepeat;
	int iRepeat = 0;

	pDlg->GetText( &pDlg->m_EditTamperRepeat, szRepeat );
	iRepeat = _wtoi( szRepeat );

	for( int i = 0; i < iRepeat; i++ )
	{
		pDlg->SetText( &pDlg->m_EditDoorTamper, L"check: %d", i+1 );
		pDlg->Execute( szResult, L"adb -s NAP shell NxCinemaDiagnostics -c 5 -d %d", pDlg->IsDebug() ? 0 : 1 );

		szResult.Trim();

		pDlg->LogPrint( pDlg->IsDebug(), NX_PERFORATED_LINE );
		pDlg->LogPrint( pDlg->IsDebug(), L"Check Door Tamper( %d / %d )\r\n", i+1, iRepeat );
		pDlg->LogPrint( pDlg->IsDebug(), szResult + L"\r\n" );

		if( pDlg->IsPass( szResult ) )
		{
			pDlg->SetText( &pDlg->m_EditDoorTamper, NX_RET_PASS );
			return;
		}
	}

	pDlg->SetText( &pDlg->m_EditDoorTamper, (0 < iRepeat) ? NX_RET_FAIL : NX_RET_NONE );
}

void CNxCinemaDiagnosticsPanelDlg::TestNetwork( void *pObj )
{
	CNxCinemaDiagnosticsPanelDlg *pDlg = (CNxCinemaDiagnosticsPanelDlg*)pObj;

	if( !pDlg->IsConnected( L"NAP" ) )
	{
		pDlg->SetText( &pDlg->m_EditNetwork, NX_RET_FAIL );
		return;
	}

	CString szResult, szRepeat;
	int iRepeat = 0;

	pDlg->GetText( &pDlg->m_EditNetworkRepeat, szRepeat );
	iRepeat = _wtoi( szRepeat );

	for( int i = 0; i < iRepeat; i++ )
	{
		BYTE nField0, nField1, nField2, nField3;
		pDlg->m_IpAddr.GetAddress( nField0, nField1, nField2, nField3 );

		pDlg->SetText( &pDlg->m_EditNetwork, L"check: %d", i+1 );
		pDlg->Execute( szResult, L"adb -s NAP shell NxCinemaDiagnostics -c 6 -i %d.%d.%d.%d -d %d", nField0, nField1, nField2, nField3, pDlg->IsDebug() ? 0 : 1 );

		szResult.Trim();

		pDlg->LogPrint( pDlg->IsDebug(), NX_PERFORATED_LINE );
		pDlg->LogPrint( pDlg->IsDebug(), L"Check Network( %d / %d )\r\n", i+1, iRepeat );
		pDlg->LogPrint( pDlg->IsDebug(), szResult + L"\r\n" );

		if( pDlg->IsError( szResult) )
		{
			pDlg->SetText( &pDlg->m_EditNetwork, NX_RET_FAIL );
			return;
		}

		if( pDlg->IsFail( szResult ) )
		{
			pDlg->SetText( &pDlg->m_EditNetwork, NX_RET_FAIL );
			return;
		}

#if 0
		if( pDlg->IsPass( szResult ) && (0 <= szResult.Find( L"Unknown ICMP message received" )) )
		{
			pDlg->SetText( &pDlg->m_EditNetwork, NX_RET_FAIL );
			return;
		}
#endif
	}

	pDlg->SetText( &pDlg->m_EditNetwork, (0 < iRepeat) ? NX_RET_PASS : NX_RET_NONE );
}
