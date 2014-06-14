//	DfcLib.h
//
//	This is the implementation file for the DFCLibrary.
//
// $Log: dfclib2.h,v $
// Revision 1.1  2002/05/24 02:38:56  dewhisna
// Initial Revision Version 1.2 (Beta 1)
//
//

#ifndef DFCLIB_H
#define DFCLIB_H

#include <fstream.h>
#include <memclass.h>

#ifndef __GENREP_DFC_IMPORT__
#define __GENREP_DFC_IMPORT__
#endif
#include <dfcapi.h>


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
// Global Variable References:

extern CDFCArray	DFCs;


#endif	// DFCLIB_H
