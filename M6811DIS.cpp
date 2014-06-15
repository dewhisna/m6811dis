// M6811DIS.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "M6811DIS.h"

#include "SysSet.h"

#include "DfcLib.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "CtlDoc.h"
#include "CtlView.h"
#include "Splash.h"
#include "SplashDg.h"
#include <dos.h>
#include <direct.h>

#include <ReadVer.h>
#include "NFWizard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CM6811DISApp

BEGIN_MESSAGE_MAP(CM6811DISApp, CSrcMdiWinApp)
	//{{AFX_MSG_MAP(CM6811DISApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
//	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CSrcMdiWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CSrcMdiWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CSrcMdiWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CM6811DISApp construction

CM6811DISApp::CM6811DISApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CM6811DISApp object

CM6811DISApp theApp;
CSystemSettings	SystemSettings;

/////////////////////////////////////////////////////////////////////////////
// CM6811DISApp initialization

BOOL CM6811DISApp::InitInstance()
{
	CSplashCommand	cmdInfo;

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Change the registry key under which our settings are stored.
	// You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Dewtronics"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(
		IDR_DSMCTLTYPE,
		RUNTIME_CLASS(CCTLFileDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CCTLFileView));
	AddDocTemplate(pDocTemplate);

	LoadDFCSettings();		// Load any registered DFC Libraries

	// Parse command line for standard shell commands, DDE, file open
	ParseCommandLine(cmdInfo);
	CSplashWnd::EnableSplashScreen(cmdInfo.m_bShowSplash);
	CSplashDlg::EnableSplashScreen(cmdInfo.m_bShowMascot);

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;

	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterFileTypes();

	// Dispatch commands specified on the command line
	if (cmdInfo.m_nShellCommand != CCommandLineInfo::FileNew) {
		if (!ProcessShellCommand(cmdInfo))
			return FALSE;
	}

	// The main window has been initialized, so show and update it.
	pMainFrame->ShowWindow(SW_SHOWMAXIMIZED);
	pMainFrame->UpdateWindow();

	CSplashDlg::ShowSplashScreen(IDB_KATHY, 6000, pMainFrame);
	// CG: The following line was added by the Splash Screen component.
	CSplashWnd::ShowSplashScreen(pMainFrame);

	return TRUE;
}

int CM6811DISApp::ExitInstance() 
{
	return CSrcMdiWinApp::ExitInstance();
}

void CM6811DISApp::OnFileNew() 
{
	CNewFileWizard	FileWiz;

	FileWiz.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual BOOL OnInitDialog();
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();	// CG:  This was added by System Info Component.

	CProjectRCVersion	VersionInfo(NULL);

	CString strVersion;
	CString	strLegalCopyright;
	CString strFreeDiskSpace;
	CString strFreeMemory;
	CString strFmt;

	// Fill Version Information
	VersionInfo.GetInfo(VINFO_PRODUCTVERSION, strVersion);
	strVersion = "Version " + strVersion;
	SetDlgItemText(IDC_VERSION, strVersion);

	// Fill Copyright Information
	VersionInfo.GetInfo(VINFO_LEGALCOPYRIGHT, strLegalCopyright);
	SetDlgItemText(IDC_COPYRIGHT, strLegalCopyright);

	// Fill available memory
	MEMORYSTATUS MemStat;
	MemStat.dwLength = sizeof(MEMORYSTATUS);
	GlobalMemoryStatus(&MemStat);
	strFmt.LoadString(CG_IDS_PHYSICAL_MEM);
	strFreeMemory.Format(strFmt, MemStat.dwTotalPhys / 1024L);

	SetDlgItemText(IDC_PHYSICAL_MEM, strFreeMemory);

	// Fill disk free information
	struct _diskfree_t diskfree;
	int nDrive = _getdrive(); // use current default drive
	if (_getdiskfree(nDrive, &diskfree) == 0)
	{
		strFmt.LoadString(CG_IDS_DISK_SPACE);
		strFreeDiskSpace.Format(strFmt,
			(DWORD)diskfree.avail_clusters *
			(DWORD)diskfree.sectors_per_cluster *
			(DWORD)diskfree.bytes_per_sector / (DWORD)1024L,
			nDrive-1 + _T('A'));
	}
	else
		strFreeDiskSpace.LoadString(CG_IDS_DISK_SPACE_UNAVAIL);

	SetDlgItemText(IDC_DISK_SPACE, strFreeDiskSpace);

	return TRUE;	// CG:  This was added by System Info Component.
}

/////////////////////////////////////////////////////////////////////////////
// CM6811DISApp commands

// App command to run the dialog
void CM6811DISApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

BOOL CM6811DISApp::PreTranslateMessage(MSG* pMsg)
{
	// CG: The following lines were added by the Splash Screen component.
	if (CSplashWnd::PreTranslateAppMessage(pMsg))
		return TRUE;

	return CSrcMdiWinApp::PreTranslateMessage(pMsg);
}
