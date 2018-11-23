
// NxCinemaI2CPanelDlg.h : 헤더 파일
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CNxCinemaI2CPanelDlg 대화 상자
class CNxCinemaI2CPanelDlg : public CDialogEx
{
// 생성입니다.
public:
	CNxCinemaI2CPanelDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
	enum { IDD = IDD_NXCINEMAI2CPANEL_DIALOG };

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
	void LogPrint( LPCTSTR szFormat, ... );
	void LogClear();

	afx_msg void OnBnClickedBtnConnect();
	afx_msg void OnBnClickedBtnClear();
	afx_msg void OnBnClickedBtnRead();
	afx_msg void OnBnClickedBtnWrite();

public:
	CEdit m_EditLog;

	CSpinButtonCtrl m_SpinPort;
	CSpinButtonCtrl m_SpinSlave;
	CSpinButtonCtrl m_SpinReg;
	CSpinButtonCtrl m_SpinData;

	CEdit m_EditPort;
	CEdit m_EditSlave;
	CEdit m_EditReg;
	CEdit m_EditData;
};
