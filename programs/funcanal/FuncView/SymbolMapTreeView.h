// SymbolMapTreeView.h : header file
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: SymbolMapTreeView.h,v $
// Revision 1.1  2003/09/13 05:45:51  dewhisna
// Initial Revision
//
//

#if !defined(AFX_SYMBOLMAPTREEVIEW_H__E57293FF_22E0_47C9_A41F_DAE7161BB334__INCLUDED_)
#define AFX_SYMBOLMAPTREEVIEW_H__E57293FF_22E0_47C9_A41F_DAE7161BB334__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CFuncViewPrjDoc;

/////////////////////////////////////////////////////////////////////////////
// CSymbolMapTreeView view

class CSymbolMapTreeView : public CTreeView
{
protected:
	CSymbolMapTreeView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CSymbolMapTreeView)

// Attributes
public:
	CFuncViewPrjDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSymbolMapTreeView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CSymbolMapTreeView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	void FillSymbolMapList();
	void FillSymbolMapSubList(HTREEITEM hParent, CStringArray &aSymbolList, DWORD nItemData);

	// Generated message map functions
protected:
	//{{AFX_MSG(CSymbolMapTreeView)
	afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in SymbolMapTreeView.cpp
inline CFuncViewPrjDoc* CSymbolMapTreeView::GetDocument()
   { return (CFuncViewPrjDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SYMBOLMAPTREEVIEW_H__E57293FF_22E0_47C9_A41F_DAE7161BB334__INCLUDED_)
