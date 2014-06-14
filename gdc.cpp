//
//	GDC.CPP	-- Implementation for Generic Disassembly Class
//
//
//	(c)1998 Donald Whisnant
//
//	GDC is a generic means for defining a disassembly process that
//	processor independent.  The functions here provide the processor independent
//	part of the code.  These classes should be overriden as appropriate
//	to handle specific processors.
//

//
// $Log: gdc.cpp,v $
// Revision 1.2  2003/01/26 06:24:26  dewhisna
// Added function identification and exportation capabilities
//
// Revision 1.1  2002/05/24 02:38:56  dewhisna
// Initial Revision Version 1.2 (Beta 1)
//
//

#include "stdafx.h"
#include "gdc.h"
#include <fstream.h>
#include <memclass.h>
#include "dfclib2.h"
#include <errmsgs.h>
#include <ctype.h>

#define VERSION 0x102				// GDC Version number 1.02

// ----------------------------------------------------------------------------
//	COpcodeEntry
// ----------------------------------------------------------------------------
COpcodeEntry::COpcodeEntry()
{
	m_OpcodeBytes.RemoveAll();
	m_Group = 0;
	m_Control = 0;
	m_UserData = 0;
	m_Mnemonic = "<undefined>";
}

COpcodeEntry::COpcodeEntry(int nNumBytes, unsigned char *Bytes, DWORD nGroup, DWORD nControl, LPCTSTR szMnemonic, DWORD nUserData)
{
	int i;

	m_OpcodeBytes.RemoveAll();
	for (i=0; i<nNumBytes; i++) {
		m_OpcodeBytes.Add(Bytes[i]);
	}
	m_Group = nGroup;
	m_Control = nControl;
	m_UserData = nUserData;
	m_Mnemonic = szMnemonic;
}

COpcodeEntry::COpcodeEntry(const COpcodeEntry& aEntry)
{
	CopyFrom(aEntry);
}

void COpcodeEntry::CopyFrom(const COpcodeEntry& aEntry)
{
	int i;

	m_OpcodeBytes.RemoveAll();
	for (i=0; i<aEntry.m_OpcodeBytes.GetSize(); i++) {
		m_OpcodeBytes.Add(aEntry.m_OpcodeBytes.GetAt(i));
	}
	m_Group = aEntry.m_Group;
	m_Control = aEntry.m_Control;
	m_UserData = aEntry.m_UserData;
	m_Mnemonic = aEntry.m_Mnemonic;
}

COpcodeEntry &COpcodeEntry::operator=(COpcodeEntry& aEntry)
{
	CopyFrom(aEntry);
	return *this;
}


// ----------------------------------------------------------------------------
//	COpcodeArray
// ----------------------------------------------------------------------------
COpcodeArray::COpcodeArray()
{

}

COpcodeArray::~COpcodeArray()
{
	FreeAll();
}

void COpcodeArray::FreeAll()
{
	int i;

	for (i=0; i<GetSize(); i++) {
		if (GetAt(i) != NULL)
			delete GetAt(i);
	}
	RemoveAll();
}


// ----------------------------------------------------------------------------
//	COpcodeTable
// ----------------------------------------------------------------------------
COpcodeTable::COpcodeTable()
{
	LastFound = NULL;
}

COpcodeTable::~COpcodeTable()
{
	FreeAll();
}

void COpcodeTable::FreeAll()
{
	POSITION pos;
	unsigned char aByte;
	COpcodeArray *anArray;

	pos = GetStartPosition();
	while (pos) {
		GetNextAssoc(pos, aByte, anArray);
		if (anArray != NULL) delete anArray;
	}
	RemoveAll();

	LastFound = NULL;
}

void COpcodeTable::AddOpcode(const COpcodeEntry& nOpcode)
{
	COpcodeArray *anArray;
	COpcodeEntry *newEntry;

	if (nOpcode.m_OpcodeBytes.GetSize() == 0) return;	// MUST have an initial byte in the opcode!

	if (!Lookup(nOpcode.m_OpcodeBytes.GetAt(0), anArray)) {
		anArray = new COpcodeArray();
		if (anArray == NULL) {
			AfxThrowMemoryException();
			return;
		}
		SetAt((unsigned char)nOpcode.m_OpcodeBytes.GetAt(0), anArray);
	}

	newEntry = new COpcodeEntry(nOpcode);
	if (newEntry == NULL) {
		AfxThrowMemoryException();
		return;
	}

	anArray->Add(newEntry);
}

POSITION COpcodeTable::FindFirst(unsigned char nInitialByte)
{
	COpcodeArray *anArray;

	if (Lookup(nInitialByte, anArray)) {
		if (anArray == NULL) return NULL;
		LastFound = anArray;
		if (anArray->GetSize() == 0) {
			LastFound = NULL;
			return NULL;
		}
		return (POSITION)1ul;
	}

	LastFound = NULL;
	return NULL;
}

void COpcodeTable::FindNext(POSITION& nPos, COpcodeEntry& nOpcode)
{
	ASSERT(LastFound != NULL);							// Can't findnext without findfirst
	ASSERT((int)(long)nPos <= LastFound->GetSize());	// Can't search past end!

	nOpcode = *LastFound->GetAt((long)(nPos)-1);
	nPos = (POSITION)(((int)(long)nPos == LastFound->GetSize()) ? NULL : (int)(long)(nPos)+1);
}


// ----------------------------------------------------------------------------
//	CDisassembler
// ----------------------------------------------------------------------------

CDisassembler::CDisassembler()
	:	m_ROMMemMap(0, 0, 0),
		m_RAMMemMap(0, 0, 0),
		m_IOMemMap(0, 0, 0)
{
	// Get system time:
	m_StartTime = CTime::GetCurrentTime();

	// Set output defaults:
	m_bAddrFlag = FALSE;
	m_bOpcodeFlag = FALSE;
	m_bAsciiFlag = FALSE;
	m_bSpitFlag = FALSE;
	m_bTabsFlag = TRUE;
	m_bAsciiBytesFlag = TRUE;
	m_bDataOpBytesFlag = FALSE;

	m_nMaxNonPrint = 8;
	m_nMaxPrint = 40;
	m_nTabWidth = 4;

	m_nBase = 16;						// See ReadControlFile function before changing these 3 here!
	m_sDefaultDFC = "binary";
	m_sInputFilename = "";

	m_nLoadAddress = 0;
	m_sOutputFilename = "";
	m_sFunctionsFilename = "";
	m_sInputFileList.RemoveAll();
	m_sControlFileList.RemoveAll();

	// Setup commands to parse:
	ParseCmds.SetAt("ENTRY", 1);
	ParseCmds.SetAt("LOAD", 2);
	ParseCmds.SetAt("INPUT", 3);
	ParseCmds.SetAt("OUTPUT", 4);
	ParseCmds.SetAt("LABEL", 5);
	ParseCmds.SetAt("ADDRESSES", 6);
	ParseCmds.SetAt("INDIRECT", 7);
	ParseCmds.SetAt("OPCODES", 8);
	ParseCmds.SetAt("OPBYTES", 8);
	ParseCmds.SetAt("ASCII", 9);
	ParseCmds.SetAt("SPIT", 10);
	ParseCmds.SetAt("BASE", 11);
	ParseCmds.SetAt("MAXNONPRINT", 12);
	ParseCmds.SetAt("MAXPRINT", 13);
	ParseCmds.SetAt("DFC", 14);
	ParseCmds.SetAt("TABS", 15);
	ParseCmds.SetAt("TABWIDTH", 16);
	ParseCmds.SetAt("ASCIIBYTES", 17);
	ParseCmds.SetAt("DATAOPBYTES", 18);
	ParseCmds.SetAt("EXITFUNCTION", 19);
	ParseCmds.SetAt("MEMMAP", 20);

	ParseCmdsOP1.SetAt("OFF", FALSE);
	ParseCmdsOP1.SetAt("ON", TRUE);
	ParseCmdsOP1.SetAt("FALSE", FALSE);
	ParseCmdsOP1.SetAt("TRUE", TRUE);
	ParseCmdsOP1.SetAt("NO", FALSE);
	ParseCmdsOP1.SetAt("YES", TRUE);

	ParseCmdsOP2.SetAt("OFF", 0);
	ParseCmdsOP2.SetAt("NONE", 0);
	ParseCmdsOP2.SetAt("0", 0);
	ParseCmdsOP2.SetAt("BIN", 2);
	ParseCmdsOP2.SetAt("BINARY", 2);
	ParseCmdsOP2.SetAt("2", 2);
	ParseCmdsOP2.SetAt("OCT", 8);
	ParseCmdsOP2.SetAt("OCTAL", 8);
	ParseCmdsOP2.SetAt("8", 8);
	ParseCmdsOP2.SetAt("DEC", 10);
	ParseCmdsOP2.SetAt("DECIMAL", 10);
	ParseCmdsOP2.SetAt("10", 10);
	ParseCmdsOP2.SetAt("HEX", 16);
	ParseCmdsOP2.SetAt("HEXADECIMAL", 16);
	ParseCmdsOP2.SetAt("16", 16);

	ParseCmdsOP3.SetAt("CODE", 0);
	ParseCmdsOP3.SetAt("DATA", 1);

	ParseCmdsOP4.SetAt("ROM", 0);
	ParseCmdsOP4.SetAt("RAM", 1);
	ParseCmdsOP4.SetAt("IO", 2);

	ParseCmdsOP5.SetAt("DISASSEMBLY", 0);
	ParseCmdsOP5.SetAt("DISASSEMBLE", 0);
	ParseCmdsOP5.SetAt("DISASSEM", 0);
	ParseCmdsOP5.SetAt("DISASM", 0);
	ParseCmdsOP5.SetAt("DIS", 0);
	ParseCmdsOP5.SetAt("DASM", 0);
	ParseCmdsOP5.SetAt("FUNCTION", 1);
	ParseCmdsOP5.SetAt("FUNCTIONS", 1);
	ParseCmdsOP5.SetAt("FUNC", 1);
	ParseCmdsOP5.SetAt("FUNCT", 1);
	ParseCmdsOP5.SetAt("FUNCTS", 1);

	m_nFilesLoaded = 0;
	m_PC = 0;

	LAdrDplyCnt=0;
}

CDisassembler::~CDisassembler()
{
	POSITION pos;
	DWORD anAddress;
	CDWordArray *anArray;
	CStringArray *asArray;

	ParseCmds.RemoveAll();
	ParseCmdsOP1.RemoveAll();
	ParseCmdsOP2.RemoveAll();
	ParseCmdsOP3.RemoveAll();
	ParseCmdsOP4.RemoveAll();
	ParseCmdsOP5.RemoveAll();

	m_sInputFileList.RemoveAll();
	m_sControlFileList.RemoveAll();

	m_EntryTable.RemoveAll();

	m_FunctionsTable.RemoveAll();
	m_FuncExitAddresses.RemoveAll();

	pos = m_BranchTable.GetStartPosition();
	while (pos) {
		m_BranchTable.GetNextAssoc(pos, anAddress, anArray);
		if (anArray != NULL) delete anArray;
	}
	m_BranchTable.RemoveAll();

	pos = m_LabelTable.GetStartPosition();
	while (pos) {
		m_LabelTable.GetNextAssoc(pos, anAddress, asArray);
		if (asArray != NULL) delete asArray;
	}
	m_LabelTable.RemoveAll();

	pos = m_LabelRefTable.GetStartPosition();
	while (pos) {
		m_LabelRefTable.GetNextAssoc(pos, anAddress, anArray);
		if (anArray != NULL) delete anArray;
	}
	m_LabelRefTable.RemoveAll();

	m_CodeIndirectTable.RemoveAll();
	m_DataIndirectTable.RemoveAll();

	if (m_Memory) {
		delete m_Memory;
		m_Memory = NULL;
	}
}

DWORD CDisassembler::GetVersionNumber()
{
	return (VERSION << 16);
}

BOOL CDisassembler::ReadControlFile(CStdioFile& inFile, BOOL bLastFile, CStdioFile *msgFile, CStdioFile *errFile, int nStartLineCount)
{
	BOOL RetVal;

	CString	aLine;
	CStringArray args;
	CString Temp;
	int pos;
	int i,j;
	POSITION mpos;
	DWORD Address;
	DWORD ResAddress;
	CString LabelName;
	DWORD Dummy1;
	CString Temp2;
	CDWordArray SortedList;
	BOOL Flag;
	CStringArray *StrArray;
	TMemRange *pMemRange;

	// Note: m_nFilesLoaded must be properly set/reset prior to calling this function
	//		This is typically done in the constructor -- but overrides, etc, doing
	//		preloading of files should properly set it as well.

	//	bLastFile causes this function to report the overall disassembly summary from
	//		control files -- such as "at least one input file" and "output file must
	//		be specified" type error messages and sets any default action that should
	//		be taken with entries, etc.  The reason for the boolean flag is to
	//		allow processing of several control files prior to reporting the summary.
	//		Typically, the last control file processed should be called with a "TRUE"
	//		and all prior control files processed should be called with a "FALSE".

	RetVal = TRUE;

	m_sControlFileList.Add(inFile.GetFileName());

	if (msgFile) {
		Temp.Format("Reading and Parsing Control File: \"%s\"...\n", inFile.GetFilePath());
		msgFile->WriteString(Temp);
	}

	m_nBase = 16;						// Reset the default base before starting new file.. Therefore, each file has same default!
	m_sDefaultDFC = "binary";			// Reset the default DFC format so every control file starts with same default!
	m_sInputFilename = "";				// Reset the Input Filename so we won't try to reload file from previous control file if this control doesn't specify one

	m_nCtrlLine = nStartLineCount;		// In case several lines were read by outside process, it should pass in correct starting number so we display correct error messages!
	while (inFile.ReadString(aLine)) {
		m_nCtrlLine++;
		m_ParseError.Format("*** Error: Unknown error");
		pos = aLine.Find(';');
		if (pos != -1) aLine = aLine.Left(pos);	// Trim off comments!
		aLine.TrimLeft();
		aLine.TrimRight();
		if (aLine.GetLength() == 0) continue;	// If it is a blank or null line or only a comment, keep going
		args.RemoveAll();
		Temp = aLine;
		while (Temp.GetLength()) {
			pos = Temp.FindOneOf("\x009\x00a\x00b\x00c\x00d\x020");
			if (pos != -1) {
				args.Add(Temp.Left(pos));
				Temp = Temp.Right(Temp.GetLength()-pos-1);
			} else {
				args.Add(Temp);
				Temp = "";
			}
			Temp.TrimLeft();
		}
		if (args.GetSize() == 0) continue;		// If we don't have any args, get next line (really shouldn't ever have no args here)

		if (ParseControlLine(aLine, args, msgFile, errFile) == FALSE) {		// Go parse it -- either internal or overrides
			if (errFile) errFile->WriteString(m_ParseError);
			Temp.Format(" in Control File \"%s\" line %d\n", inFile.GetFileName(), m_nCtrlLine);
			if (errFile) errFile->WriteString(Temp);
		}
	}

	// If a Input File was specified by the control file, then open it and read it here:
	if (m_sInputFilename.GetLength()) {
		ReadSourceFile(m_sInputFilename, m_nLoadAddress, m_sDefaultDFC, msgFile, errFile);
	}

	if (bLastFile) {
		if (m_nFilesLoaded == 0) {
			if (errFile) errFile->WriteString("*** Error: At least one input file must be specified in the control file(s) and successfully loaded\n");
			RetVal = FALSE;
		}
		if (m_sOutputFilename.GetLength() == 0) {
			if (errFile) errFile->WriteString("*** Error: Disassembly Output file must be specified in the control file(s)\n");
			RetVal = FALSE;
		}
		if ((m_bSpitFlag == FALSE) && (m_EntryTable.GetCount() == 0) && (m_CodeIndirectTable.GetCount() == 0)) {
			if (errFile) errFile->WriteString("*** Error: No entry addresses or indirect code vectors have been specified in the control file(s)\n");
			RetVal = FALSE;
		}

		if (RetVal) {
			if (msgFile) {
				msgFile->WriteString("\n");
				Temp.Format("        %d Source File%s%s",
											m_sInputFileList.GetSize(),
											((m_sInputFileList.GetSize() != 1) ? "s" : ""),
											((m_sInputFileList.GetSize() != 0) ? ":" : ""));
				if (m_sInputFileList.GetSize() == 1) {
					Temp += " " + m_sInputFileList.GetAt(0) + "\n";
				}
				msgFile->WriteString(Temp);
				if (m_sInputFileList.GetSize() != 1) {
					for (i=0; i<m_sInputFileList.GetSize(); i++) {
						Temp.Format("                %s\n", LPCTSTR(m_sInputFileList[i]));
						msgFile->WriteString(Temp);
					}
				}
				msgFile->WriteString("\n");

				if (!m_sOutputFilename.IsEmpty()) {
					Temp.Format("        Disassembly Output File: %s\n\n", LPCTSTR(m_sOutputFilename));
					msgFile->WriteString(Temp);
				}

				if (!m_sFunctionsFilename.IsEmpty()) {
					Temp.Format("        Functions Output File: %s\n\n", LPCTSTR(m_sFunctionsFilename));
					msgFile->WriteString(Temp);
				}

				msgFile->WriteString("        Memory Mappings:\n");
				msgFile->WriteString("            ROM Memory Map:");
				if (m_ROMMemMap.IsNullRange()) {
					msgFile->WriteString(" <Not Defined>\n");
				} else {
					msgFile->WriteString("\n");
					pMemRange = &m_ROMMemMap;
					while (pMemRange) {
						Temp.Format("                %s%04lX - %s%04lX  (Size: %s%04lX)\n",
									LPCTSTR(GetHexDelim()), pMemRange->GetStartAddr(),
									LPCTSTR(GetHexDelim()), pMemRange->GetStartAddr() + pMemRange->GetSize() - 1,
									LPCTSTR(GetHexDelim()), pMemRange->GetSize());
						msgFile->WriteString(Temp);
						pMemRange = pMemRange->GetNext();
					}
				}
				msgFile->WriteString("            RAM Memory Map:");
				if (m_RAMMemMap.IsNullRange()) {
					msgFile->WriteString(" <Not Defined>\n");
				} else {
					msgFile->WriteString("\n");
					pMemRange = &m_RAMMemMap;
					while (pMemRange) {
						Temp.Format("                %s%04lX - %s%04lX  (Size: %s%04lX)\n",
									LPCTSTR(GetHexDelim()), pMemRange->GetStartAddr(),
									LPCTSTR(GetHexDelim()), pMemRange->GetStartAddr() + pMemRange->GetSize() - 1,
									LPCTSTR(GetHexDelim()), pMemRange->GetSize());
						msgFile->WriteString(Temp);
						pMemRange = pMemRange->GetNext();
					}
				}
				msgFile->WriteString("             IO Memory Map:");
				if (m_IOMemMap.IsNullRange()) {
					msgFile->WriteString(" <Not Defined>\n");
				} else {
					msgFile->WriteString("\n");
					pMemRange = &m_IOMemMap;
					while (pMemRange) {
						Temp.Format("                %s%04lX - %s%04lX  (Size: %s%04lX)\n",
									LPCTSTR(GetHexDelim()), pMemRange->GetStartAddr(),
									LPCTSTR(GetHexDelim()), pMemRange->GetStartAddr() + pMemRange->GetSize() - 1,
									LPCTSTR(GetHexDelim()), pMemRange->GetSize());
						msgFile->WriteString(Temp);
						pMemRange = pMemRange->GetNext();
					}
				}
				msgFile->WriteString("\n");

				Temp.Format("        %ld Entry Point%s%s\n",
											m_EntryTable.GetCount(),
											((m_EntryTable.GetCount() != 1) ? "s" : ""),
											((m_EntryTable.GetCount() != 0) ? ":" : ""));
				msgFile->WriteString(Temp);
			}
			SortedList.RemoveAll();
			mpos = m_EntryTable.GetStartPosition();
			while (mpos) {
				m_EntryTable.GetNextAssoc(mpos, Address, Dummy1);
				Flag = FALSE;
				for (i=0; ((i<SortedList.GetSize()) && (Flag == FALSE)); i++) {
					if (Address <= SortedList.GetAt(i)) {
						SortedList.InsertAt(i, Address);
						Flag = TRUE;
					}
				}
				if (Flag == FALSE) SortedList.Add(Address);
			}
			for (i=0; i<SortedList.GetSize(); i++) {
				if (msgFile) {
					Temp.Format("                %s%04lX\n", LPCTSTR(GetHexDelim()), SortedList.GetAt(i));
					msgFile->WriteString(Temp);
				}
				if (IsAddressLoaded(SortedList.GetAt(i), 1) == FALSE) {
					if (errFile) {
						Temp.Format("    *** Warning: Entry Point Address %s%04lX is outside of loaded source file(s)...\n", LPCTSTR(GetHexDelim()), SortedList.GetAt(i));
						errFile->WriteString(Temp);
					}
				}
			}

			if (msgFile) {
				msgFile->WriteString("\n");
				Temp.Format("        %ld Exit Function%s Defined%s\n",
											m_FuncExitAddresses.GetCount(),
											((m_FuncExitAddresses.GetCount() != 1) ? "s" : ""),
											((m_FuncExitAddresses.GetCount() != 0) ? ":" : ""));
				msgFile->WriteString(Temp);
			}
			SortedList.RemoveAll();
			mpos = m_FuncExitAddresses.GetStartPosition();
			while (mpos) {
				m_FuncExitAddresses.GetNextAssoc(mpos, Address, Dummy1);
				Flag = FALSE;
				for (i=0; ((i<SortedList.GetSize()) && (Flag == FALSE)); i++) {
					if (Address <= SortedList.GetAt(i)) {
						SortedList.InsertAt(i, Address);
						Flag = TRUE;
					}
				}
				if (Flag == FALSE) SortedList.Add(Address);
			}
			for (i=0; i<SortedList.GetSize(); i++) {
				if (msgFile) {
					Temp.Format("                %s%04lX\n", LPCTSTR(GetHexDelim()), SortedList.GetAt(i));
					msgFile->WriteString(Temp);
				}
				if (IsAddressLoaded(SortedList.GetAt(i), 1) == FALSE) {
					if (errFile) {
						Temp.Format("    *** Warning: Exit Function Address %s%04lX is outside of loaded source file(s)...\n", LPCTSTR(GetHexDelim()), SortedList.GetAt(i));
						errFile->WriteString(Temp);
					}
				}
			}

			if (msgFile) {
				msgFile->WriteString("\n");

				Temp.Format("        %ld Unique Label%s Defined%s\n",
											m_LabelTable.GetCount(),
											((m_LabelTable.GetCount() != 1) ? "s" : ""),
											((m_LabelTable.GetCount() != 0) ? ":" : ""));
				msgFile->WriteString(Temp);
				SortedList.RemoveAll();
				mpos = m_LabelTable.GetStartPosition();
				while (mpos) {
					m_LabelTable.GetNextAssoc(mpos, Address, StrArray);
					Flag = FALSE;
					for (i=0; ((i<SortedList.GetSize()) && (Flag == FALSE)); i++) {
						if (Address <= SortedList.GetAt(i)) {
							SortedList.InsertAt(i, Address);
							Flag = TRUE;
						}
					}
					if (Flag == FALSE) SortedList.Add(Address);
				}
				for (i=0; i<SortedList.GetSize(); i++) {
					m_LabelTable.Lookup(SortedList.GetAt(i), StrArray);
					Temp.Format("                %s%04lX=", LPCTSTR(GetHexDelim()), SortedList.GetAt(i));
					msgFile->WriteString(Temp);
					if (StrArray) {
						for (j=0; j<StrArray->GetSize(); j++) {
							if (j != 0) msgFile->WriteString(",");
							msgFile->WriteString(StrArray->GetAt(j));
						}
					}
					msgFile->WriteString("\n");
				}
				msgFile->WriteString("\n");

				if (m_bAddrFlag) msgFile->WriteString("Writing program counter addresses to disassembly file.\n");
				if (m_bOpcodeFlag) msgFile->WriteString("Writing opcode byte values to disassembly file.\n");
				if (m_bAsciiFlag) msgFile->WriteString("Writing printable data as ASCII in disassembly file.\n");
				if (m_bSpitFlag) msgFile->WriteString("Performing a code-dump (spit) disassembly instead of code-seeking.\n");
				if (m_bTabsFlag) {
					Temp.Format("Using tab characters in disassembly file.  Tab width set at: %d\n", m_nTabWidth);
					msgFile->WriteString(Temp);
				}
				if (m_bAsciiBytesFlag) msgFile->WriteString("Writing byte value comments for ASCII data in disassembly file.\n");
				if ((m_bOpcodeFlag) && (m_bDataOpBytesFlag)) msgFile->WriteString("Writing OpBytes for data in disassembly file\n");
				msgFile->WriteString("\n");
			}


		// Ok, now we resolve the indirect tables...  This process should create the indirect
		//		label name from the resolved address if the corresponding string in the hash table is "".
		//		This process requires the purely virtual ResolveIndirect function from overrides...
			if (msgFile) msgFile->WriteString("Compiling Indirect Code (branch) Table as specified in Control File...\n");
			SortedList.RemoveAll();
			mpos = m_CodeIndirectTable.GetStartPosition();
			while (mpos) {
				m_CodeIndirectTable.GetNextAssoc(mpos, Address, Temp2);
				Flag = FALSE;
				for (i=0; ((i<SortedList.GetSize()) && (Flag == FALSE)); i++) {
					if (Address <= SortedList.GetAt(i)) {
						SortedList.InsertAt(i, Address);
						Flag = TRUE;
					}
				}
				if (Flag == FALSE) SortedList.Add(Address);
			}
			if (msgFile) {
				Temp.Format("        %ld Indirect Code Vector%s%s\n",
											SortedList.GetSize(),
											((SortedList.GetSize() != 1) ? "s" : ""),
											((SortedList.GetSize() != 0) ? ":" : ""));
				msgFile->WriteString(Temp);
			}
			for (i=0; i<SortedList.GetSize(); i++) {
				Address = SortedList.GetAt(i);
				m_CodeIndirectTable.Lookup(Address, LabelName);
				if (msgFile) {
					Temp.Format("                [%s%04lX] -> ", LPCTSTR(GetHexDelim()), Address);
					msgFile->WriteString(Temp);
				}
				if (ResolveIndirect(Address, ResAddress, 0) == FALSE) {
					if (msgFile) {
						msgFile->WriteString("ERROR\n");
					}
					if (errFile) {
						Temp.Format("    *** Warning: Vector Address %s%04lX is outside of loaded source file(s)...\n", LPCTSTR(GetHexDelim()), Address);
						errFile->WriteString(Temp);
						errFile->WriteString("                    Or the vector location conflicted with other analyzed areas.\n");

					}
				} else {
					if (msgFile) {
						Temp.Format("%s%04lX", LPCTSTR(GetHexDelim()), ResAddress);
						msgFile->WriteString(Temp);
					}
					AddLabel(ResAddress, FALSE, 0, LabelName);		// Add label for resolved name.  If NULL, add it so later we can resolve Lxxxx from it.
					m_FunctionsTable.SetAt(ResAddress, FUNCF_INDIRECT);		// Resolved code indirects are also considered start-of functions
					if (AddBranch(ResAddress, TRUE, Address) == FALSE) {
						if (errFile) {
							Temp.Format("    *** Warning: Indirect Address [%s%04lX] -> %s%04lX is outside of loaded source file(s)...\n", LPCTSTR(GetHexDelim()), Address, LPCTSTR(GetHexDelim()), ResAddress);
							errFile->WriteString(Temp);
						}
						if ((msgFile) && (msgFile != errFile)) {
							msgFile->WriteString("\n");
						}
					} else {
						if (msgFile) {
							msgFile->WriteString("\n");
						}
					}
				}
			}
			if (msgFile) msgFile->WriteString("\n");
			// Now, repeat for Data indirects:
			if (msgFile) msgFile->WriteString("Compiling Indirect Data Table as specified in Control File...\n");
			SortedList.RemoveAll();
			mpos = m_DataIndirectTable.GetStartPosition();
			while (mpos) {
				m_DataIndirectTable.GetNextAssoc(mpos, Address, Temp2);
				Flag = FALSE;
				for (i=0; ((i<SortedList.GetSize()) && (Flag == FALSE)); i++) {
					if (Address <= SortedList.GetAt(i)) {
						SortedList.InsertAt(i, Address);
						Flag = TRUE;
					}
				}
				if (Flag == FALSE) SortedList.Add(Address);
			}
			if (msgFile) {
				Temp.Format("        %ld Indirect Data Vector%s%s\n",
											SortedList.GetSize(),
											((SortedList.GetSize() != 1) ? "s" : ""),
											((SortedList.GetSize() != 0) ? ":" : ""));
				msgFile->WriteString(Temp);
			}
			for (i=0; i<SortedList.GetSize(); i++) {
				Address = SortedList.GetAt(i);
				m_DataIndirectTable.Lookup(Address, LabelName);
				if (msgFile) {
					Temp.Format("                [%s%04lX] -> ", LPCTSTR(GetHexDelim()), Address);
					msgFile->WriteString(Temp);
				}
				if (ResolveIndirect(Address, ResAddress, 1) == FALSE) {
					if (msgFile) {
						msgFile->WriteString("ERROR\n");
					}
					if (errFile) {
						Temp.Format("    *** Warning: Vector Address %s%04lX is outside of loaded source file(s)...\n", LPCTSTR(GetHexDelim()), Address);
						errFile->WriteString(Temp);
						errFile->WriteString("                    Or the vector location conflicted with other analyzed areas.\n");
					}
				} else {
					if (msgFile) {
						Temp.Format("%s%04lX\n", LPCTSTR(GetHexDelim()), ResAddress);
						msgFile->WriteString(Temp);
					}
					AddLabel(ResAddress, TRUE, Address, LabelName);		// Add label for resolved name.  If NULL, add it so later we can resolve Lxxxx from it.
				}
			}
			if (msgFile) msgFile->WriteString("\n");
		}
	}

	return RetVal;
}

BOOL CDisassembler::ParseControlLine(LPCTSTR szLine, CStringArray& argv, CStdioFile *msgFile, CStdioFile *errFile)
{
	BOOL RetVal;
	CString	Cmd;
	WORD CmdIndex;
	DWORD Address;
	DWORD Size;
	DWORD Dummy1;
	CString DummyStr;
	CString TempStr;
	CString DfcLibrary;
	CString FileName;
	WORD	TempWord;
	DWORD	TempDWord;
	int ArgError;
	BOOL TempFlag;
	BOOL TempFlag2;

	ASSERT(argv.GetSize() != 0);

	if (argv.GetSize() == 0) return FALSE;

	RetVal = TRUE;
	ArgError = 0;

	Cmd = argv[0];
	Cmd.MakeUpper();

	if (!ParseCmds.Lookup(Cmd, CmdIndex)) CmdIndex = -1;
	switch (CmdIndex) {
		case 1:		// ENTRY <addr> [<name>]
			if (argv.GetSize() < 2) {
				ArgError = 1;
				break;
			}
			if (argv.GetSize() > 3) {
				ArgError = 2;
				break;
			}
			if (argv.GetSize() == 3) {
				if (!ValidateLabelName(argv[2])) {
					ArgError = 3;
					break;
				}
			}
			Address = strtoul(argv[1], NULL, m_nBase);
			if (m_EntryTable.Lookup(Address, Dummy1)) {
				RetVal = FALSE;
				m_ParseError = "*** Warning: Duplicate entry address";
			}
			m_EntryTable.SetAt(Address, TRUE);	// Add an entry to the entry table
			m_FunctionsTable.SetAt(Address, FUNCF_ENTRY);	// Entries are also considered start-of functions
			if (argv.GetSize() == 3) {
				if (AddLabel(Address, FALSE, 0, argv[2]) == FALSE) {
					RetVal = FALSE;
					m_ParseError = "*** Warning: Duplicate label";
				}
			}
			break;
		case 2:		// LOAD <addr> | <addr> <filename> [<library>]
		{
			if (argv.GetSize() == 2) {
				m_nLoadAddress = strtoul(argv[1], NULL, m_nBase);
				break;
			}
			if (argv.GetSize() < 3) {
				ArgError = 1;
				break;
			}
			if (argv.GetSize() > 4) {
				ArgError = 2;
				break;
			}
			Address = strtoul(argv[1], NULL, m_nBase);
			if (argv.GetSize() == 4) {
				DfcLibrary = argv[3];
			} else {
				DfcLibrary = m_sDefaultDFC;
			}
			FileName = argv[2];

			ReadSourceFile(FileName, Address, DfcLibrary, msgFile, errFile);
			// We don't set RetVal to FALSE here if there is an error on ReadSource,
			//	because errors have already been printed and are not in ParseError...
			break;
		}
		case 3:		// INPUT <filename>
			if (argv.GetSize() != 2) {
				ArgError = (argv.GetSize() < 2) ? 1 : 2;
				break;
			}
			if (m_sInputFilename.GetLength()) {
				RetVal = FALSE;
				m_ParseError = "*** Warning: Input filename already defined";
			}
			m_sInputFilename = argv[1];
			break;
		case 4:		// OUTPUT [DISASSEMBLY | FUNCTIONS] <filename>
			if ((argv.GetSize() != 2) && (argv.GetSize() != 3)) {
				ArgError = (argv.GetSize() < 2) ? 1 : 2;
				break;
			}
			TempFlag = FALSE;		// TempFlag = FALSE if type not specified on line, TRUE if it is
			if (argv.GetSize() == 2) {		// If type isn't specified, assume DISASSEMBLY
				TempWord = 0;
			} else {
				TempFlag = TRUE;
				TempStr = argv[1];
				TempStr.MakeUpper();
				if (!ParseCmdsOP5.Lookup(TempStr, TempWord)) {
					ArgError = 3;
					break;
				}
			}
			switch (TempWord) {
				case 0:				// DISASSEMBLY
					if (m_sOutputFilename.GetLength()) {
						RetVal = FALSE;
						m_ParseError = "*** Warning: Disassembly Output filename already defined";
					}
					m_sOutputFilename = argv[((TempFlag) ? 2 : 1)];
					break;
				case 1:				// FUNCTIONS
					if (m_sFunctionsFilename.GetLength()) {
						RetVal = FALSE;
						m_ParseError = "*** Warning: Functions Output filename already defined";
					}
					m_sFunctionsFilename = argv[((TempFlag) ? 2 : 1)];
					break;
			}
			break;
		case 5:		// LABEL <addr> <name>
			if (argv.GetSize() != 3) {
				ArgError = (argv.GetSize() < 3) ? 1 : 2;
				break;
			}
			if (!ValidateLabelName(argv[2])) {
				ArgError = 3;
				break;
			}
			Address = strtoul(argv[1], NULL, m_nBase);
			if (AddLabel(Address, FALSE, 0, argv[2]) == FALSE) {
				RetVal = FALSE;
				m_ParseError = "*** Warning: Duplicate label";
			}
			break;
		case 6:		// ADDRESSES [ON | OFF | TRUE | FALSE | YES | NO]
			if (argv.GetSize() < 2) {
				m_bAddrFlag = TRUE;		// Default is ON
				break;
			}
			if (argv.GetSize() > 2) {
				ArgError = 2;
				break;
			}
			TempStr = argv[1];
			TempStr.MakeUpper();
			if (!ParseCmdsOP1.Lookup(TempStr, TempWord)) {
				ArgError = 3;
				break;
			}
			m_bAddrFlag = TempWord;
			break;
		case 7:		// INDIRECT [CODE | DATA] <addr> [<name>]
			if ((argv.GetSize() != 2) && (argv.GetSize() != 3) && (argv.GetSize() != 4)) {
				ArgError = (argv.GetSize() < 2) ? 1 : 2;
				break;
			}

			TempFlag = FALSE;		// TempFlag = FALSE if CODE/DATA not specified on line, TRUE if it is
			TempFlag2 = FALSE;		// TempFlag2 = FALSE if <name> not specified on line, TRUE if it is
			if (argv.GetSize() == 2) {		// If Code or Data isn't specified, assume code
				TempWord = 0;
			} else {
				TempStr = argv[1];
				TempStr.MakeUpper();
				if (!ParseCmdsOP3.Lookup(TempStr, TempWord)) {
					if (argv.GetSize() == 4) {			// If all args was specified, error out if not valid
						ArgError = 3;
						break;
					} else {					// If here, we assume we have <addr> and <name> since we are one arg short
						TempWord = 0;			// Assume Code since it isn't specified
						TempFlag2 = TRUE;		// And we do have a <name>
					}
				} else {
					TempFlag = TRUE;
					if (argv.GetSize() == 4) {			// If all args was specified, we have valid everything
						TempFlag2 = TRUE;
					}
				}
			}

			// From the following point to the end of this case, TempStr = the Indirect <name>
			//		If not specified, we set the name to "" so the Resolution function will
			//		create it from the resolved address...
			if (TempFlag2) {
				TempStr = argv[2 + ((TempFlag) ? 1 : 0)];
				if (!ValidateLabelName(TempStr)) {
					ArgError = 3;
					break;
				}
			} else {
				TempStr = "";
			}
			Address = strtoul(argv[1 + ((TempFlag) ? 1 : 0)], NULL, m_nBase);
			switch (TempWord) {
				case 0:
					if ((m_CodeIndirectTable.Lookup(Address, DummyStr)) ||
						(m_DataIndirectTable.Lookup(Address, DummyStr))) {
						RetVal = FALSE;
						m_ParseError = "*** Warning: Duplicate indirect";
					}
					m_CodeIndirectTable.SetAt(Address, TempStr);	// Add a label to the code indirect table
					break;
				case 1:
					if ((m_DataIndirectTable.Lookup(Address, DummyStr)) ||
						(m_CodeIndirectTable.Lookup(Address, DummyStr))) {
						RetVal = FALSE;
						m_ParseError = "*** Warning: Duplicate indirect";
					}
					m_DataIndirectTable.SetAt(Address, TempStr);	// Add a label to the data indirect table
					break;
			}
			break;
		case 8:		// OPCODES [ON | OFF | TRUE | FALSE | YES | NO]
					// OPBYTES [ON | OFF | TRUE | FALSE | YES | NO]
			if (argv.GetSize() < 2) {
				m_bOpcodeFlag = TRUE;		// Default is ON
				break;
			}
			if (argv.GetSize() > 2) {
				ArgError = 2;
				break;
			}
			TempStr = argv[1];
			TempStr.MakeUpper();
			if (!ParseCmdsOP1.Lookup(TempStr, TempWord)) {
				ArgError = 3;
				break;
			}
			m_bOpcodeFlag = TempWord;
			break;
		case 9:		// ASCII [ON | OFF | TRUE | FALSE | YES | NO]
			if (argv.GetSize() < 2) {
				m_bAsciiFlag = TRUE;		// Default is ON
				break;
			}
			if (argv.GetSize() > 2) {
				ArgError = 2;
				break;
			}
			TempStr = argv[1];
			TempStr.MakeUpper();
			if (!ParseCmdsOP1.Lookup(TempStr, TempWord)) {
				ArgError = 3;
				break;
			}
			m_bAsciiFlag = TempWord;
			break;
		case 10:	// SPIT [ON | OFF | TRUE | FALSE | YES | NO]
			if (argv.GetSize() < 2) {
				m_bSpitFlag = TRUE;		// Default is ON
				break;
			}
			if (argv.GetSize() > 2) {
				ArgError = 2;
				break;
			}
			TempStr = argv[1];
			TempStr.MakeUpper();
			if (!ParseCmdsOP1.Lookup(TempStr, TempWord)) {
				ArgError = 3;
				break;
			}
			m_bSpitFlag = TempWord;
			break;
		case 11:	// BASE [OFF | BIN | OCT | DEC | HEX]
			if (argv.GetSize() < 2) {
				m_nBase = 0;		// Default is OFF
				break;
			}
			if (argv.GetSize() > 2) {
				ArgError = 2;
				break;
			}
			TempStr = argv[1];
			TempStr.MakeUpper();
			if (!ParseCmdsOP2.Lookup(TempStr, TempWord)) {
				ArgError = 3;
				break;
			}
			m_nBase = TempWord;
			break;
		case 12:	// MAXNONPRINT <value>
			if (argv.GetSize() != 2) {
				ArgError = (argv.GetSize() < 2) ? 1 : 2;
				break;
			}
			m_nMaxNonPrint = (WORD)strtoul(argv[1], NULL, m_nBase);
			break;
		case 13:	// MAXPRINT <value>
			if (argv.GetSize() != 2) {
				ArgError = (argv.GetSize() < 2) ? 1 : 2;
				break;
			}
			m_nMaxPrint = (WORD)strtoul(argv[1], NULL, m_nBase);
			break;
		case 14:	// DFC <library>
			if (argv.GetSize() != 2) {
				ArgError = (argv.GetSize() < 2) ? 1 : 2;
				break;
			}
			m_sDefaultDFC = argv[1];
			break;
		case 15:	// TABS [ON | OFF | TRUE | FALSE | YES | NO]
			if (argv.GetSize() < 2) {
				m_bTabsFlag = TRUE;		// Default is ON
				break;
			}
			if (argv.GetSize() > 2) {
				ArgError = 2;
				break;
			}
			TempStr = argv[1];
			TempStr.MakeUpper();
			if (!ParseCmdsOP1.Lookup(TempStr, TempWord)) {
				ArgError = 3;
				break;
			}
			m_bTabsFlag = TempWord;
			break;
		case 16:	// TABWIDTH <value>
			if (argv.GetSize() != 2) {
				ArgError = (argv.GetSize() < 2) ? 1 : 2;
				break;
			}
			m_nTabWidth = (WORD)strtoul(argv[1], NULL, m_nBase);
			break;
		case 17:	// ASCIIBYTES [ON | OFF | TRUE | FALSE | YES | NO]
			if (argv.GetSize() < 2) {
				m_bAsciiBytesFlag = TRUE;		// Default is ON
				break;
			}
			if (argv.GetSize() > 2) {
				ArgError = 2;
				break;
			}
			TempStr = argv[1];
			TempStr.MakeUpper();
			if (!ParseCmdsOP1.Lookup(TempStr, TempWord)) {
				ArgError = 3;
				break;
			}
			m_bAsciiBytesFlag = TempWord;
			break;
		case 18:	// DATAOPBYTES [ON | OFF | TRUE | FALSE | YES | NO]
			if (argv.GetSize() < 2) {
				m_bDataOpBytesFlag = TRUE;		// Default is ON
				break;
			}
			if (argv.GetSize() > 2) {
				ArgError = 2;
				break;
			}
			TempStr = argv[1];
			TempStr.MakeUpper();
			if (!ParseCmdsOP1.Lookup(TempStr, TempWord)) {
				ArgError = 3;
				break;
			}
			m_bDataOpBytesFlag = TempWord;
			break;
		case 19:	// EXITFUNCTION <addr> [<name>]
			if (argv.GetSize() < 2) {
				ArgError = 1;
				break;
			}
			if (argv.GetSize() > 3) {
				ArgError = 2;
				break;
			}
			if (argv.GetSize() == 3) {
				if (!ValidateLabelName(argv[2])) {
					ArgError = 3;
					break;
				}
			}
			Address = strtoul(argv[1], NULL, m_nBase);
			if (m_FuncExitAddresses.Lookup(Address, Dummy1)) {
				RetVal = FALSE;
				m_ParseError = "*** Warning: Duplicate function exit address";
			}
			m_FunctionsTable.SetAt(Address, FUNCF_ENTRY);	// Exit Function Entry points are also considered start-of functions
			m_FuncExitAddresses.SetAt(Address, TRUE);	// Add function exit entry
			if (argv.GetSize() == 3) {
				if (AddLabel(Address, FALSE, 0, argv[2]) == FALSE) {
					RetVal = FALSE;
					m_ParseError = "*** Warning: Duplicate label";
				}
			}
			break;
		case 20:	// MEMMAP [ROM | RAM | IO] <addr> <size>
			if ((argv.GetSize() != 3) && (argv.GetSize() != 4)) {
				ArgError = (argv.GetSize() < 3) ? 1 : 2;
				break;
			}

			TempFlag = FALSE;		// TempFlag = FALSE if ROM/RAM/IO not specified on line, TRUE if it is
			if (argv.GetSize() == 3) {		// If ROM/RAM/IO not specified on line, assume ROM
				TempWord = 0;
			} else {
				TempStr = argv[1];
				TempStr.MakeUpper();
				if (!ParseCmdsOP4.Lookup(TempStr, TempWord)) {
					ArgError = 3;			// Error out if type not valid but specified
					break;
				} else {
					TempFlag = TRUE;		// If specified and valid, set flag
				}
			}

			// Get address and size specified:
			Address = strtoul(argv[1 + ((TempFlag) ? 1 : 0)], NULL, m_nBase);
			Size = strtoul(argv[2 + ((TempFlag) ? 1 : 0)], NULL, m_nBase);

			switch (TempWord) {
				case 0:		// ROM
					m_ROMMemMap.AddRange(Address, Size, 0);
					m_ROMMemMap.Compact();
					m_ROMMemMap.RemoveOverlaps();
					m_ROMMemMap.Sort();
					for (TempDWord = Address; ((TempDWord < (Address + Size)) && (RetVal)); TempDWord++) {
						if (m_RAMMemMap.AddressInRange(TempDWord)) {
							RetVal = FALSE;
							m_ParseError = "*** Warning: Specified ROM Mapping conflicts with RAM Mapping";
						} else {
							if (m_IOMemMap.AddressInRange(TempDWord)) {
								RetVal = FALSE;
								m_ParseError = "*** Warning: Specified ROM Mapping conflicts with IO Mapping";
							}
						}
					}
					break;
				case 1:		// RAM
					m_RAMMemMap.AddRange(Address, Size, 0);
					m_RAMMemMap.Compact();
					m_RAMMemMap.RemoveOverlaps();
					m_RAMMemMap.Sort();
					for (TempDWord = Address; ((TempDWord < (Address + Size)) && (RetVal)); TempDWord++) {
						if (m_ROMMemMap.AddressInRange(TempDWord)) {
							RetVal = FALSE;
							m_ParseError = "*** Warning: Specified RAM Mapping conflicts with ROM Mapping";
						} else {
							if (m_IOMemMap.AddressInRange(TempDWord)) {
								RetVal = FALSE;
								m_ParseError = "*** Warning: Specified RAM Mapping conflicts with IO Mapping";
							}
						}
					}
					break;
				case 2:		// IO
					m_IOMemMap.AddRange(Address, Size, 0);
					m_IOMemMap.Compact();
					m_IOMemMap.RemoveOverlaps();
					m_IOMemMap.Sort();
					for (TempDWord = Address; ((TempDWord < (Address + Size)) && (RetVal)); TempDWord++) {
						if (m_ROMMemMap.AddressInRange(TempDWord)) {
							RetVal = FALSE;
							m_ParseError = "*** Warning: Specified IO Mapping conflicts with ROM Mapping";
						} else {
							if (m_RAMMemMap.AddressInRange(TempDWord)) {
								RetVal = FALSE;
								m_ParseError = "*** Warning: Specified IO Mapping conflicts with RAM Mapping";
							}
						}
					}
					break;
			}
			break;
		default:
			m_ParseError = "*** Error: Unknown command";
			return FALSE;
	}

	if (ArgError) {
		RetVal = FALSE;
		switch (ArgError) {
			case 1:
				m_ParseError.Format("*** Error: Not enough arguments for '%s' command", argv[0]);
				break;
			case 2:
				m_ParseError.Format("*** Error: Too many arguments for '%s' command", argv[0]);
				break;
			case 3:
				m_ParseError.Format("*** Error: Illegal argument for '%s' command", argv[0]);
				break;
		}
	}

	return RetVal;
}


BOOL CDisassembler::ReadSourceFile(LPCTSTR szFilename, DWORD nLoadAddress, LPCTSTR szDFCLibrary, CStdioFile *msgFile, CStdioFile *errFile)
{
	if (szDFCLibrary == NULL) return FALSE;
	if (szFilename == NULL) return FALSE;

	fstream theFile;
	int Status;
	CDFCLibrary theDFC(szDFCLibrary);		// Create the DFC Interface and load the DFC Library (automatically unloads when function exits)
	CString Temp;
	BOOL RetVal = TRUE;

	if (msgFile) {
		Temp.Format("Loading \"%s\" at offset %s%04lX using %s library...\n", szFilename, LPCTSTR(GetHexDelim()), nLoadAddress, szDFCLibrary);
		msgFile->WriteString(Temp);
	}
	if (theDFC.IsLoaded() == 0) {
		Temp.Format("*** Error: Can't open DFC Library \"%s\" to read \"%s\"\n", szDFCLibrary, szFilename);
		if (errFile) errFile->WriteString(Temp);
		RetVal = FALSE;
	} else {
		theFile.open(szFilename, ios::in | ios::binary | ios::nocreate, filebuf::sh_read);
		if (theFile.is_open() == 0) {
			Temp.Format("*** Error: Can't open file \"%s\" for reading\n", szFilename);
			if (errFile) errFile->WriteString(Temp);
			RetVal = FALSE;
		} else {
			m_nFilesLoaded++;
			m_sInputFileList.Add(szFilename);
			try {
				Status = theDFC.ReadDataFile(&theFile, nLoadAddress, m_Memory, DMEM_LOADED);
			}
			catch (EXCEPTION_ERROR* aErr) {
				RetVal = FALSE;
				m_nFilesLoaded--;
				m_sInputFileList.RemoveAt(m_sInputFileList.GetSize()-1);
				switch (aErr->cause) {
					case EXCEPTION_ERROR::ERR_CHECKSUM:
						Temp.Format("*** Error: Checksum error reading file \"%s\"\n", szFilename);
						if (errFile) errFile->WriteString(Temp);
						break;
					case EXCEPTION_ERROR::ERR_UNEXPECTED_EOF:
						Temp.Format("*** Error: Unexpected end-of-file reading \"%s\"\n", szFilename);
						if (errFile) errFile->WriteString(Temp);
						break;
					case EXCEPTION_ERROR::ERR_OVERFLOW:
						Temp.Format("*** Error: Reading file \"%s\" extends past the defined memory limits of this processor\n", szFilename);
						if (errFile) errFile->WriteString(Temp);
						break;
					case EXCEPTION_ERROR::ERR_READFAILED:
						Temp.Format("*** Error: Reading file \"%s\"\n", szFilename);
						if (errFile) errFile->WriteString(Temp);
						break;
					default:
						Temp.Format("*** Error: Unknown DFC Error encountered while reading file \"%s\"\n", szFilename);
						if (errFile) errFile->WriteString(Temp);
				}
			}
			if (Status == 0) {
				RetVal = FALSE;
				Temp.Format("*** Warning: Reading file \"%s\", overlaps previously loaded files\n", szFilename);
				if (errFile) errFile->WriteString(Temp);
			}
			theFile.close();
		}
	}
	// Note, the DFC will unload when destructor is called upon exiting this routine
	return RetVal;
}


BOOL CDisassembler::ValidateLabelName(LPCTSTR aName)
{
	CString Temp = aName;
	int i;

	if (Temp.GetLength() == 0) return FALSE;					// Must have a length
	if ((!isalpha(Temp[0])) && (Temp[0] != '_')) return FALSE;	// Must start with alpha or '_'
	for (i=0; i<Temp.GetLength(); i++) {
		if ((!isalnum(Temp[i])) && (Temp[i] != '_')) return FALSE;	// Each character must be alphanumeric or '_'
	}
	return TRUE;
}

// BOOL CDisassembler::ResolveIndirect(DWORD nAddress, DWORD& nResAddress, int nType) = 0;		// Purely Virtual!
//		NOTE: This function should set the m_Memory type code for the affected bytes to
//				either DMEM_CODEINDIRECT or DMEM_DATAINDIRECT!!!!   It should set
//				nResAddress to the resolved address and return T/F -- True if indirect
//				vector word lies completely inside the loaded space.  False if indirect
//				vector word (or part of it) is outside.  This function is NOT to check
//				the destination for loaded status -- that is checked by the routine
//				calling this function...  nType will be 0 for Code and 1 for Data...
//				Typically nType can be added to DMEM_CODEINDIRECT to determine the correct
//				code to store.  This is because the DMEM enumeration was created with
//				this expansion and uniformity in mind.
//				It is also good for this function to check that all bytes of the indirect
//				vector itself have previously not been "looked at" or tagged.  Routines
//				calling this function for previously tagged addresses should reset the
//				vector bytes to DMEM_LOADED.


BOOL CDisassembler::IsAddressLoaded(DWORD nAddress, int nSize)
{
	BOOL RetVal = TRUE;
	int i;

	for (i=0; ((i<nSize) && (RetVal)); i++) {
		RetVal = RetVal && (m_Memory->GetDescriptor(nAddress + i) != DMEM_NOTLOADED);
	}
	return RetVal;
}


BOOL CDisassembler::AddLabel(DWORD nAddress, BOOL bAddRef, DWORD nRefAddress, LPCTSTR szLabel)
{
	CDWordArray *pRefList;
	CStringArray *pLabelList;
	CString Temp;
	int i;
	BOOL Flag;

	if (szLabel == NULL) {
		Temp = "";
	} else {
		Temp = szLabel;
	}
	if (m_LabelTable.Lookup(nAddress, pLabelList) == TRUE) {
		if (pLabelList == NULL) {
			ASSERT(FALSE);			// We should never encounter a null entry -- check adding!
			return FALSE;
		}
		if (m_LabelRefTable.Lookup(nAddress, pRefList) != TRUE) {
			ASSERT(FALSE);			// We should have an array for both label strings and addresses -- check adding!
			return FALSE;
		}
		if (bAddRef) {				// Search and add reference, even if we find the string
			Flag = FALSE;
			for (i=0; ((i<pRefList->GetSize()) && (Flag==FALSE)); i++) {
				if (pRefList->GetAt(i) == nRefAddress) Flag = TRUE;
			}
			if (Flag == FALSE) {
				pRefList->Add(nRefAddress);
			}
		}
		for (i=0; i<pLabelList->GetSize(); i++) {
			if (Temp.CompareNoCase(pLabelList->GetAt(i)) == 0) return FALSE;
		}
	} else {
		pLabelList = new CStringArray;
		if (pLabelList == NULL) AfxThrowMemoryException();
		m_LabelTable.SetAt(nAddress, pLabelList);
		pRefList = new CDWordArray;
		if (pRefList == NULL) AfxThrowMemoryException();
		m_LabelRefTable.SetAt(nAddress, pRefList);
		if (bAddRef) {				// If we are just now adding the label, we can add the ref without searching because it doesn't exist either
			pRefList->Add(nRefAddress);
		}
	}
	ASSERT(pLabelList != NULL);
	if (pLabelList->GetSize()) {
		if (Temp.GetLength()) {
			for (i=0; i<pLabelList->GetSize(); i++) {
				if (pLabelList->GetAt(i).GetLength() == 0) {
					pLabelList->RemoveAt(i);
					break;							// If adding a non-Lxxxx entry, remove previous Lxxxx entry if it exists!
				}
			}
		} else {
			return FALSE;		// Don't set Lxxxx entry if we already have at least 1 label!
		}
	}		
	pLabelList->Add(Temp);
	return TRUE;
}

BOOL CDisassembler::AddBranch(DWORD nAddress, BOOL bAddRef, DWORD nRefAddress)
{
	CDWordArray *pRefList;
	BOOL Flag;
	int i;

	if (m_BranchTable.Lookup(nAddress, pRefList) == FALSE) {
		pRefList = new CDWordArray;
		if (pRefList == NULL) AfxThrowMemoryException();
		m_BranchTable.SetAt(nAddress, pRefList);
	}
	ASSERT(pRefList != NULL);
	if (bAddRef) {				// Search and add reference
		Flag = FALSE;
		for (i=0; ((i<pRefList->GetSize()) && (Flag==FALSE)); i++) {
			if (pRefList->GetAt(i) == nRefAddress) Flag = TRUE;
		}
		if (Flag == FALSE) {
			pRefList->Add(nRefAddress);
		}
	}
	return IsAddressLoaded(nAddress, 1);
}

void CDisassembler::GenDataLabel(DWORD nAddress, DWORD nRefAddress, LPCTSTR szLabel, CStdioFile *msgFile, CStdioFile *errFile)
{
	if (AddLabel(nAddress, TRUE, nRefAddress)) OutputGenLabel(nAddress, msgFile);
}

void CDisassembler::GenAddrLabel(DWORD nAddress, DWORD nRefAddress, LPCTSTR szLabel, CStdioFile *msgFile, CStdioFile *errFile)
{
	CString Temp;

	if (AddLabel(nAddress, FALSE, nRefAddress)) OutputGenLabel(nAddress, msgFile);
	if (AddBranch(nAddress, TRUE, nRefAddress) == FALSE) {
		if (errFile) {
			if ((errFile == msgFile) || (LAdrDplyCnt != 0)) {
				errFile->WriteString("\n");
				LAdrDplyCnt = 0;
			}
			Temp.Format("     *** Warning:  Branch Ref: %s%04lX is outside of Loaded Source File.\n", LPCTSTR(GetHexDelim()), nAddress);
			errFile->WriteString(Temp);
		}
	}
}

void CDisassembler::OutputGenLabel(DWORD nAddress, CStdioFile *msgFile)
{
	CString Temp;

	if (msgFile == NULL) return;
	Temp = GenLabel(nAddress);
	Temp += ' ';
	if (Temp.GetLength() < 7) {
		Temp += "       ";
		Temp = Temp.Left(7);
	}
	if (LAdrDplyCnt >= 9) {
		if (Temp.Find(' ') != -1) Temp = Temp.Left(Temp.Find(' '));
		Temp += '\n';
		LAdrDplyCnt=0;
	} else {
		LAdrDplyCnt++;
	}

	msgFile->WriteString(Temp);
}

CString CDisassembler::GenLabel(DWORD nAddress)
{
	CString Temp;

	Temp.Format("L%04lX", nAddress);
	return Temp;
}

BOOL CDisassembler::ScanEntries(CStdioFile *msgFile, CStdioFile *errFile)
{
	POSITION pos;
	DWORD dummy;
	BOOL RetVal = TRUE;
	int Count = m_EntryTable.GetCount();

	if (m_bSpitFlag) {
		m_PC = 0;		// In spit mode, we start at first address and just spit
		return FindCode(msgFile, errFile);
	}

	pos = m_EntryTable.GetStartPosition();
	while (RetVal && (pos!=NULL)) {
		m_EntryTable.GetNextAssoc(pos, m_PC, dummy);

		RetVal = RetVal && FindCode(msgFile, errFile);

		if (Count != m_EntryTable.GetCount()) {
			Count = m_EntryTable.GetCount();
			pos = m_EntryTable.GetStartPosition();		// Since hashes have unpredictable order, if one gets added during find process, we must start over!
		}
	}
	return RetVal;
}

BOOL CDisassembler::ScanBranches(CStdioFile *msgFile, CStdioFile *errFile)
{
	POSITION pos;
	CDWordArray* dummy;
	BOOL RetVal = TRUE;
	int Count = m_BranchTable.GetCount();

	if (m_bSpitFlag) return TRUE;						// Don't do anything in spit mode cause we did it in ScanEntries!

	pos = m_BranchTable.GetStartPosition();
	while (RetVal && (pos!=NULL)) {
		m_BranchTable.GetNextAssoc(pos, m_PC, dummy);

		RetVal = RetVal && FindCode(msgFile, errFile);

		if (Count != m_BranchTable.GetCount()) {
			Count = m_BranchTable.GetCount();
			pos = m_BranchTable.GetStartPosition();		// Since hashes have unpredictable order, if one gets added during find process, we must start over!
		}
	}
	return RetVal;
}

BOOL CDisassembler::ScanData(LPCTSTR szExcludeChars, CStdioFile *msgFile, CStdioFile *errFile)
{
	unsigned char c;

	ASSERT(szExcludeChars != NULL);		// Must supply exclude chars -- use "" if none are needed
	// Note that we use an argument passed in so that the caller has a chance to change the
	//	list passed in by the GetExcludedPrintChars function!

	for (m_PC=0; m_PC<m_Memory->GetMemSize(); m_PC++) {
		if (m_Memory->GetDescriptor(m_PC) != DMEM_LOADED) continue;	// Only modify memory locations that have been loaded but not processed!
		c = m_Memory->GetByte(m_PC);
		if (isprint(c) && (strchr(szExcludeChars, c) == NULL)) {
			m_Memory->SetDescriptor(m_PC, DMEM_PRINTDATA);
		} else {
			m_Memory->SetDescriptor(m_PC, DMEM_DATA);
		}
	}
	return TRUE;
}

BOOL CDisassembler::FindCode(CStdioFile *msgFile, CStdioFile *errFile)
{
	BOOL RetVal = TRUE;
	BOOL TFlag = FALSE;

	while (TFlag == FALSE) {
		if (m_PC >= m_Memory->GetMemSize()) {
			TFlag = TRUE;
			continue;
		}
		if ((m_Memory->GetDescriptor(m_PC) != DMEM_LOADED) && (m_bSpitFlag == FALSE)) {
			TFlag = TRUE;
			continue;					// Exit when we hit an area we've already looked at
		}
		if (ReadNextObj(TRUE, msgFile, errFile)) {		// Read next opcode and tag memory since we are finding code here
			if (((m_CurrentOpcode.m_Control & OCTL_STOP) != 0) && (m_bSpitFlag == FALSE)) TFlag = TRUE;
		}	// If the ReadNextObj returns false, that means we hit an illegal opcode byte.  The ReadNextObj will have incremented the m_PC past that byte, so we'll keep processing
	}

	return RetVal;
}

BOOL CDisassembler::ReadNextObj(BOOL bTagMemory, CStdioFile *msgFile, CStdioFile *errFile)
{
	// Here, we'll read the next object code from memory and flag the memory as being
	//	code, unless an illegal opcode is encountered whereby it will be flagged as
	//	illegal code.  m_OpMemory is cleared and loaded with the memory bytes for the
	//	opcode that was processed.  m_CurrentOpcode will be also be set to the opcode
	//	that corresponds to the one found.  m_PC will also be incremented by the number
	//	of bytes in the complete opcode and in the case of an illegal opcode, it will
	//	increment only by 1 byte so we can then test the following byte to see if it
	//	is legal to allow us to get back on track.  This function will return True
	//	if the opcode was correct and properly formed and fully existed in memory.  It
	//	will return false if the opcode was illegal or if the opcode crossed over the
	//	bounds of loaded source memory.
	// If bTagMemory is false, then memory descriptors aren't changed!  This is useful
	//	if on-the-fly disassembly is desired without modifying the descriptors, such
	//	as in pass 2 of the disassembly process.

	unsigned char FirstByte;
	DWORD SavedPC;
	BOOL Flag;
	POSITION pos;
	int i;

	m_sFunctionalOpcode = "";

	FirstByte = m_Memory->GetByte(m_PC++);
	m_OpMemory.RemoveAll();
	m_OpMemory.Add(FirstByte);
	if (IsAddressLoaded(m_PC-1, 1) == FALSE) return FALSE;

	pos = m_Opcodes.FindFirst(FirstByte);
	Flag = FALSE;
	while ((pos != NULL) && (Flag == FALSE)) {
		m_Opcodes.FindNext(pos, m_CurrentOpcode);
		Flag = TRUE;
		for (i=1; ((i<m_CurrentOpcode.m_OpcodeBytes.GetSize()) && (Flag)); i++) {
			if (m_CurrentOpcode.m_OpcodeBytes[i] != m_Memory->GetByte(m_PC+i-1)) Flag = FALSE;
		}
	}
	if ((Flag == FALSE) || (IsAddressLoaded(m_PC, m_CurrentOpcode.m_OpcodeBytes.GetSize()-1) == FALSE)) {
		if (bTagMemory) m_Memory->SetDescriptor(m_PC-1, DMEM_ILLEGALCODE);
		return FALSE;
	}

	// If we get here, then we've found a valid matching opcode.  m_CurrentOpcode has been set to the opcode value
	//	so we must now finish copying to m_OpMemory, tag the bytes in memory, increment m_PC, and call CompleteObjRead
	//	to handle the processor dependent stuff.  If CompleteObjRead returns FALSE, then we'll have to undo everything
	//	and return flagging an invalid opcode.
	SavedPC = m_PC;		// Remember m_PC in case we have to undo things (remember, m_PC has already been incremented by 1)
	for (i=1; i<m_CurrentOpcode.m_OpcodeBytes.GetSize(); i++) {			// Finish copying opcode, but don't tag memory until dependent code is called successfully
		m_OpMemory.Add(m_Memory->GetByte(m_PC++));
	}

	if ((CompleteObjRead(TRUE, msgFile, errFile) == FALSE) || (IsAddressLoaded(SavedPC-1, m_OpMemory.GetSize()) == FALSE))  {
		// Undo things here:
		m_OpMemory.RemoveAll();
		m_OpMemory.Add(FirstByte);		// Keep only the first byte in OpMemory for illegal opcode id
		m_PC = SavedPC;
		if (bTagMemory) m_Memory->SetDescriptor(m_PC-1, DMEM_ILLEGALCODE);
		return FALSE;
	}


//	ASSERT(CompleteObjRead(TRUE, msgFile, errFile));


	SavedPC--;
	for (i=0; i<m_OpMemory.GetSize(); i++) {		// CompleteObjRead will add bytes to OpMemory, so we simply have to flag memory for that many bytes.  m_PC is already incremented by CompleteObjRead
		if (bTagMemory) m_Memory->SetDescriptor(SavedPC, DMEM_CODE);
		SavedPC++;
	}
	ASSERT(SavedPC == m_PC);		// If these aren't equal, something is wrong in the CompleteObjRead!  m_PC should be incremented for every byte added to m_OpMemory by the complete routine!
	return TRUE;
}

BOOL CDisassembler::Disassemble(CStdioFile *msgFile, CStdioFile *errFile, CStdioFile *outFile)
{
	BOOL RetVal = TRUE;
	CStdioFile *theOutput;
	CStdioFile aOutput;
	CString Temp;

	if (outFile) {
		theOutput = outFile;
	} else {
		if (!aOutput.Open(m_sOutputFilename, CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::shareExclusive)) {
			if (errFile) {
				Temp.Format("\n*** Error: Opening file \"%s\" for writing...\n", m_sOutputFilename);
				errFile->WriteString(Temp);
			}
			return FALSE;
		}
		theOutput = &aOutput;
	}

	while (1) {		// Setup dummy endless loop so we can use the break
		if (msgFile) msgFile->WriteString("\nPass 1 - Finding Code, Data, and Labels...\n");
		RetVal = Pass1(*theOutput, msgFile, errFile);
		if (RetVal == FALSE) break;

		if (msgFile) {
			Temp.Format("%c\nPass 2 - Disassembling to Output File...\n", (LAdrDplyCnt != 0) ? '\n' : ' ');
			msgFile->WriteString(Temp);
		}
		RetVal = Pass2(*theOutput, msgFile, errFile);
		if (RetVal == FALSE) break;

		if (m_sFunctionsFilename.GetLength() != 0) {
			if (msgFile) {
				Temp.Format("%c\nPass 3 - Creating Functions Output File...\n", (LAdrDplyCnt != 0) ? '\n' : ' ');
				msgFile->WriteString(Temp);
			}

			RetVal = Pass3(*theOutput, msgFile, errFile);
			if (RetVal == FALSE) break;
		}

		if (msgFile) {
			msgFile->WriteString("\nDisassembly Complete\n");
		}

		// Add additional Passes here and end this loop with a break
		break;
	}

	if (RetVal == FALSE) {
		Temp = "\n*** Internal error encountered while disassembling.\n";
		theOutput->WriteString(Temp);
		if (errFile) errFile->WriteString(Temp);
	}

	if (outFile == NULL) aOutput.Close();	// Close the output file if we opened it

	return RetVal;
}

BOOL CDisassembler::Pass1(CStdioFile& outFile, CStdioFile *msgFile, CStdioFile *errFile)
{
	BOOL RetVal = TRUE;

	// Note that Short-Circuiting will keep following process stages from being called in the even of an error!
	RetVal = RetVal && ScanEntries(msgFile, errFile);
	RetVal = RetVal && ScanBranches(msgFile, errFile);
	RetVal = RetVal && ScanData(GetExcludedPrintChars(), msgFile, errFile);

	return RetVal;
}

BOOL CDisassembler::Pass2(CStdioFile& outFile, CStdioFile *msgFile, CStdioFile *errFile)
{
	BOOL RetVal = TRUE;

	// Note that Short-Circuiting will keep following process stages from being called in the even of an error!
	RetVal = RetVal && WriteHeader(outFile, msgFile, errFile);
	RetVal = RetVal && WriteEquates(outFile, msgFile, errFile);
	RetVal = RetVal && WriteDisassembly(outFile, msgFile, errFile);

	return RetVal;
}

BOOL CDisassembler::Pass3(CStdioFile& outFile, CStdioFile *msgFile, CStdioFile *errFile)
{
	BOOL RetVal = TRUE;
	CStdioFile aFunctionsFile;
	DWORD nFuncFlag;
	BOOL bInFunc;
	BOOL bLastFlag;
	BOOL bBranchOutFlag;
	CStringArray *pLabelList;
	int i;
	DWORD nFuncAddr;
	DWORD nSavedPC;
	CString strTemp;
	TMemRange *pMemRange;
	BOOL bTempFlag;
	BOOL bTempFlag2;

	if (!aFunctionsFile.Open(m_sFunctionsFilename, CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::shareExclusive)) {
		if (errFile) {
			strTemp.Format("\n*** Error: Opening file \"%s\" for writing...\n", m_sFunctionsFilename);
			errFile->WriteString(strTemp);
		}
		return FALSE;
	}

	aFunctionsFile.WriteString(";\n");
	aFunctionsFile.WriteString("; " + GetGDCLongName() + " Generated Function Information File\n");
	aFunctionsFile.WriteString(";\n");
	strTemp.Format(";    Control File%s ", (m_sControlFileList.GetSize() > 1) ? "s:" : ": ");
	for (i=0; i<m_sControlFileList.GetSize(); i++) {
		if (i==0) {
			strTemp += m_sControlFileList[i] + "\n";
		} else {
			strTemp = ";                  " + m_sControlFileList[i] + "\n";
		}
		aFunctionsFile.WriteString(strTemp);
	}
	strTemp.Format(";      Input File%s ", (m_sInputFileList.GetSize() > 1) ? "s:" : ": ");
	for (i=0; i<m_sInputFileList.GetSize(); i++) {
		if (i==0) {
			strTemp += m_sInputFileList[i] + "\n";
		} else {
			strTemp = ";                  " + m_sInputFileList[i] + "\n";
		}
		aFunctionsFile.WriteString(strTemp);
	}
	aFunctionsFile.WriteString(";     Output File:  " + m_sOutputFilename + "\n");
	aFunctionsFile.WriteString(";  Functions File:  " + m_sFunctionsFilename + "\n");
	aFunctionsFile.WriteString(";\n");

	aFunctionsFile.WriteString("; Memory Mappings:");

	aFunctionsFile.WriteString("  ROM Memory Map: ");
	if (m_ROMMemMap.IsNullRange()) {
		aFunctionsFile.WriteString("<Not Defined>\n");
	} else {
		aFunctionsFile.WriteString("\n");

		pMemRange = &m_ROMMemMap;
		while (pMemRange) {
			strTemp.Format(";                       %s%04lX - %s%04lX  (Size: %s%04lX)\n",
						LPCTSTR(GetHexDelim()), pMemRange->GetStartAddr(),
						LPCTSTR(GetHexDelim()), pMemRange->GetStartAddr() + pMemRange->GetSize() - 1,
						LPCTSTR(GetHexDelim()), pMemRange->GetSize());
			aFunctionsFile.WriteString(strTemp);
			pMemRange = pMemRange->GetNext();
		}
	}

	aFunctionsFile.WriteString(";                   RAM Memory Map: ");
	if (m_RAMMemMap.IsNullRange()) {
		aFunctionsFile.WriteString("<Not Defined>\n");
	} else {
		aFunctionsFile.WriteString("\n");

		pMemRange = &m_RAMMemMap;
		while (pMemRange) {
			strTemp.Format(";                       %s%04lX - %s%04lX  (Size: %s%04lX)\n",
						LPCTSTR(GetHexDelim()), pMemRange->GetStartAddr(),
						LPCTSTR(GetHexDelim()), pMemRange->GetStartAddr() + pMemRange->GetSize() - 1,
						LPCTSTR(GetHexDelim()), pMemRange->GetSize());
			aFunctionsFile.WriteString(strTemp);
			pMemRange = pMemRange->GetNext();
		}
	}

	aFunctionsFile.WriteString(";                    IO Memory Map: ");
	if (m_IOMemMap.IsNullRange()) {
		aFunctionsFile.WriteString("<Not Defined>\n");
	} else {
		aFunctionsFile.WriteString("\n");

		pMemRange = &m_IOMemMap;
		while (pMemRange) {
			strTemp.Format(";                       %s%04lX - %s%04lX  (Size: %s%04lX)\n",
						LPCTSTR(GetHexDelim()), pMemRange->GetStartAddr(),
						LPCTSTR(GetHexDelim()), pMemRange->GetStartAddr() + pMemRange->GetSize() - 1,
						LPCTSTR(GetHexDelim()), pMemRange->GetSize());
			aFunctionsFile.WriteString(strTemp);
			pMemRange = pMemRange->GetNext();
		}
	}

	aFunctionsFile.WriteString(";\n");
	aFunctionsFile.WriteString(";       Generated:  " + m_StartTime.Format("%#c %Z") + "\n");
	aFunctionsFile.WriteString(";\n\n");

	bTempFlag = FALSE;
	if (!m_ROMMemMap.IsNullRange()) {
		pMemRange = &m_ROMMemMap;
		while (pMemRange) {
			m_sFunctionalOpcode.Format("#ROM|%04lX|%04lX\n", pMemRange->GetStartAddr(), pMemRange->GetSize());
			aFunctionsFile.WriteString(m_sFunctionalOpcode);
			pMemRange = pMemRange->GetNext();
		}
		bTempFlag = TRUE;
	}
	if (!m_RAMMemMap.IsNullRange()) {
		pMemRange = &m_RAMMemMap;
		while (pMemRange) {
			m_sFunctionalOpcode.Format("#RAM|%04lX|%04lX\n", pMemRange->GetStartAddr(), pMemRange->GetSize());
			aFunctionsFile.WriteString(m_sFunctionalOpcode);
			pMemRange = pMemRange->GetNext();
		}
		bTempFlag = TRUE;
	}
	if (!m_IOMemMap.IsNullRange()) {
		pMemRange = &m_IOMemMap;
		while (pMemRange) {
			m_sFunctionalOpcode.Format("#IO|%04lX|%04lX\n", pMemRange->GetStartAddr(), pMemRange->GetSize());
			aFunctionsFile.WriteString(m_sFunctionalOpcode);
			pMemRange = pMemRange->GetNext();
		}
		bTempFlag = TRUE;
	}
	if (bTempFlag) aFunctionsFile.WriteString("\n");

	bTempFlag = FALSE;
	for (m_PC = 0; ((m_PC < m_Memory->GetMemSize()) && (RetVal)); m_PC++) {
		if (m_LabelTable.Lookup(m_PC, pLabelList)) {
			ASSERT(pLabelList != NULL);			// Shouldn't have null list!
			if (pLabelList == NULL) {
				RetVal = FALSE;
				continue;
			}

			bTempFlag2 = FALSE;
			m_sFunctionalOpcode.Format("!%04lX|", m_PC);
			for (i=0; i<pLabelList->GetSize(); i++) {
				if (bTempFlag2) m_sFunctionalOpcode += ",";
				if (!pLabelList->GetAt(i).IsEmpty()) {
					m_sFunctionalOpcode += pLabelList->GetAt(i);
					bTempFlag2 = TRUE;
				}
			}
			m_sFunctionalOpcode += "\n";
			if (bTempFlag2) aFunctionsFile.WriteString(m_sFunctionalOpcode);
			bTempFlag = TRUE;
		}
	}
	if (bTempFlag) aFunctionsFile.WriteString("\n");

	bInFunc = FALSE;
	for (m_PC = 0; ((m_PC < m_Memory->GetMemSize()) && (RetVal)); ) {
		bLastFlag = FALSE;
		bBranchOutFlag = FALSE;

		// Check for function start/end flags:
		if (m_FunctionsTable.Lookup(m_PC, nFuncFlag)) {
			switch (nFuncFlag) {
				case FUNCF_HARDSTOP:
				case FUNCF_EXITBRANCH:
				case FUNCF_SOFTSTOP:
					bInFunc = FALSE;
					bLastFlag = TRUE;
					break;

				case FUNCF_BRANCHOUT:
					bBranchOutFlag = TRUE;
					break;

				case FUNCF_BRANCHIN:
					if (bInFunc) break;		// Continue if already inside a function
					// Else, fall-through and setup for new function:
				case FUNCF_ENTRY:
				case FUNCF_INDIRECT:
				case FUNCF_CALL:
					m_sFunctionalOpcode.Format("@%04lX|", m_PC);
					if (m_LabelTable.Lookup(m_PC, pLabelList)) {
						ASSERT(pLabelList != NULL);			// Shouldn't have null list!
						if (pLabelList == NULL) {
							RetVal = FALSE;
							continue;
						}
						for (i=0; i<pLabelList->GetSize(); i++) {
							if (i != 0) m_sFunctionalOpcode += ",";
							m_sFunctionalOpcode += FormatLabel(LC_REF, pLabelList->GetAt(i), m_PC);
						}
					} else {
						m_sFunctionalOpcode += "???";
					}

					aFunctionsFile.WriteString(m_sFunctionalOpcode);
					aFunctionsFile.WriteString("\n");

					nFuncAddr = m_PC;
					bInFunc = TRUE;
					break;

				default:
					ASSERT(FALSE);			// Unexpected Function Flag!!  Check Code!!
					RetVal = FALSE;
					continue;
			}
		}

		nSavedPC = m_PC;
		m_sFunctionalOpcode = "";
		switch (m_Memory->GetDescriptor(m_PC)) {
			case DMEM_NOTLOADED:
				m_PC++;
				bLastFlag = bInFunc;
				bInFunc = FALSE;
				break;
			case DMEM_LOADED:
				ASSERT(FALSE);		// WARNING!  All loaded code should have been evaluated.  Check override code!
				RetVal = FALSE;
				m_PC++;
				break;
			case DMEM_DATA:
			case DMEM_PRINTDATA:
			case DMEM_CODEINDIRECT:
			case DMEM_DATAINDIRECT:
			case DMEM_ILLEGALCODE:
				m_sFunctionalOpcode.Format("%04lX|", m_PC);

				if (m_LabelTable.Lookup(m_PC, pLabelList)) {
					ASSERT(pLabelList != NULL);			// Shouldn't have null list!
					if (pLabelList == NULL) return FALSE;
					for (i=0; i<pLabelList->GetSize(); i++) {
						if (i != 0) m_sFunctionalOpcode += ",";
						m_sFunctionalOpcode += FormatLabel(LC_REF, pLabelList->GetAt(i), m_PC);
					}
				}

				strTemp.Format("%02lX|", (unsigned long)(m_Memory->GetByte(m_PC)));
				m_sFunctionalOpcode += strTemp;

				m_PC++;
				break;
			case DMEM_CODE:
				RetVal = ReadNextObj(FALSE, msgFile, errFile);
				break;
			default:
				bInFunc = FALSE;
				RetVal = FALSE;
				break;
		}

		if (m_sFunctionalOpcode.GetLength() != 0) {
			strTemp.Format("%04lX|", (nSavedPC - nFuncAddr));
			m_sFunctionalOpcode = strTemp + m_sFunctionalOpcode;
		}

		if (bBranchOutFlag) {
			if (m_FunctionsTable.Lookup(m_PC, nFuncFlag)) {
				switch (nFuncFlag) {
					case FUNCF_ENTRY:
					case FUNCF_INDIRECT:
					case FUNCF_CALL:
						bInFunc = FALSE;
						bLastFlag = TRUE;
						break;
				}
			} else {
				bInFunc = FALSE;
				bLastFlag = TRUE;
			}
		}

		if (bLastFlag) {
			if (m_FunctionsTable.Lookup(m_PC, nFuncFlag)) {
				switch (nFuncFlag) {
					case FUNCF_BRANCHIN:
						bLastFlag = FALSE;
						bInFunc = TRUE;
						break;
				}
			}
		}

		if ((bInFunc) || (bLastFlag)) {
			if (m_sFunctionalOpcode.GetLength() != 0) {
				aFunctionsFile.WriteString(m_sFunctionalOpcode);
				aFunctionsFile.WriteString("\n");
			}
			if (bLastFlag) aFunctionsFile.WriteString("\n");
		}
	}

	aFunctionsFile.Close();

	return RetVal;
}

CString CDisassembler::FormatComments(MNEMONIC_CODE nMCCode)		// The default returns the reference comment string.
{
	return FormatReferences(m_PC - m_OpMemory.GetSize());
}

CString CDisassembler::FormatAddress(DWORD nAddress)
{
	CString Temp;

	Temp.Format("%04lX", nAddress);
	return Temp;
}

CString CDisassembler::FormatOpBytes()
{
	CString Temp, Temp2;
	int i;
	
	Temp = "";
	for (i=0; i<m_OpMemory.GetSize(); i++) {
		Temp2.Format("%02X", m_OpMemory.GetAt(i));
		Temp += Temp2;
		if (i < m_OpMemory.GetSize()-1) Temp += " ";
	}
	return Temp;
}

CString CDisassembler::FormatLabel(LABEL_CODE nLC, LPCTSTR szLabel, DWORD nAddress)
{
	CString Temp;

	if (szLabel == NULL) {
		Temp = "";
	} else {
		Temp = szLabel;
	}

	if (Temp.GetLength() == 0) {
		Temp.Format("L%04lX", nAddress);
	}

	return Temp;
}

CString CDisassembler::FormatReferences(DWORD nAddress)
{
	CString RetVal;
	CString Temp;
	CStringArray *pLabelList;
	CDWordArray *pRefList;
	int i;
	BOOL Flag;

	RetVal = "";
	Flag = FALSE;
	if (m_BranchTable.Lookup(nAddress, pRefList)) {
		ASSERT(pRefList != NULL);			// Shouldn't have null list!
		if (pRefList == NULL) return RetVal;

		if (pRefList->GetSize() != 0) {
			RetVal += "CRef: ";
			Flag = TRUE;
			for (i=0; i<pRefList->GetSize(); i++) {
				Temp.Format("%s%04lX", LPCTSTR(GetHexDelim()), pRefList->GetAt(i));
				RetVal += Temp;
				if (i < pRefList->GetSize()-1) RetVal += ",";
			}
		}
	}

	if (m_LabelTable.Lookup(nAddress, pLabelList)) {
		ASSERT(pLabelList != NULL);			// Shouldn't have null list!
		if (pLabelList == NULL) return RetVal;

		if (m_LabelRefTable.Lookup(nAddress, pRefList) == FALSE) {
			ASSERT(FALSE);		// Should also have a ref entry!
			return RetVal;
		}
		ASSERT(pRefList != NULL);			// Shouldn't have a null list!
		if (pRefList == NULL) return RetVal;

		if (pRefList->GetSize() != 0) {
			if (Flag) RetVal += "; ";
			RetVal += "DRef: ";
			Flag = TRUE;
			for (i=0; i<pRefList->GetSize(); i++) {
				Temp.Format("%s%04lX", LPCTSTR(GetHexDelim()), pRefList->GetAt(i));
				RetVal += Temp;
				if (i < pRefList->GetSize()-1) RetVal += ",";
			}
		}
	}

	return RetVal;
}


BOOL CDisassembler::WriteHeader(CStdioFile& outFile, CStdioFile *msgFile, CStdioFile *errFile)
{
	CStringArray OutLine;
	int i;
	CString Temp;
	TMemRange *pMemRange;

	for (i=0; i<NUM_FIELD_CODES; i++) OutLine.Add("");
	OutLine[FC_ADDRESS] = FormatAddress(0);

	OutLine[FC_LABEL].Format("%s%s\n", LPCTSTR(GetCommentStartDelim()), LPCTSTR(GetCommentEndDelim()));
	outFile.WriteString(MakeOutputLine(OutLine));
	outFile.WriteString("\n");
	OutLine[FC_LABEL].Format("%s %s Generated Source Code %s\n", LPCTSTR(GetCommentStartDelim()), LPCTSTR(GetGDCLongName()), LPCTSTR(GetCommentEndDelim()));
	outFile.WriteString(MakeOutputLine(OutLine));
	outFile.WriteString("\n");
	OutLine[FC_LABEL].Format("%s%s\n", LPCTSTR(GetCommentStartDelim()), LPCTSTR(GetCommentEndDelim()));
	outFile.WriteString(MakeOutputLine(OutLine));
	outFile.WriteString("\n");
	OutLine[FC_LABEL].Format("%s    Control File%s ", LPCTSTR(GetCommentStartDelim()), (m_sControlFileList.GetSize() > 1) ? "s:" : ": ");
	for (i=0; i<m_sControlFileList.GetSize(); i++) {
		if (i==0) {
			Temp.Format("%s  %s\n", LPCTSTR(m_sControlFileList[i]), LPCTSTR(GetCommentEndDelim()));
			OutLine[FC_LABEL] += Temp;
		} else {
			OutLine[FC_LABEL].Format("%s                  %s  %s\n", LPCTSTR(GetCommentStartDelim()), LPCTSTR(m_sControlFileList[i]), LPCTSTR(GetCommentEndDelim()));
		}
		outFile.WriteString(MakeOutputLine(OutLine));
		outFile.WriteString("\n");
	}
	OutLine[FC_LABEL].Format("%s      Input File%s ", LPCTSTR(GetCommentStartDelim()), (m_sInputFileList.GetSize() > 1) ? "s:" : ": ");
	for (i=0; i<m_sInputFileList.GetSize(); i++) {
		if (i==0) {
			Temp.Format("%s  %s\n", LPCTSTR(m_sInputFileList[i]), LPCTSTR(GetCommentEndDelim()));
			OutLine[FC_LABEL] += Temp;
		} else {
			OutLine[FC_LABEL].Format("%s                  %s  %s\n", LPCTSTR(GetCommentStartDelim()), LPCTSTR(m_sInputFileList[i]), LPCTSTR(GetCommentEndDelim()));
		}
		outFile.WriteString(MakeOutputLine(OutLine));
		outFile.WriteString("\n");
	}
	OutLine[FC_LABEL].Format("%s     Output File:  %s  %s\n", LPCTSTR(GetCommentStartDelim()), LPCTSTR(m_sOutputFilename), LPCTSTR(GetCommentEndDelim()));
	outFile.WriteString(MakeOutputLine(OutLine));
	outFile.WriteString("\n");
	if (m_sFunctionsFilename.GetLength()) {
		OutLine[FC_LABEL].Format("%s  Functions File:  %s  %s\n", LPCTSTR(GetCommentStartDelim()), LPCTSTR(m_sFunctionsFilename), LPCTSTR(GetCommentEndDelim()));
		outFile.WriteString(MakeOutputLine(OutLine));
		outFile.WriteString("\n");
	}
	OutLine[FC_LABEL].Format("%s%s\n", LPCTSTR(GetCommentStartDelim()), LPCTSTR(GetCommentEndDelim()));
	outFile.WriteString(MakeOutputLine(OutLine));
	outFile.WriteString("\n");

	OutLine[FC_LABEL] = GetCommentStartDelim() + " Memory Mappings:";

	OutLine[FC_LABEL] += "  ROM Memory Map: ";
	if (m_ROMMemMap.IsNullRange()) {
		OutLine[FC_LABEL] += "<Not Defined>" + GetCommentEndDelim() + "\n";
		outFile.WriteString(MakeOutputLine(OutLine));
		outFile.WriteString("\n");
	} else {
		OutLine[FC_LABEL] += GetCommentEndDelim() + "\n";
		outFile.WriteString(MakeOutputLine(OutLine));
		outFile.WriteString("\n");

		pMemRange = &m_ROMMemMap;
		while (pMemRange) {
			OutLine[FC_LABEL].Format("%s                       %s%04lX - %s%04lX  (Size: %s%04lX)  %s\n",
						LPCTSTR(GetCommentStartDelim()),
						LPCTSTR(GetHexDelim()), pMemRange->GetStartAddr(),
						LPCTSTR(GetHexDelim()), pMemRange->GetStartAddr() + pMemRange->GetSize() - 1,
						LPCTSTR(GetHexDelim()), pMemRange->GetSize(),
						LPCTSTR(GetCommentEndDelim()));
			outFile.WriteString(MakeOutputLine(OutLine));
			outFile.WriteString("\n");
			pMemRange = pMemRange->GetNext();
		}
	}

	OutLine[FC_LABEL] = GetCommentStartDelim() + "                   RAM Memory Map: ";
	if (m_RAMMemMap.IsNullRange()) {
		OutLine[FC_LABEL] += "<Not Defined>" + GetCommentEndDelim() + "\n";
		outFile.WriteString(MakeOutputLine(OutLine));
		outFile.WriteString("\n");
	} else {
		OutLine[FC_LABEL] += GetCommentEndDelim() + "\n";
		outFile.WriteString(MakeOutputLine(OutLine));
		outFile.WriteString("\n");

		pMemRange = &m_RAMMemMap;
		while (pMemRange) {
			OutLine[FC_LABEL].Format("%s                       %s%04lX - %s%04lX  (Size: %s%04lX)  %s\n",
						LPCTSTR(GetCommentStartDelim()),
						LPCTSTR(GetHexDelim()), pMemRange->GetStartAddr(),
						LPCTSTR(GetHexDelim()), pMemRange->GetStartAddr() + pMemRange->GetSize() - 1,
						LPCTSTR(GetHexDelim()), pMemRange->GetSize(),
						LPCTSTR(GetCommentEndDelim()));
			outFile.WriteString(MakeOutputLine(OutLine));
			outFile.WriteString("\n");
			pMemRange = pMemRange->GetNext();
		}
	}

	OutLine[FC_LABEL] = GetCommentStartDelim() + "                    IO Memory Map: ";
	if (m_IOMemMap.IsNullRange()) {
		OutLine[FC_LABEL] += "<Not Defined>" + GetCommentEndDelim() + "\n";
		outFile.WriteString(MakeOutputLine(OutLine));
		outFile.WriteString("\n");
	} else {
		OutLine[FC_LABEL] += GetCommentEndDelim() + "\n";
		outFile.WriteString(MakeOutputLine(OutLine));
		outFile.WriteString("\n");

		pMemRange = &m_IOMemMap;
		while (pMemRange) {
			OutLine[FC_LABEL].Format("%s                       %s%04lX - %s%04lX  (Size: %s%04lX)  %s\n",
						LPCTSTR(GetCommentStartDelim()),
						LPCTSTR(GetHexDelim()), pMemRange->GetStartAddr(),
						LPCTSTR(GetHexDelim()), pMemRange->GetStartAddr() + pMemRange->GetSize() - 1,
						LPCTSTR(GetHexDelim()), pMemRange->GetSize(),
						LPCTSTR(GetCommentEndDelim()));
			outFile.WriteString(MakeOutputLine(OutLine));
			outFile.WriteString("\n");
			pMemRange = pMemRange->GetNext();
		}
	}

	OutLine[FC_LABEL].Format("%s%s\n", LPCTSTR(GetCommentStartDelim()), LPCTSTR(GetCommentEndDelim()));
	outFile.WriteString(MakeOutputLine(OutLine));
	outFile.WriteString("\n");
	OutLine[FC_LABEL] = GetCommentStartDelim() + m_StartTime.Format("       Generated:  %#c %Z") + GetCommentEndDelim() + "\n";
	outFile.WriteString(MakeOutputLine(OutLine));
	outFile.WriteString("\n");
	OutLine[FC_LABEL].Format("%s%s\n", LPCTSTR(GetCommentStartDelim()), LPCTSTR(GetCommentEndDelim()));
	outFile.WriteString(MakeOutputLine(OutLine));
	outFile.WriteString("\n");


	OutLine[FC_LABEL] = "";
	outFile.WriteString(MakeOutputLine(OutLine));
	outFile.WriteString("\n");
	outFile.WriteString(MakeOutputLine(OutLine));
	outFile.WriteString("\n");
	return TRUE;
}

BOOL CDisassembler::WritePreEquates(CStdioFile& outFile, CStdioFile *msgFile, CStdioFile *errFile)		// Don't do anything by default.
{
	return TRUE;
}

BOOL CDisassembler::WriteEquates(CStdioFile& outFile, CStdioFile *msgFile, CStdioFile *errFile)
{
	CStringArray OutLine;
	CStringArray *pLabelList;
	int i;
	BOOL RetVal;

	m_OpMemory.RemoveAll();			// Remove bytes so all code will properly reference first and last PC addresses

	RetVal = FALSE;
	if (WritePreEquates(outFile, msgFile, errFile)) {
		RetVal = TRUE;
		for (i=0; i<NUM_FIELD_CODES; i++) OutLine.Add("");
		for (m_PC = 0; m_PC < m_Memory->GetMemSize(); m_PC++) {
			ASSERT(m_Memory->GetDescriptor(m_PC) != DMEM_LOADED);		// Find Routines didn't find and analyze all memory!!  Fix it!
			if (m_Memory->GetDescriptor(m_PC) != DMEM_NOTLOADED) continue;		// Loaded addresses will get outputted during main part

			if (m_LabelTable.Lookup(m_PC, pLabelList)) {
				ASSERT(pLabelList != NULL);			// Shouldn't have null list!
				if (pLabelList == NULL) continue;

				OutLine[FC_ADDRESS] = FormatAddress(m_PC);
				for (i=0; i<pLabelList->GetSize(); i++) {
					OutLine[FC_LABEL] = FormatLabel(LC_EQUATE, pLabelList->GetAt(i), m_PC);
					OutLine[FC_MNEMONIC] = FormatMnemonic(MC_EQUATE);
					OutLine[FC_OPERANDS] = FormatOperands(MC_EQUATE);
					OutLine[FC_COMMENT] = FormatComments(MC_EQUATE);
					outFile.WriteString(MakeOutputLine(OutLine));
					outFile.WriteString("\n");
				}
			}
		}
		RetVal = RetVal && WritePostEquates(outFile, msgFile, errFile);
	}
	return RetVal;
}

BOOL CDisassembler::WritePostEquates(CStdioFile& outFile, CStdioFile *msgFile, CStdioFile *errFile)		// Don't do anything by default
{
	return TRUE;
}

BOOL CDisassembler::WritePreDisassembly(CStdioFile& outFile, CStdioFile *msgFile, CStdioFile *errFile)
{
	CStringArray OutLine;
	int i;

	for (i=0; i<NUM_FIELD_CODES; i++) OutLine.Add("");
	OutLine[FC_ADDRESS] = FormatAddress(0);

	outFile.WriteString(MakeOutputLine(OutLine));
	outFile.WriteString("\n");
	outFile.WriteString(MakeOutputLine(OutLine));
	outFile.WriteString("\n");
	return TRUE;
}

BOOL CDisassembler::WriteDisassembly(CStdioFile& outFile, CStdioFile *msgFile, CStdioFile *errFile)
{
	BOOL RetVal;

	RetVal = FALSE;
	if (WritePreDisassembly(outFile, msgFile, errFile)) {
		RetVal = TRUE;

		for (m_PC = 0; ((m_PC < m_Memory->GetMemSize()) && (RetVal)); ) {	// WARNING!! WriteSection MUST increment m_PC
			switch (m_Memory->GetDescriptor(m_PC)) {
				case DMEM_NOTLOADED:
					m_PC++;
					break;
				case DMEM_LOADED:
					ASSERT(FALSE);		// WARNING!  All loaded code should have been evaluated.  Check override code!
					RetVal = FALSE;
					m_PC++;
					break;
				case DMEM_DATA:
				case DMEM_PRINTDATA:
				case DMEM_CODEINDIRECT:
				case DMEM_DATAINDIRECT:
				case DMEM_CODE:
				case DMEM_ILLEGALCODE:
					RetVal = RetVal && WriteSection(outFile, msgFile, errFile);			// WARNING!! WriteSection MUST properly increment m_PC!!
					break;
			}
		}

		RetVal = RetVal && WritePostDisassembly(outFile, msgFile, errFile);
	}

	return RetVal;
}

BOOL CDisassembler::WritePostDisassembly(CStdioFile& outFile, CStdioFile *msgFile, CStdioFile *errFile)		// Don't do anything by default
{
	return TRUE;
}

BOOL CDisassembler::WritePreSection(CStdioFile& outFile, CStdioFile *msgFile, CStdioFile *errFile)		// Don't do anything by default
{
	return TRUE;
}

BOOL CDisassembler::WriteSection(CStdioFile& outFile, CStdioFile *msgFile, CStdioFile *errFile)
{
	BOOL RetVal;
	BOOL Done;

	RetVal = FALSE;
	if (WritePreSection(outFile, msgFile, errFile)) {
		RetVal = TRUE;

		Done = FALSE;
		while ((m_PC < m_Memory->GetMemSize()) && (Done == FALSE) && (RetVal)) {
			// WARNING!! Write Data and Code section functions MUST properly increment m_PC!!
			switch (m_Memory->GetDescriptor(m_PC)) {
				case DMEM_DATA:
				case DMEM_PRINTDATA:
				case DMEM_CODEINDIRECT:
				case DMEM_DATAINDIRECT:
					RetVal = RetVal && WriteDataSection(outFile, msgFile, errFile);
					break;
				case DMEM_CODE:
				case DMEM_ILLEGALCODE:
					RetVal = RetVal && WriteCodeSection(outFile, msgFile, errFile);
					break;
				default:
					Done = TRUE;
					break;
			}
		}

		RetVal = RetVal && WritePostSection(outFile, msgFile, errFile);
	}

	return RetVal;
}

BOOL CDisassembler::WritePostSection(CStdioFile& outFile, CStdioFile *msgFile, CStdioFile *errFile)		// Don't do anything by default
{
	return TRUE;
}

BOOL CDisassembler::WritePreDataSection(CStdioFile& outFile, CStdioFile *msgFile, CStdioFile *errFile)		// Don't do anything by default
{
	return TRUE;
}

BOOL CDisassembler::WriteDataSection(CStdioFile& outFile, CStdioFile *msgFile, CStdioFile *errFile)
{
	BOOL RetVal;
	CStringArray OutLine;
	CStringArray *pLabelList;
	int i;
	BOOL Done;
	DWORD SavedPC;
	int Count;
	BOOL Flag;
	MEM_DESC Code;
	CString TempLabel;
	CByteArray TempOpMemory;

	for (i=0; i<NUM_FIELD_CODES; i++) OutLine.Add("");

	RetVal = FALSE;
	if (WritePreDataSection(outFile, msgFile, errFile)) {
		RetVal = TRUE;

		Done = FALSE;
		while ((m_PC < m_Memory->GetMemSize()) && (Done == FALSE) && (RetVal)) {
			for (i=0; i<NUM_FIELD_CODES; i++) OutLine.SetAt(i, "");
			OutLine[FC_ADDRESS] = FormatAddress(m_PC);
			if (m_LabelTable.Lookup(m_PC, pLabelList)) {
				ASSERT(pLabelList != NULL);			// Shouldn't have null list!
				if (pLabelList == NULL) {
					RetVal = FALSE;
					Done = TRUE;
					continue;
				}
				for (i=1; i<pLabelList->GetSize(); i++) {
					OutLine[FC_LABEL] = FormatLabel(LC_DATA, pLabelList->GetAt(i), m_PC);
					outFile.WriteString(MakeOutputLine(OutLine));
					outFile.WriteString("\n");
				}
				if (pLabelList->GetSize()) OutLine[FC_LABEL] = FormatLabel(LC_DATA, pLabelList->GetAt(0), m_PC);
			}

			SavedPC = m_PC;		// Keep a copy of the PC for this line because some calls will be incrementing our m_PC
			Code = (MEM_DESC)m_Memory->GetDescriptor(m_PC);
			if ((m_bAsciiFlag == FALSE) && (Code == DMEM_PRINTDATA)) Code = DMEM_DATA;		// If not doing ASCII, treat print data as data
			switch (Code) {
				case DMEM_DATA:
					Count = 0;
					m_OpMemory.RemoveAll();
					Flag = FALSE;
					while (Flag == FALSE) {
						m_OpMemory.Add(m_Memory->GetByte(m_PC++));
						Count++;
						// Stop on this line when we've either run out of data, hit the specified line limit, or hit another label
						if (Count >= m_nMaxNonPrint) Flag = TRUE;
						if (m_LabelTable.Lookup(m_PC, pLabelList)) Flag = TRUE;
						Code = (MEM_DESC)m_Memory->GetDescriptor(m_PC);
						if ((m_bAsciiFlag == FALSE) && (Code == DMEM_PRINTDATA)) Code = DMEM_DATA;		// If not doing ASCII, treat print data as data
						if (Code != DMEM_DATA) Flag = TRUE;
					}
					if (m_bDataOpBytesFlag) OutLine[FC_OPBYTES] = FormatOpBytes();
					OutLine[FC_MNEMONIC] = FormatMnemonic(MC_DATABYTE);
					OutLine[FC_OPERANDS] = FormatOperands(MC_DATABYTE);
					OutLine[FC_COMMENT] = FormatComments(MC_DATABYTE);
					outFile.WriteString(MakeOutputLine(OutLine));
					outFile.WriteString("\n");
					break;
				case DMEM_PRINTDATA:
					Count = 0;
					m_OpMemory.RemoveAll();
					Flag = FALSE;
					while (Flag == FALSE) {
						m_OpMemory.Add(m_Memory->GetByte(m_PC++));
						Count++;
						// Stop on this line when we've either run out of data, hit the specified line limit, or hit another label
						if (Count >= m_nMaxPrint) Flag = TRUE;
						if (m_LabelTable.Lookup(m_PC, pLabelList)) Flag = TRUE;
						if (m_Memory->GetDescriptor(m_PC) != DMEM_PRINTDATA) Flag = TRUE;
					}
					// First, print a line of the output bytes for reference:
					if (m_bAsciiBytesFlag) {
						TempLabel = OutLine[FC_LABEL];
						TempOpMemory.RemoveAll();
						for (i=0; i<m_OpMemory.GetSize(); i++) TempOpMemory.Add(m_OpMemory[i]);

						Count = TempOpMemory.GetSize();
						while (Count) {
							m_OpMemory.RemoveAll();
							for (i=0; ((i<Count) && (i<m_nMaxNonPrint)); i++)
								m_OpMemory.Add(TempOpMemory[TempOpMemory.GetSize()-Count+i]);
							OutLine[FC_LABEL] = GetCommentStartDelim() + " " + TempLabel +
												((TempLabel.GetLength() != 0) ? " " : "") +
												FormatOperands(MC_DATABYTE) + " " + GetCommentEndDelim();
							OutLine[FC_ADDRESS] = FormatAddress(m_PC-Count);
							OutLine[FC_MNEMONIC] = "";
							OutLine[FC_OPERANDS] = "";
							OutLine[FC_COMMENT] = "";
							outFile.WriteString(MakeOutputLine(OutLine));
							outFile.WriteString("\n");
							Count -= m_OpMemory.GetSize();
						}

						OutLine[FC_LABEL] = TempLabel;
						m_OpMemory.RemoveAll();
						for (i=0; i<TempOpMemory.GetSize(); i++) m_OpMemory.Add(TempOpMemory[i]);
					}

					// Then, print the line as it should be in the ASCII equivalent:
					OutLine[FC_ADDRESS] = FormatAddress(SavedPC);
					if (m_bDataOpBytesFlag) OutLine[FC_OPBYTES] = FormatOpBytes();
					OutLine[FC_MNEMONIC] = FormatMnemonic(MC_ASCII);
					OutLine[FC_OPERANDS] = FormatOperands(MC_ASCII);
					OutLine[FC_COMMENT] = FormatComments(MC_ASCII);
					outFile.WriteString(MakeOutputLine(OutLine));
					outFile.WriteString("\n");
					break;
				case DMEM_CODEINDIRECT:
				case DMEM_DATAINDIRECT:
					// If the following call returns false, that means that the user (or
					//	special indirect detection logic) erroneously specified (or detected)
					//	the indirect.  So instead of throwing an error or causing problems,
					//	we will treat it as a data byte instead:
					if (RetrieveIndirect(msgFile, errFile) == FALSE) {		// Bumps PC
						if (m_bDataOpBytesFlag) OutLine[FC_OPBYTES] = FormatOpBytes();
						OutLine[FC_MNEMONIC] = FormatMnemonic(MC_DATABYTE);
						OutLine[FC_OPERANDS] = FormatOperands(MC_DATABYTE);
						OutLine[FC_COMMENT] = FormatComments(MC_DATABYTE) + " -- Erroneous Indirect Stub";
						outFile.WriteString(MakeOutputLine(OutLine));
						outFile.WriteString("\n");
						break;
					}

					if (m_bDataOpBytesFlag) OutLine[FC_OPBYTES] = FormatOpBytes();
					OutLine[FC_MNEMONIC] = FormatMnemonic(MC_INDIRECT);
					OutLine[FC_OPERANDS] = FormatOperands(MC_INDIRECT);
					OutLine[FC_COMMENT] = FormatComments(MC_INDIRECT);
					outFile.WriteString(MakeOutputLine(OutLine));
					outFile.WriteString("\n");
					break;

				default:
					Done = TRUE;
			}
		}

		RetVal = RetVal && WritePostDataSection(outFile, msgFile, errFile);
	}

	return RetVal;
}

BOOL CDisassembler::WritePostDataSection(CStdioFile& outFile, CStdioFile *msgFile, CStdioFile *errFile)		// Don't do anything by default
{
	return TRUE;
}

BOOL CDisassembler::WritePreCodeSection(CStdioFile& outFile, CStdioFile *msgFile, CStdioFile *errFile)		// Don't do anything by default
{
	return TRUE;
}

BOOL CDisassembler::WriteCodeSection(CStdioFile& outFile, CStdioFile *msgFile, CStdioFile *errFile)
{
	BOOL RetVal;
	CStringArray OutLine;
	CStringArray *pLabelList;
	int i;
	BOOL Done;
	DWORD SavedPC;
	BOOL bInFunc;
	BOOL bLastFlag;
	BOOL bBranchOutFlag;
	DWORD nFuncFlag;

	for (i=0; i<NUM_FIELD_CODES; i++) OutLine.Add("");

	RetVal = FALSE;
	if (WritePreCodeSection(outFile, msgFile, errFile)) {
		RetVal = TRUE;

		bInFunc = FALSE;
		Done = FALSE;
		while ((m_PC < m_Memory->GetMemSize()) && (Done == FALSE) && (RetVal)) {
			bLastFlag = FALSE;
			bBranchOutFlag = FALSE;

			// Check for function start/end flags:
			if (m_FunctionsTable.Lookup(m_PC, nFuncFlag)) {
				switch (nFuncFlag) {
					case FUNCF_HARDSTOP:
					case FUNCF_EXITBRANCH:
					case FUNCF_SOFTSTOP:
						bInFunc = FALSE;
						bLastFlag = TRUE;
						break;

					case FUNCF_BRANCHOUT:
						bBranchOutFlag = TRUE;
						break;

					case FUNCF_BRANCHIN:
						if (bInFunc) {
							RetVal = RetVal && WriteIntraFunctionSep(outFile, msgFile, errFile);
							break;		// Continue if already inside a function (after printing a soft-break)
						}
						// Else, fall-through and setup for new function:
					case FUNCF_ENTRY:
					case FUNCF_INDIRECT:
					case FUNCF_CALL:
						RetVal = RetVal && WritePreFunction(outFile, msgFile, errFile);
						bInFunc = TRUE;
						break;

					default:
						ASSERT(FALSE);			// Unexpected Function Flag!!  Check Code!!
						RetVal = FALSE;
						continue;
				}
			}

			for (i=0; i<NUM_FIELD_CODES; i++) OutLine.SetAt(i, "");
			OutLine[FC_ADDRESS] = FormatAddress(m_PC);

			if (m_LabelTable.Lookup(m_PC, pLabelList)) {
				ASSERT(pLabelList != NULL);			// Shouldn't have null list!
				if (pLabelList == NULL) {
					RetVal = FALSE;
					Done = TRUE;
					continue;
				}
				for (i=1; i<pLabelList->GetSize(); i++) {
					OutLine[FC_LABEL] = FormatLabel(LC_CODE, pLabelList->GetAt(i), m_PC);
					outFile.WriteString(MakeOutputLine(OutLine));
					outFile.WriteString("\n");
				}
				if (pLabelList->GetSize()) OutLine[FC_LABEL] = FormatLabel(LC_CODE, pLabelList->GetAt(0), m_PC);
			}

			SavedPC = m_PC;		// Keep a copy of the PC for this line because some calls will be incrementing our m_PC
			switch (m_Memory->GetDescriptor(m_PC)) {
				case DMEM_ILLEGALCODE:
					m_OpMemory.RemoveAll();
					m_OpMemory.Add(m_Memory->GetByte(m_PC++));
					OutLine[FC_OPBYTES] = FormatOpBytes();
					OutLine[FC_MNEMONIC] = FormatMnemonic(MC_ILLOP);
					OutLine[FC_OPERANDS] = FormatOperands(MC_ILLOP);
					OutLine[FC_COMMENT] = FormatComments(MC_ILLOP);
					outFile.WriteString(MakeOutputLine(OutLine));
					outFile.WriteString("\n");
					break;
				case DMEM_CODE:
					// If the following call returns false, that means that we erroneously
					//	detected code in the first pass so instead of throwing an error or
					//	causing problems, we will treat it as an illegal opcode:
					if (ReadNextObj(FALSE, msgFile, errFile) == FALSE) {		// Bumps PC
						// Flag it as illegal and then process it as such:
						m_Memory->SetDescriptor(SavedPC, DMEM_ILLEGALCODE);
						OutLine[FC_OPBYTES] = FormatOpBytes();
						OutLine[FC_MNEMONIC] = FormatMnemonic(MC_ILLOP);
						OutLine[FC_OPERANDS] = FormatOperands(MC_ILLOP);
						OutLine[FC_COMMENT] = FormatComments(MC_ILLOP);
						outFile.WriteString(MakeOutputLine(OutLine));
						outFile.WriteString("\n");
						break;
					}
					OutLine[FC_OPBYTES] = FormatOpBytes();
					OutLine[FC_MNEMONIC] = FormatMnemonic(MC_OPCODE);
					OutLine[FC_OPERANDS] = FormatOperands(MC_OPCODE);
					OutLine[FC_COMMENT] = FormatComments(MC_OPCODE);
					outFile.WriteString(MakeOutputLine(OutLine));
					outFile.WriteString("\n");
					break;
				default:
					Done = TRUE;
			}

			if (bBranchOutFlag) {
				if (m_FunctionsTable.Lookup(m_PC, nFuncFlag)) {
					switch (nFuncFlag) {
						case FUNCF_ENTRY:
						case FUNCF_INDIRECT:
						case FUNCF_CALL:
							bInFunc = FALSE;
							bLastFlag = TRUE;
							break;
					}
				} else {
					bInFunc = FALSE;
					bLastFlag = TRUE;
				}
			}

			if (bLastFlag) {
				if (m_FunctionsTable.Lookup(m_PC, nFuncFlag)) {
					switch (nFuncFlag) {
						case FUNCF_BRANCHIN:
							bLastFlag = FALSE;
							bInFunc = TRUE;
							break;
					}
				}
			}

			if (/*(bInFunc) || */ (bLastFlag)) {
				RetVal = RetVal & WritePostFunction(outFile, msgFile, errFile);
			}
		}

		RetVal = RetVal && WritePostCodeSection(outFile, msgFile, errFile);
	}

	return RetVal;
}

BOOL CDisassembler::WritePostCodeSection(CStdioFile& outFile, CStdioFile *msgFile, CStdioFile *errFile)		// Don't do anything by default
{
	return TRUE;
}

BOOL CDisassembler::WritePreFunction(CStdioFile& outFile, CStdioFile *msgFile, CStdioFile *errFile)		// Default is separator bar
{
	CStringArray OutLine;
	int i;

	for (i=0; i<NUM_FIELD_CODES; i++) OutLine.Add("");
	OutLine[FC_ADDRESS] = FormatAddress(m_PC);
	outFile.WriteString(MakeOutputLine(OutLine));
	outFile.WriteString("\n");
	OutLine[FC_LABEL] = GetCommentStartDelim();
	OutLine[FC_LABEL] += " ";
	for (i=(GetFieldWidth(FC_LABEL)+
			GetFieldWidth(FC_MNEMONIC)+
			GetFieldWidth(FC_OPERANDS)); i; i--) OutLine[FC_LABEL] += "=";
	OutLine[FC_LABEL] += " ";
	OutLine[FC_LABEL] += GetCommentEndDelim();
	outFile.WriteString(MakeOutputLine(OutLine));
	outFile.WriteString("\n");

	return TRUE;
}

BOOL CDisassembler::WriteIntraFunctionSep(CStdioFile& outFile, CStdioFile *msgFile, CStdioFile *errFile)	// Default is separator bar
{
	CStringArray OutLine;
	int i;

	for (i=0; i<NUM_FIELD_CODES; i++) OutLine.Add("");
	OutLine[FC_ADDRESS] = FormatAddress(m_PC);
	OutLine[FC_LABEL] = GetCommentStartDelim();
	OutLine[FC_LABEL] += " ";
	for (i=(GetFieldWidth(FC_LABEL)+
			GetFieldWidth(FC_MNEMONIC)+
			GetFieldWidth(FC_OPERANDS)); i; i--) OutLine[FC_LABEL] += "-";
	OutLine[FC_LABEL] += " ";
	OutLine[FC_LABEL] += GetCommentEndDelim();
	outFile.WriteString(MakeOutputLine(OutLine));
	outFile.WriteString("\n");

	return TRUE;
}

BOOL CDisassembler::WritePostFunction(CStdioFile& outFile, CStdioFile *msgFile, CStdioFile *errFile)		// Default is separator bar
{
	CStringArray OutLine;
	int i;

	for (i=0; i<NUM_FIELD_CODES; i++) OutLine.Add("");
	OutLine[FC_ADDRESS] = FormatAddress(m_PC);
	OutLine[FC_LABEL] = GetCommentStartDelim();
	OutLine[FC_LABEL] += " ";
	for (i=(GetFieldWidth(FC_LABEL)+
			GetFieldWidth(FC_MNEMONIC)+
			GetFieldWidth(FC_OPERANDS)); i; i--) OutLine[FC_LABEL] += "=";
	OutLine[FC_LABEL] += " ";
	OutLine[FC_LABEL] += GetCommentEndDelim();
	outFile.WriteString(MakeOutputLine(OutLine));
	outFile.WriteString("\n");
	for (i=0; i<NUM_FIELD_CODES; i++) OutLine.SetAt(i, "");
	OutLine[FC_ADDRESS] = FormatAddress(m_PC);
	outFile.WriteString(MakeOutputLine(OutLine));
	outFile.WriteString("\n");

	return TRUE;
}

int CDisassembler::GetFieldWidth(FIELD_CODE nFC)
{
	switch (nFC) {
		case FC_ADDRESS:	return CALC_FIELD_WIDTH(8);
		case FC_OPBYTES:	return CALC_FIELD_WIDTH(12);
		case FC_LABEL:		return CALC_FIELD_WIDTH(14);
		case FC_MNEMONIC:	return CALC_FIELD_WIDTH(8);
		case FC_OPERANDS:	return CALC_FIELD_WIDTH(44);
		case FC_COMMENT:	return CALC_FIELD_WIDTH(60);
	}
	return 0;
}

CString CDisassembler::MakeOutputLine(CStringArray& saOutputData)
{
	CString OutputStr;
	CString Temp,Temp2;
	CString Item;
	int i,j;
	int fw;
	int pos;
	int tfw;
	BOOL Flag;
	int TempPos;
	CString OpBytePart;
	CString CommentPart;
	CStringArray WrapSave;
	int Toss;
	BOOL LabelBreak;
	CString SaveLabel;

	// This function will "wrap" the FC_OPBYTES and FC_COMMENT fields and create a
	//	result that spans multiple lines if they are longer than the field width.
	//	Wrapping is first attempted via white-space and/or one of ",;:.!?" symbols.  If
	//	that fails, the line is truncated at the specified length.  All other
	//	fields are not wrapped.  Note that embedded tabs count as 1 character for the
	//	wrapping mechanism!  Also, a '\n' can be used in the FC_COMMENT field to force
	//	a break.  All other fields should NOT have a '\n' character!
	//
	//	A '\v' (vertical tab or 0x0B) at the end of FC_LABEL will cause the label to
	//	appear on a line by itself -- the '\v' for labels should appear only at the end
	//	of the label -- not in the middle of it!.
	//

	OpBytePart = "";
	CommentPart = "";
	LabelBreak = FALSE;

	OutputStr = "";
	pos = 0;
	tfw = 0;
	fw = 0;
	for (i=0; i<saOutputData.GetSize(); i++) {
		Item = saOutputData[i];

		tfw += fw;
		fw = GetFieldWidth((FIELD_CODE) i);

		if ((m_bAddrFlag == FALSE) && (i == FC_ADDRESS)) {
			pos += fw;
			continue;
		}
		if ((m_bOpcodeFlag == FALSE) && (i == FC_OPBYTES)) {
			pos += fw;
			continue;
		}

		if (i == FC_LABEL) {			// Check for Label Break:
			if (Item.Find('\v') != -1) {
				while (Item.Find('\v') != -1) {
					Item = Item.Left(Item.Find('\v')) + Item.Right(Item.GetLength() - Item.Find('\v') - 1);
				}
				SaveLabel = Item;
				Item = "";
				LabelBreak = TRUE;
			}
		}

		if (i == FC_OPBYTES) {			// Check OpBytes "wrap"
			if (Item.GetLength() > fw) {
				Temp = Item.Left(fw);
				Temp.MakeReverse();
				TempPos = Temp.FindOneOf(" \t,;:.!?");
				if (TempPos != -1) {
					Toss = ((Temp.FindOneOf(" \t")==TempPos) ? 1 : 0);
					Temp = Item.Left(Temp.GetLength() - TempPos - Toss);	// Throw ' ' and '\t' away
					OpBytePart = Item.Right(Item.GetLength() - Temp.GetLength() - Toss);
					Item = Temp;
				} else {
					OpBytePart = Item.Right(Item.GetLength() - fw);
					Item = Item.Left(fw);
				}
			}
		}
		if (i == FC_COMMENT) {			// Check comment "wrap"
			Temp = GetCommentStartDelim() + " " + Item + " " + GetCommentEndDelim();
			Temp2 = GetCommentStartDelim() + "  " + GetCommentEndDelim();
			if ((Temp.GetLength() > fw) || (Temp.Find('\n') != -1)) {
				Temp = Item.Left(fw - Temp2.GetLength());
				Temp.MakeReverse();
				TempPos = Temp.FindOneOf(" \n\t,;:.!?");
				if (TempPos != -1) {
					Toss = ((Temp.FindOneOf(" \n\t")==TempPos) ? 1 : 0);
					Temp = Item.Left(Temp.GetLength() - TempPos - Toss);	// Throw ' ' and '\t' away
					CommentPart = "  " + Item.Right(Item.GetLength() - Temp.GetLength() - Toss);
					Item = Temp;
				} else {
					CommentPart = "  " + Item.Right(Item.GetLength() - fw + Temp2.GetLength());
					Item = Item.Left(fw - Temp2.GetLength());
				}
			}
		}

		if (i == FC_COMMENT) {
			if (Item.GetLength())
				Item = GetCommentStartDelim() + " " + Item + " " + GetCommentEndDelim();
		}

		if (m_bTabsFlag) {
			while (pos < tfw) {
				if (m_nTabWidth != 0) {
					while ((tfw - pos) % m_nTabWidth) {
						OutputStr += " ";
						pos++;
					}
					while ((tfw - pos) / m_nTabWidth) {
						OutputStr += "\t";
						pos += m_nTabWidth;
					}
				} else {
					OutputStr += " ";
					pos++;
				}
			}
			OutputStr += Item;
		} else {
			Temp = "";
			for (j=0; j<m_nTabWidth; j++) Temp += " ";
			while (Item.Find('\t') != -1) {
				TempPos = Item.Find('\t');
				Item = Item.Left(TempPos) + Temp + Item.Right(Item.GetLength() - TempPos - 1);
			}
			while (pos < tfw) {
				OutputStr += " ";
				pos++;
			}
			OutputStr += Item;
		}
		pos += Item.GetLength();
		if (pos >= (tfw+fw)) pos = tfw+fw-1;		// Sub 1 to force at least 1 space between fields
	}

	// Remove all extra white space on right-hand side:
	Flag = (OutputStr.GetLength() > 0);
	while (Flag) {
		if (isspace(OutputStr.Right(1)[0]) == FALSE) {
			Flag = FALSE;
			continue;
		}
		OutputStr = OutputStr.Left(OutputStr.GetLength()-1);
		Flag = (OutputStr.GetLength() > 0);
	}

	if ((OpBytePart.GetLength() != 0) || (CommentPart.GetLength() != 0) || (LabelBreak)) {
		// Save data:
		WrapSave.RemoveAll();
		for (i=0; i<saOutputData.GetSize(); i++) {
			WrapSave.Add(saOutputData[i]);
		}
		if (saOutputData.GetSize() < NUM_FIELD_CODES)
			for (i=saOutputData.GetSize(); i<NUM_FIELD_CODES; i++) saOutputData.Add("");

		if (LabelBreak) {
			saOutputData[FC_OPBYTES] = "";
			saOutputData[FC_LABEL] = SaveLabel;
			saOutputData[FC_MNEMONIC] = "";
			saOutputData[FC_OPERANDS] = "";
			saOutputData[FC_COMMENT] = "";
			OutputStr = MakeOutputLine(saOutputData) + "\n" + OutputStr;	// Call recursively
		}
		if ((OpBytePart.GetLength() != 0) || (CommentPart.GetLength() != 0)) {
			saOutputData[FC_OPBYTES] = OpBytePart;
			saOutputData[FC_LABEL] = "";
			saOutputData[FC_MNEMONIC] = "";
			saOutputData[FC_OPERANDS] = "";
			saOutputData[FC_COMMENT] = CommentPart;
			OutputStr += "\n";
			OutputStr += MakeOutputLine(saOutputData);		// Call recursively
		}

		// Restore data:
		saOutputData.RemoveAll();
		for (i=0; i<WrapSave.GetSize(); i++) {
			saOutputData.Add(WrapSave[i]);
		}
	}

	return OutputStr;
}
