#if !defined(AFX_SPLASHDG_H__2B8FAD21_FF05_11D0_805D_787202C10627__INCLUDED_)
#define AFX_SPLASHDG_H__2B8FAD21_FF05_11D0_805D_787202C10627__INCLUDED_


#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SplashDg.h : header file
//

#include "resource.h"		// main symbols
#include <afxwin.h>

/////////////////////////////////////////////////////////////////////////////
// CSplashDlg dialog

class CSplashDlg : public CWnd
{
// Construction
protected:
	CSplashDlg();   // standard constructor

public:
	CBitmap m_bitmap;

	static void EnableSplashScreen(BOOL bEnable = TRUE);
	static void ShowSplashScreen(UINT anID, UINT aTime = 0, CWnd* pParentWnd = NULL);
	static BOOL PreTranslateAppMessage(MSG* pMsg);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSplashDlg)
	//}}AFX_VIRTUAL

// Implementation
public:
	~CSplashDlg();
	virtual void PostNcDestroy();

protected:
	BOOL Create(UINT anID, UINT aTime = 0, CWnd* pParentWnd = NULL);
	void HideSplashScreen();
	static UINT SplashTime;
	static BOOL c_bShowSplashDlg;
	static CSplashDlg* c_pSplashDlg;

public:
	// Generated message map functions
	//{{AFX_MSG(CSplashDlg)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnPaint();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


class CSplashCommand : public CCommandLineInfo  
{
public:
	CSplashCommand();
	virtual ~CSplashCommand();

	virtual void ParseParam( LPCTSTR lpszParam, BOOL bFlag, BOOL bLast );

	BOOL	m_bShowMascot;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPLASHDG_H__2B8FAD21_FF05_11D0_805D_787202C10627__INCLUDED_)
