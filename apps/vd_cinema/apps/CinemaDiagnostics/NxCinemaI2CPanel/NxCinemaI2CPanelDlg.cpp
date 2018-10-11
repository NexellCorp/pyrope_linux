
// NxCinemaI2CPanelDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "NxCinemaI2CPanel.h"
#include "NxCinemaI2CPanelDlg.h"
#include "afxdialogex.h"

#include "NX_I2CControl.h"

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


// CNxCinemaI2CPanelDlg 대화 상자




CNxCinemaI2CPanelDlg::CNxCinemaI2CPanelDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CNxCinemaI2CPanelDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CNxCinemaI2CPanelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_LOG, m_EditLog);
	DDX_Control(pDX, IDC_SPIN_PORT, m_SpinPort);
	DDX_Control(pDX, IDC_SPIN_SLAVE, m_SpinSlave);
	DDX_Control(pDX, IDC_SPIN_REG, m_SpinReg);
	DDX_Control(pDX, IDC_SPIN_DATA, m_SpinData);
	DDX_Control(pDX, IDC_EDIT_PORT, m_EditPort);
	DDX_Control(pDX, IDC_EDIT_SLAVE, m_EditSlave);
	DDX_Control(pDX, IDC_EDIT_REG, m_EditReg);
	DDX_Control(pDX, IDC_EDIT_DATA, m_EditData);
}

BEGIN_MESSAGE_MAP(CNxCinemaI2CPanelDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_CONNECT, &CNxCinemaI2CPanelDlg::OnBnClickedBtnConnect)
	ON_BN_CLICKED(IDC_BTN_CLEAR, &CNxCinemaI2CPanelDlg::OnBnClickedBtnClear)
	ON_BN_CLICKED(IDC_BTN_READ, &CNxCinemaI2CPanelDlg::OnBnClickedBtnRead)
	ON_BN_CLICKED(IDC_BTN_WRITE, &CNxCinemaI2CPanelDlg::OnBnClickedBtnWrite)
END_MESSAGE_MAP()


// CNxCinemaI2CPanelDlg 메시지 처리기

BOOL CNxCinemaI2CPanelDlg::OnInitDialog()
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

	UDACCEL udAccels[] = {{0, -1}};
	m_SpinPort.SetAccel( 1, udAccels );
	m_SpinSlave.SetAccel( 1, udAccels );
	m_SpinReg.SetAccel( 1, udAccels );
	m_SpinData.SetAccel( 1, udAccels );

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CNxCinemaI2CPanelDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CNxCinemaI2CPanelDlg::OnPaint()
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
HCURSOR CNxCinemaI2CPanelDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CNxCinemaI2CPanelDlg::LogPrint( LPCTSTR szFormat, ... )
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

void CNxCinemaI2CPanelDlg::LogClear()
{
	m_EditLog.SetReadOnly( FALSE );
	m_EditLog.SetSel( 0, -1 );
	m_EditLog.Clear();
	m_EditLog.SetReadOnly( TRUE );
}

void CNxCinemaI2CPanelDlg::OnBnClickedBtnConnect()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if( FALSE == NX_I2CConnect() )
	{
		LogPrint(_T("fail, connect.\r\n"));
	}
	else
	{
		LogPrint(_T("connected.\r\n"));
	}
}

void CNxCinemaI2CPanelDlg::OnBnClickedBtnClear()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	LogClear();
}

void CNxCinemaI2CPanelDlg::OnBnClickedBtnRead()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	CString szPort, szSlave, szReg;
	m_EditPort.GetWindowTextW(szPort);
	m_EditSlave.GetWindowTextW(szSlave);
	m_EditReg.GetWindowTextW(szReg);

	int iReadData = NX_I2CRead( _tstoi(szPort), _tstoi(szSlave), _tstoi(szReg) );
	if( 0 > iReadData )
	{
		LogPrint(_T("fail, i2c read.\r\n"));
	}
	else
	{
		LogPrint(_T("[rd] i2c-%d, slave: %d ( 0x%02X ), reg: %d ( 0x%04X ), data: %d ( 0x%04X )\r\n"),
			_tstoi(szPort),
			_tstoi(szSlave), _tstoi(szSlave),
			_tstoi(szReg), _tstoi(szReg),
			iReadData, iReadData
		);

		CString szTemp;
		szTemp.Format( _T("%d"), iReadData );
		m_EditData.SetWindowTextW( szTemp );
	}
}

void CNxCinemaI2CPanelDlg::OnBnClickedBtnWrite()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	CString szPort, szSlave, szReg, szData;
	m_EditPort.GetWindowTextW(szPort);
	m_EditSlave.GetWindowTextW(szSlave);
	m_EditReg.GetWindowTextW(szReg);
	m_EditData.GetWindowTextW(szData);

	int iRet = NX_I2CWrite( _tstoi(szPort), _tstoi(szSlave), _tstoi(szReg), _tstoi(szData) );
	if( 0 > iRet )
	{
		LogPrint(_T("fail, i2c write.\r\n"));
	}
	else
	{
		LogPrint(_T("[wr] i2c-%d, slave: %d ( 0x%02X ), reg: %d ( 0x%04X ), data: %d ( 0x%04X )\r\n"),
			_tstoi(szPort),
			_tstoi(szSlave), _tstoi(szSlave),
			_tstoi(szReg), _tstoi(szReg),
			_tstoi(szData), _tstoi(szData)
		);
	}
}

