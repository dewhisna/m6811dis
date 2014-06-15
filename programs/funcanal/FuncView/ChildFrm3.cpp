// ChildFrm3.cpp : implementation file
//
// Frame Window for the Symbol Map Views
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: ChildFrm3.cpp,v $
// Revision 1.1  2003/09/13 05:45:33  dewhisna
// Initial Revision
//
//

#include "stdafx.h"
#include "FuncView.h"
#include "ChildFrm3.h"

#include "SymbolMapTreeView.h"
#include "SymbolMapListView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChildFrame3

IMPLEMENT_DYNCREATE(CChildFrame3, CChildFrameBase)

CChildFrame3::CChildFrame3()
{
}

CChildFrame3::~CChildFrame3()
{
}


BEGIN_MESSAGE_MAP(CChildFrame3, CChildFrameBase)
	//{{AFX_MSG_MAP(CChildFrame3)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChildFrame3 message handlers

BOOL CChildFrame3::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	if (!m_wndSplitter.CreateStatic(this, 1, 2)) return FALSE;

	if (!m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CSymbolMapTreeView), CSize(300, 0), pContext)) return FALSE;
	if (!m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CSymbolMapListView), CSize(300, 0), pContext)) return FALSE;

	return TRUE;

//	return CChildFrameBase::OnCreateClient(lpcs, pContext);
}

