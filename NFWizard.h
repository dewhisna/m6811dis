// NFWizard.h : header file
//
// This class defines custom modal property sheet 
// CNewFileWizard.
 
#ifndef __NFWIZARD_H__
#define __NFWIZARD_H__

#include "NFPages.h"

/////////////////////////////////////////////////////////////////////////////
// CNewFileWizard

class CNewFileWizard : public CPropertySheet
{
	DECLARE_DYNAMIC(CNewFileWizard)

// Construction
public:
	CNewFileWizard(CWnd* pWndParent = NULL);

// Attributes
public:
	CNewFileIntroPage m_Page1;
	CNewFileTypePage m_Page2;
	CNewFileSettingsPage m_Page3;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewFileWizard)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CNewFileWizard();

// Generated message map functions
protected:
	//{{AFX_MSG(CNewFileWizard)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif	// __NFWIZARD_H__
