// SSDFCPg.cpp : implementation file
//

#include "stdafx.h"
#include "M6811DIS.h"
#include "SSDFCPg.h"

#include "DfcLib.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CDFCSettingsPage, CPropertyPage)


/////////////////////////////////////////////////////////////////////////////
// CDFCSettingsPage property page

CDFCSettingsPage::CDFCSettingsPage() : CPropertyPage(CDFCSettingsPage::IDD)
{
	//{{AFX_DATA_INIT(CDFCSettingsPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CDFCSettingsPage::~CDFCSettingsPage()
{
}

void CDFCSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDFCSettingsPage)
	DDX_Control(pDX, IDC_UNLOAD, m_CUnload);
	DDX_Control(pDX, IDC_LOAD, m_CLoad);
	DDX_Control(pDX, IDC_DFC_LIST, m_CDFCList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDFCSettingsPage, CPropertyPage)
	//{{AFX_MSG_MAP(CDFCSettingsPage)
	ON_BN_CLICKED(IDC_LOAD, OnLoad)
	ON_BN_CLICKED(IDC_UNLOAD, OnUnload)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_DFC_LIST, OnItemchangedDfcList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



BOOL CDFCSettingsPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	m_CDFCList.InsertColumn(0, "Module Name", LVCFMT_LEFT, 135, 0);
	m_CDFCList.InsertColumn(1, "Description", LVCFMT_LEFT, 200, 1);
	m_CDFCList.InsertColumn(2, "Default Ext", LVCFMT_LEFT, 75, 2);

	FillDFCList();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDFCSettingsPage::FillDFCList()
{
	int i;
	int	NewItem;
	CDFCLibrary*	aDFC;
	CFile	aFile;

	m_CDFCList.DeleteAllItems();
	for (i=0; i<DFCs.GetSize(); i++) {
		aDFC = DFCs.GetAt(i);
		if (aDFC->IsLoaded()) {
			aFile.SetFilePath(aDFC->GetLibraryName());
			NewItem = m_CDFCList.InsertItem(i, aFile.GetFileName());
			m_CDFCList.SetItemText(NewItem, 1, aDFC->GetDescription());
			m_CDFCList.SetItemText(NewItem, 2, aDFC->DefaultExtension());
			m_CDFCList.SetItemData(NewItem, i);
		}
	}
	SetButtons();
}

void CDFCSettingsPage::SetButtons()
{
	int	SelItem;

	m_CLoad.EnableWindow(TRUE);

	SelItem = m_CDFCList.GetNextItem(-1, LVNI_SELECTED);
	if (SelItem != -1) {
		// Only allow unloading if it isn't in use -- i.e. load count < 2
		//	The system loading will be the first count, and each instance will be additional counts.
		if ((DFCs.GetAt((int)m_CDFCList.GetItemData(SelItem))->GetLoadCount() < 2) &&
			(DFCs.GetAt((int)m_CDFCList.GetItemData(SelItem))->CanUnload()))
			m_CUnload.EnableWindow(TRUE);
		else
			m_CUnload.EnableWindow(FALSE);
	} else {
		m_CUnload.EnableWindow(FALSE);
	}
}

void CDFCSettingsPage::OnLoad() 
{
	CString	Filters;
	CString	TempFilter;

	Filters.LoadString(IDS_FILTER_DFCFILES);
	TempFilter.LoadString(IDS_FILTER_DLLFILES);
	Filters += TempFilter;
	TempFilter.LoadString(IDS_FILTER_ALLFILES);
	Filters += TempFilter;
	Filters += '|';

	CFileDialog	FileDlg(TRUE, "DFC", NULL, OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST |
							OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR, Filters, this);

	if (FileDlg.DoModal() == IDOK) {
		POSITION	NextFilePos;
		CString		LibraryName;
		CString		Temp;
		CDFCLibrary*	aDFC;

		NextFilePos = FileDlg.GetStartPosition();
		while (NextFilePos) {
			LibraryName = FileDlg.GetNextPathName(NextFilePos);
			aDFC = new CDFCLibrary(LibraryName);
			if (aDFC == NULL) continue;
			if (aDFC->IsLoaded()) {
				if (DFCs.AlreadyLoaded(*aDFC)) {
					AfxFormatString1(Temp, IDS_DFC_ALREADY_LOADED_ERROR, LibraryName);
					AfxMessageBox(Temp, MB_OK | MB_ICONEXCLAMATION);
					aDFC->Unload();
					delete aDFC;
					continue;
				}
				DFCs.Add(aDFC);
				theApp.AddSourceFileTemplate(aDFC);
				theApp.RegisterFileTypes();
				CancelToClose();
			} else {
				AfxFormatString1(Temp, IDS_DFC_LOAD_ERROR, LibraryName);
				AfxMessageBox(Temp, MB_OK | MB_ICONEXCLAMATION);
				delete aDFC;
			}
		}
		FillDFCList();
	}
}

void CDFCSettingsPage::OnUnload() 
{
	int	SelItem;
	CDFCLibrary* aDFC;

	m_CLoad.EnableWindow(TRUE);
	SelItem = m_CDFCList.GetNextItem(-1, LVNI_SELECTED);
	if (SelItem != -1) {
		// Only allow unloading if it isn't in use -- i.e. load count < 2
		//	The system loading will be the first count, and each instance will be additional counts.
		if ((DFCs.GetAt((int)m_CDFCList.GetItemData(SelItem))->GetLoadCount() < 2) &&
			(DFCs.GetAt((int)m_CDFCList.GetItemData(SelItem))->CanUnload())) {
			aDFC = DFCs.GetAt((int)m_CDFCList.GetItemData(SelItem));
			theApp.UnregisterFileTypes();
			theApp.DeleteSourceFileTemplate(*aDFC);
			theApp.RegisterFileTypes();
			delete aDFC;
			DFCs.RemoveAt((int)m_CDFCList.GetItemData(SelItem));
			CancelToClose();
			FillDFCList();
		}
	}
}

void CDFCSettingsPage::OnItemchangedDfcList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	SetButtons();

	*pResult = 0;
}
