//
//	Motorola 6811 Disassembler
//	Copyright(c)1996 - 2014 by Donna Whisnant
//

/*
      INTELDFC -- IntelHex DataFileConverter Class

      This is the C++ IntelHex DataFileConverter Object Class Header File.
      It is a child of the DFC Object class.

	  Author:  Donna Whisnant
      Date:    January 29, 1997

      Written in: Borland C++ 4.52

      Updated: June 23, 1997 to work with both MS Visual C++ 4.0/5.0
				as well as Borland C++.

*/

#ifndef INTELDFC_H
#define INTELDFC_H

#include <iostream>
#include "dfc.h"
#include "memclass.h"
#include "errmsgs.h"

class TDFC_Intel : virtual public CDFCObject {
public:
	virtual std::string GetLibraryName(void) const { return "intel"; }
	virtual std::string GetShortDescription(void) const { return "Intel Hex Data File Converter"; }
	virtual std::string GetDescription(void) const { return "Intel Hex Data File Converter"; }

	const char *DefaultExtension(void) const { return "hex"; }

	int RetrieveFileMapping(std::istream *aFile, unsigned long NewBase, TMemRange *aRange) const;

	int ReadDataFile(std::istream *aFile, unsigned long NewBase, TMemObject *aMemory, unsigned char aDesc) const;

	int WriteDataFile(std::ostream *aFile, TMemRange *aRange, unsigned long NewBase, TMemObject *aMemory,
								unsigned char aDesc, int UsePhysicalAddr, unsigned int FillMode) const;

private:
	int _ReadDataFile(std::istream *aFile, unsigned long NewBase, TMemObject *aMemory, TMemRange *aRange, unsigned char aDesc) const;

};

#endif   // INTELDFC_H

