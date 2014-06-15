// ChildFrm6.h : header file
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: ChildFrm6.h,v $
// Revision 1.1  2003/09/13 05:45:36  dewhisna
// Initial Revision
//
//

#if !defined(AFX_CHILDFRM6_H__92F9AA21_0C5F_4C43_BA72_95EF6BD85F4A__INCLUDED_)
#define AFX_CHILDFRM6_H__92F9AA21_0C5F_4C43_BA72_95EF6BD85F4A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ChildFrmBase.h"

/////////////////////////////////////////////////////////////////////////////
// CChildFrame6 frame

class CChildFrame6 : public CChildFrameBase
{
	DECLARE_DYNCREATE(CChildFrame6)
protected:
	CChildFrame6();           // protected constructor used by dynamic creation

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChildFrame6)
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CChildFrame6();

	// Generated message map functions
	//{{AFX_MSG(CChildFrame6)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHILDFRM6_H__92F9AA21_0C5F_4C43_BA72_95EF6BD85F4A__INCLUDED_)
