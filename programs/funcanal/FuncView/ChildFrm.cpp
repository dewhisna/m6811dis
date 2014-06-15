// ChildFrm.cpp : implementation of the CChildFrame class
//
// Frame Window for the Project Views
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: ChildFrm.cpp,v $
// Revision 1.1  2003/09/13 05:45:31  dewhisna
// Initial Revision
//
//

#include "stdafx.h"
#include "FuncView.h"

#include "ChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CChildFrameBase)

BEGIN_MESSAGE_MAP(CChildFrame, CChildFrameBase)
	//{{AFX_MSG_MAP(CChildFrame)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChildFrame construction/destruction

CChildFrame::CChildFrame()
{
}

CChildFrame::~CChildFrame()
{
}

/////////////////////////////////////////////////////////////////////////////
// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CChildFrameBase::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CChildFrameBase::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CChildFrame message handlers

