// ChildFrmBase.cpp : implementation of the CChildFrameBase class
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: ChildFrmBase.cpp,v $
// Revision 1.1  2003/09/13 05:45:36  dewhisna
// Initial Revision
//
//

#include "stdafx.h"
#include "FuncView.h"

#include "ChildFrmBase.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChildFrameBase

IMPLEMENT_DYNCREATE(CChildFrameBase, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrameBase, CMDIChildWnd)
	//{{AFX_MSG_MAP(CChildFrameBase)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChildFrameBase construction/destruction

CChildFrameBase::CChildFrameBase()
{
	m_strFrameTitle = "";
}

CChildFrameBase::~CChildFrameBase()
{

}

BOOL CChildFrameBase::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	m_strFrameTitle = cs.lpszName;

	return CMDIChildWnd::PreCreateWindow(cs);
}


/////////////////////////////////////////////////////////////////////////////
// CChildFrameBase diagnostics

#ifdef _DEBUG
void CChildFrameBase::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CChildFrameBase::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CChildFrameBaes message handlers

void CChildFrameBase::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	// update our parent window first
	GetMDIFrame()->OnUpdateFrameTitle(bAddToTitle);

	CDocument* pDocument = GetActiveDocument();
	if (bAddToTitle)
	{
		TCHAR szText[256+_MAX_PATH];
		if (pDocument == NULL) {
			lstrcpy(szText, m_strTitle);
		} else {
			lstrcpy(szText, pDocument->GetTitle());
			if (!m_strFrameTitle.IsEmpty()) lstrcat(szText, " - ");
			lstrcat(szText, m_strFrameTitle);
		}
		if (m_nWindow > 0)
			wsprintf(szText + lstrlen(szText), _T(":%d"), m_nWindow);

		// set title if changed, but don't remove completely
		SetWindowText(szText);
	}
}

