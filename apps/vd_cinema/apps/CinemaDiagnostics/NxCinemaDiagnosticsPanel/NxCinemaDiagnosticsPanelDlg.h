
// NxCinemaDiagnosticsPanelDlg.h : 헤더 파일
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#define NX_PERFORATED_LINE	L"--------------------------------------------------------------------------------\r\n"
#define NX_RET_PASS			L"PASS"
#define NX_RET_FAIL			L"FAIL"
#define NX_RET_ERROR		L"device not found"
#define NX_RET_NONE			L"NOT TESTED"

#define NX_ENABLE_LOG_ROTATION	0

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
	afx_msg void OnBnClickedBtnWorkingSelect();
	afx_msg void OnBnClickedBtnWorkingOpen();
	afx_msg void OnBnClickedBtnTest();
	afx_msg void OnBnClickedBtnRun();
	afx_msg void OnBnClickedBtnRestartApp();
	afx_msg void OnBnClickedBtnReboot();
	afx_msg void OnBnClickedBtnClear();

	BOOL Execute( LPCTSTR szFormat, ... );
	BOOL Execute( CString &szResult, LPCTSTR szFormat, ... );

	BOOL IsConnected( CString szProduct );
	BOOL IsPass( CString szResult );
	BOOL IsFail( CString szResult );
	BOOL IsError( CString szResult );
	BOOL IsDebug();

	void EnableControl( BOOL bEnable );
	void ClearEditControl();

	void SetText( CWnd *pWnd, LPCTSTR szFormat, ... );
	void GetText( CWnd *pWnd, CString &szResult );

	void LogPrint( LPCTSTR szFormat, ... );
	void LogPrint( BOOL bEnable, LPCTSTR szFormat, ... );
	void LogClear();

	void LoadConfig();
	void SaveConfig();

private:
	enum { MAX_LOG_TEXT = 2147483647, MAX_LOG_LINE = 5000 };
	enum { MAX_REPEAT_NUM = 9999, MIN_REPEAT_NUM = 0, MAX_WAIT_TIME = 3 };

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
	static void RunTest( void *pObj );
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
	CButton m_BtnClear;

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
