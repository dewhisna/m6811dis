// ChildFrm6.cpp : implementation file
//
// Frame Window for the CompDiffEdit Views
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: ChildFrm6.cpp,v $
// Revision 1.1  2003/09/13 05:45:35  dewhisna
// Initial Revision
//
//

#include "stdafx.h"
#include "FuncView.h"
#include "ChildFrm6.h"

#include "CompDiffEditView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChildFrame6

IMPLEMENT_DYNCREATE(CChildFrame6, CChildFrameBase)

CChildFrame6::CChildFrame6()
{
}

CChildFrame6::~CChildFrame6()
{
}


BEGIN_MESSAGE_MAP(CChildFrame6, CChildFrameBase)
	//{{AFX_MSG_MAP(CChildFrame6)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChildFrame6 message handlers

BOOL CChildFrame6::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	ASSERT(pContext != NULL);

	pContext->m_pNewViewClass = RUNTIME_CLASS(CCompDiffEditView);
	if (CreateView(pContext, AFX_IDW_PANE_FIRST) == NULL) return FALSE;

	return TRUE;

//	return CChildFrameBase::OnCreateClient(lpcs, pContext);
}

