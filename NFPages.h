// NFPages.h : header file
//

#ifndef __NFPAGES_H__
#define __NFPAGES_H__

/////////////////////////////////////////////////////////////////////////////
// CNewFileIntroPage dialog

class CNewFileIntroPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CNewFileIntroPage)

// Construction
public:
	CNewFileIntroPage();
	~CNewFileIntroPage();

// Dialog Data
	//{{AFX_DATA(CNewFileIntroPage)
	enum { IDD = IDD_NEWFILE_INTRO };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CNewFileIntroPage)
	public:
	virtual LRESULT OnWizardNext();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void SetWizardButtons(void);
	// Generated message map functions
	//{{AFX_MSG(CNewFileIntroPage)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////
// CNewFileTypePage dialog

class CNewFileTypePage : public CPropertyPage
{
	DECLARE_DYNCREATE(CNewFileTypePage)

// Construction
public:
	CNewFileTypePage();
	~CNewFileTypePage();

// Dialog Data
	//{{AFX_DATA(CNewFileTypePage)
	enum { IDD = IDD_NEWFILE_TYPE };
	CListBox	m_CFileTypeList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CNewFileTypePage)
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void FillFileTypeList(void);
	void SetWizardButtons(void);
	// Generated message map functions
	//{{AFX_MSG(CNewFileTypePage)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////
// CNewFileSettingsPage dialog

class CNewFileSettingsPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CNewFileSettingsPage)

// Construction
public:
	CNewFileSettingsPage();
	~CNewFileSettingsPage();

// Dialog Data
	//{{AFX_DATA(CNewFileSettingsPage)
	enum { IDD = IDD_NEWFILE_SETTINGS };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CNewFileSettingsPage)
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void SetWizardButtons(void);
	// Generated message map functions
	//{{AFX_MSG(CNewFileSettingsPage)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};



#endif // __NFPAGES_H__
