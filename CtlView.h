// CtlView.h : interface of the CCTLFileView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CTLVIEW_H__25668D37_606B_11D1_858F_00600828300C__INCLUDED_)
#define AFX_CTLVIEW_H__25668D37_606B_11D1_858F_00600828300C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CCTLFileView : public CFormView
{
protected: // create from serialization only
	CCTLFileView();
	DECLARE_DYNCREATE(CCTLFileView)

public:
	//{{AFX_DATA(CCTLFileView)
	enum{ IDD = IDD_M6811DIS_FORM };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Attributes
public:
	CCTLFileDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCTLFileView)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo*);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CCTLFileView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CCTLFileView)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in CtlView.cpp
inline CCTLFileDoc* CCTLFileView::GetDocument()
   { return (CCTLFileDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CTLVIEW_H__25668D37_606B_11D1_858F_00600828300C__INCLUDED_)
