// SSSheet.h : header file
//
// This class defines custom modal property sheet 
// CSystemSettingsSheet.
 
#ifndef __SSSHEET_H__
#define __SSSHEET_H__

#include "SSDFCPg.h"

/////////////////////////////////////////////////////////////////////////////
// CSystemSettingsSheet

class CSystemSettingsSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CSystemSettingsSheet)

// Construction
public:
	CSystemSettingsSheet(CWnd* pWndParent = NULL);

// Attributes
public:
	CDFCSettingsPage m_Page1;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSystemSettingsSheet)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSystemSettingsSheet();

// Generated message map functions
protected:
	//{{AFX_MSG(CSystemSettingsSheet)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif	// __SSSHEET_H__
