
// NxCinemaI2CPanelDlg.h : ��� ����
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CNxCinemaI2CPanelDlg ��ȭ ����
class CNxCinemaI2CPanelDlg : public CDialogEx
{
// �����Դϴ�.
public:
	CNxCinemaI2CPanelDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_NXCINEMAI2CPANEL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.


// �����Դϴ�.
protected:
	HICON m_hIcon;

	// ������ �޽��� �� �Լ�
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
