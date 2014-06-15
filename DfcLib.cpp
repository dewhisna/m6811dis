//	DfcLib.cpp

//	This is the implementation file for the DFCLibrary as well as the Src Data File template types
//	for MDI and SDI templates.  It also contains a Src Data File DocMgr type as well as both
//	MDI and SDI Appl template classes.  Include this component in your apps to read/write DFC
//	Library types.  This component handles automatic registration and allows the application to
//	dynamically unregister/reregister the file types.

#include "stdafx.h"
#include <afxpriv.h>
#include <dfc.h>
#include "DfcLib.h"
#include <errmsgs.h>

#include <fstream.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//	GLOBAL VARIABLES:
CDFCArray	DFCs;

//	STATIC VARIABLES:
static const TCHAR szShellOpenFmt[] = _T("%s\\shell\\open\\%s");
static const TCHAR szShellPrintFmt[] = _T("%s\\shell\\print\\%s");
static const TCHAR szShellPrintToFmt[] = _T("%s\\shell\\printto\\%s");
static const TCHAR szDefaultIconFmt[] = _T("%s\\DefaultIcon");
static const TCHAR szShellNewFmt[] = _T("%s\\ShellNew");

#define DEFAULT_ICON_INDEX 0
#define AfxDllExtractIcon ::ExtractIcon

static const TCHAR szIconIndexFmt[] = _T(",%d");
static const TCHAR szCommand[] = _T("command");
static const TCHAR szOpenArg[] = _T(" \"%1\"");
static const TCHAR szPrintArg[] = _T(" /p \"%1\"");
static const TCHAR szPrintToArg[] = _T(" /pt \"%1\" \"%2\" \"%3\" \"%4\"");
static const TCHAR szDDEArg[] = _T(" /dde");

static const TCHAR szDDEExec[] = _T("ddeexec");
static const TCHAR szDDEOpen[] = _T("[open(\"%1\")]");
static const TCHAR szDDEPrint[] = _T("[print(\"%1\")]");
static const TCHAR szDDEPrintTo[] = _T("[printto(\"%1\",\"%2\",\"%3\",\"%4\")]");

static const TCHAR szShellNewValueName[] = _T("NullFile");
static const TCHAR szShellNewValue[] = _T("");



IMPLEMENT_DYNAMIC(CSrcMultiDocTemplate, CMultiDocTemplate)
/////////////////////////////////////////////////////////////////////////////
// CSrcMultiDocTemplate construction/destruction

CSrcMultiDocTemplate::CSrcMultiDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass,
	CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass, CDFCLibrary* aLibrary, LPCTSTR anAppName)
	: CMultiDocTemplate(nIDResource, pDocClass, pFrameClass, pViewClass)
{
	m_Description = aLibrary->GetShortDescription();
	m_Extension = aLibrary->DefaultExtension();
	m_AppName = anAppName;
	theLibrary = aLibrary;
}


/////////////////////////////////////////////////////////////////////////////
// CSrcMultiDocTemplate implementation

BOOL CSrcMultiDocTemplate::GetDocString(CString& rString, enum DocStringIndex i) const
{
	CString	Temp;
	CString	AppDesc;

	AppDesc = "";
	Temp = m_AppName + m_Description;
	while (Temp.Find(' ') != -1) {
		AppDesc += Temp.Left(Temp.Find(' '));
		Temp = Temp.Right(Temp.GetLength() - Temp.Find(' ') - 1);
	}
	AppDesc += Temp;

	Temp = _T("\n") + m_Description + _T(" Data File");
	Temp += _T("\n") + m_Description + _T(" File");
	Temp += _T("\n") + m_Description + _T(" Files (*.") + m_Extension + _T(")");
	Temp += _T("\n.") + m_Extension;
	Temp += _T("\n") + AppDesc + _T(".Document");
	Temp += _T("\n") + m_Description + _T(" File");

	return AfxExtractSubString(rString, Temp, (int)i);
}

CDFCLibrary& CSrcMultiDocTemplate::GetLibrary()
{
	return (*theLibrary);
}


/////////////////////////////////////////////////////////////////////////////
// CSrcMdiFrame

IMPLEMENT_DYNCREATE(CSrcMdiFrame, CMDIChildWnd)

CSrcMdiFrame::CSrcMdiFrame()
{
}

CSrcMdiFrame::~CSrcMdiFrame()
{
}


BEGIN_MESSAGE_MAP(CSrcMdiFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CSrcMdiFrame)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSrcMdiFrame message handlers

BOOL CSrcMdiFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style = WS_CHILD | WS_VISIBLE | WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_SYSMENU
		| FWS_ADDTOTITLE | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;

	return CMDIChildWnd::PreCreateWindow(cs);
}

void CSrcMdiFrame::OnSize(UINT nType, int cx, int cy) 
{
	CMDIChildWnd::OnSize(nType, cx, cy);
}

void CSrcMdiFrame::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
	CMDIChildWnd::OnGetMinMaxInfo(lpMMI);

	lpMMI->ptMinTrackSize.x = 150;
	lpMMI->ptMinTrackSize.y = 150;
}

/////////////////////////////////////////////////////////////////////////////
// CSrcDocManager used for Src File Types

IMPLEMENT_DYNAMIC(CSrcDocManager, CDocManager)

BOOL CSrcDocManager::DeleteDocTemplate(CDocTemplate* pTemplate)
{
	if (pTemplate == NULL)
		return FALSE;

	if (m_templateList.Find(pTemplate, NULL) == NULL)
		return FALSE;		// It must be in the list!

	if (pTemplate->GetFirstDocPosition() != NULL)
		return FALSE;		// It must have no open documents

	m_templateList.RemoveAt(m_templateList.Find(pTemplate, NULL));
	delete pTemplate;
	return TRUE;
}

static BOOL AFXAPI SetRegKey(LPCTSTR lpszKey, LPCTSTR lpszValue, LPCTSTR lpszValueName = NULL)
{
	if (lpszValueName == NULL)
	{
		if (::RegSetValue(HKEY_CLASSES_ROOT, lpszKey, REG_SZ,
			  lpszValue, lstrlen(lpszValue) * sizeof(TCHAR)) != ERROR_SUCCESS)
		{
			TRACE1("Warning: registration database update failed for key '%s'.\n",
				lpszKey);
			return FALSE;
		}
		return TRUE;
	}
	else
	{
		HKEY hKey;

		if(::RegCreateKey(HKEY_CLASSES_ROOT, lpszKey, &hKey) == ERROR_SUCCESS)
		{
			LONG lResult = ::RegSetValueEx(hKey, lpszValueName, 0, REG_SZ,
				(CONST BYTE*)lpszValue, (lstrlen(lpszValue) + 1) * sizeof(TCHAR));

			if(::RegCloseKey(hKey) == ERROR_SUCCESS && lResult == ERROR_SUCCESS)
				return TRUE;
		}
		TRACE1("Warning: registration database update failed for key '%s'.\n", lpszKey);
		return FALSE;
	}
}

void CSrcDocManager::RegisterShellFileTypes(BOOL bCompat)
{
	ASSERT(!m_templateList.IsEmpty());  // must have some doc templates

	CString strPathName, strTemp;
	BOOL	Flag;

	AfxGetModuleShortFileName(AfxGetInstanceHandle(), strPathName);

	POSITION pos = m_templateList.GetHeadPosition();
	for (int nTemplateIndex = 1; pos != NULL; nTemplateIndex++)
	{
		CDocTemplate* pTemplate = (CDocTemplate*)m_templateList.GetNext(pos);

		CString strOpenCommandLine = strPathName;
		CString strPrintCommandLine = strPathName;
		CString strPrintToCommandLine = strPathName;
		CString strDefaultIconCommandLine = strPathName;

		Flag = FALSE;
		if (pTemplate->IsKindOf(RUNTIME_CLASS(CSrcMultiDocTemplate ))) {
			AfxGetModuleShortFileName(((CSrcMultiDocTemplate *)pTemplate)->GetLibrary().GetHandle(),
						strDefaultIconCommandLine);
			Flag = TRUE;
		}

		if (bCompat)
		{
			CString strIconIndex;
			strIconIndex.Format(szIconIndexFmt, DEFAULT_ICON_INDEX);
			if (Flag == FALSE) {
				HICON hIcon = AfxDllExtractIcon(AfxGetInstanceHandle(), strPathName, nTemplateIndex);
				if (hIcon != NULL)
				{
					strIconIndex.Format(szIconIndexFmt, nTemplateIndex);
					DestroyIcon(hIcon);
				}
			}
			strDefaultIconCommandLine += strIconIndex;
		}

		CString strFilterExt, strFileTypeId, strFileTypeName;
		if (pTemplate->GetDocString(strFileTypeId,
		   CDocTemplate::regFileTypeId) && !strFileTypeId.IsEmpty())
		{
			// enough info to register it
			if (!pTemplate->GetDocString(strFileTypeName,
			   CDocTemplate::regFileTypeName))
				strFileTypeName = strFileTypeId;    // use id name

			ASSERT(strFileTypeId.Find(' ') == -1);  // no spaces allowed

			// first register the type ID of our server
			if (!SetRegKey(strFileTypeId, strFileTypeName))
				continue;       // just skip it

			if (bCompat)
			{
				// path\DefaultIcon = path,1
				strTemp.Format(szDefaultIconFmt, (LPCTSTR)strFileTypeId);
				if (!SetRegKey(strTemp, strDefaultIconCommandLine))
					continue;       // just skip it
			}

			// If MDI Application
			if (!pTemplate->GetDocString(strTemp, CDocTemplate::windowTitle) ||
				strTemp.IsEmpty())
			{
				// path\shell\open\ddeexec = [open("%1")]
				strTemp.Format(szShellOpenFmt, (LPCTSTR)strFileTypeId,
					(LPCTSTR)szDDEExec);
				if (!SetRegKey(strTemp, szDDEOpen))
					continue;       // just skip it

				if (bCompat)
				{
					// path\shell\print\ddeexec = [print("%1")]
					strTemp.Format(szShellPrintFmt, (LPCTSTR)strFileTypeId,
						(LPCTSTR)szDDEExec);
					if (!SetRegKey(strTemp, szDDEPrint))
						continue;       // just skip it

					// path\shell\printto\ddeexec = [printto("%1","%2","%3","%4")]
					strTemp.Format(szShellPrintToFmt, (LPCTSTR)strFileTypeId,
						(LPCTSTR)szDDEExec);
					if (!SetRegKey(strTemp, szDDEPrintTo))
						continue;       // just skip it

					// path\shell\open\command = path /dde
					// path\shell\print\command = path /dde
					// path\shell\printto\command = path /dde
					strOpenCommandLine += szDDEArg;
					strPrintCommandLine += szDDEArg;
					strPrintToCommandLine += szDDEArg;
				}
				else
				{
					strOpenCommandLine += szOpenArg;
				}
			}
			else
			{
				// path\shell\open\command = path filename
				// path\shell\print\command = path /p filename
				// path\shell\printto\command = path /pt filename printer driver port
				strOpenCommandLine += szOpenArg;
				if (bCompat)
				{
					strPrintCommandLine += szPrintArg;
					strPrintToCommandLine += szPrintToArg;
				}
			}

			// path\shell\open\command = path filename
			strTemp.Format(szShellOpenFmt, (LPCTSTR)strFileTypeId,
				(LPCTSTR)szCommand);
			if (!SetRegKey(strTemp, strOpenCommandLine))
				continue;       // just skip it

			if (bCompat)
			{
				// path\shell\print\command = path /p filename
				strTemp.Format(szShellPrintFmt, (LPCTSTR)strFileTypeId,
					(LPCTSTR)szCommand);
				if (!SetRegKey(strTemp, strPrintCommandLine))
					continue;       // just skip it

				// path\shell\printto\command = path /pt filename printer driver port
				strTemp.Format(szShellPrintToFmt, (LPCTSTR)strFileTypeId,
					(LPCTSTR)szCommand);
				if (!SetRegKey(strTemp, strPrintToCommandLine))
					continue;       // just skip it
			}

			pTemplate->GetDocString(strFilterExt, CDocTemplate::filterExt);
			if (!strFilterExt.IsEmpty())
			{
				ASSERT(strFilterExt[0] == '.');

				LONG lSize = _MAX_PATH * 2;
				LONG lResult = ::RegQueryValue(HKEY_CLASSES_ROOT, strFilterExt,
					strTemp.GetBuffer(lSize), &lSize);
				strTemp.ReleaseBuffer();

				if (lResult != ERROR_SUCCESS || strTemp.IsEmpty() ||
					strTemp == strFileTypeId)
				{
					// no association for that suffix
					if (!SetRegKey(strFilterExt, strFileTypeId))
						continue;

					if (bCompat)
					{
						strTemp.Format(szShellNewFmt, (LPCTSTR)strFilterExt);
						(void)SetRegKey(strTemp, szShellNewValue, szShellNewValueName);
					}
				}
			}
		}
	}
}

static void AppendFilterSuffix(CString& filter, OPENFILENAME& ofn,
	CDocTemplate* pTemplate, CString* pstrDefaultExt)
{
	ASSERT_VALID(pTemplate);
	ASSERT_KINDOF(CDocTemplate, pTemplate);

	CString strFilterExt, strFilterName;
	if (pTemplate->GetDocString(strFilterExt, CDocTemplate::filterExt) &&
	 !strFilterExt.IsEmpty() &&
	 pTemplate->GetDocString(strFilterName, CDocTemplate::filterName) &&
	 !strFilterName.IsEmpty())
	{
		// a file based document template - add to filter list
		ASSERT(strFilterExt[0] == '.');
		if (pstrDefaultExt != NULL)
		{
			// set the default extension
			*pstrDefaultExt = ((LPCTSTR)strFilterExt) + 1;  // skip the '.'
			ofn.lpstrDefExt = (LPTSTR)(LPCTSTR)(*pstrDefaultExt);
			ofn.nFilterIndex = ofn.nMaxCustFilter + 1;  // 1 based number
		}

		// add to filter
		filter += strFilterName;
		ASSERT(!filter.IsEmpty());  // must have a file type name
		filter += (TCHAR)'\0';  // next string please
		filter += (TCHAR)'*';
		filter += strFilterExt;
		filter += (TCHAR)'\0';  // next string please
		ofn.nMaxCustFilter++;
	}
}

CDocTemplate *CSrcDocManager::DoPromptFileName(CString& fileName, UINT nIDSTitle, DWORD lFlags, BOOL bOpenFileDialog, CDocTemplate* pTemplate, BOOL AllDFCs)
{
	CFileDialog dlgFile(bOpenFileDialog);

	CString title;
	VERIFY(title.LoadString(nIDSTitle));

	dlgFile.m_ofn.Flags |= lFlags;

	CString strFilter;
	CString strDefault;
	if (pTemplate != NULL)
	{
		ASSERT_VALID(pTemplate);
		AppendFilterSuffix(strFilter, dlgFile.m_ofn, pTemplate, &strDefault);
		if ((AllDFCs) && (pTemplate->IsKindOf(RUNTIME_CLASS(CSrcMultiDocTemplate)))) {
			POSITION pos = m_templateList.GetHeadPosition();
			while (pos != NULL) {
				CDocTemplate* aTemplate = (CDocTemplate*)m_templateList.GetNext(pos);
				if ((aTemplate->IsKindOf(RUNTIME_CLASS(CSrcMultiDocTemplate))) && (aTemplate != pTemplate)) {
					AppendFilterSuffix(strFilter, dlgFile.m_ofn, aTemplate, NULL);
				}
			}
		}
	}
	else
	{
		// do for all doc template
		POSITION pos = m_templateList.GetHeadPosition();
		BOOL bFirst = TRUE;
		while (pos != NULL)
		{
			CDocTemplate* aTemplate = (CDocTemplate*)m_templateList.GetNext(pos);
			AppendFilterSuffix(strFilter, dlgFile.m_ofn, aTemplate,
				bFirst ? &strDefault : NULL);
			bFirst = FALSE;
		}
	}

	// append the "*.*" all files filter
	CString allFilter;
	VERIFY(allFilter.LoadString(AFX_IDS_ALLFILTER));
	strFilter += allFilter;
	strFilter += (TCHAR)'\0';   // next string please
	strFilter += _T("*.*");
	strFilter += (TCHAR)'\0';   // last string
	dlgFile.m_ofn.nMaxCustFilter++;

	dlgFile.m_ofn.lpstrFilter = strFilter;
	dlgFile.m_ofn.lpstrTitle = title;
	dlgFile.m_ofn.lpstrFile = fileName.GetBuffer(_MAX_PATH);

	BOOL bResult = dlgFile.DoModal() == IDOK ? TRUE : FALSE;
	fileName.ReleaseBuffer();

	CDocTemplate *RetVal = NULL;
	if (bResult) {
		if ((dlgFile.m_ofn.nFilterIndex == 0) ||
			(dlgFile.m_ofn.nFilterIndex == 1) ||
			(dlgFile.m_ofn.nFilterIndex == dlgFile.m_ofn.nMaxCustFilter) ||
			(!AllDFCs) ||
			(!pTemplate->IsKindOf(RUNTIME_CLASS(CSrcMultiDocTemplate)))) {
			RetVal = pTemplate;
		} else {
			POSITION pos = m_templateList.GetHeadPosition();
			DWORD aIndex = 1;
			BOOL Done = FALSE;
			while ((pos != NULL) && (!Done)) {
				CDocTemplate* aTemplate = (CDocTemplate*)m_templateList.GetNext(pos);
				if ((aTemplate->IsKindOf(RUNTIME_CLASS(CSrcMultiDocTemplate))) && (aTemplate != pTemplate)) {
					if (++aIndex == dlgFile.m_ofn.nFilterIndex) {
						RetVal = aTemplate;
						Done = TRUE;
					}
				}
			}
		}
	}

	return RetVal;
}


/////////////////////////////////////////////////////////////////////////////
// CSrcMdiWinApp implementation


BEGIN_MESSAGE_MAP(CSrcMdiWinApp, CWinApp)
	//{{AFX_MSG_MAP(CSrcMdiWinApp)
	//}}AFX_MSG_MAP
	// Standard file based document commands
END_MESSAGE_MAP()


void CSrcMdiWinApp::RegisterFileTypes()
{
	RegisterShellFileTypes(TRUE);
}

void CSrcMdiWinApp::UnregisterFileTypes()
{
	UnregisterShellFileTypes();
}

void CSrcMdiWinApp::AddSourceFileTemplate(CDFCLibrary* aLibrary)
{
	ASSERT(aLibrary != NULL);

	CSrcMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CSrcMultiDocTemplate(
		IDR_SRCTYPE,
		RUNTIME_CLASS(CSrcDoc),
		RUNTIME_CLASS(CSrcMdiFrame), // custom MDI child frame
		RUNTIME_CLASS(CSrcView),
		aLibrary, m_pszAppName);
	AddDocTemplate(pDocTemplate);
	aLibrary->AddLibraryTemplate(pDocTemplate);			// Add reference in Library
}

void CSrcMdiWinApp::AddDocTemplate(CDocTemplate* pTemplate)
{
	if (m_pDocManager == NULL)
		m_pDocManager = new CSrcDocManager;
	m_pDocManager->AddDocTemplate(pTemplate);
}

void CSrcMdiWinApp::DeleteSourceFileTemplate(CDFCLibrary& aLibrary)
{
	POSITION aPos;

	aPos = aLibrary.GetFirstLibraryTemplate();
	while (aPos != NULL) {
		DeleteDocTemplate(aLibrary.GetNextLibraryTemplate(aPos));
	}
	aLibrary.RemoveAllLibraryTemplates();
}

BOOL CSrcMdiWinApp::DeleteDocTemplate(CDocTemplate* pTemplate)
{
	if (m_pDocManager != NULL)
		return (((CSrcDocManager *)m_pDocManager)->DeleteDocTemplate(pTemplate));

	return FALSE;
}

int CSrcMdiWinApp::ExitInstance()
{
	SaveDFCSettings();

	return CWinApp::ExitInstance();
}

void CSrcMdiWinApp::LoadDFCSettings()
{
	int	i;
	CString	Temp;
	int	NumLibraries;
	CString	LibraryName;
	CDFCLibrary*	aDFC;

	NumLibraries = GetProfileInt("DFC Libraries", "Count", 0);
	for (i=0; i<NumLibraries; i++) {
		Temp.Format("%d", i);
		LibraryName = GetProfileString("DFC Libraries", Temp, "");
		if (LibraryName != _T("")) {
			aDFC = new CDFCLibrary(LibraryName);
			if (aDFC != NULL) {
				if (aDFC->IsLoaded()) {
					DFCs.Add(aDFC);
					AddSourceFileTemplate(aDFC);
				} else {
					delete aDFC;
					AfxFormatString1(Temp, IDS_DFC_LOAD_ERROR, LibraryName);
					AfxMessageBox(Temp, MB_OK | MB_ICONEXCLAMATION);
				}
			}
		}
	}
}

void CSrcMdiWinApp::SaveDFCSettings()
{
	int	i;
	CString	Temp;

	WriteProfileInt("DFC Libraries", "Count", DFCs.GetSize());
	for (i=0; i<DFCs.GetSize(); i++) {
		Temp.Format("%d", i);
		WriteProfileString("DFC Libraries", Temp, DFCs.GetAt(i)->GetLibraryName());
	}
}


/////////////////////////////////////////////////////////////////////////////
// CDFCLibrary Implementation
CDFCLibrary::CDFCLibrary()
{
	ValidLoad = 0;
	LoadCount = 0;
	LibraryName = "";
	LibraryHandle = NULL;
	EGetShortDescription = NULL;
	EGetDescription = NULL;
	EDefaultExtension = NULL;
	ERetrieveFileMapping = NULL;
	EReadDataFile = NULL;
	EWriteDataFile = NULL;

	LibraryTemplates.RemoveAll();
}

CDFCLibrary::CDFCLibrary(LPCSTR aLibraryName)
{
	ValidLoad = 0;
	LoadCount = 0;
	LibraryName = aLibraryName;
	LibraryHandle = NULL;
	EGetShortDescription = NULL;
	EGetDescription = NULL;
	EDefaultExtension = NULL;
	ERetrieveFileMapping = NULL;
	EReadDataFile = NULL;
	EWriteDataFile = NULL;

	LibraryTemplates.RemoveAll();

	Load();
}

CDFCLibrary::~CDFCLibrary()
{
	LibraryTemplates.RemoveAll();		// Force an unload condition
	while (LoadCount)
		if (Unload() == -1) break;
}

int CDFCLibrary::Load()
{
	if (LoadCount) {
		LoadCount++;
	} else {
		if ((LibraryName == "") || (LibraryName.IsEmpty())) return 0;
		LibraryHandle = LoadLibrary(LibraryName);
		if (!LibraryHandle) return 0;
		ValidLoad = 1;

		EGetShortDescription = (GDFC_TYPE(GetShortDescription))GetProcAddress(LibraryHandle,
			"GetShortDescription");
		EGetDescription = (GDFC_TYPE(GetDescription))GetProcAddress(LibraryHandle,
			"GetDescription");
		EDefaultExtension = (GDFC_TYPE(DefaultExtension))GetProcAddress(LibraryHandle,
			"DefaultExtension");
		ERetrieveFileMapping = (GDFC_TYPE(RetrieveFileMapping))GetProcAddress(LibraryHandle,
			"RetrieveFileMapping");
		EReadDataFile = (GDFC_TYPE(ReadDataFile))GetProcAddress(LibraryHandle,
			"ReadDataFile");
		EWriteDataFile = (GDFC_TYPE(WriteDataFile))GetProcAddress(LibraryHandle,
			"WriteDataFile");

		LoadCount++;

		if (!((EGetShortDescription) &&
				(EGetDescription) &&
				(EDefaultExtension) &&
				(ERetrieveFileMapping) &&
				(EReadDataFile) &&
				(EWriteDataFile))) {
			Unload();
			return 0;
		}
	}

	return (LoadCount);
}

int CDFCLibrary::Unload()
{
	if ((LoadCount) && (CanUnload())) {
		LoadCount--;
		if (!LoadCount) {
			EGetShortDescription = NULL;
			EGetDescription = NULL;
			EDefaultExtension = NULL;
			ERetrieveFileMapping = NULL;
			EReadDataFile = NULL;
			EWriteDataFile = NULL;

			LibraryTemplates.RemoveAll();

			if (ValidLoad) {
				ValidLoad = 0;
				if (!FreeLibrary(LibraryHandle))
					return -1;
			}
		}
	}
	return (LoadCount);
}

//	This library can be unloaded if and only if the associated template has no documents attached to it.
BOOL CDFCLibrary::CanUnload()
{
	BOOL RetVal = TRUE;
	POSITION	aPos = LibraryTemplates.GetHeadPosition();

	while ((aPos != NULL) && (RetVal)) {
		if (LibraryTemplates.GetNext(aPos)->GetFirstDocPosition() != NULL)
			RetVal = FALSE;
	}
	return RetVal;
}

HINSTANCE CDFCLibrary::GetHandle()
{
	return LibraryHandle;
}

BOOL CDFCLibrary::AddLibraryTemplate(CDocTemplate* aTemplate)
{
// If already here, don't add it
	if (LibraryTemplates.Find(aTemplate, NULL) != NULL)
		return FALSE;

	LibraryTemplates.AddTail(aTemplate);
	return TRUE;
}

BOOL CDFCLibrary::RemoveLibraryTemplate(CDocTemplate* aTemplate)
{
// If it doesn't exist, can't remove it:
	POSITION aPos = LibraryTemplates.Find(aTemplate, NULL);

	if (aPos == NULL)
		return FALSE;

	LibraryTemplates.RemoveAt(aPos);
	return TRUE;
}

POSITION CDFCLibrary::GetFirstLibraryTemplate()
{
	return LibraryTemplates.GetHeadPosition();
}

CDocTemplate* CDFCLibrary::GetNextLibraryTemplate(POSITION &aPos)
{
	return LibraryTemplates.GetNext(aPos);
}

void CDFCLibrary::RemoveAllLibraryTemplates()
{
	LibraryTemplates.RemoveAll();
}

int CDFCLibrary::IsLoaded()
{
	return (ValidLoad);
}

int CDFCLibrary::GetLoadCount()
{
	return (LoadCount);
}

void CDFCLibrary::SetLibraryName(LPCSTR aLibraryName)
{
	LibraryName = aLibraryName;
}

LPCSTR CDFCLibrary::GetLibraryName()
{
	return (LibraryName);
}

LPCSTR CDFCLibrary::GetShortDescription()
{
	if (EGetShortDescription)
		return (EGetShortDescription());
	return ("");
}

LPCSTR CDFCLibrary::GetDescription()
{
	if (EGetDescription)
		return (EGetDescription());
	return ("");
}

LPCSTR CDFCLibrary::DefaultExtension()
{
	if (EDefaultExtension)
		return (EDefaultExtension());
	return ("");
}

int CDFCLibrary::RetrieveFileMapping(istream *aFile, unsigned long NewBase, TMemRange *aRange)
{
	if (ERetrieveFileMapping)
		return (ERetrieveFileMapping(aFile, NewBase, aRange));
	return 0;
}

int CDFCLibrary::ReadDataFile(istream *aFile, unsigned long NewBase, TMemObject *aMemory, unsigned char aDesc)
{
	if (EReadDataFile)
		return (EReadDataFile(aFile, NewBase, aMemory, aDesc));
	return 0;
}

int CDFCLibrary::WriteDataFile(ostream *aFile, TMemRange *aRange, unsigned long NewBase, TMemObject *aMemory,
                        unsigned char aDesc, int UsePhysicalAddr, unsigned int FillMode)
{
	if (EWriteDataFile)
		return (EWriteDataFile(aFile, aRange, NewBase, aMemory, aDesc, UsePhysicalAddr, FillMode));
	return 0;
}


/////////////////////////////////////////////////////////////////////////////
// CDFCArray Implementation
CDFCArray::~CDFCArray()
{
	int i;

	for (i=0; i<GetSize(); i++)
		delete (GetAt(i));
}

BOOL CDFCArray::AlreadyLoaded(CDFCLibrary& aLibrary)
{
	int i;
	CString Temp;

	Temp = aLibrary.GetShortDescription();
	for (i=0; i<GetSize(); i++) {
		if (GetAt(i)->GetHandle() == aLibrary.GetHandle())
			return TRUE;
		if (Temp.CompareNoCase(GetAt(i)->GetShortDescription()) == 0)
			return TRUE;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CNewTypeDlg

class CNewTypeDlg : public CDialog
{
protected:
	CPtrList*   m_pList;        // actually a list of doc templates
public:
	CDocTemplate*   m_pSelectedTemplate;

public:
	//{{AFX_DATA(CNewTypeDlg)
	enum { IDD = AFX_IDD_NEWTYPEDLG };
	//}}AFX_DATA
	CNewTypeDlg(CPtrList* pList) : CDialog(CNewTypeDlg::IDD)
	{
		m_pList = pList;
		m_pSelectedTemplate = NULL;
	}
	~CNewTypeDlg() { }

protected:
	BOOL OnInitDialog();
	void OnOK();
	//{{AFX_MSG(CNewTypeDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CNewTypeDlg, CDialog)
	//{{AFX_MSG_MAP(CNewTypeDlg)
	ON_LBN_DBLCLK(AFX_IDC_LISTBOX, OnOK)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CNewTypeDlg::OnInitDialog()
{
	CListBox* pListBox = (CListBox*)GetDlgItem(AFX_IDC_LISTBOX);
	ASSERT(pListBox != NULL);

	// fill with document templates in list
	pListBox->ResetContent();

	POSITION pos = m_pList->GetHeadPosition();
	// add all the CDocTemplates in the list by name
	while (pos != NULL)
	{
		CDocTemplate* pTemplate = (CDocTemplate*)m_pList->GetNext(pos);
		ASSERT_KINDOF(CDocTemplate, pTemplate);

		CString strTypeName;
		if (pTemplate->GetDocString(strTypeName, CDocTemplate::fileNewName) &&
		   !strTypeName.IsEmpty())
		{
			// add it to the listbox
			int nIndex = pListBox->AddString(strTypeName);
			if (nIndex == -1)
			{
				EndDialog(-1);
				return FALSE;
			}
			pListBox->SetItemDataPtr(nIndex, pTemplate);
		}
	}

	int nTemplates = pListBox->GetCount();
	if (nTemplates == 0)
	{
		TRACE0("Error: no document templates to select from!\n");
		EndDialog(-1); // abort
	}
	else if (nTemplates == 1)
	{
		// get the first/only item
		m_pSelectedTemplate = (CDocTemplate*)pListBox->GetItemDataPtr(0);
		ASSERT_VALID(m_pSelectedTemplate);
		ASSERT_KINDOF(CDocTemplate, m_pSelectedTemplate);
		EndDialog(IDOK);    // done
	}
	else
	{
		// set selection to the first one (NOT SORTED)
		pListBox->SetCurSel(0);
	}

	return CDialog::OnInitDialog();
}

void CNewTypeDlg::OnOK()
{
	CListBox* pListBox = (CListBox*)GetDlgItem(AFX_IDC_LISTBOX);
	ASSERT(pListBox != NULL);
	// if listbox has selection, set the selected template
	int nIndex;
	if ((nIndex = pListBox->GetCurSel()) == -1)
	{
		// no selection
		m_pSelectedTemplate = NULL;
	}
	else
	{
		m_pSelectedTemplate = (CDocTemplate*)pListBox->GetItemDataPtr(nIndex);
		ASSERT_VALID(m_pSelectedTemplate);
		ASSERT_KINDOF(CDocTemplate, m_pSelectedTemplate);
	}
	CDialog::OnOK();
}

/////////////////////////////////////////////////////////////////////////////
// CSrcDoc

IMPLEMENT_DYNCREATE(CSrcDoc, CDocument)

CSrcDoc::CSrcDoc()
{
	m_pMemRange = new TMemRange(0, 0, 0);
	if (m_pMemRange == NULL)
		AfxThrowMemoryException();
	m_pMemory = NULL;

	ReadOffset = 0ul;
	WriteOffset = 0ul;
	ReadDesc = 1;
	WriteDesc = 1;
	WriteUsingPhysical = 0;
	WriteFillMode = CONDITIONAL_FILL_WITH(0);
	UseDesc = 1;
	MemFillByte = 0;
}

void CSrcDoc::DeleteContents() 
{
	if (m_pMemRange)
		m_pMemRange->PurgeRange();

	if (m_pMemory) {
		delete m_pMemory;
		m_pMemory = NULL;
	}

	ReadOffset = 0ul;
	WriteOffset = 0ul;
	ReadDesc = 1;
	WriteDesc = 1;
	WriteUsingPhysical = 0;
	WriteFillMode = CONDITIONAL_FILL_WITH(0);
	UseDesc = 1;
	MemFillByte = 0;

	CDocument::DeleteContents();
}

BOOL CSrcDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	return TRUE;
}

BOOL CSrcDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	DeleteContents();
	SetModifiedFlag();

// Need to add support here for SDI template type
	ASSERT(GetDocTemplate()->IsKindOf(RUNTIME_CLASS( CSrcMultiDocTemplate )));

	CSrcMultiDocTemplate *theTemplate = (CSrcMultiDocTemplate *)GetDocTemplate();
	fstream	*TheFile;


	TheFile = new fstream(lpszPathName, ios::in | ios::binary);
	if (TheFile == NULL) {
		ReportSaveLoadException(lpszPathName, NULL,
			FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
		return FALSE;
	}

	m_pMemRange->PurgeRange();
	try {
		theTemplate->GetLibrary().RetrieveFileMapping(TheFile, ReadOffset, m_pMemRange);
		if (m_pMemory) {
			delete m_pMemory;
			m_pMemory = NULL;
		}
		m_pMemory = new TMemObject(*m_pMemRange, MemFillByte, UseDesc);
		if (m_pMemory == NULL)
			AfxThrowMemoryException();
	}
	catch (EXCEPTION_ERROR *ex) {
		switch (ex->cause) {
			case EXCEPTION_ERROR::ERR_OUT_OF_MEMORY:
				AfxThrowMemoryException();
				break;
			default:
				ReportSaveLoadException(lpszPathName, NULL,
					FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
		}
	}


	try {
		theTemplate->GetLibrary().ReadDataFile(TheFile, ReadOffset, m_pMemory, ReadDesc);
	}
	catch (EXCEPTION_ERROR *ex) {
		switch (ex->cause) {
			case EXCEPTION_ERROR::ERR_CHECKSUM:
				ReportSaveLoadException(lpszPathName, NULL,
					FALSE, IDS_DFC_ERR_CHECKSUM);
				break;
			case EXCEPTION_ERROR::ERR_UNEXPECTED_EOF:
				ReportSaveLoadException(lpszPathName, NULL,
					FALSE, IDS_DFC_ERR_UNEXPECTED_EOF);
				break;
			case EXCEPTION_ERROR::ERR_OVERFLOW:
				ReportSaveLoadException(lpszPathName, NULL,
					FALSE, IDS_DFC_ERR_OVERFLOW);
				break;
			default:
				ReportSaveLoadException(lpszPathName, NULL,
					FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
		}
	}

	TheFile->close();
	delete TheFile;

	SetModifiedFlag(FALSE);

	return TRUE;
}

BOOL CSrcDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
// Need to add support here for SDI template type
	ASSERT(GetDocTemplate()->IsKindOf(RUNTIME_CLASS( CSrcMultiDocTemplate )));

	CSrcMultiDocTemplate *theTemplate = (CSrcMultiDocTemplate *)GetDocTemplate();
	fstream	*TheFile;

	TheFile = new fstream(lpszPathName, ios::out | ios::binary);
	if (TheFile == NULL) {
		ReportSaveLoadException(lpszPathName, NULL,
			TRUE, AFX_IDP_INVALID_FILENAME);
		return FALSE;
	}

	try {
		if (m_pMemory != NULL) {
			theTemplate->GetLibrary().WriteDataFile(TheFile, m_pMemRange, WriteOffset, m_pMemory, WriteDesc,
									WriteUsingPhysical, WriteFillMode);
		}
	}
	catch (EXCEPTION_ERROR *ex) {
		switch (ex->cause) {
			case EXCEPTION_ERROR::ERR_CHECKSUM:
				ReportSaveLoadException(lpszPathName, NULL,
					TRUE, IDS_DFC_ERR_CHECKSUM);
				break;
			case EXCEPTION_ERROR::ERR_UNEXPECTED_EOF:
				ReportSaveLoadException(lpszPathName, NULL,
					TRUE, IDS_DFC_ERR_UNEXPECTED_EOF);
				break;
			case EXCEPTION_ERROR::ERR_OVERFLOW:
				ReportSaveLoadException(lpszPathName, NULL,
					TRUE, IDS_DFC_ERR_OVERFLOW);
				break;
			default:
				ReportSaveLoadException(lpszPathName, NULL,
					TRUE, AFX_IDP_FAILED_TO_SAVE_DOC);
		}
	}

	TheFile->close();
	delete TheFile;

	SetModifiedFlag(FALSE);
	return TRUE;
}

void CSrcDoc::CopyFrom(CSrcDoc& aSource)
{
	if (m_pMemory) {
		delete m_pMemory;
		m_pMemory = NULL;
	}
	if (aSource.m_pMemory) {
		m_pMemory = new TMemObject(*aSource.m_pMemory);
	}

	if (m_pMemRange) {
		delete m_pMemRange;
		m_pMemRange = NULL;
	}
	if (aSource.m_pMemRange) {
		m_pMemRange = new TMemRange(*aSource.m_pMemRange);
	}

	ReadOffset = aSource.ReadOffset;
	WriteOffset = aSource.WriteOffset;
	ReadDesc = aSource.ReadDesc;
	WriteDesc = aSource.WriteDesc;
	WriteUsingPhysical = aSource.WriteUsingPhysical;
	WriteFillMode = aSource.WriteFillMode;
	UseDesc = aSource.UseDesc;
	MemFillByte = aSource.MemFillByte;

	SetModifiedFlag();
}

void CSrcDoc::OnFileConvert() 
{
	CPtrList	DFCList;
	CDocTemplate*	pTemplate;

	POSITION pos = AfxGetApp()->GetFirstDocTemplatePosition();
	while (pos != NULL) {
		CDocTemplate* aTemplate = AfxGetApp()->GetNextDocTemplate(pos);
		if (aTemplate->IsKindOf(RUNTIME_CLASS(CSrcMultiDocTemplate))) {
			DFCList.AddTail(aTemplate);
		}
	}

	// more than one document template to choose from
	// bring up dialog prompting user
	CNewTypeDlg dlg(&DFCList);
	int nID = dlg.DoModal();
	if (nID == IDOK)
		pTemplate = dlg.m_pSelectedTemplate;
	else
		return;     // none - cancel operation

	ASSERT(pTemplate != NULL);
	ASSERT_KINDOF(CDocTemplate, pTemplate);

	CDocument *pDocument = pTemplate->OpenDocumentFile(NULL);
	if (pDocument) {
		((CSrcDoc *)pDocument)->CopyFrom(*this);
	}
}

CSrcDoc::~CSrcDoc()
{
	if (m_pMemRange) {
		delete m_pMemRange;
		m_pMemRange = NULL;
	}

	if (m_pMemory) {
		delete m_pMemory;
		m_pMemory = NULL;
	}
}


BEGIN_MESSAGE_MAP(CSrcDoc, CDocument)
	//{{AFX_MSG_MAP(CSrcDoc)
	ON_COMMAND(ID_FILE_CONVERT, OnFileConvert)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSrcDoc diagnostics

#ifdef _DEBUG
void CSrcDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CSrcDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


void CSrcDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{

	}
	else
	{

	}
}


/////////////////////////////////////////////////////////////////////////////
// CSrcDoc commands

TMemObject* CSrcDoc::GetMemory()
{
	return (m_pMemory);
}

TMemRange* CSrcDoc::GetRange()
{
	return (m_pMemRange);
}


/////////////////////////////////////////////////////////////////////////////
// CSrcView

IMPLEMENT_DYNCREATE(CSrcView, CFormView)

CSrcView::CSrcView()
	: CFormView(CSrcView::IDD)
{
	//{{AFX_DATA_INIT(CSrcView)
	//}}AFX_DATA_INIT
}

CSrcView::~CSrcView()
{
}

void CSrcView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSrcView)
	DDX_Control(pDX, IDC_HEXEDIT, m_CHexEdit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSrcView, CFormView)
	//{{AFX_MSG_MAP(CSrcView)
	ON_WM_SIZE()
	ON_COMMAND(ID_HEXEDIT_PROPERTIES, OnHexeditProperties)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSrcView diagnostics

#ifdef _DEBUG
void CSrcView::AssertValid() const
{
	CFormView::AssertValid();
}

void CSrcView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSrcView message handlers

void CSrcView::OnSize(UINT nType, int cx, int cy) 
{
	CFormView::OnSize(nType, cx, cy);

	if (::IsWindow(m_CHexEdit.m_hWnd)) {
		m_CHexEdit.SetWindowPos(NULL, 0, 0, cx, cy, SWP_NOZORDER);
	}
}

void CSrcView::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();

	CRect	rcWnd;

	GetClientRect(&rcWnd);
	m_CHexEdit.SetWindowPos(NULL, 0, 0, rcWnd.Width(), rcWnd.Height(), SWP_NOZORDER);
	m_CHexEdit.SetFocus();

//	m_CHexEdit.GetWindowRect(&rcWnd);
	GetParentFrame()->RecalcLayout();
	SetScaleToFitSize(CSize(rcWnd.Width(), rcWnd.Height()));

	CSrcDoc* theDoc = (CSrcDoc*)GetDocument();
	m_CHexEdit.SetMemory((long)theDoc->GetMemory());
	m_CHexEdit.SetMemoryRange((long)theDoc->GetRange());

//	EnableToolTips(TRUE);
}

void CSrcView::OnHexeditProperties() 
{
	m_CHexEdit.DoProperties();
}

//int CSrcView::OnToolHitTest( CPoint point, TOOLINFO* pTI ) const
//{
//	static int hit = 0;
//
//	ASSERT(pTI != NULL);
//
//	if (m_hWnd == NULL) {
//		return -1;
//	}
//
//	if (!::IsWindow(m_hWnd)) {
//		return -1;
//	}
//
//	pTI->hwnd = m_CHexEdit.m_hWnd;
//	pTI->uId = (UINT)m_CHexEdit.m_hWnd;
////	pTI->uFlags = TTF_SUBCLASS;
////	pTI->uFlags = TTF_IDISHWND;
//	pTI->uFlags = /* TTF_IDISHWND | */ TTF_SUBCLASS | TTF_NOTBUTTON | TTF_ALWAYSTIP;
//	m_CHexEdit.GetClientRect(&pTI->rect);
////	pTI->rect.left = point.x;
////	pTI->rect.right = point.x+1;
////	pTI->rect.top = point.y;
////	pTI->rect.bottom = point.y+1;
//	pTI->lpszText = LPSTR_TEXTCALLBACK;
//	hit = 1-hit;
//	return hit;
////	return 1;
//}
