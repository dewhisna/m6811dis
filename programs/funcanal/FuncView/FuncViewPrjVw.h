// FuncViewPrjVw.h : interface of the CFuncViewPrjView class
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: FuncViewPrjVw.h,v $
// Revision 1.1  2003/09/13 05:45:45  dewhisna
// Initial Revision
//
//

#if !defined(AFX_FUNCVIEWPRJVW_H__D64C9E65_798E_486F_8563_28B4FD7D6EDD__INCLUDED_)
#define AFX_FUNCVIEWPRJVW_H__D64C9E65_798E_486F_8563_28B4FD7D6EDD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CFuncViewPrjDoc;

/////////////////////////////////////////////////////////////////////////////
// CFuncViewPrjView view

class CFuncViewPrjView : public CTreeView
{
protected: // create from serialization only
	CFuncViewPrjView();
	DECLARE_DYNCREATE(CFuncViewPrjView)

// Attributes
public:
	CFuncViewPrjDoc* GetDocument();

protected:
	HTREEITEM m_hMatrixBaseNode;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFuncViewPrjView)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFuncViewPrjView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	void FillFileLists();
	void FillMatrixLists();

// Generated message map functions
protected:
	//{{AFX_MSG(CFuncViewPrjView)
	afx_msg void OnEditClear();
	afx_msg void OnUpdateEditClear(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in FuncViewPrjVw.cpp
inline CFuncViewPrjDoc* CFuncViewPrjView::GetDocument()
   { return (CFuncViewPrjDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FUNCVIEWPRJVW_H__D64C9E65_798E_486F_8563_28B4FD7D6EDD__INCLUDED_)
