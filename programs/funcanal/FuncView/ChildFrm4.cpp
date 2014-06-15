// ChildFrm4.cpp : implementation file
//
// Frame Window for the Output Views
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: ChildFrm4.cpp,v $
// Revision 1.1  2003/09/13 05:45:34  dewhisna
// Initial Revision
//
//

#include "stdafx.h"
#include "FuncView.h"
#include "ChildFrm4.h"

#include "OutputView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChildFrame4

IMPLEMENT_DYNCREATE(CChildFrame4, CChildFrameBase)

CChildFrame4::CChildFrame4()
{
	m_pOutputView = NULL;
}

CChildFrame4::~CChildFrame4()
{
}


BEGIN_MESSAGE_MAP(CChildFrame4, CChildFrameBase)
	//{{AFX_MSG_MAP(CChildFrame4)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChildFrame4 message handlers

BOOL CChildFrame4::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	ASSERT(pContext != NULL);

	pContext->m_pNewViewClass = RUNTIME_CLASS(COutputView);

	m_pOutputView = (COutputView *)CreateView(pContext, AFX_IDW_PANE_FIRST);
	if (m_pOutputView == NULL) return FALSE;

	return TRUE;

//	return CChildFrameBase::OnCreateClient(lpcs, pContext);
}

