// ChildFrm5.cpp : implementation file
//
// Frame Window for the CompMatrix Views
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: ChildFrm5.cpp,v $
// Revision 1.1  2003/09/13 05:45:35  dewhisna
// Initial Revision
//
//

#include "stdafx.h"
#include "FuncView.h"
#include "ChildFrm5.h"

#include "CompMatrixView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChildFrame5

IMPLEMENT_DYNCREATE(CChildFrame5, CChildFrameBase)

CChildFrame5::CChildFrame5()
{
}

CChildFrame5::~CChildFrame5()
{
}


BEGIN_MESSAGE_MAP(CChildFrame5, CChildFrameBase)
	//{{AFX_MSG_MAP(CChildFrame5)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChildFrame5 message handlers

BOOL CChildFrame5::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	ASSERT(pContext != NULL);

	pContext->m_pNewViewClass = RUNTIME_CLASS(CCompMatrixView);
	if (CreateView(pContext, AFX_IDW_PANE_FIRST) == NULL) return FALSE;

	return TRUE;

//	return CChildFrameBase::OnCreateClient(lpcs, pContext);
}

