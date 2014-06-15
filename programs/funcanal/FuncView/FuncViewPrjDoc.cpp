// FuncViewPrjDoc.cpp : implementation of the CFuncViewPrjDoc class
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: FuncViewPrjDoc.cpp,v $
// Revision 1.1  2003/09/13 05:45:44  dewhisna
// Initial Revision
//
//

#include "stdafx.h"
#include "FuncView.h"

#include "ProgDlg.h"

#include "MainFrm.h"
#include "FuncViewPrjDoc.h"			// Project Document View -- ChildFrame
#include "ChildFrm2.h"				// Func Diff View -- ChildFrame2
#include "FuncListView.h"
#include "ChildFrm3.h"				// Symbol Map View -- ChildFrame3
#include "ChildFrm4.h"				// Output View -- ChildFrame4
#include "OutputView.h"
#include "ChildFrm5.h"				// Comp Matrix View -- ChildFrame5
#include "ChildFrm6.h"				// Comp Diff Edit View -- ChildFrame6

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const char *m_arrstrFileSetSectionNames[CFuncViewPrjDoc::FVFS_COUNT] =
{ "LeftFiles", "RightFiles" };

static const char *m_strMatrixSectionName = "MatrixFiles";

static const char m_strNumFilesEntry[] = "NumFiles";
static const char m_strFileEntryPrefix[] = "File";


/////////////////////////////////////////////////////////////////////////////
// CFuncViewPrjDoc

IMPLEMENT_DYNCREATE(CFuncViewPrjDoc, CDocument)

BEGIN_MESSAGE_MAP(CFuncViewPrjDoc, CDocument)
	//{{AFX_MSG_MAP(CFuncViewPrjDoc)
	ON_COMMAND(ID_VIEW_FUNC_DIFF, OnViewFuncDiff)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FUNC_DIFF, OnUpdateViewFuncDiff)
	ON_COMMAND(ID_SYMBOLS_RESET, OnSymbolsReset)
	ON_COMMAND(ID_VIEW_SYMBOL_MAP, OnViewSymbolMap)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SYMBOL_MAP, OnUpdateViewSymbolMap)
	ON_COMMAND(ID_VIEW_OUTPUT_WINDOW, OnViewOutputWindow)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OUTPUT_WINDOW, OnUpdateViewOutputWindow)
	ON_COMMAND(ID_VIEW_COMP_MATRIX, OnViewCompMatrix)
	ON_UPDATE_COMMAND_UI(ID_VIEW_COMP_MATRIX, OnUpdateViewCompMatrix)
	ON_COMMAND(ID_MATRIX_REBUILD, OnMatrixRebuild)
	ON_COMMAND(ID_MATRIX_REBUILD_FORCE, OnMatrixRebuildForce)
	ON_COMMAND(ID_SYMBOLS_EXPORT, OnSymbolsExport)
	ON_UPDATE_COMMAND_UI(ID_SYMBOLS_EXPORT, OnUpdateSymbolsExport)
	ON_COMMAND(ID_VIEW_COMP_DIFF, OnViewCompDiff)
	ON_UPDATE_COMMAND_UI(ID_VIEW_COMP_DIFF, OnUpdateViewCompDiff)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFuncViewPrjDoc construction/destruction

CFuncViewPrjDoc::CFuncViewPrjDoc()
{
	m_bFileOpenInProgress = FALSE;

	m_nMatrixRowCount = 0;
	m_nMatrixColCount = 0;
	m_ppnMatrix = NULL;
}

CFuncViewPrjDoc::~CFuncViewPrjDoc()
{
	FreeMatrixMemory();
}

/////////////////////////////////////////////////////////////////////////////
// CFuncViewPrjDoc callbacks

BOOL CALLBACK CFuncViewPrjDoc::FuncDescFileProgressCallback(int nProgressPos,
									int nProgressMax, BOOL bAllowCancel, LPARAM lParam)
{
	CFuncViewPrjDoc *pDoc = (CFuncViewPrjDoc*)lParam;
	ASSERT_VALID(pDoc);

	MSG msg;
	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);  
	}

	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CFuncViewPrjDoc serialization

void CFuncViewPrjDoc::Serialize(CArchive& ar)
{
	ASSERT(FALSE);			// This function isn't used -- OnOpenDocument and OnSaveDocument are used instead

	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CFuncViewPrjDoc diagnostics

#ifdef _DEBUG
void CFuncViewPrjDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CFuncViewPrjDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFuncViewPrjDoc commands

BOOL CFuncViewPrjDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

void CFuncViewPrjDoc::DeleteContents() 
{
	int i;

	for (i=0; i<GetFileSetCount(); i++) {
		m_arrFileSets[i].DeleteContents();
	}
	m_objSymbolMap.FreeAll();

	m_mapLeftToRightFunc.RemoveAll();
	m_mapRightToLeftFunc.RemoveAll();

	m_strCompDiffText = "";

	m_strOutputText = "";

	SetMatrixPathName("", TRUE);

	CDocument::DeleteContents();
}

BOOL CFuncViewPrjDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (IsModified())
		TRACE0("Warning: OnOpenDocument replaces an unsaved document.\n");

	CFileException fe;
	CFile* pFile = GetFile(lpszPathName,
		CFile::modeRead|CFile::shareDenyWrite, &fe);
	if (pFile == NULL)
	{
		ReportSaveLoadException(lpszPathName, &fe,
			FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
		return FALSE;
	}
	pFile->Close();
	delete pFile;

	DeleteContents();
	SetModifiedFlag();  // dirty during de-serialize

	m_bFileOpenInProgress = TRUE;

	OnViewOutputWindow();

	CChildFrame4 *pFrame = (CChildFrame4 *)FindFrame(RUNTIME_CLASS(CChildFrame4));
	COutputView *pOutputView = NULL;
	COutputViewFile *pOutputViewFile = NULL;
	if (pFrame) pOutputView = pFrame->m_pOutputView;
	if (pOutputView) pOutputViewFile = pOutputView->GetOutputFile();

	if (pFrame) pFrame->BeginModalState();			// Put it in modal state during reading

	int i;
	int ndx;
	CString strTemp;
	CString strTemp2;
	LPTSTR pBuffer;
	int nCount;

	for (ndx = 0; ndx < GetFileSetCount(); ndx++) {
		nCount = ::GetPrivateProfileInt(m_arrstrFileSetSectionNames[ndx], m_strNumFilesEntry, 0, lpszPathName);
		for (i=0; i<nCount; i++) {
			strTemp.Format("%s%ld", LPCTSTR(m_strFileEntryPrefix), i);
			pBuffer = strTemp2.GetBuffer(1024);
			::GetPrivateProfileString(m_arrstrFileSetSectionNames[ndx], strTemp, _T(""), pBuffer, 1024, lpszPathName);
			strTemp2.ReleaseBuffer();
			AddFile(ndx, strTemp2, pOutputViewFile, pOutputViewFile);
		}
	}

	// Currently, only support 1 matrix file:
	strTemp.Format("%s0", LPCTSTR(m_strFileEntryPrefix));
	pBuffer = strTemp2.GetBuffer(1024);
	::GetPrivateProfileString(m_strMatrixSectionName, strTemp, _T(""), pBuffer, 1024, lpszPathName);
	strTemp2.ReleaseBuffer();
	SetMatrixPathName(strTemp2, TRUE);

	SetModifiedFlag(FALSE);     // start off with unmodified

	if (pFrame) pFrame->EndModalState();			// Exit modal state for that window
	if (pOutputViewFile) pOutputViewFile->Close();

	m_bFileOpenInProgress = FALSE;

	return TRUE;
}

BOOL CFuncViewPrjDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	CFileException fe;
	CFile* pFile = NULL;
	pFile = GetFile(lpszPathName, CFile::modeCreate |
		CFile::modeReadWrite | CFile::shareExclusive, &fe);
	if (pFile == NULL)
	{
		ReportSaveLoadException(lpszPathName, &fe,
			TRUE, AFX_IDP_INVALID_FILENAME);
		return FALSE;
	}
	pFile->Close();

	int i;
	int ndx;
	CString strTemp;
	int nCount;

	for (ndx = 0; ndx < GetFileSetCount(); ndx++) {
		nCount = GetFileCount(ndx);
		strTemp.Format("%ld", nCount);
		::WritePrivateProfileString(m_arrstrFileSetSectionNames[ndx], m_strNumFilesEntry, strTemp, lpszPathName);
		for (i=0; i<nCount; i++) {
			strTemp.Format("%s%ld", LPCTSTR(m_strFileEntryPrefix), i);
			::WritePrivateProfileString(m_arrstrFileSetSectionNames[ndx], strTemp, GetFilePathName(ndx, i), lpszPathName);
		}
	}

	// Currently, only support 1 matrix file:
	strTemp.Format("%s0", LPCTSTR(m_strFileEntryPrefix));
	::WritePrivateProfileString(m_strMatrixSectionName, strTemp, GetMatrixPathName(), lpszPathName);

	SetModifiedFlag(FALSE);     // back to unmodified

	return TRUE;        // success
}

CString CFuncViewPrjDoc::GetFileSetDescription(int nFileSet)
{
	switch (nFileSet) {
		case FVFS_LEFT_FILES:
			return "Left-Hand Function Definition Files";

		case FVFS_RIGHT_FILES:
			return "Right-Hand Function Definition Files";
	}

	return "";
}

BOOL CFuncViewPrjDoc::AddFile(int nFileSet, LPCTSTR pszFilePathName, CFile *pMsgFile, CFile *pErrFile)
{
	if ((nFileSet < 0) || (nFileSet >= GetFileSetCount())) return FALSE;

	BOOL bFileOpenInProgress = m_bFileOpenInProgress;
	m_bFileOpenInProgress = TRUE;
	BOOL bRetVal = m_arrFileSets[nFileSet].AddFile(pszFilePathName, pMsgFile, pErrFile, FuncDescFileProgressCallback, (LPARAM)this);
	m_bFileOpenInProgress = bFileOpenInProgress;

	return bRetVal;
}

int CFuncViewPrjDoc::FindFile(int nFileSet, LPCTSTR pszFilePathName)
{
	if ((nFileSet < 0) || (nFileSet >= GetFileSetCount())) return -1;

	return m_arrFileSets[nFileSet].FindFile(pszFilePathName);
}

BOOL CFuncViewPrjDoc::RemoveFile(int nFileSet, int nIndex)
{
	if ((nFileSet < 0) || (nFileSet >= GetFileSetCount())) return FALSE;

	return m_arrFileSets[nFileSet].RemoveFile(nIndex);
}

int CFuncViewPrjDoc::GetFileCount(int nFileSet)
{
	if ((nFileSet < 0) || (nFileSet >= GetFileSetCount())) return 0;

	return m_arrFileSets[nFileSet].GetFileCount();
}

CString CFuncViewPrjDoc::GetFilePathName(int nFileSet, int nIndex)
{
	if ((nFileSet < 0) || (nFileSet >= GetFileSetCount())) return "";

	return m_arrFileSets[nFileSet].GetFilePathName(nIndex);
}

CString CFuncViewPrjDoc::GetFileName(int nFileSet, int nIndex)
{
	if ((nFileSet < 0) || (nFileSet >= GetFileSetCount())) return "";

	return m_arrFileSets[nFileSet].GetFileName(nIndex);
}

CFuncDescFile *CFuncViewPrjDoc::GetFuncDescFile(int nFileSet, int nIndex)
{
	if ((nFileSet < 0) || (nFileSet >= GetFileSetCount())) return NULL;

	return m_arrFileSets[nFileSet].GetFuncDescFile(nIndex);
}

int CFuncViewPrjDoc::GetFuncCount(int nFileSet, int nIndex)
{
	if ((nFileSet < 0) || (nFileSet >= GetFileSetCount())) return 0;

	return m_arrFileSets[nFileSet].GetFuncCount(nIndex);
}

BOOL CFuncViewPrjDoc::AllocateMatrixMemory()
{
	FreeMatrixMemory();		// First, make sure old memory has been freed

	int nRows = GetFuncCount(FVFS_LEFT_FILES, -1);
	int nCols = GetFuncCount(FVFS_RIGHT_FILES, -1);
	int i;
	BOOL bFail = FALSE;

	if ((nRows == 0) || (nCols == 0)) return FALSE;

	m_nMatrixRowCount = nRows;
	m_nMatrixColCount = nCols;

	m_ppnMatrix = new double*[nRows];
	if (m_ppnMatrix == NULL) {
		m_nMatrixRowCount = 0;
		m_nMatrixColCount = 0;
		return FALSE;
	}

	for (i=0; i<nRows; i++) {
		m_ppnMatrix[i] = new double[nCols];
		if (m_ppnMatrix[i] == NULL) bFail = TRUE;
	}

	if (bFail) {
		FreeMatrixMemory();
		return FALSE;
	}

	return TRUE;
}

void CFuncViewPrjDoc::FreeMatrixMemory()
{
	if ((m_nMatrixRowCount == 0) ||
		(m_ppnMatrix == NULL)) {
		m_nMatrixRowCount = 0;
		m_nMatrixColCount = 0;
		m_ppnMatrix = NULL;
		return;
	}

	int i;

	for (i=0; i<m_nMatrixRowCount; i++) {
		if (m_ppnMatrix[i] == NULL) continue;
		delete[] (m_ppnMatrix[i]);
	}
	delete[] m_ppnMatrix;

	m_ppnMatrix = NULL;
	m_nMatrixRowCount = 0;
	m_nMatrixColCount = 0;
}

CString CFuncViewPrjDoc::GetMatrixPathName()
{
	return m_strMatrixPathName;
}

void CFuncViewPrjDoc::SetMatrixPathName(LPCTSTR pszPathName, BOOL bFreeCurrentMatrix)
{
	if (bFreeCurrentMatrix) FreeMatrixMemory();

	m_strMatrixPathName = pszPathName;
}

BOOL CFuncViewPrjDoc::ReadOrBuildMatrix(BOOL bForceRebuild)
{
	CStdioFile mtxFile;
	BOOL bHaveFile = FALSE;
	BOOL bFileValid = FALSE;
	CString strTemp;
	CString strTemp2;
	int nPos;
	int nRows;
	int nCols;

	// A call to allocate will clear the old memory,
	//		allocate new, and set the row/col counts:
	if (!AllocateMatrixMemory()) return FALSE;

	if (!m_strMatrixPathName.IsEmpty()) {
		bHaveFile = TRUE;

		// If we aren't forcing a rebuild, see if we can open the file:
		if ((!bForceRebuild) &&
			(mtxFile.Open(m_strMatrixPathName, CFile::modeRead | CFile::shareDenyWrite | CFile::typeText))) {
			strTemp = "";
			if (mtxFile.ReadString(strTemp)) {
				strTemp.TrimLeft();
				strTemp.TrimRight();
				nPos = strTemp.Find(',');
				if (nPos != -1) {
					nRows = atoi(strTemp.Left(nPos));
					nCols = atoi(strTemp.Mid(nPos + 1));
				} else {
					nRows = atoi(strTemp);
					nCols = 0;
				}

				if ((nRows == m_nMatrixRowCount) &&
					(nCols == m_nMatrixColCount)) bFileValid = TRUE;
			}
			if (!bFileValid) mtxFile.Close();
		}
	}

	if ((bHaveFile) && (!bFileValid)) {
		// If we have a filepath but it isn't valid (or we are forcing an overwrite),
		//		try and open the file for writing.  If that fails, assume we
		//		just don't have a file:
		if (!mtxFile.Open(m_strMatrixPathName,
			CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive | CFile::typeText))
			bHaveFile = FALSE;
	}

	// At this point, we have one of the following conditions:
	//		bHaveFile	bFileValid		Reasons				Action
	//		---------	----------		------------------	----------------------------------------
	//		FALSE		FALSE			No path specified	Compare putting results only into memory
	//									Failed to create
	//		TRUE		FALSE			File doesn't exist	File has been opened for writing so compare
	//									File's wrong size	to memory and also write to hard file
	//									Forcing Overwrite
	//		FALSE		TRUE			Should never happen	Compare putting results only into memory
	//		TRUE		TRUE			File's correct		File has been opened for reading, read
	//														it into memory rather than doing compare

	CProgressDlg progDlg(IDS_PROGRESS_MATRIX_COMPARE);
	int i, j;
	int nLeftFileIndex;
	int nLeftFuncIndex;
	int nRightFileIndex;
	int nRightFuncIndex;
	CFuncDesc *pLeftFunc;
	CFuncDesc *pRightFunc;

	CWnd *pParent = ((CMDIFrameWnd*)theApp.m_pMainWnd)->GetActiveView();
	if (pParent == NULL) pParent = ((CMDIFrameWnd*)theApp.m_pMainWnd)->GetActiveFrame();

	progDlg.Create(pParent);
	progDlg.SetRange(0, ((bHaveFile && bFileValid) ? m_nMatrixRowCount : m_nMatrixRowCount*m_nMatrixColCount));
	progDlg.SetStep(1);

	if (bHaveFile) {
		if (bFileValid) {
			progDlg.SetStatus("Reading from Matrix File...");

			// If reading from a file and verify x-breakpoints (i.e. righthand function names):
			strTemp = "";
			mtxFile.ReadString(strTemp);
			strTemp.TrimLeft();
			strTemp.TrimRight();
			nPos = strTemp.Find(',');
			strTemp = strTemp.Mid(nPos+1);		// Toss first (unused) field
			strTemp.TrimLeft();
			nRightFileIndex = 0;
			nRightFuncIndex = 0;
			for (j=0; j<m_nMatrixColCount; j++) {
				nPos = strTemp.Find(',');
				if (nPos != -1) {
					strTemp2 = strTemp.Left(nPos);
					strTemp2.TrimRight();
					strTemp = strTemp.Mid(nPos+1);
					strTemp.TrimLeft();
				} else {
					strTemp2 = strTemp;
					strTemp = "";
				}

				pRightFunc = GetNextFunction(FVFS_RIGHT_FILES, nRightFileIndex, nRightFuncIndex);
				if ((pRightFunc == NULL) ||
					(strTemp2.Compare(pRightFunc->GetMainName()) != 0)) {
					// Right-side function names don't match.  Close file to reset things
					//		and call recursively but force the overwrite:
					mtxFile.Close();
					progDlg.DestroyWindow();
					return ReadOrBuildMatrix(TRUE);
				}
			}
		} else {
			progDlg.SetStatus("Comparing and Writing to Matrix File...");

			// If writing to a file, write the size info and x-breakpoints:
			strTemp.Format("%ld,%ld\n", m_nMatrixRowCount, m_nMatrixColCount);
			mtxFile.WriteString(strTemp);
			nRightFileIndex = 0;
			nRightFuncIndex = 0;
			strTemp = "";
			for (j=0; j<m_nMatrixColCount; j++) {
				pRightFunc = GetNextFunction(FVFS_RIGHT_FILES, nRightFileIndex, nRightFuncIndex);
				if (pRightFunc == NULL) continue;
				strTemp += "," + pRightFunc->GetMainName();
			}
			strTemp += "\n";
			mtxFile.WriteString(strTemp);
		}
	} else {
		progDlg.SetStatus("Comparing...");
	}

	nLeftFileIndex = 0;
	nLeftFuncIndex = 0;
	for (i=0; i<m_nMatrixRowCount; i++) {
		pLeftFunc = GetNextFunction(FVFS_LEFT_FILES, nLeftFileIndex, nLeftFuncIndex);
		if (pLeftFunc == NULL) continue;

		if ((bHaveFile) && (bFileValid)) {
			// If reading from a file, do so:
			strTemp = "";
			strTemp2 = "";
			mtxFile.ReadString(strTemp);
			strTemp.TrimLeft();
			strTemp.TrimRight();
			nPos = strTemp.Find(',');
			// Read x-breakpoint
			if (nPos != -1) {
				strTemp2 = strTemp.Left(nPos);
				strTemp2.TrimRight();
			}
			if (strTemp2.Compare(pLeftFunc->GetMainName()) != 0) {
				// Left-side function names don't match.  Close file to reset things
				//		and call recursively but force the overwrite:
				mtxFile.Close();
				progDlg.DestroyWindow();
				return ReadOrBuildMatrix(TRUE);
			}
			strTemp = strTemp.Mid(nPos+1);
			strTemp.TrimLeft();
			for (j=0; j<m_nMatrixColCount; j++) {
				nPos = strTemp.Find(',');
				if (nPos != -1) {
					strTemp2 = strTemp.Left(nPos);
					strTemp2.TrimRight();
					m_ppnMatrix[i][j] = atof(strTemp2);
					strTemp = strTemp.Mid(nPos+1);
					strTemp.TrimLeft();
				} else {
					m_ppnMatrix[i][j] = atof(strTemp);
					strTemp = "";
				}
			}

			progDlg.StepIt();
			if (progDlg.CheckCancelButton()) {
				FreeMatrixMemory();
				return FALSE;
			}
		} else {
			// If not reading from file, we must do full compare
			//		(even if not writing back to a file):
			nRightFileIndex = 0;
			nRightFuncIndex = 0;
			strTemp = pLeftFunc->GetMainName();
			for (j=0; j<m_nMatrixColCount; j++) {
				pRightFunc = GetNextFunction(FVFS_RIGHT_FILES, nRightFileIndex, nRightFuncIndex);
				if (pRightFunc == NULL) continue;

				// Note: Function indexes have already been incremented during GetNextFunction
				m_ppnMatrix[i][j] = ::CompareFunctions(FCM_DYNPROG_GREEDY,
										GetFuncDescFile(FVFS_LEFT_FILES, nLeftFileIndex), nLeftFuncIndex-1,
										GetFuncDescFile(FVFS_RIGHT_FILES, nRightFileIndex), nRightFuncIndex-1,
										FALSE);
				strTemp2.Format(",%.12g", m_ppnMatrix[i][j]);
				strTemp += strTemp2;

				progDlg.StepIt();
				if (progDlg.CheckCancelButton()) {
					FreeMatrixMemory();
					return FALSE;
				}
			}
			strTemp += "\n";
			// If we are writing to a file, do so:
			if ((bHaveFile) && (!bFileValid)) mtxFile.WriteString(strTemp);
		}
	}

	if (bHaveFile) mtxFile.Close();

	return TRUE;
}

BOOL CFuncViewPrjDoc::HaveMatrix()
{
	if ((m_nMatrixRowCount == 0) ||
		(m_nMatrixColCount == 0) ||
		(m_ppnMatrix == NULL)) return FALSE;

	int nRows = GetFuncCount(FVFS_LEFT_FILES, -1);
	int nCols = GetFuncCount(FVFS_RIGHT_FILES, -1);

	if ((nRows != m_nMatrixRowCount) ||
		(nCols != m_nMatrixColCount)) return FALSE;

	return TRUE;
}

CFuncDesc *CFuncViewPrjDoc::GetNextFunction(int nFileSet, int &nFileIndex, int &nFuncIndex)
{
	CFuncDescFile *pFuncFile;
	BOOL bFound;

	if (nFileIndex >= GetFileCount(nFileSet)) return NULL;

	bFound = FALSE;
	for (; (nFileIndex < GetFileCount(nFileSet)); nFileIndex++) {
		pFuncFile = GetFuncDescFile(nFileSet, nFileIndex);
		if (pFuncFile == NULL) {
			nFuncIndex = 0;
			continue;
		}
		if (nFuncIndex < pFuncFile->GetFuncCount()) {
			bFound = TRUE;
			break;
		}
		nFuncIndex = 0;
	}
	if ((!bFound) || (pFuncFile == NULL)) return NULL;

	CFuncDesc *pRetVal = pFuncFile->GetFunc(nFuncIndex);

	nFuncIndex++;

	return pRetVal;
}

CFuncDesc *CFuncViewPrjDoc::GetFileFuncIndexFromLinearIndex(int nFileSet, int nLinearIndex,
															int &nFileIndex, int &nFuncIndex)
{
	CFuncDescFile *pFuncFile;
	BOOL bFound;
	int nLinear;

	nLinear = 0;
	bFound = FALSE;
	for (nFileIndex = 0; nFileIndex < GetFileCount(nFileSet); nFileIndex++) {
		pFuncFile = GetFuncDescFile(nFileSet, nFileIndex);
		if (pFuncFile == NULL) continue;

		for (nFuncIndex = 0; nFuncIndex < pFuncFile->GetFuncCount(); nFuncIndex++) {
			if (nLinear == nLinearIndex) {
				bFound = TRUE;
				break;
			}

			nLinear++;
		}
		if (bFound) break;
	}
	if ((!bFound) || (pFuncFile == NULL)) return NULL;

	return pFuncFile->GetFunc(nFuncIndex);
}


/////////////////////////////////////////////////////////////////////////////
// CFuncViewPrjDoc::CFileSet

void CFuncViewPrjDoc::CFileSet::DeleteContents() {
	m_arrFuncDesc.FreeAll();
}

BOOL CFuncViewPrjDoc::CFileSet::AddFile(LPCTSTR pszFilePathName, CFile *pMsgFile, CFile *pErrFile, LPFUNCANALPROGRESSCALLBACK pfnCallback, LPARAM lParamCallback)
{
	CString strFilePathName = pszFilePathName;
	if (strFilePathName.IsEmpty()) return FALSE;

	CFuncDescFile *pFuncDescFile;

	pFuncDescFile = new CFuncDescFile;
	if (pFuncDescFile == NULL) return FALSE;

	pFuncDescFile->SetProgressCallback(pfnCallback, lParamCallback);

	CFile theFile;

	if (!theFile.Open(strFilePathName, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite)) {
		delete pFuncDescFile;
		return FALSE;
	}

	if (!pFuncDescFile->ReadFuncDescFile(theFile, pMsgFile, pErrFile)) {
		theFile.Close();
		delete pFuncDescFile;
		return FALSE;
	}

	theFile.Close();

	m_arrFuncDesc.Add(pFuncDescFile);

	return TRUE;
}

int CFuncViewPrjDoc::CFileSet::FindFile(LPCTSTR pszFilePathName)
{
	CString strFilePathName = pszFilePathName;
	int i;
	CFuncDescFile *pFuncDescFile;

	for (i=0; i<m_arrFuncDesc.GetSize(); i++) {
		pFuncDescFile = m_arrFuncDesc.GetAt(i);
		if (pFuncDescFile == NULL) continue;

		if (pFuncDescFile->GetFuncPathName().Compare(strFilePathName) == 0) return i;
	}

	return -1;
}

BOOL CFuncViewPrjDoc::CFileSet::RemoveFile(int nIndex)
{
	ASSERT((nIndex >= 0) && (nIndex < m_arrFuncDesc.GetSize()));

	if ((nIndex < 0) || (nIndex >= m_arrFuncDesc.GetSize())) return FALSE;
	CFuncDescFile *pFuncDesc = m_arrFuncDesc.GetAt(nIndex);

	m_arrFuncDesc.RemoveAt(nIndex);
	if (pFuncDesc) delete pFuncDesc;

	return TRUE;
}

int CFuncViewPrjDoc::CFileSet::GetFileCount()
{
	return m_arrFuncDesc.GetSize();
}

CString CFuncViewPrjDoc::CFileSet::GetFilePathName(int nIndex)
{
	ASSERT((nIndex >= 0) && (nIndex < m_arrFuncDesc.GetSize()));

	if ((nIndex < 0) || (nIndex >= m_arrFuncDesc.GetSize())) return "";
	CFuncDescFile *pFuncDesc = m_arrFuncDesc.GetAt(nIndex);
	if (pFuncDesc == NULL) return "";

	return pFuncDesc->GetFuncPathName();
}

CString CFuncViewPrjDoc::CFileSet::GetFileName(int nIndex)
{
	ASSERT((nIndex >= 0) && (nIndex < m_arrFuncDesc.GetSize()));

	if ((nIndex < 0) || (nIndex >= m_arrFuncDesc.GetSize())) return "";
	CFuncDescFile *pFuncDesc = m_arrFuncDesc.GetAt(nIndex);
	if (pFuncDesc == NULL) return "";

	return pFuncDesc->GetFuncFileName();
}

CFuncDescFile *CFuncViewPrjDoc::CFileSet::GetFuncDescFile(int nIndex)
{
	ASSERT((nIndex >= 0) && (nIndex < m_arrFuncDesc.GetSize()));

	if ((nIndex < 0) || (nIndex >= m_arrFuncDesc.GetSize())) return NULL;
	CFuncDescFile *pFuncDesc = m_arrFuncDesc.GetAt(nIndex);

	return pFuncDesc;
}

int CFuncViewPrjDoc::CFileSet::GetFuncCount(int nIndex)
{
	ASSERT((nIndex >= -1) && (nIndex < m_arrFuncDesc.GetSize()));

	if ((nIndex < -1) || (nIndex >= m_arrFuncDesc.GetSize())) return 0;

	if (nIndex != -1) {
		CFuncDescFile *pFuncDesc = m_arrFuncDesc.GetAt(nIndex);
		if (pFuncDesc == NULL) return 0;
		return pFuncDesc->GetFuncCount();
	}

	return m_arrFuncDesc.GetFuncCount();
}


/////////////////////////////////////////////////////////////////////////////
// CFuncViewPrjDoc commands

CFrameWnd *CFuncViewPrjDoc::FindFrame(CRuntimeClass *pFrameClass)
{
	CFrameWnd *pFrame;
	CView *pView;
	POSITION pos;

	pos = GetFirstViewPosition();
	while (pos) {
		pView = GetNextView(pos);
		if (pView == NULL) continue;
		pFrame = pView->GetParentFrame();
		if (pFrame == NULL) continue;
		if (pFrame->IsKindOf(pFrameClass)) return pFrame;
	}

	return NULL;
}

CFrameWnd *CFuncViewPrjDoc::CreateFrame(UINT nIDResource, CRuntimeClass *pFrameClass, BOOL bDoInitialUpdate)
{
	CCreateContext context;
	context.m_pCurrentFrame = NULL;
	context.m_pCurrentDoc = this;
	context.m_pNewViewClass = NULL;
	context.m_pLastView = NULL;
	context.m_pNewDocTemplate = GetDocTemplate();

	CFrameWnd *pFrame = (CFrameWnd*)(pFrameClass->CreateObject());
	if (pFrame == NULL)
	{
		TRACE1("Warning: Dynamic create of frame %hs failed.\n",
			pFrameClass->m_lpszClassName);
		return NULL;
	}
	ASSERT_KINDOF(CFrameWnd, pFrame);

	// create new from resource
	if (!pFrame->LoadFrame(nIDResource,
			WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,   // default frame styles
			NULL, &context))
	{
		TRACE0("Warning: Couldn't create a frame.\n");
		// frame will be deleted in PostNcDestroy cleanup
		return NULL;
	}

	if (bDoInitialUpdate) pFrame->InitialUpdateFrame(this, TRUE);

	return pFrame;
}

void CFuncViewPrjDoc::OnViewFuncDiff() 
{
	CFrameWnd *pFrame;

	pFrame = FindFrame(RUNTIME_CLASS(CChildFrame2));
	if (pFrame) {
//		((CMainFrame*)theApp.m_pMainWnd)->MDIActivate(pFrame);
		ASSERT_KINDOF(CMDIChildWnd, pFrame);
		((CMDIChildWnd*)pFrame)->MDIDestroy();
		return;
	}

	pFrame = CreateFrame(IDR_FUNCDIFFTYPE, RUNTIME_CLASS(CChildFrame2), FALSE);
	if (pFrame == NULL) return;

	CChildFrame2 *pChildFrame = (CChildFrame2 *)pFrame;
	ASSERT(pChildFrame->m_pLeftFuncList != NULL);
	ASSERT(pChildFrame->m_pRightFuncList != NULL);

	if ((pChildFrame->m_pLeftFuncList == NULL) ||
		(pChildFrame->m_pRightFuncList == NULL)) {
		AfxMessageBox("ERROR : Couldn't locate function list views!", MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	pChildFrame->m_pLeftFuncList->SetFileSetIndex(FVFS_LEFT_FILES);
	pChildFrame->m_pRightFuncList->SetFileSetIndex(FVFS_RIGHT_FILES);

	pFrame->InitialUpdateFrame(this, TRUE);
}

void CFuncViewPrjDoc::OnUpdateViewFuncDiff(CCmdUI* pCmdUI)
{
	CFrameWnd *pFrame = FindFrame(RUNTIME_CLASS(CChildFrame2));

	pCmdUI->Enable(!m_bFileOpenInProgress);
	pCmdUI->SetCheck((pFrame != NULL) ? 1 : 0);
}

void CFuncViewPrjDoc::OnViewSymbolMap() 
{
	CFrameWnd *pFrame;

	pFrame = FindFrame(RUNTIME_CLASS(CChildFrame3));
	if (pFrame) {
//		((CMainFrame*)theApp.m_pMainWnd)->MDIActivate(pFrame);
		ASSERT_KINDOF(CMDIChildWnd, pFrame);
		((CMDIChildWnd*)pFrame)->MDIDestroy();
		return;
	}

	pFrame = CreateFrame(IDR_SYMBOLMAPTYPE, RUNTIME_CLASS(CChildFrame3));
	if (pFrame == NULL) return;
}

void CFuncViewPrjDoc::OnUpdateViewSymbolMap(CCmdUI* pCmdUI) 
{
	CFrameWnd *pFrame = FindFrame(RUNTIME_CLASS(CChildFrame3));

	pCmdUI->Enable(!m_bFileOpenInProgress);
	pCmdUI->SetCheck((pFrame != NULL) ? 1 : 0);
}

void CFuncViewPrjDoc::OnViewOutputWindow() 
{
	CFrameWnd *pFrame;

	pFrame = FindFrame(RUNTIME_CLASS(CChildFrame4));
	if (pFrame) {
//		((CMainFrame*)theApp.m_pMainWnd)->MDIActivate(pFrame);
		ASSERT_KINDOF(CMDIChildWnd, pFrame);
		((CMDIChildWnd*)pFrame)->MDIDestroy();
		return;
	}

	pFrame = CreateFrame(IDR_OUTPUTTYPE, RUNTIME_CLASS(CChildFrame4));
	if (pFrame == NULL) return;
}

void CFuncViewPrjDoc::OnUpdateViewOutputWindow(CCmdUI* pCmdUI) 
{
	CFrameWnd *pFrame = FindFrame(RUNTIME_CLASS(CChildFrame4));

	pCmdUI->Enable(!m_bFileOpenInProgress);
	pCmdUI->SetCheck((pFrame != NULL) ? 1 : 0);
}

void CFuncViewPrjDoc::OnViewCompMatrix() 
{
	CFrameWnd *pFrame;

	pFrame = FindFrame(RUNTIME_CLASS(CChildFrame5));
	if (pFrame) {
//		((CMainFrame*)theApp.m_pMainWnd)->MDIActivate(pFrame);
		ASSERT_KINDOF(CMDIChildWnd, pFrame);
		((CMDIChildWnd*)pFrame)->MDIDestroy();
		return;
	}

	pFrame = CreateFrame(IDR_COMPMATRIXTYPE, RUNTIME_CLASS(CChildFrame5));
	if (pFrame == NULL) return;
}

void CFuncViewPrjDoc::OnUpdateViewCompMatrix(CCmdUI* pCmdUI) 
{
	CFrameWnd *pFrame = FindFrame(RUNTIME_CLASS(CChildFrame5));

	pCmdUI->Enable(!m_bFileOpenInProgress);
	pCmdUI->SetCheck((pFrame != NULL) ? 1 : 0);
}

void CFuncViewPrjDoc::OnViewCompDiff() 
{
	CFrameWnd *pFrame;

	pFrame = FindFrame(RUNTIME_CLASS(CChildFrame6));
	if (pFrame) {
//		((CMainFrame*)theApp.m_pMainWnd)->MDIActivate(pFrame);
		ASSERT_KINDOF(CMDIChildWnd, pFrame);
		((CMDIChildWnd*)pFrame)->MDIDestroy();
		return;
	}

	pFrame = CreateFrame(IDR_COMPDIFFEDITTYPE, RUNTIME_CLASS(CChildFrame6));
	if (pFrame == NULL) return;
}

void CFuncViewPrjDoc::OnUpdateViewCompDiff(CCmdUI* pCmdUI) 
{
	CFrameWnd *pFrame = FindFrame(RUNTIME_CLASS(CChildFrame6));

	pCmdUI->Enable(!m_bFileOpenInProgress);
	pCmdUI->SetCheck((pFrame != NULL) ? 1 : 0);
}

void CFuncViewPrjDoc::UpdateFrameCounts()
{
	CMap<CString, LPCSTR, int, int> nFrames;
	CMap<CString, LPCSTR, int, int> iFrame;
	int nTemp;

	// walk all frames of views (mark and sweep approach)
	POSITION pos = GetFirstViewPosition();
	while (pos != NULL)
	{
		CView* pView = GetNextView(pos);
		ASSERT_VALID(pView);
		ASSERT(::IsWindow(pView->m_hWnd));
		if (pView->IsWindowVisible())   // Do not count invisible windows.
		{
			CFrameWnd* pFrame = pView->GetParentFrame();
			if (pFrame != NULL)
				pFrame->m_nWindow = -1;     // unknown

			nFrames.SetAt(pFrame->GetRuntimeClass()->m_lpszClassName, 0);
			iFrame.SetAt(pFrame->GetRuntimeClass()->m_lpszClassName, 1);
		}
	}

	// now do it again counting the unique ones
	pos = GetFirstViewPosition();
	while (pos != NULL)
	{
		CView* pView = GetNextView(pos);
		ASSERT_VALID(pView);
		ASSERT(::IsWindow(pView->m_hWnd));
		if (pView->IsWindowVisible())   // Do not count invisible windows.
		{
			CFrameWnd* pFrame = pView->GetParentFrame();
			if (pFrame != NULL && pFrame->m_nWindow == -1)
			{
				ASSERT_VALID(pFrame);
				// not yet counted (give it a 1 based number)
				nFrames.Lookup(pFrame->GetRuntimeClass()->m_lpszClassName, nTemp);
				pFrame->m_nWindow = ++nTemp;
				nFrames.SetAt(pFrame->GetRuntimeClass()->m_lpszClassName, nTemp);
			}
		}
	}

	// lastly walk the frames and update titles (assume same order)
	// go through frames updating the appropriate one
	pos = GetFirstViewPosition();
	while (pos != NULL)
	{
		CView* pView = GetNextView(pos);
		ASSERT_VALID(pView);
		ASSERT(::IsWindow(pView->m_hWnd));
		if (pView->IsWindowVisible())   // Do not count invisible windows.
		{
			CFrameWnd* pFrame = pView->GetParentFrame();
			if (pFrame != NULL)
			{
				iFrame.Lookup(pFrame->GetRuntimeClass()->m_lpszClassName, nTemp);
				if (pFrame->m_nWindow == nTemp)
				{
					ASSERT_VALID(pFrame);
					nFrames.Lookup(pFrame->GetRuntimeClass()->m_lpszClassName, nTemp);
					if (nTemp == 1)
						pFrame->m_nWindow = 0;      // the only one of its kind
					pFrame->OnUpdateFrameTitle(TRUE);
					iFrame.Lookup(pFrame->GetRuntimeClass()->m_lpszClassName, nTemp);
					iFrame.SetAt(pFrame->GetRuntimeClass()->m_lpszClassName, nTemp + 1);
				}
			}
		}
	}
}

void CFuncViewPrjDoc::OnSymbolsReset() 
{
	if (AfxMessageBox("Are you sure you want to reset all symbols?", MB_YESNO | MB_ICONQUESTION) != IDYES) return;

	m_objSymbolMap.FreeAll();
	m_mapLeftToRightFunc.RemoveAll();
	m_mapRightToLeftFunc.RemoveAll();

	m_strCompDiffText = "";

	UpdateAllViews(NULL, SYMBOL_MAP_ALL, NULL);			// Signal all symbol table 
	UpdateAllViews(NULL, COMP_DIFF_TYPE_BASE, NULL);	// Update CompDiffEditViews
}

void CFuncViewPrjDoc::OnSymbolsExport() 
{
	static const char pszFilter[] = "Symbol Files (*.sym)|*.sym|All Files (*.*)|*.*||";

	CFileDialog dlg(FALSE, "sym", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST,
						pszFilter, NULL);

	if (dlg.DoModal() != IDOK) return;

	CStdioFile outFile;

	if (!outFile.Open(dlg.GetPathName(), CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite | CFile::typeText)) {
		AfxMessageBox("Failed to create export file!", MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	CStringArray arrSym;
	CStringArray arrSymMatch;
	CDWordArray arrHitCount;
	DWORD nTotalHitCount;
	int i,j,k;
	CString strTemp;

	CWaitCursor wait;

	for (i=0; i<4; i++) {
		switch (i) {
			case 0:
				m_objSymbolMap.GetLeftSideCodeSymbolList(arrSym);
				outFile.WriteString("Left-Side Code Symbols:\n");
				break;

			case 1:
				m_objSymbolMap.GetLeftSideDataSymbolList(arrSym);
				outFile.WriteString("\nLeft-Side Data Symbols:\n");
				break;

			case 2:
				m_objSymbolMap.GetRightSideCodeSymbolList(arrSym);
				outFile.WriteString("\n\nRight-Side Code Symbols:\n");
				break;

			case 3:
				m_objSymbolMap.GetRightSideDataSymbolList(arrSym);
				outFile.WriteString("\nRight-Side Data Symbols:\n");
				break;

			default:
				continue;
		}

		for (j=0; j<arrSym.GetSize(); j++) {
			switch (i) {
				case 0:
					nTotalHitCount = m_objSymbolMap.GetLeftSideCodeHitList(arrSym.GetAt(j), arrSymMatch, arrHitCount);
					break;

				case 1:
					nTotalHitCount = m_objSymbolMap.GetLeftSideDataHitList(arrSym.GetAt(j), arrSymMatch, arrHitCount);
					break;

				case 2:
					nTotalHitCount = m_objSymbolMap.GetRightSideCodeHitList(arrSym.GetAt(j), arrSymMatch, arrHitCount);
					break;

				case 3:
					nTotalHitCount = m_objSymbolMap.GetRightSideDataHitList(arrSym.GetAt(j), arrSymMatch, arrHitCount);
					break;

				default:
					continue;
			}

			outFile.WriteString(arrSym.GetAt(j));

			ASSERT(arrSymMatch.GetSize() == arrHitCount.GetSize());		// These should have same number of members!

			for (k=0; k<arrSymMatch.GetSize(); k++) {
				ASSERT(nTotalHitCount != 0);							// Should never have members with no hits!
				strTemp.Format(",%s,%ld,%f%%", LPCTSTR(arrSymMatch.GetAt(k)), arrHitCount.GetAt(k),
												((double)arrHitCount.GetAt(k)/nTotalHitCount)*100);
				outFile.WriteString(strTemp);
			}
			outFile.WriteString("\n");
		}
	}

	outFile.Close();
}

void CFuncViewPrjDoc::OnUpdateSymbolsExport(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!m_objSymbolMap.IsEmpty());
}

void CFuncViewPrjDoc::OnMatrixRebuild() 
{
	if (!ReadOrBuildMatrix(FALSE)) {
		AfxMessageBox("Comparison Matrix Build Failed!", MB_OK | MB_ICONEXCLAMATION);
	}

	UpdateAllViews(NULL, MATRIX_TYPE_BASE, NULL);
}

void CFuncViewPrjDoc::OnMatrixRebuildForce() 
{
	if (!ReadOrBuildMatrix(TRUE)) {
		AfxMessageBox("Comparison Matrix Build Failed!", MB_OK | MB_ICONEXCLAMATION);
	}

	UpdateAllViews(NULL, MATRIX_TYPE_BASE, NULL);
}

void CFuncViewPrjDoc::AddFunctionAssociation(LPCTSTR pszLeftFuncName, LPCTSTR pszRightFuncName)
{
	m_mapLeftToRightFunc.SetAt(pszLeftFuncName, pszRightFuncName);
	m_mapRightToLeftFunc.SetAt(pszRightFuncName, pszLeftFuncName);
}

BOOL CFuncViewPrjDoc::LookupLeftFuncAssociation(LPCTSTR pszLeftFuncName, CString &strRightFuncName)
{
	return m_mapLeftToRightFunc.Lookup(pszLeftFuncName, strRightFuncName);
}

BOOL CFuncViewPrjDoc::LookupRightFuncAssociation(LPCTSTR pszRightFuncName, CString &strLeftFuncName)
{
	return m_mapRightToLeftFunc.Lookup(pszRightFuncName, strLeftFuncName);
}

