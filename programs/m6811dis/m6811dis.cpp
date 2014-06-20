//
//	Motorola 6811 Disassembler
//	Copyright(c)1996 - 2014 by Donna Whisnant
//
//	Original Written in Turbo Pascal V7.0
//	Ported to MS Visual C++ 5.0/6.0
//  This version ported to GCC to be platform independent
//
//	1.0 Initial release
//
//	1.1 "Spit" version --- created August 28, 1997 by Donna Whisnant
//	This version has the new "SPIT" command added to the .CTL
//	file parsing to enable/disable code-seeking.  Also, the .OP
//	file was done away with and the file-io bugs and index ranges
//	were fixed.
//
// 	1.2 Translated code from Pascal to C++ to allow for easy expansion
//	of label size and number of labels.  Added in support from m6811dm
//	for multiple input modules.  Control files from previous versions
//	are backward compatible (except for special m6811dm compile)...
//	The only thing not compatible with the old version control files
//	is that on the old version if no entry or indirect values were
//	specified, the file load addressed was used.  On this version, the
//	default behavior of the control file parser is to issue an error
//	if there are no entry points or code-indirects specified.  This is
//	necessary because the true load address isn't necessarily known
//	because of file loading and offsetting is handled exclusively by
//	DFC libraries.
//

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "m6811dis.h"

#include "m6811gdc.h"


int main(int argc, char *argv[])
{
	bool OkFlag;
	int Count;

	CM6811Disassembler	theDisassembler;

	printf("M6811 Disassembler V%ld.%02ld -- GDC V%ld.%02ld\n",	(theDisassembler.GetVersionNumber()&0x0000FF00ul)>>8,
																(theDisassembler.GetVersionNumber()&0x000000FFul),
																(theDisassembler.GetVersionNumber()&0xFF000000ul)>>24,
																(theDisassembler.GetVersionNumber()&0x00FF0000ul)>>16);
	printf("Copyright(c)1996-2014 by Donna Whisnant\n");
	printf("\n");
	if (argc<2) {
		printf("Usage: m6811dis <ctrl filename1> [<ctrl filename2> <ctrl filename3> ...]\n");
		return -1;
	}

	OkFlag = true;
	Count = 1;
	while ((OkFlag) && (Count < argc)) {
		ifstreamControlFile CtrlFile(argv[Count]);
		if (!CtrlFile.is_open()) {
			printf("*** Error: Opening control file \"%s\" for reading...\n", argv[Count]);
			return -2;
		}
		OkFlag = theDisassembler.ReadControlFile(CtrlFile, (Count == argc-1), &std::cout, &std::cerr);
		CtrlFile.close();
		Count++;
	}

	if (OkFlag) theDisassembler.Disassemble(&std::cout, &std::cerr);

	return 0;
}
