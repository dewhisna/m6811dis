// FuncDiffEditView.h : header file
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: FuncDiffEditView.h,v $
// Revision 1.1  2003/09/13 05:45:39  dewhisna
// Initial Revision
//
//

#if !defined(AFX_FUNCDIFFEDITVIEW_H__70724D9C_D78A_4475_AB58_24E1DA5853DC__INCLUDED_)
#define AFX_FUNCDIFFEDITVIEW_H__70724D9C_D78A_4475_AB58_24E1DA5853DC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FuncViewPrjDoc.h"

/////////////////////////////////////////////////////////////////////////////
// CFuncDiffEditView view

class CFuncDiffEditView : public CEditView
{
protected:
	CFuncDiffEditView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CFuncDiffEditView)

// Attributes
public:
	CFuncViewPrjDoc* GetDocument();
	CString m_strMatchPercentage;

protected:
	CFuncViewFuncRef m_LeftFunc;
	CFuncViewFuncRef m_RightFunc;

	CFont m_fntEditor;

	friend class CChildFrame2;

// Operations
public:

protected:
	void DoDiff();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFuncDiffEditView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CFuncDiffEditView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CFuncDiffEditView)
	afx_msg void OnSymbolsAdd();
	afx_msg void OnUpdateSymbolsAdd(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in FuncDiffEditView.cpp
inline CFuncViewPrjDoc* CFuncDiffEditView::GetDocument()
   { return (CFuncViewPrjDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FUNCDIFFEDITVIEW_H__70724D9C_D78A_4475_AB58_24E1DA5853DC__INCLUDED_)
