// FuncView.h : main header file for the FUNCVIEW application
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: FuncView.h,v $
// Revision 1.1  2003/09/13 05:45:42  dewhisna
// Initial Revision
//
//

#if !defined(AFX_FUNCVIEW_H__B8D0CB16_BC1E_4ED0_856D_C48685662F00__INCLUDED_)
#define AFX_FUNCVIEW_H__B8D0CB16_BC1E_4ED0_856D_C48685662F00__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CFuncViewApp:
// See FuncView.cpp for the implementation of this class
//

class CFuncViewApp : public CWinApp
{
public:
	CFuncViewApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFuncViewApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CFuncViewApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CFuncViewApp theApp;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FUNCVIEW_H__B8D0CB16_BC1E_4ED0_856D_C48685662F00__INCLUDED_)
