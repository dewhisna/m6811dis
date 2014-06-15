// M6811DIS.h : main header file for the M6811DIS application
//

#if !defined(AFX_M6811DIS_H__25668D2D_606B_11D1_858F_00600828300C__INCLUDED_)
#define AFX_M6811DIS_H__25668D2D_606B_11D1_858F_00600828300C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "DfcLib.h"
#include "SysSet.h"

/////////////////////////////////////////////////////////////////////////////
// CM6811DISApp:
// See M6811DIS.cpp for the implementation of this class
//

class CM6811DISApp : public CSrcMdiWinApp
{
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CM6811DISApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CM6811DISApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CM6811DISApp)
	afx_msg void OnAppAbout();
	afx_msg void OnFileNew();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
};


/////////////////////////////////////////////////////////////////////////////

extern	CM6811DISApp	theApp;
extern	CSystemSettings	SystemSettings;


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_M6811DIS_H__25668D2D_606B_11D1_858F_00600828300C__INCLUDED_)
