
// NxCinemaDiagnosticsPanelDlg.h : 헤더 파일
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"

typedef struct NX_ADB_INFO {
	CString		szProduct;
	CString		szStatus;
} NX_ADB_INFO;

// CNxCinemaDiagnosticsPanelDlg 대화 상자
class CNxCinemaDiagnosticsPanelDlg : public CDialogEx
{
// 생성입니다.
public:
	CNxCinemaDiagnosticsPanelDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
	enum { IDD = IDD_NXCINEMADIAGNOSTICSPANEL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnClose();
	afx_msg void OnBnClickedBtnTest();
	afx_msg void OnBnClickedBtnWorkingSelect();
	afx_msg void OnBnClickedBtnWorkingOpen();
	afx_msg void OnBnClickedBtnRun();
	afx_msg void OnBnClickedBtnRestartApp();
	afx_msg void OnBnClickedBtnReboot();

	BOOL Execute( CString szCommand, CString *pResult = NULL );

	BOOL IsConnected( CString szProduct );
	BOOL IsPass( CString szResult );
	BOOL IsFail( CString szResult );

	void EnableControl( BOOL bEnable );
	void ClearEditControl();
	
	void LogPrint( const CString& szMsg, ... );
	void LogClear();

	void LoadConfig();
	void SaveConfig();

private:
	enum { MAX_REPEAT_NUM = 1000, MIN_REPEAT_NUM = 1 };

	void Run( void (*cbFunc)(void *pObj), void *pObj );

	static UINT ThreadStub( LPVOID lpParam );
	void ThreadProc( void );

private:
	enum{ MAX_INFO_NUM = 32 };

	CWinThread* m_pThread;
	void (*m_ProcessCallbackFunc)( void * );
	void *m_pObj;

	NX_ADB_INFO		m_Info[MAX_INFO_NUM];
	int				m_InfoNum;

public:
	static void RunDiagnostics( void *pObj );
	static void RunRestartApplication( void *pObj );
	static void RunReboot( void *pObj );

	static void Init( void *pObj );
	static void Deinit( void *pObj );

	static void StartControlPanel( void *pObj );
	static void StopControlPanel( void *pObj );

	static BOOL CheckFile( CString szFile, void *pObj );
	static BOOL CheckProcess( CString szProcess, void *pObj );

	static void GetProduct( void *pObj );
	static void GetSerial( void *pObj );
	static void GetLeafCertificate( void *pObj );
	static void GetLeafCertificateText( void *pObj );

	static void TestTconLeft( void *pObj );
	static void TestTconRight( void *pObj );
	static void TestPfpga( void *pObj );
	static void TestEEPRom( void *pObj );
	static void TestMarriageTamper( void *pObj );
	static void TestDoorTamper( void *pObj );
	static void TestNetwork( void *pObj );

public:
	CEdit m_EditLog;
	CButton m_BtnTest;
	CButton m_BtnRun;
	CButton m_BtnReboot;
	CButton m_BtnRestartApp;
	CButton m_BtnWorkingOpen;
	CButton m_BtnWorkingSelect;

	CEdit m_EditStatusSap;
	CEdit m_EditSerial;
	CEdit m_EditProduct;
	CEdit m_EditCertificate;
	CEdit m_EditCertificateText;
	CEdit m_EditStatusNap;
	CEdit m_EditTconLeft;
	CEdit m_EditTconRight;
	CEdit m_EditPfpga;
	CEdit m_EditEEPRom;
	CEdit m_EditMarriageTamper;
	CEdit m_EditDoorTamper;
	CEdit m_EditNetwork;

	CToolTipCtrl m_ToolTip;

	CEdit m_EditWorking;
	CEdit m_EditTconRepeat;
	CEdit m_EditPfpgaRepeat;
	CEdit m_EditTamperRepeat;
	CEdit m_EditNetworkRepeat;
	CEdit m_EditVersion;

	CSpinButtonCtrl m_SpinTconRepeat;
	CSpinButtonCtrl m_SpinPfpgaRepeat;
	CSpinButtonCtrl m_SpinTamperRepeat;
	CSpinButtonCtrl m_SpinNetworkRepeat;

	CIPAddressCtrl m_IpAddr;
};
