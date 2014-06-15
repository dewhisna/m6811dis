// FuncListView.h : header file
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: FuncListView.h,v $
// Revision 1.1  2003/09/13 05:45:40  dewhisna
// Initial Revision
//
//

#if !defined(AFX_FUNCLISTVIEW_H__E5ABE2A7_C95F_4260_930A_1B4737A42CAB__INCLUDED_)
#define AFX_FUNCLISTVIEW_H__E5ABE2A7_C95F_4260_930A_1B4737A42CAB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CFuncViewPrjDoc;

/////////////////////////////////////////////////////////////////////////////
// CFuncListView view

class CFuncListView : public CListView
{
protected:
	CFuncListView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CFuncListView)

// Attributes
public:
	CFuncViewPrjDoc* GetDocument();

protected:
	int m_nFileSet;					// Index into CFuncViewPrjDoc FileSets for this list
	CDWordArray m_arrFileIndexes;	// Index into FileSet Files for a specific Function File
	CDWordArray m_arrFuncIndexes;	// Index into a specific Function File for a specific Function

// Operations
public:
	int GetFileSetIndex();
	void SetFileSetIndex(int nFileSet);

protected:
	void FillFuncList();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFuncListView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CFuncListView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CFuncListView)
	afx_msg void OnItemchanged(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in FuncListView.cpp
inline CFuncViewPrjDoc* CFuncListView::GetDocument()
   { return (CFuncViewPrjDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FUNCLISTVIEW_H__E5ABE2A7_C95F_4260_930A_1B4737A42CAB__INCLUDED_)
