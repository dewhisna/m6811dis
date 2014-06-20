//
//	Motorola 6811 Disassembler
//	Copyright(c)1996 - 2014 by Donna Whisnant
//

/*
      BINARYDFC -- Binary Data File Converter Class

      This is the C++ Binary DataFileConverter Object Class Header File.
      It is a child of the DFC Object class.

	  Author:  Donna Whisnant
      Date:    March 7, 1997

      Written in: Borland C++ 4.52

      Updated: June 23, 1997 to work with both MS Visual C++ 4.0/5.0
				as well as Borland C++.

*/

#ifndef BINDFC_H
#define BINDFC_H

#include <iostream>
#include "dfc.h"
#include "memclass.h"
#include "errmsgs.h"


class TDFC_Binary : virtual public CDFCObject {
public:
	virtual std::string GetLibraryName(void) const { return "binary"; }
	virtual std::string GetShortDescription(void) const { return "Binary Data File Converter"; }
	virtual std::string GetDescription(void) const { return "Binary Data File Converter"; }

	virtual const char *DefaultExtension(void) const { return "bin"; }

	virtual int RetrieveFileMapping(std::istream *aFile, unsigned long NewBase, TMemRange *aRange) const;

	virtual int ReadDataFile(std::istream *aFile, unsigned long NewBase, TMemObject *aMemory, unsigned char aDesc) const;

	virtual int WriteDataFile(std::ostream *aFile, TMemRange *aRange, unsigned long NewBase, TMemObject *aMemory,
						unsigned char aDesc, int UsePhysicalAddr, unsigned int FillMode) const;
};

#endif   // BINDFC_H

