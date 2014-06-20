//
//	Motorola 6811 Disassembler
//	Copyright(c)1996 - 2014 by Donna Whisnant
//

/*
      DFC   -- DataFileConverter Class

      This is the C++ object definition for a Data File Converter Object.  The
      Data File Converter object is an object that must be overriden by inherited
      classes to perform the task of reading and writing a data file.  This
      class allows the flexibility of file independacy for file converter programs,
      disassemblers, etc.

	  Author:  Donna Whisnant
      Date:    January 29, 1997

      Written in: Borland C++ 4.52

      Updated: June 22, 1997 to work with both MS Visual C++ 4.0/5.0
				as well as Borland C++.

      Description:

         Each Data File Converter is simply a Read function and a Write
      function, as defined here:

*/

#include <iostream>
#include "dfc.h"
#include "stringhelp.h"
#include "memclass.h"
#include "errmsgs.h"

#include "bindfc.h"
#include "inteldfc.h"
#include "sfiledfc.h"

/////////////////////////////////////////////////////////////////////////////
// CDFCObject Implementation

CDFCObject::CDFCObject()
{

}

CDFCObject::~CDFCObject()
{

}

int CDFCObject::RetrieveFileMapping(std::istream * /*aFile*/, unsigned long NewBase, TMemRange *aRange) const
{
	if (aRange) {
		aRange->PurgeRange();
		aRange->SetStartAddr(NewBase);
		aRange->SetSize(0);
	}
	return(1);
}


/////////////////////////////////////////////////////////////////////////////
// CDFCArray Implementation

CDFCArray *CDFCArray::instance()
{
	static CDFCArray DFCs;
	return &DFCs;
}

CDFCArray::CDFCArray()
	:	std::vector<CDFCObject *>()
{
	static TDFC_Binary binDFC;
	static TDFC_Intel hexDFC;
	static TDFC_SFile motDFC;

	push_back(&binDFC);
	push_back(&hexDFC);
	push_back(&motDFC);
}

CDFCArray::~CDFCArray()
{

}

const CDFCObject *CDFCArray::locateDFC(const std::string &strLibraryName) const
{
	for (unsigned int ndx = 0; ndx < size(); ++ndx) {
		if (compareNoCase(strLibraryName, at(ndx)->GetLibraryName()) == 0) return at(ndx);
	}
	return NULL;
}
