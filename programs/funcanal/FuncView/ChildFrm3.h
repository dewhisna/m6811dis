// ChildFrm3.h : header file
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: ChildFrm3.h,v $
// Revision 1.1  2003/09/13 05:45:33  dewhisna
// Initial Revision
//
//

#if !defined(AFX_CHILDFRM3_H__A86FCD44_79E0_4642_8520_2C75C5B83EF6__INCLUDED_)
#define AFX_CHILDFRM3_H__A86FCD44_79E0_4642_8520_2C75C5B83EF6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ChildFrmBase.h"

/////////////////////////////////////////////////////////////////////////////
// CChildFrame3 frame

class CChildFrame3 : public CChildFrameBase
{
	DECLARE_DYNCREATE(CChildFrame3)
protected:
	CChildFrame3();           // protected constructor used by dynamic creation

// Attributes
public:

protected:
	CSplitterWnd m_wndSplitter;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChildFrame3)
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CChildFrame3();

	// Generated message map functions
	//{{AFX_MSG(CChildFrame3)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHILDFRM3_H__A86FCD44_79E0_4642_8520_2C75C5B83EF6__INCLUDED_)
