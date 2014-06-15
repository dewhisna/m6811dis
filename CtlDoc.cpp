// CtlDoc.cpp : implementation of the CCTLFileDoc class
//

#include "stdafx.h"
#include "M6811DIS.h"

#include "CtlDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSourceFile Class

IMPLEMENT_SERIAL(CSourceFile, CObject, CTL_VERSION)


CSourceFile::CSourceFile()
{
	FilePathName = "";
	LoadAddress = 0;
	DFCName = "";
}

CSourceFile::CSourceFile(CSourceFile& aSrcFile)
{
	CopyFrom(aSrcFile);
}

CSourceFile::~CSourceFile()
{

}

void CSourceFile::CopyFrom(CSourceFile& aSrcFile)
{
	FilePathName = aSrcFile.FilePathName;
	LoadAddress = aSrcFile.LoadAddress;
	DFCName = aSrcFile.DFCName;
}

void CSourceFile::Serialize(CArchive& ar)
{
	// Call base class first:
	CObject::Serialize(ar);

	if (ar.IsStoring())
	{
		ar << FilePathName;
		ar << LoadAddress;
		ar << DFCName;
	}
	else
	{
		ar >> FilePathName;
		ar >> LoadAddress;
		ar >> DFCName;
	}
}


/////////////////////////////////////////////////////////////////////////////
// CSourceFileArray Class

IMPLEMENT_SERIAL(CSourceFileArray, CObject, CTL_VERSION)

CSourceFileArray::~CSourceFileArray()
{
	FreeAll();
}

void CSourceFileArray::CopyFrom(CSourceFileArray& aSrcFileArray)
{
	int	i;

	FreeAll();
	for (i=0; i<aSrcFileArray.GetSize(); i++) {
		Add(new CSourceFile(*aSrcFileArray.GetAt(i)));
	}
}

void CSourceFileArray::FreeAll()
{
	int i;

	for (i=0; i<GetSize(); i++) {
		delete GetAt(i);
	}
	RemoveAll();
}

void CSourceFileArray::Serialize(CArchive& ar)
{
	int			i;
	int			ArraySize;
	CSourceFile	Temp;

	// Call base class first:
	CObject::Serialize(ar);

	if (ar.IsStoring())
	{
		ArraySize = GetSize();
		ar << ArraySize;
		for (i=0; i<ArraySize; i++) {
			GetAt(i)->Serialize(ar);
		}
	}
	else
	{
		FreeAll();
		ar >> ArraySize;
		for (i=0; i<ArraySize; i++) {
			Temp.Serialize(ar);
			Add(new CSourceFile(Temp));
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CCTLFileDoc

IMPLEMENT_DYNCREATE(CCTLFileDoc, CDocument)

BEGIN_MESSAGE_MAP(CCTLFileDoc, CDocument)
	//{{AFX_MSG_MAP(CCTLFileDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCTLFileDoc construction/destruction

CCTLFileDoc::CCTLFileDoc()
{
	// TODO: add one-time construction code here

}

CCTLFileDoc::~CCTLFileDoc()
{
}

BOOL CCTLFileDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CCTLFileDoc serialization

void CCTLFileDoc::Serialize(CArchive& ar)
{
	int	Version;

	if (ar.IsStoring())
	{
		Version = CTL_VERSION;
		ar << Version;
		SourceFiles.Serialize(ar);
	}
	else
	{
		ar >> Version;
		// Handle Multiple File Version Decodes:
		switch (Version) {
			// Current Version:
			case CTL_VERSION:
				SourceFiles.Serialize(ar);
				break;
			// If not supported, issue error:
			default:
				AfxThrowNotSupportedException();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CCTLFileDoc diagnostics

#ifdef _DEBUG
void CCTLFileDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CCTLFileDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCTLFileDoc commands
