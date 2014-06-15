// ChildFrm4.h : header file
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: ChildFrm4.h,v $
// Revision 1.1  2003/09/13 05:45:34  dewhisna
// Initial Revision
//
//

#if !defined(AFX_CHILDFRM4_H__852AB5AF_96B1_4F85_ABF2_00FE3BEB96DD__INCLUDED_)
#define AFX_CHILDFRM4_H__852AB5AF_96B1_4F85_ABF2_00FE3BEB96DD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ChildFrmBase.h"

class COutputView;

/////////////////////////////////////////////////////////////////////////////
// CChildFrame4 frame

class CChildFrame4 : public CChildFrameBase
{
	DECLARE_DYNCREATE(CChildFrame4)
protected:
	CChildFrame4();           // protected constructor used by dynamic creation

// Attributes
public:
	COutputView *m_pOutputView;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChildFrame4)
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CChildFrame4();

	// Generated message map functions
	//{{AFX_MSG(CChildFrame4)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHILDFRM4_H__852AB5AF_96B1_4F85_ABF2_00FE3BEB96DD__INCLUDED_)
