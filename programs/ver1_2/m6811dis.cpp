//
//		6811 Disassembler V1.2
//
//	Copyright(c)1996 - 1999 by Donald Whisnant
//
//	For use on IBM PC Compatible
//	Original Written in Turbo Pascal V7.0
//	This version written in MS Visual C++ 5.0
//
//	1.0 Initial release
//
//	1.1 "Spit" version --- created August 28, 1997 by Donald Whisnant
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

//
// $Log: m6811dis.cpp,v $
// Revision 1.1  2002/05/24 02:38:57  dewhisna
// Initial Revision Version 1.2 (Beta 1)
//
//

#include <stdlib.h>
#include <stdio.h>
#include "stdafx.h"
#include "m6811dis.h"

#include "m6811gdc.h"


void main(int argc, char *argv[])
{
	BOOL OkFlag;
	int Count;

	CM6811Disassembler	theDisassembler;
	CStdioFile	CtrlFile;
	CStdioFile	_StdOut(stdout);		// Make stdout into a CFile object for passing to parsers

	printf("M6811 Disassembler V%d.%02d -- GDC V%d.%02d\n",	(theDisassembler.GetVersionNumber()&0x0000FF00ul)>>8,
															(theDisassembler.GetVersionNumber()&0x000000FFul),
															(theDisassembler.GetVersionNumber()&0xFF000000ul)>>24,
															(theDisassembler.GetVersionNumber()&0x00FF0000ul)>>16);
	printf("DOS-32 Version\n");
	printf("Copyright(c)1996-1999 by Donald Whisnant\n");
	printf("\n");
	if (argc<2) {
		printf("Usage: M6811DIS <ctrl filename1> [<ctrl filename2> <ctrl filename3> ...]\n");
		_StdOut.m_pStream = NULL;	// prevent closing of stdout
		return;
	}

	OkFlag = TRUE;
	Count = 1;
	while ((OkFlag) && (Count < argc)) {
		if (!CtrlFile.Open(argv[Count], CFile::modeRead | CFile::typeText | CFile::shareDenyWrite)) {
			printf("*** Error: Opening control file \"%s\" for reading...\n", argv[Count]);
			_StdOut.m_pStream = NULL;	// prevent closing of stdout
			return;
		}
		OkFlag = theDisassembler.ReadControlFile(CtrlFile, (Count == argc-1), &_StdOut, &_StdOut);
		CtrlFile.Close();
		Count++;
	}

	if (OkFlag) theDisassembler.Disassemble(&_StdOut, &_StdOut);

	_StdOut.m_pStream = NULL;	// prevent closing of stdout
}
