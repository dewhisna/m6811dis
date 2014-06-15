// ChildFrm.h : interface of the CChildFrame class
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: ChildFrm.h,v $
// Revision 1.1  2003/09/13 05:45:32  dewhisna
// Initial Revision
//
//

#if !defined(AFX_CHILDFRM_H__ECCE1C4D_DEA2_4247_AD3B_0DA1072EB30F__INCLUDED_)
#define AFX_CHILDFRM_H__ECCE1C4D_DEA2_4247_AD3B_0DA1072EB30F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ChildFrmBase.h"

class CChildFrame : public CChildFrameBase
{
	DECLARE_DYNCREATE(CChildFrame)
public:
	CChildFrame();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChildFrame)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CChildFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(CChildFrame)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHILDFRM_H__ECCE1C4D_DEA2_4247_AD3B_0DA1072EB30F__INCLUDED_)

