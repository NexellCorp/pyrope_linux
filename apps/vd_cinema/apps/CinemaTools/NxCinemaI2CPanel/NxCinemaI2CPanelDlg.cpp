
// NxCinemaI2CPanelDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "NxCinemaI2CPanel.h"
#include "NxCinemaI2CPanelDlg.h"
#include "afxdialogex.h"

#include "NX_I2CControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ���� ���α׷� ������ ���Ǵ� CAboutDlg ��ȭ �����Դϴ�.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

// �����Դϴ�.
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


// CNxCinemaI2CPanelDlg ��ȭ ����




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


// CNxCinemaI2CPanelDlg �޽��� ó����

BOOL CNxCinemaI2CPanelDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// �ý��� �޴��� "����..." �޴� �׸��� �߰��մϴ�.

	// IDM_ABOUTBOX�� �ý��� ��� ������ �־�� �մϴ�.
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

	// �� ��ȭ ������ �������� �����մϴ�. ���� ���α׷��� �� â�� ��ȭ ���ڰ� �ƴ� ��쿡��
	//  �����ӿ�ũ�� �� �۾��� �ڵ����� �����մϴ�.
	SetIcon(m_hIcon, TRUE);			// ū �������� �����մϴ�.
	SetIcon(m_hIcon, FALSE);		// ���� �������� �����մϴ�.

	// TODO: ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.

	UDACCEL udAccels[] = {{0, -1}};
	m_SpinPort.SetAccel( 1, udAccels );
	m_SpinSlave.SetAccel( 1, udAccels );
	m_SpinReg.SetAccel( 1, udAccels );
	m_SpinData.SetAccel( 1, udAccels );

	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
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

// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
//  �Ʒ� �ڵ尡 �ʿ��մϴ�. ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
//  �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.

void CNxCinemaI2CPanelDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ�Դϴ�.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ŭ���̾�Ʈ �簢������ �������� ����� ����ϴ�.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �������� �׸��ϴ�.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ���
//  �� �Լ��� ȣ���մϴ�.
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
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
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
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	LogClear();
}

void CNxCinemaI2CPanelDlg::OnBnClickedBtnRead()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.

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
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.

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

