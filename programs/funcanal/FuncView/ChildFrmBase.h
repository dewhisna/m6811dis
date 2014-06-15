// ChildFrmBase.h : interface of the CChildFrame class
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: ChildFrmBase.h,v $
// Revision 1.1  2003/09/13 05:45:36  dewhisna
// Initial Revision
//
//

#if !defined(_CHILDFRMBASE_H__INCLUDED_)
#define _CHILDFRMBASE_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CChildFrameBase : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CChildFrameBase)
public:
	CChildFrameBase();

// Attributes:
public:
	CString m_strFrameTitle;

// Operations:
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChildFrameBase)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);

// Implementation
public:
	virtual ~CChildFrameBase();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(CChildFrameBase)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(_CHILDFRMBASE_H__INCLUDED_)
