// CompMatrixView.h : header file
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: CompMatrixView.h,v $
// Revision 1.1  2003/09/13 05:45:38  dewhisna
// Initial Revision
//
//

#if !defined(AFX_COMPMATRIXVIEW_H__ECF95B71_7B54_4A0D_95E0_C871FC74DD2F__INCLUDED_)
#define AFX_COMPMATRIXVIEW_H__ECF95B71_7B54_4A0D_95E0_C871FC74DD2F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GridCtrl.h"

class CFuncViewPrjDoc;

/////////////////////////////////////////////////////////////////////////////
// CCompMatrixView view

class CCompMatrixView : public CView
{
protected:
	CCompMatrixView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CCompMatrixView)

// Attributes
public:
	CFuncViewPrjDoc* GetDocument();

protected:
	CGridCtrl *m_pGridCtrl;

// Operations
public:

protected:
	void FillMatrixHeaders();
	void FillMatrixData(int nSingleRow = -1, int nSingleCol = -1);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCompMatrixView)
	public:
	virtual void OnInitialUpdate();
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnDraw(CDC* pDC);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CCompMatrixView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CCompMatrixView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSymbolsAdd();
	afx_msg void OnUpdateSymbolsAdd(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in CompMatrixView.cpp
inline CFuncViewPrjDoc* CCompMatrixView::GetDocument()
   { return (CFuncViewPrjDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COMPMATRIXVIEW_H__ECF95B71_7B54_4A0D_95E0_C871FC74DD2F__INCLUDED_)
