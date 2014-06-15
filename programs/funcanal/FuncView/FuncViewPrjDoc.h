// FuncViewPrjDoc.h : interface of the CFuncViewPrjDoc class
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: FuncViewPrjDoc.h,v $
// Revision 1.1  2003/09/13 05:45:44  dewhisna
// Initial Revision
//
//

#if !defined(AFX_FUNCVIEWPRJDOC_H__3E867243_733D_48AF_9052_42CDC35AB42F__INCLUDED_)
#define AFX_FUNCVIEWPRJDOC_H__3E867243_733D_48AF_9052_42CDC35AB42F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../funcdesc.h"

#define FILESET_TYPE_BASE 1000
#define FILESET_TYPE_LAST 1999
#define SYMBOL_MAP_TYPE_BASE 2000
#define SYMBOL_MAP_ALL 2000			// Used in updating all views with symbol table into
#define SYMBOL_MAP_LEFT_CODE 2001
#define SYMBOL_MAP_RIGHT_CODE 2002
#define SYMBOL_MAP_LEFT_DATA 2003
#define SYMBOL_MAP_RIGHT_DATA 2004
#define SYMBOL_MAP_TYPE_LAST 2999
#define MATRIX_TYPE_BASE 3000
#define MATRIX_TYPE_LAST 3999
#define COMP_DIFF_TYPE_BASE 4000
#define COMP_DIFF_TYPE_ALL 4000
#define COMP_DIFF_TYPE_LAST 4999

class CFuncViewFuncRef : public CObject
{
public:
	CFuncDescFile *m_pFile;			// Pointer to Function Description File
	int m_nIndex;					// Index to a specific function in File

	CFuncViewFuncRef() { Reset(); }
	void Reset() {
		m_pFile = NULL;
		m_nIndex = -1;
	}
	CFuncViewFuncRef& operator=(const CFuncViewFuncRef& aSrc)
	{
		m_pFile = aSrc.m_pFile;
		m_nIndex = aSrc.m_nIndex;
		return *this;
	}
};

class CFuncViewPrjDoc : public CDocument
{
protected: // create from serialization only
	CFuncViewPrjDoc();
	DECLARE_DYNCREATE(CFuncViewPrjDoc)

// Attributes
public:

// Operations
public:

protected:
	CFrameWnd *FindFrame(CRuntimeClass *pFrameClass);
	CFrameWnd *CreateFrame(UINT nIDResource, CRuntimeClass *pFrameClass, BOOL bDoInitialUpdate = TRUE);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFuncViewPrjDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual void DeleteContents();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFuncViewPrjDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual void UpdateFrameCounts();

	enum FUNC_VIEW_FILE_SETS {
			FVFS_LEFT_FILES = 0,
			FVFS_RIGHT_FILES = 1,
			FVFS_COUNT = 2					// Always set to the number of file set types
	};

	int GetFileSetCount() const { return FVFS_COUNT; }
	CString GetFileSetDescription(int nFileSet);
	BOOL AddFile(int nFileSet, LPCTSTR pszFilePathName, CFile *pMsgFile = NULL, CFile *pErrFile = NULL);	// Loads and sets a new filename in the specified fileset
	int FindFile(int nFileSet, LPCTSTR pszFilePathName);	// Finds the specified file in the fileset, returning its 0-based index (or -1 if not found)
	BOOL RemoveFile(int nFileSet, int nIndex);				// Unloads and removes the specified file from the specified fileset
	int GetFileCount(int nFileSet);							// Returns the number of files in the specified set
	CString GetFilePathName(int nFileSet, int nIndex);		// Returns the specified file path name
	CString GetFileName(int nFileSet, int nIndex);			// Returns the specified file name
	CFuncDescFile *GetFuncDescFile(int nFileSet, int nIndex);	// Returns the specified function description
	int GetFuncCount(int nFileSet, int nIndex);				// Returns the number of functions in the specified file of the specified fileset or all functions in the set if index is -1

	CFuncDesc *GetNextFunction(int nFileSet, int &nFileIndex, int &nFuncIndex);
	CFuncDesc *GetFileFuncIndexFromLinearIndex(int nFileSet, int nLinearIndex, int &nFileIndex, int &nFuncIndex);		// Calculates/sets nFileIndex and nFuncIndex from a linear index and returns pointer to function

	CSymbolMap &GetSymbolMap() { return m_objSymbolMap; }

	void AddFunctionAssociation(LPCTSTR pszLeftFuncName, LPCTSTR pszRightFuncName);
	BOOL LookupLeftFuncAssociation(LPCTSTR pszLeftFuncName, CString &strRightFuncName);
	BOOL LookupRightFuncAssociation(LPCTSTR pszRightFuncName, CString &strLeftFuncName);

	CString &GetCompDiffText() { return m_strCompDiffText; }

	CString &GetOutputText() { return m_strOutputText; }

	CString GetMatrixPathName();
	void SetMatrixPathName(LPCTSTR pszPathName, BOOL bFreeCurrentMatrix);
	BOOL ReadOrBuildMatrix(BOOL bForceRebuild = FALSE);
	BOOL HaveMatrix();
	int GetMatrixRowCount() { return m_nMatrixRowCount; }
	int GetMatrixColumnCount() { return m_nMatrixColCount; }
	double **GetMatrix() { return m_ppnMatrix; }

protected:
	class CFileSet {
	public:
		void DeleteContents();
		BOOL AddFile(LPCTSTR pszFilePathName, CFile *pMsgFile = NULL, CFile *pErrFile = NULL, LPFUNCANALPROGRESSCALLBACK pfnCallback = NULL, LPARAM lParamCallback = 0);		// Loads and sets a new filename
		int FindFile(LPCTSTR pszFilePathName);		// Finds the specified file, returning its 0-based index (or -1 if not found)
		BOOL RemoveFile(int nIndex);				// Unloads and removes the specified file
		int GetFileCount();							// Returns the number of files
		CString GetFilePathName(int nIndex);		// Returns the specified file path name
		CString GetFileName(int nIndex);			// Returns the specified file name
		CFuncDescFile *GetFuncDescFile(int nIndex);	// Returns the specified function description
		int GetFuncCount(int nIndex);				// Returns the number of functions in the specified file or all files if index is -1
	protected:
		CFuncDescFileArray m_arrFuncDesc;			// Function Desc Files
	} m_arrFileSets[FVFS_COUNT];

	CSymbolMap m_objSymbolMap;
	CMapStringToString m_mapLeftToRightFunc;		// Mapping of left-hand to right-hand functions
	CMapStringToString m_mapRightToLeftFunc;		// Mapping of right-hand to left-hand functions

	CString m_strOutputText;

	BOOL m_bFileOpenInProgress;

	BOOL AllocateMatrixMemory();
	void FreeMatrixMemory();

	CString m_strMatrixPathName;
	int m_nMatrixRowCount;			// Comparison matrix row count (left files)
	int m_nMatrixColCount;			// Comparison matrix col count (right files)
	double **m_ppnMatrix;			// Comparison matrix data

	CString m_strCompDiffText;

private:
	static BOOL CALLBACK FuncDescFileProgressCallback(int nProgressPos, int nProgressMax, BOOL bAllowCancel, LPARAM lParam);

// Generated message map functions
protected:
	//{{AFX_MSG(CFuncViewPrjDoc)
	afx_msg void OnViewFuncDiff();
	afx_msg void OnUpdateViewFuncDiff(CCmdUI* pCmdUI);
	afx_msg void OnSymbolsReset();
	afx_msg void OnViewSymbolMap();
	afx_msg void OnUpdateViewSymbolMap(CCmdUI* pCmdUI);
	afx_msg void OnViewOutputWindow();
	afx_msg void OnUpdateViewOutputWindow(CCmdUI* pCmdUI);
	afx_msg void OnViewCompMatrix();
	afx_msg void OnUpdateViewCompMatrix(CCmdUI* pCmdUI);
	afx_msg void OnMatrixRebuild();
	afx_msg void OnMatrixRebuildForce();
	afx_msg void OnSymbolsExport();
	afx_msg void OnUpdateSymbolsExport(CCmdUI* pCmdUI);
	afx_msg void OnViewCompDiff();
	afx_msg void OnUpdateViewCompDiff(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FUNCVIEWPRJDOC_H__3E867243_733D_48AF_9052_42CDC35AB42F__INCLUDED_)
