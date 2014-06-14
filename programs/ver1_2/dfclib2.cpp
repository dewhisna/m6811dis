//	DfcLib.cpp
//
//	This is the implementation file for the DFCLibrary.
//
// $Log: dfclib2.cpp,v $
// Revision 1.1  2002/05/24 02:38:55  dewhisna
// Initial Revision Version 1.2 (Beta 1)
//
//

#include "stdafx.h"
#include <afxpriv.h>
#include <dfc.h>
#include "dfclib2.h"
#include <errmsgs.h>

#include <fstream.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//	GLOBAL VARIABLES:
CDFCArray	DFCs;


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

	Load();
}

CDFCLibrary::~CDFCLibrary()
{
	while (LoadCount)
		if (Unload() == -1) break;
}

int CDFCLibrary::Load()
{
	char path_buffer[_MAX_PATH];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	if (LoadCount) {
		LoadCount++;
	} else {
		if ((LibraryName == "") || (LibraryName.IsEmpty())) return 0;
		LibraryHandle = LoadLibrary(LibraryName);
		if (!LibraryHandle) {
			_splitpath(LibraryName, drive, dir, fname, ext);
			_makepath(path_buffer, drive, dir, fname, "dfc");		// Try adding a "DFC" to the end
			LibraryHandle = LoadLibrary(path_buffer);
		}
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

	return RetVal;
}

HINSTANCE CDFCLibrary::GetHandle()
{
	return LibraryHandle;
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

