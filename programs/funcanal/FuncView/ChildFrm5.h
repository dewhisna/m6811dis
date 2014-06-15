// ChildFrm5.h : header file
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: ChildFrm5.h,v $
// Revision 1.1  2003/09/13 05:45:35  dewhisna
// Initial Revision
//
//

#if !defined(AFX_CHILDFRM5_H__BCBE933F_40AB_4822_9B37_535399CF6EBA__INCLUDED_)
#define AFX_CHILDFRM5_H__BCBE933F_40AB_4822_9B37_535399CF6EBA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ChildFrmBase.h"

/////////////////////////////////////////////////////////////////////////////
// CChildFrame5 frame

class CChildFrame5 : public CChildFrameBase
{
	DECLARE_DYNCREATE(CChildFrame5)
protected:
	CChildFrame5();           // protected constructor used by dynamic creation

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChildFrame5)
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CChildFrame5();

	// Generated message map functions
	//{{AFX_MSG(CChildFrame5)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHILDFRM5_H__BCBE933F_40AB_4822_9B37_535399CF6EBA__INCLUDED_)
