// CompDiffEditView.h : header file
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: CompDiffEditView.h,v $
// Revision 1.1  2003/09/13 05:45:37  dewhisna
// Initial Revision
//
//

#if !defined(AFX_COMPDIFFEDITVIEW_H__1A399480_51DB_4106_8311_DE331ED9A3A0__INCLUDED_)
#define AFX_COMPDIFFEDITVIEW_H__1A399480_51DB_4106_8311_DE331ED9A3A0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FuncViewPrjDoc.h"

/////////////////////////////////////////////////////////////////////////////
// CCompDiffEditView view

class CCompDiffEditView : public CEditView
{
protected:
	CCompDiffEditView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CCompDiffEditView)

// Attributes
public:
	CFuncViewPrjDoc* GetDocument();

protected:
	CFont m_fntEditor;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCompDiffEditView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CCompDiffEditView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CCompDiffEditView)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in CompDiffEditView.cpp
inline CFuncViewPrjDoc* CCompDiffEditView::GetDocument()
   { return (CFuncViewPrjDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COMPDIFFEDITVIEW_H__1A399480_51DB_4106_8311_DE331ED9A3A0__INCLUDED_)
