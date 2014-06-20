//
//	Motorola 6811 Disassembler
//	Copyright(c)1996 - 2014 by Donna Whisnant
//

/*
	SFileDFC -- Motorola S-File DataFileConverter Class

	This is the C++ Motorola S-File DataFileConverter Object Class Header File.
	It is a child of the DFC Object class.

	Author:	Donna Whisnant
	Date:	February 10, 2000

	Written in: MS Visual C++ 5.0

*/

#ifndef SFILEDFC_H
#define SFILEDFC_H

#include <iostream>
#include "dfc.h"
#include "memclass.h"
#include "errmsgs.h"

class TDFC_SFile : virtual public CDFCObject {
public:
	virtual std::string GetLibraryName(void) const { return "motorola"; }
	virtual std::string GetShortDescription(void) const { return "Motorola Hex Data File Converter"; }
	virtual std::string GetDescription(void) const { return "Motorola Hex Data File Converter"; }

	const char *DefaultExtension(void) const { return "s19";	/* Could use "mot" */ }

	int RetrieveFileMapping(std::istream *aFile, unsigned long NewBase, TMemRange *aRange) const;

	int ReadDataFile(std::istream *aFile, unsigned long NewBase, TMemObject *aMemory, unsigned char aDesc) const;

	int WriteDataFile(std::ostream *aFile, TMemRange *aRange, unsigned long NewBase, TMemObject *aMemory,
								unsigned char aDesc, int UsePhysicalAddr, unsigned int FillMode) const;

private:
	int _ReadDataFile(std::istream *aFile, unsigned long NewBase, TMemObject *aMemory, TMemRange *aRange, unsigned char aDesc) const;

};

#endif   // SFILEDFC_H

