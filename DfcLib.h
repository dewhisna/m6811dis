//	DfcLib.h

//	This is the implementation file for the DFCLibrary as well as the Src Data File template types
//	for MDI and SDI templates.  It also contains a Src Data File DocMgr type as well as both
//	MDI and SDI Appl template classes.  Include this component in your apps to read/write DFC
//	Library types.  This component handles automatic registration and allows the application to
//	dynamically unregister/reregister the file types.
//{{AFX_INCLUDES()
#include "hexedit.h"
//}}AFX_INCLUDES

#ifndef DFCLIB_H
#define DFCLIB_H

#include "resource.h"

#include <fstream.h>
#include <memclass.h>

#ifndef __GENREP_DFC_IMPORT__
#define __GENREP_DFC_IMPORT__
#endif
#include <dfcapi.h>

class CDFCLibrary;

/////////////////////////////////////////////////////////////////////////////
// CSrcMultiDocTemplate used for Src File Types

// MDI support (zero or more documents)
class CSrcMultiDocTemplate : public CMultiDocTemplate
{
// Constructors
public:
	CSrcMultiDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass,
		CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass, CDFCLibrary* aLibrary, LPCTSTR anAppName);

	DECLARE_DYNAMIC(CSrcMultiDocTemplate)

// Implementation
public:
	virtual BOOL GetDocString(CString& rString,
		enum DocStringIndex index) const; // get one of the info strings
	virtual CDFCLibrary& GetLibrary(void);

protected:
	CString	m_Description;
	CString m_Extension;
	CString m_AppName;
	CDFCLibrary*	theLibrary;
};


/////////////////////////////////////////////////////////////////////////////
// CSrcMdiFrame frame

class CSrcMdiFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CSrcMdiFrame)
protected:
	CSrcMdiFrame();           // protected constructor used by dynamic creation

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSrcMdiFrame)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:

protected:
	virtual ~CSrcMdiFrame();

	// Generated message map functions
	//{{AFX_MSG(CSrcMdiFrame)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
// CSrcDocManager used for Src File Types

class CSrcDocManager : public CDocManager
{
	DECLARE_DYNAMIC(CSrcDocManager)
public:
	virtual BOOL DeleteDocTemplate(CDocTemplate* pTemplate);
	virtual void RegisterShellFileTypes(BOOL bCompat);
	virtual CDocTemplate *DoPromptFileName(CString& fileName, UINT nIDSTitle,
			DWORD lFlags, BOOL bOpenFileDialog, CDocTemplate* pTemplate, BOOL AllDFCs);
};


/////////////////////////////////////////////////////////////////////////////
// CSrcMdiWinApp:
//

class CSrcMdiWinApp : public CWinApp
{
public:
	void AddDocTemplate(CDocTemplate* pTemplate);
	virtual void AddSourceFileTemplate(CDFCLibrary* aLibrary);
	BOOL DeleteDocTemplate(CDocTemplate* pTemplate);
	virtual void DeleteSourceFileTemplate(CDFCLibrary& aLibrary);
	virtual void RegisterFileTypes(void);
	virtual void UnregisterFileTypes(void);

	virtual int ExitInstance(); // return app exit code

protected:
	void LoadDFCSettings();
	void SaveDFCSettings();

public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSrcMdiWinApp)
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSrcMdiWinApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
};


/////////////////////////////////////////////////////////////////////////////
// CSrcDocTemplateList definition

typedef CList<CDocTemplate*, CDocTemplate*> CSrcDocTemplateList;


/////////////////////////////////////////////////////////////////////////////
// CDFCLibrary and CDFCArray definitions

class CDFCLibrary : public CObject
{
public:
// Construction
	CDFCLibrary();
	CDFCLibrary(LPCSTR aLibraryName);
	~CDFCLibrary();

	int Load(void);
	int Unload(void);
	int IsLoaded(void);
	int GetLoadCount(void);
	BOOL CanUnload(void);
	HINSTANCE GetHandle(void);
	BOOL AddLibraryTemplate(CDocTemplate* aTemplate);
	BOOL RemoveLibraryTemplate(CDocTemplate* aTemplate);
	POSITION GetFirstLibraryTemplate(void);
	CDocTemplate* GetNextLibraryTemplate(POSITION &aPos);
	void RemoveAllLibraryTemplates(void);
	void SetLibraryName(LPCSTR aLibraryName);
	LPCSTR GetLibraryName(void);
	LPCSTR GetShortDescription(void);
	LPCSTR GetDescription(void);
	LPCSTR DefaultExtension(void);
	int RetrieveFileMapping(istream *aFile, unsigned long NewBase, TMemRange *aRange);
	int ReadDataFile(istream *aFile, unsigned long NewBase, TMemObject *aMemory, unsigned char aDesc);
	int WriteDataFile(ostream *aFile, TMemRange *aRange, unsigned long NewBase, TMemObject *aMemory,
                        unsigned char aDesc, int UsePhysicalAddr, unsigned int FillMode);

protected:
	CSrcDocTemplateList	LibraryTemplates;
	CString		LibraryName;
	int			ValidLoad;
	int			LoadCount;
	HINSTANCE	LibraryHandle;
	GDFC_TYPE(GetShortDescription)	EGetShortDescription;
	GDFC_TYPE(GetDescription)		EGetDescription;
	GDFC_TYPE(DefaultExtension)		EDefaultExtension;
	GDFC_TYPE(RetrieveFileMapping)	ERetrieveFileMapping;
	GDFC_TYPE(ReadDataFile)			EReadDataFile;
	GDFC_TYPE(WriteDataFile)		EWriteDataFile;
};

class CDFCArray : public CArray<CDFCLibrary *, CDFCLibrary *>
{
public:
	~CDFCArray();

	BOOL AlreadyLoaded(CDFCLibrary& aLibrary);
};


/////////////////////////////////////////////////////////////////////////////
// CSrcDoc document

class CSrcDoc : public CDocument
{
protected:
	CSrcDoc();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CSrcDoc)

// Attributes
public:

protected:
	TMemObject*	m_pMemory;
	TMemRange*	m_pMemRange;
	unsigned long	ReadOffset;
	unsigned long	WriteOffset;
	unsigned char	ReadDesc;
	unsigned char	WriteDesc;
	int				WriteUsingPhysical;
	unsigned int	WriteFillMode;
	int				UseDesc;
	unsigned char	MemFillByte;

// Operations
public:
	void CopyFrom(CSrcDoc& aSource);

	TMemObject*	GetMemory();
	TMemRange*	GetRange();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSrcDoc)
	public:
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void DeleteContents();
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	protected:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSrcDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CSrcDoc)
	afx_msg void OnFileConvert();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CSrcView form view

class CSrcView : public CFormView
{
protected:
	CSrcView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CSrcView)

// Form Data
public:
	//{{AFX_DATA(CSrcView)
	enum { IDD = IDD_SRCDOC_FORM };
	CHexEdit	m_CHexEdit;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:
//	virtual int OnToolHitTest( CPoint point, TOOLINFO* pTI ) const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSrcView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CSrcView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CSrcView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHexeditProperties();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////
// Global Variable References:

extern CDFCArray	DFCs;


#endif	// DFCLIB_H
