
// ServerChatDlg.h : header file
//

#pragma once
#include "afxwin.h"

// CServerChatDlg dialog
class CServerChatDlg : public CDialog
{
// Construction
public:
	CServerChatDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SERVERCHAT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedKickbutton();
	afx_msg LRESULT OnNewUser(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnRemoveUser(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNewLog(WPARAM wParam, LPARAM lParam);
	CButton m_KickButton;
	CListBox m_UserList;
	CEdit m_LogDisplay;
};
