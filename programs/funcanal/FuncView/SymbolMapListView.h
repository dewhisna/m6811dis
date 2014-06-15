// SymbolMapListView.h : header file
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: SymbolMapListView.h,v $
// Revision 1.1  2003/09/13 05:45:50  dewhisna
// Initial Revision
//
//

#if !defined(AFX_SYMBOLMAPLISTVIEW_H__FAB9EBAB_755D_4BD3_AFE9_966D0A3BF19B__INCLUDED_)
#define AFX_SYMBOLMAPLISTVIEW_H__FAB9EBAB_755D_4BD3_AFE9_966D0A3BF19B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CFuncViewPrjDoc;

/////////////////////////////////////////////////////////////////////////////
// CSymbolMapListView view

class CSymbolMapListView : public CListView
{
protected:
	CSymbolMapListView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CSymbolMapListView)

// Attributes
public:
	CFuncViewPrjDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSymbolMapListView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CSymbolMapListView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	void FillSymbolMapList(DWORD nSource = 0, LPCTSTR pszSymbolName = NULL);

	// Generated message map functions
protected:
	//{{AFX_MSG(CSymbolMapListView)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in SymbolMapListView.cpp
inline CFuncViewPrjDoc* CSymbolMapListView::GetDocument()
   { return (CFuncViewPrjDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SYMBOLMAPLISTVIEW_H__FAB9EBAB_755D_4BD3_AFE9_966D0A3BF19B__INCLUDED_)
