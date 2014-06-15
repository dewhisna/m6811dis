// ChildFrm2.h : header file
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: ChildFrm2.h,v $
// Revision 1.1  2003/09/13 05:45:33  dewhisna
// Initial Revision
//
//

#if !defined(AFX_CHILDFRM2_H__31E158BD_BFD8_4AB8_B28B_361FE203A9DD__INCLUDED_)
#define AFX_CHILDFRM2_H__31E158BD_BFD8_4AB8_B28B_361FE203A9DD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ChildFrmBase.h"

class CFuncListView;
class CFuncDiffEditView;

/////////////////////////////////////////////////////////////////////////////
// CChildFrame2 frame

class CChildFrame2 : public CChildFrameBase
{
	DECLARE_DYNCREATE(CChildFrame2)
protected:
	CChildFrame2();           // protected constructor used by dynamic creation

// Attributes
public:
	CFuncListView *m_pLeftFuncList;
	CFuncListView *m_pRightFuncList;
	CFuncDiffEditView *m_pDiffView;

protected:
	CSplitterWnd m_wndSplitter;
	CStatusBar  m_wndStatusBar;


// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChildFrame2)
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CChildFrame2();

	// Generated message map functions
	//{{AFX_MSG(CChildFrame2)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSymbolsAdd();
	afx_msg void OnUpdateSymbolsAdd(CCmdUI* pCmdUI);
	//}}AFX_MSG
	afx_msg void OnUpdateMatchPercent(CCmdUI* pCmdUI);

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHILDFRM2_H__31E158BD_BFD8_4AB8_B28B_361FE203A9DD__INCLUDED_)
