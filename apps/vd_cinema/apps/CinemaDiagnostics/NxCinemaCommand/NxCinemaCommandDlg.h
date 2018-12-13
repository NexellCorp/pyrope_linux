
// NxCinemaCommandDlg.h : ��� ����
//

#pragma once
#include <stdint.h>

#include "afxwin.h"
#include "afxcmn.h"

// CNxCinemaCommandDlg ��ȭ ����
class CNxCinemaCommandDlg : public CDialogEx
{
// �����Դϴ�.
public:
	CNxCinemaCommandDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_NXCINEMACOMMAND_DIALOG };

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

	virtual BOOL PreTranslateMessage(MSG* pMsg);

public:
	void LoadConfig();
	void SaveConfig();
	void LogPrint( LPCTSTR szFormat, ... );
	void LogPrint( uint8_t *pBuf, int32_t iBufSize, LPCTSTR szFormat, ... );

	void LogClear();
	void GetIpAddress( CString &szAddress );
	BOOL Ping( CString szIpAdddr );

	afx_msg void OnBnClickedBtnQue();
	afx_msg void OnBnClickedBtnUploadOpen();
	afx_msg void OnBnClickedBtnUploadFile();
	afx_msg void OnBnClickedBtnDownFile();
	afx_msg void OnBnClickedBtnDelFile();
	afx_msg void OnClose();
	afx_msg void OnBnClickedBtnPing();
	afx_msg void OnBnClickedBtnClearLog();

	static int32_t ProcessCommandQue(int32_t iSock, void *pObj );
	static int32_t ProcessCommandUpload(int32_t iSock, void *pObj );
	static int32_t ProcessCommandDownload(int32_t iSock, void *pObj );
	static int32_t ProcessCommandDelete(int32_t iSock, void *pObj );

public:
	CEdit m_EditLog;
	CEdit m_EditQue;
	CEdit m_EditUploadFile;
	CComboBox m_CmbDownMode;
	CComboBox m_CmbDownColor;
	CComboBox m_CmbDelMode;
	CIPAddressCtrl m_IpAddr;
	CToolTipCtrl m_ToolTip;
};
