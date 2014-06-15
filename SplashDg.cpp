// SplashDg.cpp : implementation file
//

#include "resource.h"		// main symbols
#include "stdafx.h"
#include "SplashDg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSplashDlg dialog


UINT CSplashDlg::SplashTime;
BOOL CSplashDlg::c_bShowSplashDlg;
CSplashDlg* CSplashDlg::c_pSplashDlg;
CSplashDlg::CSplashDlg()
{
	//{{AFX_DATA_INIT(CSplashDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


CSplashDlg::~CSplashDlg()
{
	// Clear the static window pointer.
	ASSERT(c_pSplashDlg == this);
	c_pSplashDlg = NULL;
}


BEGIN_MESSAGE_MAP(CSplashDlg, CWnd)
	//{{AFX_MSG_MAP(CSplashDlg)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_ACTIVATE()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSplashDlg message handlers


void CSplashDlg::EnableSplashScreen(BOOL bEnable /*= TRUE*/)
{
	c_bShowSplashDlg = bEnable;
}

void CSplashDlg::ShowSplashScreen(UINT anID, UINT aTime /* = 0 */, CWnd* pParentWnd /*= NULL*/)
{
	if ((!c_bShowSplashDlg) || (c_pSplashDlg != NULL))
		return;

	// Allocate a new splash screen, and create the window.
	c_pSplashDlg = new CSplashDlg;
	if (!c_pSplashDlg->Create(anID, aTime, pParentWnd))
		delete c_pSplashDlg;
	else {
		c_pSplashDlg->UpdateWindow();
	}
}

int CSplashDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Center the window.
	CenterWindow();

	// Set a timer to destroy the splash screen.
	if (SplashTime) {
		SetTimer(1, SplashTime, NULL);
	}

	return 0;
}

BOOL CSplashDlg::Create(UINT anID, UINT aTime /* = 0 */, CWnd* pParentWnd /*= NULL*/)
{
	if (!m_bitmap.LoadBitmap(anID))
		return FALSE;

	SplashTime = aTime;

	BITMAP bm;
	m_bitmap.GetBitmap(&bm);

	return CreateEx(0,
		AfxRegisterWndClass(0, AfxGetApp()->LoadStandardCursor(IDC_ARROW)),
		NULL, WS_POPUP | WS_VISIBLE, 0, 0, bm.bmWidth, bm.bmHeight, pParentWnd->GetSafeHwnd(), NULL);
}

void CSplashDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	CWnd::OnActivate(nState, pWndOther, bMinimized);

	if (SplashTime == 0) {
		if (((nState == WA_ACTIVE) || (nState == WA_CLICKACTIVE)) && (pWndOther)) {
			pWndOther->SetActiveWindow();
		}
	}
}

void CSplashDlg::HideSplashScreen()
{
	// Destroy the window, and update the mainframe.
	DestroyWindow();
	AfxGetMainWnd()->UpdateWindow();
}

void CSplashDlg::OnTimer(UINT nIDEvent) 
{
	// Destroy the splash screen window.
	HideSplashScreen();
}

BOOL CSplashDlg::PreTranslateMessage(MSG* pMsg)
{
	if (c_pSplashDlg == NULL)
		return FALSE;

	if (SplashTime == 0)
		return FALSE;

	// If we get a keyboard or mouse message, hide the splash screen.
	if (pMsg->message == WM_KEYDOWN ||
	    pMsg->message == WM_SYSKEYDOWN ||
	    pMsg->message == WM_LBUTTONDOWN ||
	    pMsg->message == WM_RBUTTONDOWN ||
	    pMsg->message == WM_MBUTTONDOWN ||
	    pMsg->message == WM_NCLBUTTONDOWN ||
	    pMsg->message == WM_NCRBUTTONDOWN ||
	    pMsg->message == WM_NCMBUTTONDOWN)
	{
		c_pSplashDlg->HideSplashScreen();
		return TRUE;	// message handled here
	}

	return FALSE;	// message not handled
}

void CSplashDlg::PostNcDestroy()
{
	// Free the C++ class.
	delete this;
}

void CSplashDlg::OnPaint()
{
	CPaintDC dc(this);

	CDC dcImage;
	if (!dcImage.CreateCompatibleDC(&dc))
		return;

	BITMAP bm;
	m_bitmap.GetBitmap(&bm);

	// Paint the image.
	CBitmap* pOldBitmap = dcImage.SelectObject(&m_bitmap);
	dc.BitBlt(0, 0, bm.bmWidth, bm.bmHeight, &dcImage, 0, 0, SRCCOPY);
	dcImage.SelectObject(pOldBitmap);
}


//////////////////////////////////////////////////////////////////////
// CSplashCommand Class
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSplashCommand::CSplashCommand()
{
	m_bShowMascot = FALSE;
}

CSplashCommand::~CSplashCommand()
{

}

void CSplashCommand::ParseParam( LPCTSTR lpszParam, BOOL bFlag, BOOL bLast )
{
	CString	KeyWord;

	KeyWord = lpszParam;
	KeyWord.MakeUpper();
	if ((bFlag) && (KeyWord == "MASCOT")) {
		m_bShowMascot = TRUE;
	} else {
		m_bShowMascot = FALSE;
		CCommandLineInfo::ParseParam(lpszParam, bFlag, bLast);
	}
}
