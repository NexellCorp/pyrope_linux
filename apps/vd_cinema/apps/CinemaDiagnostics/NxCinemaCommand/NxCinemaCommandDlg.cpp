
// NxCinemaCommandDlg.cpp : 구현 파일
//
#include "stdafx.h"
#include "NxCinemaCommand.h"
#include "NxCinemaCommandDlg.h"
#include "afxdialogex.h"

#include "gdc_protocol.h"
#include "NX_TMSClient.h"

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


// CNxCinemaCommandDlg 대화 상자




CNxCinemaCommandDlg::CNxCinemaCommandDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CNxCinemaCommandDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CNxCinemaCommandDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_LOG, m_EditLog);
	DDX_Control(pDX, IDC_EDIT_QUE, m_EditQue);
	DDX_Control(pDX, IDC_EDIT_UPLOAD_FILE, m_EditUploadFile);
	DDX_Control(pDX, IDC_CMB_DOWN_MODE, m_CmbDownMode);
	DDX_Control(pDX, IDC_CMB_DOWN_COLOR, m_CmbDownColor);
	DDX_Control(pDX, IDC_CMB_DEL_MODE, m_CmbDelMode);
	DDX_Control(pDX, IDC_IPADDRESS, m_IpAddr);
}

BEGIN_MESSAGE_MAP(CNxCinemaCommandDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_QUE, &CNxCinemaCommandDlg::OnBnClickedBtnQue)
	ON_BN_CLICKED(IDC_BTN_UPLOAD_OPEN, &CNxCinemaCommandDlg::OnBnClickedBtnUploadOpen)
	ON_BN_CLICKED(IDC_BTN_UPLOAD_FILE, &CNxCinemaCommandDlg::OnBnClickedBtnUploadFile)
	ON_BN_CLICKED(IDC_BTN_DOWN_FILE, &CNxCinemaCommandDlg::OnBnClickedBtnDownFile)
	ON_BN_CLICKED(IDC_BTN_DEL_FILE, &CNxCinemaCommandDlg::OnBnClickedBtnDelFile)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BTN_PING, &CNxCinemaCommandDlg::OnBnClickedBtnPing)
	ON_BN_CLICKED(IDC_BTN_CLEAR_LOG, &CNxCinemaCommandDlg::OnBnClickedBtnClearLog)
END_MESSAGE_MAP()


// CNxCinemaCommandDlg 메시지 처리기

BOOL CNxCinemaCommandDlg::OnInitDialog()
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
	m_CmbDelMode.AddString( _T("ALL") );

	for( int i = 0; i < 20; i++ )
	{
		CString szTemp;
		szTemp.Format( _T("Mode #%02d"), i + 10 );

		m_CmbDownMode.AddString( szTemp );
		m_CmbDelMode.AddString( szTemp );
	}

	m_CmbDownColor.AddString( _T("R") );
	m_CmbDownColor.AddString( _T("G") );
	m_CmbDownColor.AddString( _T("B") );

	m_ToolTip.Create(this);

	LoadConfig();

	return FALSE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CNxCinemaCommandDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CNxCinemaCommandDlg::OnPaint()
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
HCURSOR CNxCinemaCommandDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


BOOL CNxCinemaCommandDlg::PreTranslateMessage(MSG* pMsg)
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
			return TRUE;
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}


void CNxCinemaCommandDlg::LoadConfig()
{
	TCHAR tcApp[MAX_PATH];
	GetModuleFileName( NULL, tcApp, MAX_PATH );

	CString szConfig;
	szConfig.Format( L"%s", tcApp );
	szConfig = szConfig.Left( szConfig.ReverseFind('.') );
	szConfig += L".ini";

	TCHAR tcValue[MAX_PATH];

	CString szIpAddress;
	GetIpAddress( szIpAddress );
	GetPrivateProfileString( _T("app"),		_T("ip_address"),		szIpAddress,	tcValue,	MAX_PATH,	szConfig );

	CString szField, szField0, szField1, szField2, szField3;
	szField.Format( L"%s", tcValue );

	AfxExtractSubString( szField0, szField, 0, '.' );
	AfxExtractSubString( szField1, szField, 1, '.' );
	AfxExtractSubString( szField2, szField, 2, '.' );
	AfxExtractSubString( szField3, szField, 3, '.' );
	m_IpAddr.SetAddress( _wtoi(szField0), _wtoi(szField1), _wtoi(szField2), _wtoi(szField3) );

	GetPrivateProfileString( _T("app"),		_T("que"),				_T("0"),		tcValue,	MAX_PATH,	szConfig );
	m_EditQue.SetWindowTextW( (LPCTSTR)tcValue );

	GetPrivateProfileString( _T("app"),		_T("upload_file"),		_T(""),			tcValue,	MAX_PATH,	szConfig );
	m_EditUploadFile.SetSel( 0, -1 );
	m_EditUploadFile.ReplaceSel( (LPCTSTR)tcValue );
	//m_EditUploadFile.SetFocus();
	m_ToolTip.AddTool( GetDlgItem(IDC_EDIT_UPLOAD_FILE), (LPCTSTR)tcValue );

	GetPrivateProfileString( _T("app"),		_T("down_mode"),		_T("0"),		tcValue,	MAX_PATH,	szConfig );
	m_CmbDownMode.SetCurSel( _wtoi(tcValue) );

	GetPrivateProfileString( _T("app"),		_T("down_color"),		_T("0"),		tcValue,	MAX_PATH,	szConfig );
	m_CmbDownColor.SetCurSel( _wtoi(tcValue) );

	GetPrivateProfileString( _T("app"),		_T("del_mode"),			_T("0"),		tcValue,	MAX_PATH,	szConfig );
	m_CmbDelMode.SetCurSel( _wtoi(tcValue) );
}

void CNxCinemaCommandDlg::SaveConfig()
{
	TCHAR tcApp[MAX_PATH];
	GetModuleFileName( NULL, tcApp, MAX_PATH );

	CString szConfig;
	szConfig.Format( L"%s", tcApp );
	szConfig = szConfig.Left( szConfig.ReverseFind('.') );
	szConfig += L".ini";

	CString szValue;

	BYTE nField0, nField1, nField2, nField3;
	m_IpAddr.GetAddress( nField0, nField1, nField2, nField3 );
	szValue.Format( L"%d.%d.%d.%d", nField0, nField1, nField2, nField3 );
	WritePrivateProfileStringW( _T("app"),		_T("ip_address"),		szValue,		szConfig );

	m_EditQue.GetWindowTextW( szValue );
	WritePrivateProfileStringW( _T("app"),		_T("que"),				szValue,		szConfig );

	m_EditUploadFile.GetWindowTextW( szValue );
	WritePrivateProfileStringW( _T("app"),		_T("upload_file"),		szValue,		szConfig );

	szValue.Format(_T("%d"), m_CmbDownMode.GetCurSel() );
	WritePrivateProfileStringW( _T("app"),		_T("down_mode"),		szValue,		szConfig );

	szValue.Format(_T("%d"), m_CmbDownColor.GetCurSel() );
	WritePrivateProfileStringW( _T("app"),		_T("down_color"),		szValue,		szConfig );

	szValue.Format(_T("%d"), m_CmbDelMode.GetCurSel() );
	WritePrivateProfileStringW( _T("app"),		_T("del_mode"),			szValue,		szConfig );
}

void CNxCinemaCommandDlg::LogPrint( LPCTSTR szFormat, ... )
{
	CString szText;

	va_list args;
	va_start( args, szFormat );
	szText.FormatV( szFormat, args );
	va_end( args );

	int nLength = m_EditLog.GetWindowTextLengthW();
	m_EditLog.SetSel( nLength, nLength );
	m_EditLog.ReplaceSel( szText );
	m_EditLog.LineScroll( m_EditLog.GetLineCount() );
}

void CNxCinemaCommandDlg::LogPrint( uint8_t *pBuf, int32_t iBufSize, LPCTSTR szFormat, ... )
{
	CString szText;

	va_list args;
	va_start( args, szFormat );
	szText.FormatV( szFormat, args );
	va_end( args );

	CString szHex = _T("");
	for( int32_t i = 0; i < iBufSize; i++ )
	{
		CString szTemp;
		szTemp.Format( _T("0x%02X\t"), pBuf[i] );
		szHex += szTemp;
	}
	szHex += "\r\n";

	szText += szHex;
	int nLength = m_EditLog.GetWindowTextLengthW();
	m_EditLog.SetSel( nLength, nLength );
	m_EditLog.ReplaceSel( szText );
	m_EditLog.LineScroll( m_EditLog.GetLineCount() );
}

void CNxCinemaCommandDlg::LogClear()
{
	m_EditLog.SetReadOnly( FALSE );
	m_EditLog.SetSel( 0, -1 );
	m_EditLog.Clear();
	m_EditLog.SetReadOnly( TRUE );
}

#include <afxsock.h>

void CNxCinemaCommandDlg::GetIpAddress( CString &szAddress )
{
    WORD wVersionRequested;
    WSADATA wsaData;

	char name[255];
    PHOSTENT hostinfo;
    wVersionRequested = MAKEWORD(2, 0);

    if(WSAStartup(wVersionRequested, &wsaData) == 0)
    {
        if(gethostname(name, sizeof(name)) == 0)
        {
            if((hostinfo = gethostbyname(name)) != NULL)
			{
                szAddress = inet_ntoa(*(struct in_addr *)*hostinfo->h_addr_list);
			}

			else
			{
				szAddress = _T("127.0.0.1");
			}
        }
        WSACleanup();
    }
}

BOOL CNxCinemaCommandDlg::Ping( CString szIpAdddr )
{
	return TRUE;
}

void CNxCinemaCommandDlg::OnBnClickedBtnQue()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString szTemp;
	BYTE nField0, nField1, nField2, nField3;
	m_IpAddr.GetAddress( nField0, nField1, nField2, nField3 );
	szTemp.Format( L"%d.%d.%d.%d", nField0, nField1, nField2, nField3 );

	CT2A ip(szTemp);
	NX_TMSSendCommand( ip.m_psz, this, &ProcessCommandQue );
}

void CNxCinemaCommandDlg::OnBnClickedBtnUploadOpen()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString szFilter = _T("Text File(*.txt)|*.txt|All Files(*.*)|*.*||");
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter);

	CString szTemp;
	m_EditUploadFile.GetWindowTextW(szTemp);
	szTemp = szTemp.Left( szTemp.ReverseFind('\\') );
	dlg.m_ofn.lpstrInitialDir = szTemp;

	if( IDOK == dlg.DoModal() )
	{
		m_EditUploadFile.SetSel( 0, -1 );
		m_EditUploadFile.ReplaceSel( dlg.GetPathName() );
		m_EditUploadFile.SetFocus();
		m_ToolTip.AddTool( GetDlgItem(IDC_EDIT_UPLOAD_FILE), dlg.GetPathName() );
	}
}

void CNxCinemaCommandDlg::OnBnClickedBtnUploadFile()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString szTemp;
	BYTE nField0, nField1, nField2, nField3;
	m_IpAddr.GetAddress( nField0, nField1, nField2, nField3 );
	szTemp.Format( L"%d.%d.%d.%d", nField0, nField1, nField2, nField3 );

	CT2A ip(szTemp);
	NX_TMSSendCommand( ip.m_psz, this, &ProcessCommandUpload );
}

void CNxCinemaCommandDlg::OnBnClickedBtnDownFile()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString szTemp;
	BYTE nField0, nField1, nField2, nField3;
	m_IpAddr.GetAddress( nField0, nField1, nField2, nField3 );
	szTemp.Format( L"%d.%d.%d.%d", nField0, nField1, nField2, nField3 );

	CT2A ip(szTemp);
	NX_TMSSendCommand( ip.m_psz, this, &ProcessCommandDownload );
}

void CNxCinemaCommandDlg::OnBnClickedBtnDelFile()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString szTemp;
	BYTE nField0, nField1, nField2, nField3;
	m_IpAddr.GetAddress( nField0, nField1, nField2, nField3 );
	szTemp.Format( L"%d.%d.%d.%d", nField0, nField1, nField2, nField3 );

	CT2A ip(szTemp);
	NX_TMSSendCommand( ip.m_psz, this, &ProcessCommandDelete );
}

void CNxCinemaCommandDlg::OnClose()
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	SaveConfig();

	CDialogEx::OnClose();
}

#include <winsock.h>
#include <Icmpapi.h>

#pragma comment( lib, "Iphlpapi" )
#pragma comment( lib, "wsock32" )

void CNxCinemaCommandDlg::OnBnClickedBtnPing()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	// Declare and initialize variables
	HANDLE hIcmpFile;
	unsigned long ipaddr = INADDR_NONE;
	DWORD dwRetVal = 0;
	char  SendData[] = "Data Buffer";
	char  szReplyBuffer[1500];

	// Validate the parameters
	CString szTemp;
	BYTE nField0, nField1, nField2, nField3;
	m_IpAddr.GetAddress( nField0, nField1, nField2, nField3 );
	szTemp.Format( L"%d.%d.%d.%d", nField0, nField1, nField2, nField3 );

	CT2A ip(szTemp);
	ipaddr = inet_addr(ip.m_psz);
	if( ipaddr == INADDR_NONE )
	{
		//LogPrint(_T("Fail, Wrong IP format. (%S) \r\n"), ip.m_psz);
		return;
	}

	hIcmpFile = IcmpCreateFile();
	if( hIcmpFile == INVALID_HANDLE_VALUE )
	{
		//LogPrint(_T("Unable to open handle.\n"));
		//LogPrint(_T("IcmpCreatefile returned error: %ld\n"), GetLastError() );
		return ;
	}

	dwRetVal = IcmpSendEcho( hIcmpFile, ipaddr, SendData, sizeof(SendData), NULL, szReplyBuffer, sizeof(szReplyBuffer), 1000 );
	if( dwRetVal != 0 )
	{
		PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)szReplyBuffer;
		struct in_addr ReplyAddr;
		ReplyAddr.S_un.S_addr = pEchoReply->Address;
		LogPrint( _T("Sent icmp message to %S.\n"), ip.m_psz);
		LogPrint( _T("Received from %S. ( status: %ld, roundtrip time: %ld mSec )\n"),
			inet_ntoa( ReplyAddr ), pEchoReply->Status, pEchoReply->RoundTripTime );

		//if( dwRetVal > 1 )
		//{
		//	LogPrint(_T("Received %ld icmp message responses\n"), dwRetVal);
		//	LogPrint(_T("Information from the first response:\n"));
		//}
		//else
		//{
		//	LogPrint(_T("Received %ld icmp message response\n"), dwRetVal);
		//	LogPrint(_T("Information from this response:\n"));
		//}
		//LogPrint(_T("  Received from %S\n"), inet_ntoa( ReplyAddr ) );
		//LogPrint(_T("  Status = %ld\n"), pEchoReply->Status);
		//LogPrint(_T("  Roundtrip time = %ld milliseconds\n"), pEchoReply->RoundTripTime);
	}
	else
	{
		LogPrint( _T("Fail, IcmpSendEcho(). ( %ld )\n"), GetLastError() );

		//LogPrint(_T("Call to IcmpSendEcho failed.\n"));
		//LogPrint(_T("IcmpSendEcho returned error: %ld\n"), GetLastError() );
	}
}

void CNxCinemaCommandDlg::OnBnClickedBtnClearLog()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	LogClear();
}

int32_t CNxCinemaCommandDlg::ProcessCommandQue(int32_t iSock, void *pObj )
{
	uint8_t sendBuf[MAX_PAYLOAD_SIZE + 6];
	uint8_t recvBuf[MAX_PAYLOAD_SIZE + 6];
	uint8_t buf[MAX_PAYLOAD_SIZE];

	int32_t iSendSize, iRecvSize;
	int32_t iBufSize;

	uint32_t iKey;
	void *pPayload;
	int16_t iPayloadSize;

	CNxCinemaCommandDlg *pDlg = (CNxCinemaCommandDlg*)pObj;

	{
		CString szTemp;
		pDlg->m_EditQue.GetWindowTextW(szTemp);

		buf[0]   = _wtoi(szTemp);
		iBufSize = 1;

		iSendSize = GDC_MakePacket( KEY_GDC(CMD_IMB(IMB_CMD_QUE)), buf, iBufSize, sendBuf, sizeof(sendBuf) );
		if( 0 > iSendSize )
		{
			pDlg->LogPrint(_T("Fail, GDC_MakePacket().\r\n"));
			return -1;
		}

		NX_TMSSendPacket( iSock, sendBuf, iSendSize );
		pDlg->LogPrint( sendBuf, iSendSize, _T("Send #0 :\r\n") );
	}

	{
		iRecvSize = NX_TMSRecvPacket( iSock, recvBuf, sizeof(recvBuf) );
		if( 0 != GDC_ParsePacket( recvBuf, iRecvSize, &iKey, &pPayload, &iPayloadSize ) )
		{
			pDlg->LogPrint(_T("Fail, GDC_ParsePacket().\n"));
			return -1;
		}

		pDlg->LogPrint( recvBuf, iRecvSize, _T("Recv #0 :\r\n"));
	}

	return 0;
}

int32_t CNxCinemaCommandDlg::ProcessCommandDownload(int32_t iSock, void *pObj )
{
	uint8_t sendBuf[MAX_PAYLOAD_SIZE + 6];
	uint8_t recvBuf[MAX_PAYLOAD_SIZE + 6];
	uint8_t buf[MAX_PAYLOAD_SIZE];

	int32_t iSendSize, iRecvSize;
	int32_t iBufSize;

	uint32_t iKey;
	void *pPayload;
	int16_t iPayloadSize;

	char szPrefix[1024] = { 0x00, };

	CNxCinemaCommandDlg *pDlg = (CNxCinemaCommandDlg*)pObj;

	{
		int pos = pDlg->m_CmbDownMode.GetCurSel();
		buf[0] = 201;
		buf[1] = pos + 10;
		iBufSize = 2;

		iSendSize = GDC_MakePacket( KEY_GDC(CMD_IMB(IMB_CMD_QUE)), buf, iBufSize, sendBuf, sizeof(sendBuf) );
		if( 0 > iSendSize )
		{
			pDlg->LogPrint(_T("Fail, GDC_MakePacket().\r\n"));
			return -1;
		}

		NX_TMSSendPacket( iSock, sendBuf, iSendSize );
		pDlg->LogPrint( sendBuf, iSendSize, _T("Send #0 ( %d bytes ) :\r\n"), iSendSize );
	}

	{
		iRecvSize = NX_TMSRecvPacket( iSock, recvBuf, sizeof(recvBuf) );
		if( 0 != GDC_ParsePacket( recvBuf, iRecvSize, &iKey, &pPayload, &iPayloadSize ) )
		{
			pDlg->LogPrint(_T("Fail, GDC_ParsePacket().\r\n"));
			return -1;
		}

		pDlg->LogPrint( recvBuf, iRecvSize, _T("Recv #0 ( %d bytes ) :\r\n"), iRecvSize );

		if( iPayloadSize == 0 )
			return -1;

		memcpy( szPrefix, pPayload, iPayloadSize );
		pDlg->LogPrint( _T("> %S\r\n"), szPrefix );
	}

	{
		int pos = pDlg->m_CmbDownColor.GetCurSel();
		buf[0] = 201;
		buf[1] = (pos == 0) ? 'R' : ((pos == 1) ? 'G' : 'B');
		iBufSize = 2;

		iSendSize = GDC_MakePacket( KEY_GDC(CMD_IMB(IMB_CMD_QUE)), buf, iBufSize, sendBuf, sizeof(sendBuf) );
		if( 0 > iSendSize )
		{
			pDlg->LogPrint(_T("Fail, GDC_MakePacket().\r\n"));
			return -1;
		}

		NX_TMSSendPacket( iSock, sendBuf, iSendSize );
		pDlg->LogPrint( sendBuf, iSendSize, _T("Send #1 ( %d bytes ) :\r\n"), iSendSize );
	}

	{
		iRecvSize = NX_TMSRecvPacket( iSock, recvBuf, sizeof(recvBuf) );
		if( 0 != GDC_ParsePacket( recvBuf, iRecvSize, &iKey, &pPayload, &iPayloadSize ) )
		{
			pDlg->LogPrint(_T("Fail, GDC_ParsePacket().\r\n"));
			return -1;
		}

		pDlg->LogPrint( recvBuf, 9, _T("Recv #1 ( %d bytes ) :\r\n"), iRecvSize );

		//
		//
		//
		TCHAR tcApp[MAX_PATH];
		GetModuleFileName( NULL, tcApp, MAX_PATH );

		CString szPath;
		szPath.Format( L"%s", tcApp );
		szPath = szPath.Left( szPath.ReverseFind('\\') );

		CString szFile;
		szFile.Format( _T("%s\\Mode%02d_%S_%s.txt"),
			szPath,
			pDlg->m_CmbDownMode.GetCurSel() + 10,
			szPrefix,
			(pDlg->m_CmbDownColor.GetCurSel() == 0) ? L"R" : ((pDlg->m_CmbDownColor.GetCurSel() == 1) ? L"G" : L"B")
		);

		CFile hFile;
		hFile.Open( szFile, CFile::modeCreate | CFile::modeWrite );
		hFile.Write( pPayload, iPayloadSize );
		hFile.Close();
	}

	return 0;
}

int32_t CNxCinemaCommandDlg::ProcessCommandUpload(int32_t iSock, void *pObj )
{
	uint8_t sendBuf[MAX_PAYLOAD_SIZE + 6];
	uint8_t recvBuf[MAX_PAYLOAD_SIZE + 6];
	uint8_t buf[MAX_PAYLOAD_SIZE];

	int32_t iSendSize, iRecvSize;
	int32_t iBufSize;

	uint32_t iKey;
	void *pPayload;
	int16_t iPayloadSize;

	CNxCinemaCommandDlg *pDlg = (CNxCinemaCommandDlg*)pObj;

	{
		CString szTemp;
		pDlg->m_EditUploadFile.GetWindowTextW(szTemp);
		
		CFileStatus fs;
		if( !CFile::GetStatus( szTemp, fs ) )
		{
			pDlg->LogPrint( _T("Fail, Invalid File. ( %s )\r\n"), szTemp );
			return -1;
		}
	}

	{
		buf[0]   = 200;
		iBufSize = 1;

		iSendSize = GDC_MakePacket( KEY_GDC(CMD_IMB(IMB_CMD_QUE)), buf, iBufSize, sendBuf, sizeof(sendBuf) );
		if( 0 > iSendSize )
		{
			pDlg->LogPrint(_T("Fail, GDC_MakePacket().\r\n"));
			return -1;
		}

		NX_TMSSendPacket( iSock, sendBuf, iSendSize );
		pDlg->LogPrint( sendBuf, iSendSize, _T("Send #0 ( %d bytes ) :\r\n"), iSendSize );
	}

	{
		iRecvSize = NX_TMSRecvPacket( iSock, recvBuf, sizeof(recvBuf) );
		if( 0 != GDC_ParsePacket( recvBuf, iRecvSize, &iKey, &pPayload, &iPayloadSize ) )
		{
			pDlg->LogPrint(_T("Fail, GDC_ParsePacket().\r\n"));
			return -1;
		}

		pDlg->LogPrint( recvBuf, iRecvSize, _T("Recv #0 ( %d bytes ) :\r\n"), iRecvSize );
	}

	{
		CString szTemp;
		pDlg->m_EditUploadFile.GetWindowTextW(szTemp);

		CFile hFile;
		hFile.Open( szTemp, CFile::modeRead | CFile::typeBinary );

		iBufSize = (int32_t)hFile.GetLength();

		int iReadSize = hFile.Read( buf, iBufSize );
		if( iReadSize != iBufSize )
		{
			pDlg->LogPrint( _T("Fail, Unexpected ReadSize. ( read: %d, expcted: %d )\r\n"), iReadSize, iBufSize );
			return -1;
		}
		hFile.Close();

		iSendSize = GDC_MakePacket( KEY_GDC(CMD_IMB(IMB_CMD_QUE)), buf, iBufSize, sendBuf, sizeof(sendBuf) );
		if( 0 > iSendSize )
		{
			pDlg->LogPrint(_T("Fail, GDC_MakePacket().\r\n"));
			return -1;
		}

		NX_TMSSendPacket( iSock, sendBuf, iSendSize );
		pDlg->LogPrint( sendBuf, 9, _T("Send #1 ( %d bytes ) :%d bytes \r\n"), iSendSize, iBufSize );
	}

	{
		iRecvSize = NX_TMSRecvPacket( iSock, recvBuf, sizeof(recvBuf) );
		if( 0 != GDC_ParsePacket( recvBuf, iRecvSize, &iKey, &pPayload, &iPayloadSize ) )
		{
			pDlg->LogPrint(_T("Fail, GDC_ParsePacket().\n"));
			return -1;
		}

		uint8_t *pResult = (uint8_t*)pPayload;
		pDlg->LogPrint( recvBuf, iRecvSize, _T("Recv #1 ( %d bytes ) : %d bytes \r\n"), iRecvSize, ((((int)pResult[0] << 8) & 0xFF00) | ((int)pResult[1] & 0x00FF)));
	}

	return 0;
}

int32_t CNxCinemaCommandDlg::ProcessCommandDelete(int32_t iSock, void *pObj )
{
	uint8_t sendBuf[MAX_PAYLOAD_SIZE + 6];
	uint8_t recvBuf[MAX_PAYLOAD_SIZE + 6];
	uint8_t buf[MAX_PAYLOAD_SIZE];

	int32_t iSendSize, iRecvSize;
	int32_t iBufSize;

	uint32_t iKey;
	void *pPayload;
	int16_t iPayloadSize;

	CNxCinemaCommandDlg *pDlg = (CNxCinemaCommandDlg*)pObj;

	{
		int pos = pDlg->m_CmbDelMode.GetCurSel();

		buf[0]   = pos + 30;
		iBufSize = 1;

		iSendSize = GDC_MakePacket( KEY_GDC(CMD_IMB(IMB_CMD_QUE)), buf, iBufSize, sendBuf, sizeof(sendBuf) );
		if( 0 > iSendSize )
		{
			pDlg->LogPrint(_T("Fail, GDC_MakePacket().\r\n"));
			return -1;
		}

		NX_TMSSendPacket( iSock, sendBuf, iSendSize );
		pDlg->LogPrint( sendBuf, iSendSize, _T("Send #0 :\r\n") );
	}

	{
		iRecvSize = NX_TMSRecvPacket( iSock, recvBuf, sizeof(recvBuf) );
		if( 0 != GDC_ParsePacket( recvBuf, iRecvSize, &iKey, &pPayload, &iPayloadSize ) )
		{
			pDlg->LogPrint(_T("Fail, GDC_ParsePacket().\n"));
			return -1;
		}

		pDlg->LogPrint( recvBuf, iRecvSize, _T("Recv #0 :\r\n"));
	}

	return 0;
}
