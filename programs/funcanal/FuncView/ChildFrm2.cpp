// ChildFrm2.cpp : implementation file
//
// Frame Window for the Function Diff Views
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: ChildFrm2.cpp,v $
// Revision 1.1  2003/09/13 05:45:32  dewhisna
// Initial Revision
//
//

#include "stdafx.h"
#include "FuncView.h"
#include "ChildFrm2.h"

#include "FuncListView.h"
#include "FuncDiffEditView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_MATCH_PERCENT,
};

/////////////////////////////////////////////////////////////////////////////
// CChildFrame2

IMPLEMENT_DYNCREATE(CChildFrame2, CChildFrameBase)

CChildFrame2::CChildFrame2()
{
	m_pLeftFuncList = NULL;
	m_pRightFuncList = NULL;
	m_pDiffView = NULL;
}

CChildFrame2::~CChildFrame2()
{
}

BEGIN_MESSAGE_MAP(CChildFrame2, CChildFrameBase)
	//{{AFX_MSG_MAP(CChildFrame2)
	ON_WM_CREATE()
	ON_COMMAND(ID_SYMBOLS_ADD, OnSymbolsAdd)
	ON_UPDATE_COMMAND_UI(ID_SYMBOLS_ADD, OnUpdateSymbolsAdd)
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_MATCH_PERCENT, OnUpdateMatchPercent)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChildFrame2 message handlers

BOOL CChildFrame2::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	if (!m_wndSplitter.CreateStatic(this, 1, 3)) return FALSE;

	if (!m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CFuncListView), CSize(230, 0), pContext)) return FALSE;
	if (!m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CFuncListView), CSize(230, 0), pContext)) return FALSE;
	if (!m_wndSplitter.CreateView(0, 2, RUNTIME_CLASS(CFuncDiffEditView), CSize(300, 0), pContext)) return FALSE;

	m_pLeftFuncList = (CFuncListView *)m_wndSplitter.GetPane(0, 0);
	m_pRightFuncList = (CFuncListView *)m_wndSplitter.GetPane(0, 1);
	m_pDiffView = (CFuncDiffEditView *)m_wndSplitter.GetPane(0, 2);

	return TRUE;

//	return CChildFrameBase::OnCreateClient(lpcs, pContext);
}

int CChildFrame2::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CChildFrameBase::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	return 0;
}

void CChildFrame2::OnUpdateMatchPercent(CCmdUI* pCmdUI)
{
	if (m_pDiffView == NULL) {
		pCmdUI->SetText("");
	} else {
		pCmdUI->SetText(m_pDiffView->m_strMatchPercentage);
	}
}

void CChildFrame2::OnSymbolsAdd() 
{
	// Pass on to diff view to handle:
	if (m_pDiffView) m_pDiffView->OnSymbolsAdd();
}

void CChildFrame2::OnUpdateSymbolsAdd(CCmdUI* pCmdUI) 
{
	// Pass on to diff view to handle:
	if (m_pDiffView) {
		m_pDiffView->OnUpdateSymbolsAdd(pCmdUI);
	} else {
		pCmdUI->Enable(FALSE);
	}
}

