// SSDFCPg.h : header file
//

#ifndef __SSDFCPG_H__
#define __SSDFCPG_H__

#include <LstCtlEx.h>

/////////////////////////////////////////////////////////////////////////////
// CDFCSettingsPage dialog

class CDFCSettingsPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CDFCSettingsPage)

// Construction
public:
	CDFCSettingsPage();
	~CDFCSettingsPage();

// Dialog Data
	//{{AFX_DATA(CDFCSettingsPage)
	enum { IDD = IDD_SS_DFC_SETTINGS };
	CButton	m_CUnload;
	CButton	m_CLoad;
	CListCtrlEx	m_CDFCList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CDFCSettingsPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void SetButtons(void);
	void FillDFCList();
	// Generated message map functions
	//{{AFX_MSG(CDFCSettingsPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnLoad();
	afx_msg void OnUnload();
	afx_msg void OnItemchangedDfcList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};



#endif // __SSDFCPG_H__
