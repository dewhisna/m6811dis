// NFPages.cpp : implementation file
//

#include "stdafx.h"
#include "M6811DIS.h"
#include "NFPages.h"

#include "SysSet.h"
#include "DfcLib.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CNewFileIntroPage, CPropertyPage)
IMPLEMENT_DYNCREATE(CNewFileTypePage, CPropertyPage)
IMPLEMENT_DYNCREATE(CNewFileSettingsPage, CPropertyPage)


/////////////////////////////////////////////////////////////////////////////
// CNewFileIntroPage property page

CNewFileIntroPage::CNewFileIntroPage() : CPropertyPage(CNewFileIntroPage::IDD)
{
	//{{AFX_DATA_INIT(CNewFileIntroPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CNewFileIntroPage::~CNewFileIntroPage()
{
}

void CNewFileIntroPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewFileIntroPage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewFileIntroPage, CPropertyPage)
	//{{AFX_MSG_MAP(CNewFileIntroPage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CNewFileIntroPage::OnInitDialog() 
{
	CButton*	CFButton;
	CButton*	DFButton;

	CPropertyPage::OnInitDialog();

	CFButton = (CButton *)GetDlgItem(IDC_NFTYPE_CONTROL);
	DFButton = (CButton *)GetDlgItem(IDC_NFTYPE_DATA);

	if (CFButton)
		CFButton->SetCheck(1);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT CNewFileIntroPage::OnWizardNext() 
{
	CButton*	CFButton;
	CButton*	DFButton;

	CFButton = (CButton *)GetDlgItem(IDC_NFTYPE_CONTROL);
	DFButton = (CButton *)GetDlgItem(IDC_NFTYPE_DATA);

	if (CFButton) {
		if (CFButton->GetCheck()) {
			return IDD_NEWFILE_SETTINGS;
		} else {
			return IDD_NEWFILE_TYPE;
		}
	}
	return -1;
}

BOOL CNewFileIntroPage::OnSetActive() 
{
	((CPropertySheet *)GetParent())->SetWizardButtons(PSWIZB_NEXT);
	return CPropertyPage::OnSetActive();
}

void CNewFileIntroPage::SetWizardButtons()
{

}

/////////////////////////////////////////////////////////////////////////////
// CNewFileTypePage property page

CNewFileTypePage::CNewFileTypePage() : CPropertyPage(CNewFileTypePage::IDD)
{
	//{{AFX_DATA_INIT(CNewFileTypePage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CNewFileTypePage::~CNewFileTypePage()
{
}

void CNewFileTypePage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewFileTypePage)
	DDX_Control(pDX, IDC_FILE_TYPE_LIST, m_CFileTypeList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewFileTypePage, CPropertyPage)
	//{{AFX_MSG_MAP(CNewFileTypePage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CNewFileTypePage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	FillFileTypeList();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CNewFileTypePage::OnSetActive() 
{
	SetWizardButtons();
	return CPropertyPage::OnSetActive();
}

LRESULT CNewFileTypePage::OnWizardBack() 
{
	return IDD_NEWFILE_INTRO;
}

void CNewFileTypePage::SetWizardButtons()
{
	((CPropertySheet *)GetParent())->SetWizardButtons(PSWIZB_BACK);
}

void CNewFileTypePage::FillFileTypeList()
{
	int i;
	int NewItem;
	CString	Temp;

	m_CFileTypeList.ResetContent();
	for (i=0; i<DFCs.GetSize(); i++) {
		Temp = DFCs.GetAt(i)->GetShortDescription();
		Temp +=" File";
		NewItem = m_CFileTypeList.InsertString(-1, Temp);
		m_CFileTypeList.SetItemData(NewItem, i);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CNewFileSettingsPage property page

CNewFileSettingsPage::CNewFileSettingsPage() : CPropertyPage(CNewFileSettingsPage::IDD)
{
	//{{AFX_DATA_INIT(CNewFileSettingsPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CNewFileSettingsPage::~CNewFileSettingsPage()
{
}

void CNewFileSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewFileSettingsPage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewFileSettingsPage, CPropertyPage)
	//{{AFX_MSG_MAP(CNewFileSettingsPage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CNewFileSettingsPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CNewFileSettingsPage::OnSetActive() 
{
	SetWizardButtons();
	return CPropertyPage::OnSetActive();
}

LRESULT CNewFileSettingsPage::OnWizardBack() 
{
	return IDD_NEWFILE_INTRO;
}

void CNewFileSettingsPage::SetWizardButtons()
{
	((CPropertySheet *)GetParent())->SetWizardButtons(PSWIZB_BACK);
}
