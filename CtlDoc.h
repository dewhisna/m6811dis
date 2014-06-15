// CtlDoc.h : interface of the CCTLFileDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CTLDOC_H__25668D35_606B_11D1_858F_00600828300C__INCLUDED_)
#define AFX_CTLDOC_H__25668D35_606B_11D1_858F_00600828300C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


#define	CTL_VERSION	100


/////////////////////////////////////////////////////////////////////////////
// CSourceFile Class
//		This class defines the settings for a single source file
class CSourceFile : public CObject
{
public:
	CString	FilePathName;
	DWORD	LoadAddress;
	CString	DFCName;

	CSourceFile();
	CSourceFile(CSourceFile& aSrcFile);
	virtual ~CSourceFile();

	virtual void CopyFrom(CSourceFile& aSrcFile);
	virtual void Serialize(CArchive& ar);
	DECLARE_SERIAL(CSourceFile)
} ;


/////////////////////////////////////////////////////////////////////////////
// CSourceFileArray Class
//		This class defines an array of the above CSourceFile class
class CSourceFileArray : public CArray<CSourceFile*, CSourceFile*>
{
public:
	virtual ~CSourceFileArray();

	virtual void CopyFrom(CSourceFileArray& aSrcFileArray);
	virtual void FreeAll();
	virtual void Serialize(CArchive& ar);
	DECLARE_SERIAL(CSourceFileArray)
};


/////////////////////////////////////////////////////////////////////////////
// CCTLFileDoc Class

class CCTLFileDoc : public CDocument
{
protected: // create from serialization only
	CCTLFileDoc();
	DECLARE_DYNCREATE(CCTLFileDoc)

// Attributes
public:
	CSourceFileArray	SourceFiles;		// Array of source files to use

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCTLFileDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CCTLFileDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CCTLFileDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CTLDOC_H__25668D35_606B_11D1_858F_00600828300C__INCLUDED_)
