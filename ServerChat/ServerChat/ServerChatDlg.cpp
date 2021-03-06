
// ServerChatDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ServerChat.h"
#include "ServerChatDlg.h"
#include "afxdialogex.h"
#include "Server.h"
#include <atlstr.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CServerChatDlg dialog



CServerChatDlg::CServerChatDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_SERVERCHAT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CServerChatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_KickButton, m_KickButton);
	DDX_Control(pDX, IDC_UserList, m_UserList);
	DDX_Control(pDX, IDC_LogDisplay, m_LogDisplay);
}

BEGIN_MESSAGE_MAP(CServerChatDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_KickButton, &CServerChatDlg::OnBnClickedKickbutton)
	ON_MESSAGE(MESSAGE_NEW_USER,OnNewUser)
	ON_MESSAGE(MESSAGE_NEW_LOG, OnNewLog)
	ON_MESSAGE(MESSAGE_REMOVE_USER, OnRemoveUser)
END_MESSAGE_MAP()


// CServerChatDlg message handlers

BOOL CServerChatDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CServerChatDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CServerChatDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CServerChatDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CServerChatDlg::OnBnClickedKickbutton()
{
	// TODO: Add your control notification handler code here
	int selIndex = m_UserList.GetCurSel();
	//Get string and delete it
	CString tmpUN; 
	m_UserList.GetText(selIndex, tmpUN);
	//Convert to string
	CT2CA cvrted(tmpUN);
	string uName(cvrted);
	CommonData data;
	data.type = "forceKick";
	data.message = uName;
	CSocket sock;
	int toPort;
	sock.Create();
	sock.Connect(L"127.0.0.1", 1234);
	sock.Receive(&toPort, sizeof(int), 0);
	sock.Close();
	sock.Create();
	sock.Connect(L"127.0.0.1", toPort);
	SendCommonData(sock, data);
	sock.Close();
}

LRESULT CServerChatDlg::OnNewUser(WPARAM wParam, LPARAM lParam) {
	CString cString = (LPCTSTR)lParam;
	m_UserList.AddString(cString);
	return 0L;
}

LRESULT CServerChatDlg::OnRemoveUser(WPARAM wParam, LPARAM lParam) {
	CString cString = (LPCTSTR)lParam;
	CString tmpString;
	for (int i = 0; i < m_UserList.GetCount(); i++)
	{
		m_UserList.GetText(i, tmpString);
		if (tmpString == cString)
		{
			m_UserList.DeleteString(i);
			break;
		}
	}
	return 0L;
}

LRESULT CServerChatDlg::OnNewLog(WPARAM wParam, LPARAM lParam) {
	LPCTSTR toAppend = (LPCTSTR)lParam;
	CString Line;
	Line.Format(_T("\r\n%s"), toAppend);
	// get the initial text length
	int nLength = m_LogDisplay.GetWindowTextLength();
	// put the selection at the end of text
	m_LogDisplay.SetSel(nLength, nLength);
	// replace the selection
	m_LogDisplay.ReplaceSel(Line);
	return 0L;
}