//
//	Motorola 6811 Disassembler
//	Copyright(c)1996 - 2014 by Donna Whisnant
//

//
//	GDC.CPP	-- Implementation for Generic Disassembly Class
//
//	GDC is a generic means for defining a disassembly process that
//	processor independent.  The functions here provide the processor independent
//	part of the code.  These classes should be overriden as appropriate
//	to handle specific processors.
//

#include "gdc.h"
#include "dfc.h"
#include "stringhelp.h"
#include "memclass.h"
#include "errmsgs.h"
#include <ctime>
#include <ctype.h>
#include <iomanip>
#include <sstream>

#include <assert.h>

#define VERSION 0x200				// GDC Version number 2.00

// ----------------------------------------------------------------------------
//	COpcodeEntry
// ----------------------------------------------------------------------------
COpcodeEntry::COpcodeEntry()
{
	m_OpcodeBytes.clear();
	m_Group = 0;
	m_Control = 0;
	m_UserData = 0;
	m_Mnemonic = "<undefined>";
}

COpcodeEntry::COpcodeEntry(int nNumBytes, unsigned char *Bytes, unsigned int nGroup, unsigned int nControl, const std::string & szMnemonic, unsigned int nUserData)
{
	m_OpcodeBytes.clear();
	for (int i=0; i<nNumBytes; ++i) {
		m_OpcodeBytes.push_back(Bytes[i]);
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
	m_OpcodeBytes.clear();
	for (unsigned int i=0; i<aEntry.m_OpcodeBytes.size(); ++i) {
		m_OpcodeBytes.push_back(aEntry.m_OpcodeBytes.at(i));
	}
	m_Group = aEntry.m_Group;
	m_Control = aEntry.m_Control;
	m_UserData = aEntry.m_UserData;
	m_Mnemonic = aEntry.m_Mnemonic;
}

COpcodeEntry &COpcodeEntry::operator=(const COpcodeEntry& aEntry)
{
	CopyFrom(aEntry);
	return *this;
}


// ----------------------------------------------------------------------------
//	COpcodeArray
// ----------------------------------------------------------------------------
COpcodeArray::COpcodeArray()
	:	std::vector<COpcodeEntry>()
{
}

COpcodeArray::~COpcodeArray()
{
}


// ----------------------------------------------------------------------------
//	COpcodeTable
// ----------------------------------------------------------------------------
COpcodeTable::COpcodeTable()
{
}

COpcodeTable::~COpcodeTable()
{
}

void COpcodeTable::AddOpcode(const COpcodeEntry& nOpcode)
{
	if (nOpcode.m_OpcodeBytes.size() == 0) return;	// MUST have an initial byte in the opcode!

	iterator itrArray = find(nOpcode.m_OpcodeBytes.at(0));
	if (itrArray == end()) {
		COpcodeArray anArray;
		anArray.push_back(nOpcode);
		operator [](nOpcode.m_OpcodeBytes.at(0)) = anArray;
	} else {
		itrArray->second.push_back(nOpcode);
	}
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
	m_StartTime = time(NULL);

	// Set output defaults:
	m_bAddrFlag = false;
	m_bOpcodeFlag = false;
	m_bAsciiFlag = false;
	m_bSpitFlag = false;
	m_bTabsFlag = true;
	m_bAsciiBytesFlag = true;
	m_bDataOpBytesFlag = false;

	m_nMaxNonPrint = 8;
	m_nMaxPrint = 40;
	m_nTabWidth = 4;

	m_nBase = 16;						// See ReadControlFile function before changing these 3 here!
	m_sDefaultDFC = "binary";
	m_sInputFilename = "";

	m_nLoadAddress = 0;
	m_sOutputFilename = "";
	m_sFunctionsFilename = "";
	m_sInputFileList.clear();
	m_sControlFileList.clear();

	// Setup commands to parse:
	ParseCmds["ENTRY"] = 1;
	ParseCmds["LOAD"] = 2;
	ParseCmds["INPUT"] = 3;
	ParseCmds["OUTPUT"] = 4;
	ParseCmds["LABEL"] = 5;
	ParseCmds["ADDRESSES"] = 6;
	ParseCmds["INDIRECT"] = 7;
	ParseCmds["OPCODES"] = 8;
	ParseCmds["OPBYTES"] = 8;
	ParseCmds["ASCII"] = 9;
	ParseCmds["SPIT"] = 10;
	ParseCmds["BASE"] = 11;
	ParseCmds["MAXNONPRINT"] = 12;
	ParseCmds["MAXPRINT"] = 13;
	ParseCmds["DFC"] = 14;
	ParseCmds["TABS"] = 15;
	ParseCmds["TABWIDTH"] = 16;
	ParseCmds["ASCIIBYTES"] = 17;
	ParseCmds["DATAOPBYTES"] = 18;
	ParseCmds["EXITFUNCTION"] = 19;
	ParseCmds["MEMMAP"] = 20;

	ParseCmdsOP1["OFF"] = false;
	ParseCmdsOP1["ON"] = true;
	ParseCmdsOP1["FALSE"] = false;
	ParseCmdsOP1["TRUE"] = true;
	ParseCmdsOP1["NO"] = false;
	ParseCmdsOP1["YES"] = true;

	ParseCmdsOP2["OFF"] = 0;
	ParseCmdsOP2["NONE"] = 0;
	ParseCmdsOP2["0"] = 0;
	ParseCmdsOP2["BIN"] = 2;
	ParseCmdsOP2["BINARY"] = 2;
	ParseCmdsOP2["2"] = 2;
	ParseCmdsOP2["OCT"] = 8;
	ParseCmdsOP2["OCTAL"] = 8;
	ParseCmdsOP2["8"] = 8;
	ParseCmdsOP2["DEC"] = 10;
	ParseCmdsOP2["DECIMAL"] = 10;
	ParseCmdsOP2["10"] = 10;
	ParseCmdsOP2["HEX"] = 16;
	ParseCmdsOP2["HEXADECIMAL"] = 16;
	ParseCmdsOP2["16"] = 16;

	ParseCmdsOP3["CODE"] = 0;
	ParseCmdsOP3["DATA"] = 1;

	ParseCmdsOP4["ROM"] = 0;
	ParseCmdsOP4["RAM"] = 1;
	ParseCmdsOP4["IO"] = 2;

	ParseCmdsOP5["DISASSEMBLY"] = 0;
	ParseCmdsOP5["DISASSEMBLE"] = 0;
	ParseCmdsOP5["DISASSEM"] = 0;
	ParseCmdsOP5["DISASM"] = 0;
	ParseCmdsOP5["DIS"] = 0;
	ParseCmdsOP5["DASM"] = 0;
	ParseCmdsOP5["FUNCTION"] = 1;
	ParseCmdsOP5["FUNCTIONS"] = 1;
	ParseCmdsOP5["FUNC"] = 1;
	ParseCmdsOP5["FUNCT"] = 1;
	ParseCmdsOP5["FUNCTS"] = 1;

	m_Memory = NULL;
	m_nFilesLoaded = 0;
	m_PC = 0;

	LAdrDplyCnt=0;
}

CDisassembler::~CDisassembler()
{
	ParseCmds.clear();
	ParseCmdsOP1.clear();
	ParseCmdsOP2.clear();
	ParseCmdsOP3.clear();
	ParseCmdsOP4.clear();
	ParseCmdsOP5.clear();

	m_sInputFileList.clear();
	m_sControlFileList.clear();

	m_EntryTable.clear();

	m_FunctionsTable.clear();
	m_FuncExitAddresses.clear();

	m_BranchTable.clear();
	m_LabelTable.clear();
	m_LabelRefTable.clear();

	m_CodeIndirectTable.clear();
	m_DataIndirectTable.clear();

	if (m_Memory) {
		delete m_Memory;
		m_Memory = NULL;
	}
}

unsigned int CDisassembler::GetVersionNumber()
{
	return (VERSION << 16);
}

bool CDisassembler::ReadControlFile(ifstreamControlFile& inFile, bool bLastFile, std::ostream *msgFile, std::ostream *errFile, int nStartLineCount)
{
	bool RetVal;

	std::string aLine;
	TStringArray args;
	unsigned int Address;
	unsigned int ResAddress;
	TDWordArray SortedList;
	TMemRange *pMemRange;

	// Note: m_nFilesLoaded must be properly set/reset prior to calling this function
	//		This is typically done in the constructor -- but overrides, etc, doing
	//		preloading of files should properly set it as well.

	//	bLastFile causes this function to report the overall disassembly summary from
	//		control files -- such as "at least one input file" and "output file must
	//		be specified" type error messages and sets any default action that should
	//		be taken with entries, etc.  The reason for the boolean flag is to
	//		allow processing of several control files prior to reporting the summary.
	//		Typically, the last control file processed should be called with a "true"
	//		and all prior control files processed should be called with a "FALSE".

	RetVal = true;

	m_sControlFileList.push_back(inFile.getFilename());

	if (msgFile) {
		(*msgFile) << "Reading and Parsing Control File: \"" << inFile.getFilename() << "\"...\n";
	}

	m_nBase = 16;						// Reset the default base before starting new file.. Therefore, each file has same default!
	m_sDefaultDFC = "binary";			// Reset the default DFC format so every control file starts with same default!
	m_sInputFilename = "";				// Reset the Input Filename so we won't try to reload file from previous control file if this control doesn't specify one

	m_nCtrlLine = nStartLineCount;		// In case several lines were read by outside process, it should pass in correct starting number so we display correct error messages!
	while (inFile.good()) {
		std::getline(inFile, aLine);
		m_nCtrlLine++;
		m_ParseError = "*** Error: Unknown error";
		std::size_t pos = aLine.find(';');
		if (pos != std::string::npos) aLine = aLine.substr(0, pos);		// Trim off comments!
		trim(aLine);
		if (aLine.size() == 0) continue;	// If it is a blank or null line or only a comment, keep going
		args.clear();
		std::string Temp = aLine;
		while (Temp.size()) {
			pos = Temp.find_first_of("\x009\x00a\x00b\x00c\x00d\x020");
			if (pos != std::string::npos) {
				args.push_back(Temp.substr(0, pos));
				Temp = Temp.substr(pos+1);
			} else {
				args.push_back(Temp);
				Temp.clear();
			}
			ltrim(Temp);
		}
		if (args.size() == 0) continue;		// If we don't have any args, get next line (really shouldn't ever have no args here)

		if (ParseControlLine(aLine, args, msgFile, errFile) == false) {		// Go parse it -- either internal or overrides
			if (errFile) {
				(*errFile) << m_ParseError;
				(*errFile) << " in Control File \"" << inFile.getFilename() << "\" line " << m_nCtrlLine << "\n";
			}
		}
	}

	// If a Input File was specified by the control file, then open it and read it here:
	if (!m_sInputFilename.empty()) {
		ReadSourceFile(m_sInputFilename, m_nLoadAddress, m_sDefaultDFC, msgFile, errFile);
	}

	if (bLastFile) {
		if (m_nFilesLoaded == 0) {
			if (errFile) (*errFile) << "*** Error: At least one input file must be specified in the control file(s) and successfully loaded\n";
			RetVal = false;
		}
		if (m_sOutputFilename.empty()) {
			if (errFile) (*errFile) << "*** Error: Disassembly Output file must be specified in the control file(s)\n";
			RetVal = false;
		}
		if ((m_bSpitFlag == false) && (m_EntryTable.size() == 0) && (m_CodeIndirectTable.size() == 0)) {
			if (errFile) (*errFile) << "*** Error: No entry addresses or indirect code vectors have been specified in the control file(s)\n";
			RetVal = false;
		}

		if (RetVal) {
			if (msgFile) {
				(*msgFile) << "\n";
				(*msgFile) << "        " << m_sInputFileList.size() << " Source File"
										<< ((m_sInputFileList.size() != 1) ? "s" : "")
										<< ((m_sInputFileList.size() != 0) ? ":" : "");
				if (m_sInputFileList.size() == 1) {
					(*msgFile) << " " << m_sInputFileList.at(0) << "\n";
				} else {
					for (unsigned int i=0; i<m_sInputFileList.size(); ++i) {
						(*msgFile) << "                " << m_sInputFileList.at(i) << "\n";
					}
				}
				(*msgFile) << "\n";

				if (!m_sOutputFilename.empty()) {
					(*msgFile) << "        Disassembly Output File: " << m_sOutputFilename << "\n\n";
				}

				if (!m_sFunctionsFilename.empty()) {
					(*msgFile) << "        Functions Output File: " << m_sFunctionsFilename << "\n\n";
				}

				(*msgFile) << "        Memory Mappings:\n";
				(*msgFile) << "            ROM Memory Map:";
				if (m_ROMMemMap.IsNullRange()) {
					(*msgFile) << " <Not Defined>\n";
				} else {
					(*msgFile) << "\n";
					pMemRange = &m_ROMMemMap;
					while (pMemRange) {
						(*msgFile) << "                "
									<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << pMemRange->GetStartAddr()
									<< std::nouppercase << std::setbase(0) << " - "
									<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << (pMemRange->GetStartAddr() + pMemRange->GetSize() - 1)
									<< std::nouppercase << std::setbase(0) << "  (Size: "
									<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << pMemRange->GetSize()
									<< std::nouppercase << std::setbase(0) << ")\n";
						pMemRange = pMemRange->GetNext();
					}
				}
				(*msgFile) << "            RAM Memory Map:";
				if (m_RAMMemMap.IsNullRange()) {
					(*msgFile) << " <Not Defined>\n";
				} else {
					(*msgFile) << "\n";
					pMemRange = &m_RAMMemMap;
					while (pMemRange) {
						(*msgFile) << "                "
									<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << pMemRange->GetStartAddr()
									<< std::nouppercase << std::setbase(0) << " - "
									<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << (pMemRange->GetStartAddr() + pMemRange->GetSize() - 1)
									<< std::nouppercase << std::setbase(0) << "  (Size: "
									<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << pMemRange->GetSize()
									<< std::nouppercase << std::setbase(0) << ")\n";
						pMemRange = pMemRange->GetNext();
					}
				}
				(*msgFile) << "             IO Memory Map:";
				if (m_IOMemMap.IsNullRange()) {
					(*msgFile) << " <Not Defined>\n";
				} else {
					(*msgFile) << "\n";
					pMemRange = &m_IOMemMap;
					while (pMemRange) {
						(*msgFile) << "                "
									<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << pMemRange->GetStartAddr()
									<< std::nouppercase << std::setbase(0) << " - "
									<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << (pMemRange->GetStartAddr() + pMemRange->GetSize() - 1)
									<< std::nouppercase << std::setbase(0) << "  (Size: "
									<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << pMemRange->GetSize()
									<< std::nouppercase << std::setbase(0) << ")\n";
						pMemRange = pMemRange->GetNext();
					}
				}
				(*msgFile) << "\n";
				(*msgFile) << "        " << m_EntryTable.size() << " Entry Point"
											<< ((m_EntryTable.size() != 1) ? "s" : "")
											<< ((m_EntryTable.size() != 0) ? ":" : "")
											<< "\n";
			}
			SortedList.clear();
			for (TAddressMap::const_iterator itrEntry = m_EntryTable.begin(); itrEntry != m_EntryTable.end(); ++itrEntry) {
				TDWordArray::iterator itrSorted;
				for (itrSorted = SortedList.begin(); itrSorted != SortedList.end(); ++itrSorted) {
					if (itrEntry->first <= *itrSorted) {
						break;
					}
				}
				SortedList.insert(itrSorted, itrEntry->first);
			}
			for (unsigned int i=0; i<SortedList.size(); ++i) {
				if (msgFile) {
					(*msgFile) << "                "
								<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << SortedList.at(i)
								<< std::nouppercase << std::setbase(0) << "\n";
				}
				if (IsAddressLoaded(SortedList.at(i), 1) == false) {
					if (errFile) {
						(*errFile) << "    *** Warning: Entry Point Address "
									<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << SortedList.at(i)
									<< std::nouppercase << std::setbase(0) << " is outside of loaded source file(s)...\n";
					}
				}
			}

			if (msgFile) {
				(*msgFile) << "\n";
				(*msgFile) << "        " << m_FuncExitAddresses.size()
							<< " Exit Function"
							<< ((m_FuncExitAddresses.size() != 1) ? "s" : "")
							<< " Defined"
							<< ((m_FuncExitAddresses.size() != 0) ? ":" : "")
							<< "\n";
			}
			SortedList.clear();
			for (TAddressMap::const_iterator itrEntry = m_FuncExitAddresses.begin(); itrEntry != m_FuncExitAddresses.end(); ++itrEntry) {
				TDWordArray::iterator itrSorted;
				for (itrSorted = SortedList.begin(); itrSorted != SortedList.end(); ++itrSorted) {
					if (itrEntry->first <= *itrSorted) {
						break;
					}
				}
				SortedList.insert(itrSorted, itrEntry->first);
			}
			for (unsigned int i=0; i<SortedList.size(); ++i) {
				if (msgFile) {
					(*msgFile) << "                "
								<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << SortedList.at(i)
								<< std::nouppercase << std::setbase(0) << "\n";
				}
				if (IsAddressLoaded(SortedList.at(i), 1) == false) {
					if (errFile) {
						(*errFile) << "    *** Warning: Exit Function Address "
									<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << SortedList.at(i)
									<< std::nouppercase << std::setbase(0) << " is outside of loaded source file(s)...\n";
					}
				}
			}

			if (msgFile) {
				(*msgFile) << "\n";
				(*msgFile) << "        " << m_LabelTable.size()
							<< " Unique Label"
							<< ((m_LabelTable.size() != 1) ? "s" : "")
							<< " Defined"
							<< ((m_LabelTable.size() != 0) ? ":" : "")
							<< "\n";
				SortedList.clear();
				for (TLabelTableMap::const_iterator itrEntry = m_LabelTable.begin(); itrEntry != m_LabelTable.end(); ++itrEntry) {
					TDWordArray::iterator itrSorted;
					for (itrSorted = SortedList.begin(); itrSorted != SortedList.end(); ++itrSorted) {
						if (itrEntry->first <= *itrSorted) {
							break;
						}
					}
					SortedList.insert(itrSorted, itrEntry->first);
				}
				for (unsigned int i=0; i<SortedList.size(); ++i) {
					TLabelTableMap::const_iterator itrLabelTable = m_LabelTable.find(SortedList.at(i));
					assert(itrLabelTable != m_LabelTable.end());
					(*msgFile) << "                "
								<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << SortedList.at(i)
								<< std::nouppercase << std::setbase(0) << "=";
					for (unsigned int j=0; j<itrLabelTable->second.size(); ++j) {
						if (j != 0) (*msgFile) << ",";
						(*msgFile) << (itrLabelTable->second.at(j));
					}
					(*msgFile) << "\n";
				}
				(*msgFile) << "\n";

				if (m_bAddrFlag) (*msgFile) << "Writing program counter addresses to disassembly file.\n";
				if (m_bOpcodeFlag) (*msgFile) << "Writing opcode byte values to disassembly file.\n";
				if (m_bAsciiFlag) (*msgFile) << "Writing printable data as ASCII in disassembly file.\n";
				if (m_bSpitFlag) (*msgFile) << "Performing a code-dump (spit) disassembly instead of code-seeking.\n";
				if (m_bTabsFlag) {
					(*msgFile) << "Using tab characters in disassembly file.  Tab width set at: " << m_nTabWidth << "\n";
				}
				if (m_bAsciiBytesFlag) (*msgFile) << "Writing byte value comments for ASCII data in disassembly file.\n";
				if ((m_bOpcodeFlag) && (m_bDataOpBytesFlag)) (*msgFile) << "Writing OpBytes for data in disassembly file\n";
				(*msgFile) << "\n";
			}


		// Ok, now we resolve the indirect tables...  This process should create the indirect
		//		label name from the resolved address if the corresponding string in the hash table is "".
		//		This process requires the purely virtual ResolveIndirect function from overrides...
			if (msgFile) (*msgFile) << "Compiling Indirect Code (branch) Table as specified in Control File...\n";
			SortedList.clear();
			for (TAddressLabelMap::const_iterator itrEntry = m_CodeIndirectTable.begin(); itrEntry != m_CodeIndirectTable.end(); ++itrEntry) {
				TDWordArray::iterator itrSorted;
				for (itrSorted = SortedList.begin(); itrSorted != SortedList.end(); ++itrSorted) {
					if (itrEntry->first <= *itrSorted) {
						break;
					}
				}
				SortedList.insert(itrSorted, itrEntry->first);
			}

			if (msgFile) {
				(*msgFile) << "        " << SortedList.size() << " Indirect Code Vector"
							<< ((SortedList.size() != 1) ? "s" : "")
							<< ((SortedList.size() != 0) ? ":" : "")
							<< "\n";
			}
			for (unsigned int i=0; i<SortedList.size(); ++i) {
				Address = SortedList.at(i);
				TAddressLabelMap::const_iterator itrTable = m_CodeIndirectTable.find(Address);
				assert(itrTable != m_CodeIndirectTable.end());
				if (msgFile) {
					(*msgFile) << "                ["
								<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << Address
								<< std::nouppercase << std::setbase(0) << "] -> ";
				}
				if (ResolveIndirect(Address, ResAddress, 0) == false) {
					if (msgFile) (*msgFile) << "ERROR\n";
					if (errFile) {
						(*errFile) << "    *** Warning: Vector Address "
									<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << Address
									<< std::nouppercase << std::setbase(0) << " is outside of loaded source file(s)...\n";
						(*errFile) << "                    Or the vector location conflicted with other analyzed areas.\n";
					}
				} else {
					if (msgFile) {
						(*msgFile) << GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << ResAddress
									<< std::nouppercase << std::setbase(0);
					}
					AddLabel(ResAddress, false, 0, itrTable->second);	// Add label for resolved name.  If NULL, add it so later we can resolve Lxxxx from it.
					m_FunctionsTable[ResAddress] = FUNCF_INDIRECT;		// Resolved code indirects are also considered start-of functions
					if (AddBranch(ResAddress, true, Address) == false) {
						if (errFile) {
							(*errFile) << "    *** Warning: Indirect Address ["
										<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << Address
										<< std::nouppercase << std::setbase(0) << "] -> "
										<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << ResAddress
										<< std::nouppercase << std::setbase(0) << " is outside of loaded source file(s)...\n";
						}
						if ((msgFile) && (msgFile != errFile)) {
							(*msgFile) << "\n";
						}
					} else {
						if (msgFile) {
							(*msgFile) << "\n";
						}
					}
				}
			}
			if (msgFile) (*msgFile) << "\n";
			// Now, repeat for Data indirects:
			if (msgFile) (*msgFile) << "Compiling Indirect Data Table as specified in Control File...\n";
			SortedList.clear();
			for (TAddressLabelMap::const_iterator itrEntry = m_DataIndirectTable.begin(); itrEntry != m_DataIndirectTable.end(); ++itrEntry) {
				TDWordArray::iterator itrSorted;
				for (itrSorted = SortedList.begin(); itrSorted != SortedList.end(); ++itrSorted) {
					if (itrEntry->first <= *itrSorted) {
						break;
					}
				}
				SortedList.insert(itrSorted, itrEntry->first);
			}

			if (msgFile) {
				(*msgFile) << "        " << SortedList.size() << " Indirect Data Vector"
							<< ((SortedList.size() != 1) ? "s" : "")
							<< ((SortedList.size() != 0) ? ":" : "")
							<< "\n";
			}
			for (unsigned int i=0; i<SortedList.size(); ++i) {
				Address = SortedList.at(i);
				TAddressLabelMap::const_iterator itrTable = m_DataIndirectTable.find(Address);
				assert(itrTable != m_DataIndirectTable.end());
				if (msgFile) {
					(*msgFile) << "                ["
								<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << Address
								<< std::nouppercase << std::setbase(0) << "] -> ";
				}
				if (ResolveIndirect(Address, ResAddress, 1) == false) {
					if (msgFile) (*msgFile) << "ERROR\n";
					if (errFile) {
						(*errFile) << "    *** Warning: Vector Address "
									<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << Address
									<< std::nouppercase << std::setbase(0) << " is outside of loaded source file(s)...\n";
						(*errFile) << "                    Or the vector location conflicted with other analyzed areas.\n";
					}
				} else {
					if (msgFile) {
						(*msgFile) << GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << ResAddress
									<< std::nouppercase << std::setbase(0) << "\n";
					}
					AddLabel(ResAddress, true, Address, itrTable->second);		// Add label for resolved name.  If NULL, add it so later we can resolve Lxxxx from it.
				}
			}
			if (msgFile) (*msgFile) << "\n";
		}
	}

	return RetVal;
}

bool CDisassembler::ParseControlLine(const std::string & /* szLine */, TStringArray& argv, std::ostream *msgFile, std::ostream *errFile)
{
	bool RetVal;
	std::string	Cmd;
	unsigned int CmdIndex;
	unsigned int Address;
	unsigned int Size;
	std::string TempStr;
	std::string DfcLibrary;
	std::string FileName;
	unsigned int TempWord;
	unsigned int TempDWord;
	int ArgError;
	bool TempFlag;
	bool TempFlag2;

	assert(argv.size() != 0);

	if (argv.size() == 0) return false;

	RetVal = true;
	ArgError = 0;

	Cmd = argv[0];
	makeUpper(Cmd);

	CmdIndex = -1;
	TParseCmdMap::const_iterator itrParse = ParseCmds.find(Cmd);
	if (itrParse != ParseCmds.end()) CmdIndex = itrParse->second;
	switch (CmdIndex) {
		case 1:		// ENTRY <addr> [<name>]
			if (argv.size() < 2) {
				ArgError = 1;
				break;
			}
			if (argv.size() > 3) {
				ArgError = 2;
				break;
			}
			if (argv.size() == 3) {
				if (!ValidateLabelName(argv[2])) {
					ArgError = 3;
					break;
				}
			}
			Address = strtoul(argv[1].c_str(), NULL, m_nBase);
			if (m_EntryTable.find(Address) != m_EntryTable.end()) {
				RetVal = false;
				m_ParseError = "*** Warning: Duplicate entry address";
			}
			m_EntryTable[Address] = true;	// Add an entry to the entry table
			m_FunctionsTable[Address] = FUNCF_ENTRY;	// Entries are also considered start-of functions
			if (argv.size() == 3) {
				if (AddLabel(Address, false, 0, argv[2]) == false) {
					RetVal = false;
					m_ParseError = "*** Warning: Duplicate label";
				}
			}
			break;
		case 2:		// LOAD <addr> | <addr> <filename> [<library>]
		{
			if (argv.size() == 2) {
				m_nLoadAddress = strtoul(argv[1].c_str(), NULL, m_nBase);
				break;
			}
			if (argv.size() < 3) {
				ArgError = 1;
				break;
			}
			if (argv.size() > 4) {
				ArgError = 2;
				break;
			}
			Address = strtoul(argv[1].c_str(), NULL, m_nBase);
			if (argv.size() == 4) {
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
			if (argv.size() != 2) {
				ArgError = (argv.size() < 2) ? 1 : 2;
				break;
			}
			if (!m_sInputFilename.empty()) {
				RetVal = false;
				m_ParseError = "*** Warning: Input filename already defined";
			}
			m_sInputFilename = argv[1];
			break;
		case 4:		// OUTPUT [DISASSEMBLY | FUNCTIONS] <filename>
			if ((argv.size() != 2) && (argv.size() != 3)) {
				ArgError = (argv.size() < 2) ? 1 : 2;
				break;
			}
			TempFlag = false;		// TempFlag = FALSE if type not specified on line, true if it is
			if (argv.size() == 2) {		// If type isn't specified, assume DISASSEMBLY
				TempWord = 0;
			} else {
				TempFlag = true;
				TempStr = argv[1];
				makeUpper(TempStr);
				itrParse = ParseCmdsOP5.find(TempStr);
				if (itrParse == ParseCmdsOP5.end()) {
					ArgError = 3;
					break;
				} else {
					TempWord = itrParse->second;
				}
			}
			switch (TempWord) {
				case 0:				// DISASSEMBLY
					if (!m_sOutputFilename.empty()) {
						RetVal = false;
						m_ParseError = "*** Warning: Disassembly Output filename already defined";
					}
					m_sOutputFilename = argv[((TempFlag) ? 2 : 1)];
					break;
				case 1:				// FUNCTIONS
					if (!m_sFunctionsFilename.empty()) {
						RetVal = false;
						m_ParseError = "*** Warning: Functions Output filename already defined";
					}
					m_sFunctionsFilename = argv[((TempFlag) ? 2 : 1)];
					break;
			}
			break;
		case 5:		// LABEL <addr> <name>
			if (argv.size() != 3) {
				ArgError = (argv.size() < 3) ? 1 : 2;
				break;
			}
			if (!ValidateLabelName(argv[2])) {
				ArgError = 3;
				break;
			}
			Address = strtoul(argv[1].c_str(), NULL, m_nBase);
			if (AddLabel(Address, false, 0, argv[2]) == false) {
				RetVal = false;
				m_ParseError = "*** Warning: Duplicate label";
			}
			break;
		case 6:		// ADDRESSES [ON | OFF | TRUE | FALSE | YES | NO]
			if (argv.size() < 2) {
				m_bAddrFlag = true;		// Default is ON
				break;
			}
			if (argv.size() > 2) {
				ArgError = 2;
				break;
			}
			TempStr = argv[1];
			makeUpper(TempStr);
			itrParse = ParseCmdsOP1.find(TempStr);
			if (itrParse != ParseCmdsOP1.end()) {
				m_bAddrFlag = itrParse->second;
			} else {
				ArgError = 3;
			}
			break;
		case 7:		// INDIRECT [CODE | DATA] <addr> [<name>]
			if ((argv.size() != 2) && (argv.size() != 3) && (argv.size() != 4)) {
				ArgError = (argv.size() < 2) ? 1 : 2;
				break;
			}

			TempFlag = false;		// TempFlag = FALSE if CODE/DATA not specified on line, true if it is
			TempFlag2 = false;		// TempFlag2 = FALSE if <name> not specified on line, true if it is
			if (argv.size() == 2) {		// If Code or Data isn't specified, assume code
				TempWord = 0;
			} else {
				TempStr = argv[1];
				makeUpper(TempStr);
				itrParse = ParseCmdsOP3.find(TempStr);
				if (itrParse == ParseCmdsOP3.end()) {
					if (argv.size() == 4) {			// If all args was specified, error out if not valid
						ArgError = 3;
						break;
					} else {					// If here, we assume we have <addr> and <name> since we are one arg short
						TempWord = 0;			// Assume Code since it isn't specified
						TempFlag2 = true;		// And we do have a <name>
					}
				} else {
					TempWord = itrParse->second;
					TempFlag = true;
					if (argv.size() == 4) {			// If all args was specified, we have valid everything
						TempFlag2 = true;
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
			Address = strtoul(argv[1 + ((TempFlag) ? 1 : 0)].c_str(), NULL, m_nBase);
			switch (TempWord) {
				case 0:
					if ((m_CodeIndirectTable.find(Address) != m_CodeIndirectTable.end()) ||
						(m_DataIndirectTable.find(Address) != m_DataIndirectTable.end())) {
						RetVal = false;
						m_ParseError = "*** Warning: Duplicate indirect";
					}
					m_CodeIndirectTable[Address] = TempStr;	// Add a label to the code indirect table
					break;
				case 1:
					if ((m_DataIndirectTable.find(Address) != m_DataIndirectTable.end()) ||
						(m_CodeIndirectTable.find(Address) != m_CodeIndirectTable.end())) {
						RetVal = false;
						m_ParseError = "*** Warning: Duplicate indirect";
					}
					m_DataIndirectTable[Address] = TempStr;	// Add a label to the data indirect table
					break;
			}
			break;
		case 8:		// OPCODES [ON | OFF | TRUE | FALSE | YES | NO]
					// OPBYTES [ON | OFF | TRUE | FALSE | YES | NO]
			if (argv.size() < 2) {
				m_bOpcodeFlag = true;		// Default is ON
				break;
			}
			if (argv.size() > 2) {
				ArgError = 2;
				break;
			}
			TempStr = argv[1];
			makeUpper(TempStr);
			itrParse = ParseCmdsOP1.find(TempStr);
			if (itrParse != ParseCmdsOP1.end()) {
				m_bOpcodeFlag = itrParse->second;
			} else {
				ArgError = 3;
			}
			break;
		case 9:		// ASCII [ON | OFF | TRUE | FALSE | YES | NO]
			if (argv.size() < 2) {
				m_bAsciiFlag = true;		// Default is ON
				break;
			}
			if (argv.size() > 2) {
				ArgError = 2;
				break;
			}
			TempStr = argv[1];
			makeUpper(TempStr);
			itrParse = ParseCmdsOP1.find(TempStr);
			if (itrParse != ParseCmdsOP1.end()) {
				m_bAsciiFlag = itrParse->second;
			} else {
				ArgError = 3;
			}
			break;
		case 10:	// SPIT [ON | OFF | TRUE | FALSE | YES | NO]
			if (argv.size() < 2) {
				m_bSpitFlag = true;		// Default is ON
				break;
			}
			if (argv.size() > 2) {
				ArgError = 2;
				break;
			}
			TempStr = argv[1];
			makeUpper(TempStr);
			itrParse = ParseCmdsOP1.find(TempStr);
			if (itrParse != ParseCmdsOP1.end()) {
				m_bSpitFlag = itrParse->second;
			} else {
				ArgError = 3;
			}
			break;
		case 11:	// BASE [OFF | BIN | OCT | DEC | HEX]
			if (argv.size() < 2) {
				m_nBase = 0;		// Default is OFF
				break;
			}
			if (argv.size() > 2) {
				ArgError = 2;
				break;
			}
			TempStr = argv[1];
			makeUpper(TempStr);
			itrParse = ParseCmdsOP2.find(TempStr);
			if (itrParse != ParseCmdsOP2.end()) {
				m_nBase = itrParse->second;
			} else {
				ArgError = 3;
			}
			break;
		case 12:	// MAXNONPRINT <value>
			if (argv.size() != 2) {
				ArgError = (argv.size() < 2) ? 1 : 2;
				break;
			}
			m_nMaxNonPrint = strtoul(argv[1].c_str(), NULL, m_nBase);
			break;
		case 13:	// MAXPRINT <value>
			if (argv.size() != 2) {
				ArgError = (argv.size() < 2) ? 1 : 2;
				break;
			}
			m_nMaxPrint = strtoul(argv[1].c_str(), NULL, m_nBase);
			break;
		case 14:	// DFC <library>
			if (argv.size() != 2) {
				ArgError = (argv.size() < 2) ? 1 : 2;
				break;
			}
			m_sDefaultDFC = argv[1];
			break;
		case 15:	// TABS [ON | OFF | TRUE | FALSE | YES | NO]
			if (argv.size() < 2) {
				m_bTabsFlag = true;		// Default is ON
				break;
			}
			if (argv.size() > 2) {
				ArgError = 2;
				break;
			}
			TempStr = argv[1];
			makeUpper(TempStr);
			itrParse = ParseCmdsOP1.find(TempStr);
			if (itrParse != ParseCmdsOP1.end()) {
				m_bTabsFlag = itrParse->second;
			} else{
				ArgError = 3;
			}
			break;
		case 16:	// TABWIDTH <value>
			if (argv.size() != 2) {
				ArgError = (argv.size() < 2) ? 1 : 2;
				break;
			}
			m_nTabWidth = strtoul(argv[1].c_str(), NULL, m_nBase);
			break;
		case 17:	// ASCIIBYTES [ON | OFF | TRUE | FALSE | YES | NO]
			if (argv.size() < 2) {
				m_bAsciiBytesFlag = true;		// Default is ON
				break;
			}
			if (argv.size() > 2) {
				ArgError = 2;
				break;
			}
			TempStr = argv[1];
			makeUpper(TempStr);
			itrParse = ParseCmdsOP1.find(TempStr);
			if (itrParse != ParseCmdsOP1.end()) {
				m_bAsciiBytesFlag = itrParse->second;
			} else {
				ArgError = 3;
			}
			break;
		case 18:	// DATAOPBYTES [ON | OFF | TRUE | FALSE | YES | NO]
			if (argv.size() < 2) {
				m_bDataOpBytesFlag = true;		// Default is ON
				break;
			}
			if (argv.size() > 2) {
				ArgError = 2;
				break;
			}
			TempStr = argv[1];
			makeUpper(TempStr);
			itrParse = ParseCmdsOP1.find(TempStr);
			if (itrParse != ParseCmdsOP1.end()) {
				m_bDataOpBytesFlag = itrParse->second;
			} else {
				ArgError = 3;
			}
			break;
		case 19:	// EXITFUNCTION <addr> [<name>]
			if (argv.size() < 2) {
				ArgError = 1;
				break;
			}
			if (argv.size() > 3) {
				ArgError = 2;
				break;
			}
			if (argv.size() == 3) {
				if (!ValidateLabelName(argv[2])) {
					ArgError = 3;
					break;
				}
			}
			Address = strtoul(argv[1].c_str(), NULL, m_nBase);
			if (m_FuncExitAddresses.find(Address) != m_FuncExitAddresses.end()) {
				RetVal = false;
				m_ParseError = "*** Warning: Duplicate function exit address";
			}
			m_FunctionsTable[Address] = FUNCF_ENTRY;	// Exit Function Entry points are also considered start-of functions
			m_FuncExitAddresses[Address] = true;		// Add function exit entry
			if (argv.size() == 3) {
				if (AddLabel(Address, false, 0, argv[2]) == false) {
					RetVal = false;
					m_ParseError = "*** Warning: Duplicate label";
				}
			}
			break;
		case 20:	// MEMMAP [ROM | RAM | IO] <addr> <size>
			if ((argv.size() != 3) && (argv.size() != 4)) {
				ArgError = (argv.size() < 3) ? 1 : 2;
				break;
			}

			TempFlag = false;		// TempFlag = FALSE if ROM/RAM/IO not specified on line, true if it is
			if (argv.size() == 3) {		// If ROM/RAM/IO not specified on line, assume ROM
				TempWord = 0;
			} else {
				TempStr = argv[1];
				makeUpper(TempStr);
				itrParse = ParseCmdsOP4.find(TempStr);
				if (itrParse != ParseCmdsOP4.end()) {
					TempWord = itrParse->second;
					TempFlag = true;		// If specified and valid, set flag
				} else {
					ArgError = 3;			// Error out if type not valid but specified
					break;
				}
			}

			// Get address and size specified:
			Address = strtoul(argv[1 + ((TempFlag) ? 1 : 0)].c_str(), NULL, m_nBase);
			Size = strtoul(argv[2 + ((TempFlag) ? 1 : 0)].c_str(), NULL, m_nBase);

			switch (TempWord) {
				case 0:		// ROM
					m_ROMMemMap.AddRange(Address, Size, 0);
					m_ROMMemMap.Compact();
					m_ROMMemMap.RemoveOverlaps();
					m_ROMMemMap.Sort();
					for (TempDWord = Address; ((TempDWord < (Address + Size)) && (RetVal)); TempDWord++) {
						if (m_RAMMemMap.AddressInRange(TempDWord)) {
							RetVal = false;
							m_ParseError = "*** Warning: Specified ROM Mapping conflicts with RAM Mapping";
						} else {
							if (m_IOMemMap.AddressInRange(TempDWord)) {
								RetVal = false;
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
							RetVal = false;
							m_ParseError = "*** Warning: Specified RAM Mapping conflicts with ROM Mapping";
						} else {
							if (m_IOMemMap.AddressInRange(TempDWord)) {
								RetVal = false;
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
							RetVal = false;
							m_ParseError = "*** Warning: Specified IO Mapping conflicts with ROM Mapping";
						} else {
							if (m_RAMMemMap.AddressInRange(TempDWord)) {
								RetVal = false;
								m_ParseError = "*** Warning: Specified IO Mapping conflicts with RAM Mapping";
							}
						}
					}
					break;
			}
			break;
		default:
			m_ParseError = "*** Error: Unknown command";
			return false;
	}

	if (ArgError) {
		RetVal = false;
		switch (ArgError) {
			case 1:
				m_ParseError = "*** Error: Not enough arguments for '" + argv[0] + "' command";
				break;
			case 2:
				m_ParseError = "*** Error: Too many arguments for '" + argv[0] + "' command";
				break;
			case 3:
				m_ParseError = "*** Error: Illegal argument for '" + argv[0] + "' command";
				break;
		}
	}

	return RetVal;
}


bool CDisassembler::ReadSourceFile(const std::string & szFilename, unsigned int nLoadAddress, const std::string & szDFCLibrary, std::ostream *msgFile, std::ostream *errFile)
{
	if (szDFCLibrary.empty()) return false;
	if (szFilename.empty()) return false;

	std::fstream theFile;
	const CDFCObject *theDFC = CDFCArray::instance()->locateDFC(szDFCLibrary);
	bool RetVal = true;

	if (msgFile) {
		(*msgFile) << "Loading \"" << szFilename << "\" at offset "
					<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << nLoadAddress
					<< std::nouppercase << std::setbase(0) << " using " << szDFCLibrary << " library...\n";
	}
	if (theDFC == NULL) {
		if (errFile) {
			(*errFile) << "*** Error: Can't open DFC Library \"" << szDFCLibrary << "\" to read \"" << szFilename << "\"\n";
		}
		RetVal = false;
	} else {
		theFile.open(szFilename.c_str(), std::ios_base::in | std::ios_base::binary);
		if (theFile.is_open() == 0) {
			if (errFile) {
				(*errFile) << "*** Error: Can't open file \"" << szFilename << "\" for reading\n";
			}
			RetVal = false;
		} else {
			int Status = 1;
			m_nFilesLoaded++;
			m_sInputFileList.push_back(szFilename);
			try {
				Status = theDFC->ReadDataFile(&theFile, nLoadAddress, m_Memory, DMEM_LOADED);
			}
			catch (const EXCEPTION_ERROR* aErr) {
				RetVal = false;
				m_nFilesLoaded--;
				m_sInputFileList.erase(m_sInputFileList.end()-1);
				switch (aErr->cause) {
					case EXCEPTION_ERROR::ERR_CHECKSUM:
						if (errFile) {
							(*errFile) << "*** Error: Checksum error reading file \"" << szFilename << "\"\n";
						}
						break;
					case EXCEPTION_ERROR::ERR_UNEXPECTED_EOF:
						if (errFile) {
							(*errFile) << "*** Error: Unexpected end-of-file reading \"" << szFilename << "\"\n";
						}
						break;
					case EXCEPTION_ERROR::ERR_OVERFLOW:
						if (errFile) {
							(*errFile) << "*** Error: Reading file \"" << szFilename << "\" extends past the defined memory limits of this processor\n";
						}
						break;
					case EXCEPTION_ERROR::ERR_READFAILED:
						if (errFile) {
							(*errFile) << "*** Error: Reading file \"" << szFilename << "\"\n";
						}
						break;
					default:
						if (errFile) {
							(*errFile) << "*** Error: Unknown DFC Error encountered while reading file \"" << szFilename << "\"\n";
						}
				}
			}
			if (Status == 0) {
				RetVal = false;
				if (errFile) {
					(*errFile) << "*** Warning: Reading file \"" << szFilename << "\", overlaps previously loaded files\n";
				}
			}
			theFile.close();
		}
	}
	// Note, the DFC will unload when destructor is called upon exiting this routine
	return RetVal;
}


bool CDisassembler::ValidateLabelName(const std::string & aName)
{
	if (aName.empty()) return false;					// Must have a length
	if ((!isalpha(aName.at(0))) && (aName.at(0) != '_')) return false;	// Must start with alpha or '_'
	for (unsigned int i=1; i<aName.size(); ++i) {
		if ((!isalnum(aName.at(i))) && (aName.at(i) != '_')) return false;	// Each character must be alphanumeric or '_'
	}
	return true;
}

// bool CDisassembler::ResolveIndirect(unsigned int nAddress, unsigned int& nResAddress, int nType) = 0;		// Purely Virtual!
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


bool CDisassembler::IsAddressLoaded(unsigned int nAddress, int nSize)
{
	bool RetVal = true;

	for (int i=0; ((i<nSize) && (RetVal)); ++i) {
		RetVal = RetVal && (m_Memory->GetDescriptor(nAddress + i) != DMEM_NOTLOADED);
	}
	return RetVal;
}


bool CDisassembler::AddLabel(unsigned int nAddress, bool bAddRef, unsigned int nRefAddress, const std::string & szLabel)
{
	TLabelTableMap::iterator itrLabelTable = m_LabelTable.find(nAddress);
	TAddressTableMap::iterator itrRefTable = m_LabelRefTable.find(nAddress);
	if (itrLabelTable != m_LabelTable.end()) {
		if (itrRefTable == m_LabelRefTable.end()) {
			assert(false);			// We should have an array for both label strings and addresses -- check adding!
			return false;
		}
		if (bAddRef) {				// Search and add reference, even if we find the string
			bool bFound = false;
			for (unsigned int i=0; i<itrRefTable->second.size(); ++i) {
				if (itrRefTable->second.at(i) == nRefAddress) {
					bFound = true;
					break;
				}
			}
			if (!bFound) itrRefTable->second.push_back(nRefAddress);
		}
		for (unsigned int i=0; i<itrLabelTable->second.size(); ++i) {
			if (compareNoCase(szLabel, itrLabelTable->second.at(i)) == 0) return false;
		}
	} else {
		assert(itrRefTable == m_LabelRefTable.end());		// If we don't have an entry in the label tabel, we shouldn't have any reference label table
		m_LabelTable[nAddress] = TStringArray();
		itrLabelTable = m_LabelTable.find(nAddress);
		m_LabelRefTable[nAddress] = TDWordArray();
		itrRefTable = m_LabelRefTable.find(nAddress);
		if (bAddRef) {				// If we are just now adding the label, we can add the ref without searching because it doesn't exist either
			itrRefTable->second.push_back(nRefAddress);
		}
	}
	if (itrLabelTable->second.size()) {
		if (!szLabel.empty()) {
			for (TStringArray::iterator itrLabels = itrLabelTable->second.begin(); itrLabels != itrLabelTable->second.end(); ++itrLabels) {
				if (itrLabels->empty()) {
					itrLabelTable->second.erase(itrLabels);
					break;							// If adding a non-Lxxxx entry, remove previous Lxxxx entry if it exists!
				}
			}
		} else {
			return false;		// Don't set Lxxxx entry if we already have at least 1 label!
		}
	}
	itrLabelTable->second.push_back(szLabel);
	return true;
}

bool CDisassembler::AddBranch(unsigned int nAddress, bool bAddRef, unsigned int nRefAddress)
{
	TAddressTableMap::iterator itrRefList = m_BranchTable.find(nAddress);
	if (itrRefList == m_BranchTable.end()) {
		m_BranchTable[nAddress] = TDWordArray();
		itrRefList = m_BranchTable.find(nAddress);
	}
	if (bAddRef) {				// Search and add reference
		bool bFound = false;
		for (unsigned int i = 0; i < itrRefList->second.size(); ++i) {
			if (itrRefList->second.at(i) == nRefAddress) {
				bFound = true;
				break;
			}
		}
		if (!bFound) itrRefList->second.push_back(nRefAddress);
	}
	return IsAddressLoaded(nAddress, 1);
}

void CDisassembler::GenDataLabel(unsigned int nAddress, unsigned int nRefAddress, const std::string & /* szLabel */, std::ostream *msgFile, std::ostream * /* errFile */)
{
	if (AddLabel(nAddress, true, nRefAddress)) OutputGenLabel(nAddress, msgFile);
}

void CDisassembler::GenAddrLabel(unsigned int nAddress, unsigned int nRefAddress, const std::string & /* szLabel */, std::ostream *msgFile, std::ostream *errFile)
{
	if (AddLabel(nAddress, false, nRefAddress)) OutputGenLabel(nAddress, msgFile);
	if (AddBranch(nAddress, true, nRefAddress) == false) {
		if (errFile) {
			if ((errFile == msgFile) || (LAdrDplyCnt != 0)) {
				(*errFile) << "\n";
				LAdrDplyCnt = 0;
			}
			(*errFile) << "     *** Warning:  Branch Ref: "
						<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << nAddress
						<< std::nouppercase << std::setbase(0) << " is outside of Loaded Source File.\n";
		}
	}
}

void CDisassembler::OutputGenLabel(unsigned int nAddress, std::ostream *msgFile)
{
	std::string Temp;

	if (msgFile == NULL) return;
	Temp = GenLabel(nAddress);
	Temp += ' ';
	if (Temp.size() < 7) {
		Temp += "       ";
		Temp = Temp.substr(0, 7);
	}
	if (LAdrDplyCnt >= 9) {
		std::size_t nPos = Temp.find(' ');
		if (nPos != std::string::npos) Temp = Temp.substr(0, nPos);
		Temp += '\n';
		LAdrDplyCnt=0;
	} else {
		LAdrDplyCnt++;
	}

	(*msgFile) << Temp;
}

std::string CDisassembler::GenLabel(unsigned int nAddress)
{
	std::ostringstream sstrTemp;

	sstrTemp << "L" << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << nAddress;
	return sstrTemp.str();
}

bool CDisassembler::ScanEntries(std::ostream *msgFile, std::ostream *errFile)
{
	bool RetVal = true;
	unsigned int nCount = m_EntryTable.size();

	if (m_bSpitFlag) {
		m_PC = 0;		// In spit mode, we start at first address and just spit
		return FindCode(msgFile, errFile);
	}

	TAddressMap::const_iterator itrEntry = m_EntryTable.begin();
	while (RetVal && (itrEntry != m_EntryTable.end())) {
		m_PC = itrEntry->first;
		++itrEntry;

		RetVal = RetVal && FindCode(msgFile, errFile);

		if (nCount != m_EntryTable.size()) {
			nCount = m_EntryTable.size();
			itrEntry = m_EntryTable.begin();			// If one gets added during the find process, our iterator gets invalidated and we must start over...
		}
	}
	return RetVal;
}

bool CDisassembler::ScanBranches(std::ostream *msgFile, std::ostream *errFile)
{
	bool RetVal = true;
	unsigned int nCount = m_BranchTable.size();

	if (m_bSpitFlag) return true;						// Don't do anything in spit mode cause we did it in ScanEntries!

	TAddressTableMap::const_iterator itrEntry = m_BranchTable.begin();
	while (RetVal && (itrEntry != m_BranchTable.end())) {
		m_PC = itrEntry->first;
		++itrEntry;

		RetVal = RetVal && FindCode(msgFile, errFile);

		if (nCount != m_BranchTable.size()) {
			nCount = m_BranchTable.size();
			itrEntry = m_BranchTable.begin();			// If one gets added during the find process, our iterator gets invalidated and we must start over...
		}
	}
	return RetVal;
}

bool CDisassembler::ScanData(const std::string & szExcludeChars, std::ostream * /* msgFile */, std::ostream * /* errFile */)
{
	unsigned char c;

	// Note that we use an argument passed in so that the caller has a chance to change the
	//	list passed in by the GetExcludedPrintChars function!

	for (m_PC=0; m_PC<m_Memory->GetMemSize(); m_PC++) {
		if (m_Memory->GetDescriptor(m_PC) != DMEM_LOADED) continue;	// Only modify memory locations that have been loaded but not processed!
		c = m_Memory->GetByte(m_PC);
		if (isprint(c) && (szExcludeChars.find(c) == std::string::npos)) {
			m_Memory->SetDescriptor(m_PC, DMEM_PRINTDATA);
		} else {
			m_Memory->SetDescriptor(m_PC, DMEM_DATA);
		}
	}
	return true;
}

bool CDisassembler::FindCode(std::ostream *msgFile, std::ostream *errFile)
{
	bool RetVal = true;
	bool TFlag = false;

	while (TFlag == false) {
		if (m_PC >= m_Memory->GetMemSize()) {
			TFlag = true;
			continue;
		}
		if ((m_Memory->GetDescriptor(m_PC) != DMEM_LOADED) && (m_bSpitFlag == false)) {
			TFlag = true;
			continue;					// Exit when we hit an area we've already looked at
		}
		if (ReadNextObj(true, msgFile, errFile)) {		// Read next opcode and tag memory since we are finding code here
			if (((m_CurrentOpcode.m_Control & OCTL_STOP) != 0) && (m_bSpitFlag == false)) TFlag = true;
		}	// If the ReadNextObj returns false, that means we hit an illegal opcode byte.  The ReadNextObj will have incremented the m_PC past that byte, so we'll keep processing
	}

	return RetVal;
}

bool CDisassembler::ReadNextObj(bool bTagMemory, std::ostream *msgFile, std::ostream *errFile)
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
	unsigned int SavedPC;
	bool Flag;

	m_sFunctionalOpcode = "";

	FirstByte = m_Memory->GetByte(m_PC++);
	m_OpMemory.clear();
	m_OpMemory.push_back(FirstByte);
	if (IsAddressLoaded(m_PC-1, 1) == false) return false;

	COpcodeTable::const_iterator itrOpcode = m_Opcodes.find(FirstByte);
	Flag = false;
	if (itrOpcode != m_Opcodes.end()) {
		for (unsigned int ndx = 0; ((ndx < itrOpcode->second.size()) && (Flag == false)); ++ndx) {
			m_CurrentOpcode = itrOpcode->second.at(ndx);
			Flag = true;
			for (unsigned int i=1; ((i<m_CurrentOpcode.m_OpcodeBytes.size()) && (Flag)); ++i) {
				if (m_CurrentOpcode.m_OpcodeBytes.at(i) != m_Memory->GetByte(m_PC+i-1)) Flag = false;
			}
		}
	}
	if ((Flag == false) || (IsAddressLoaded(m_PC, m_CurrentOpcode.m_OpcodeBytes.size()-1) == false)) {
		if (bTagMemory) m_Memory->SetDescriptor(m_PC-1, DMEM_ILLEGALCODE);
		return false;
	}

	// If we get here, then we've found a valid matching opcode.  m_CurrentOpcode has been set to the opcode value
	//	so we must now finish copying to m_OpMemory, tag the bytes in memory, increment m_PC, and call CompleteObjRead
	//	to handle the processor dependent stuff.  If CompleteObjRead returns FALSE, then we'll have to undo everything
	//	and return flagging an invalid opcode.
	SavedPC = m_PC;		// Remember m_PC in case we have to undo things (remember, m_PC has already been incremented by 1)
	for (unsigned int i=1; i<m_CurrentOpcode.m_OpcodeBytes.size(); ++i) {			// Finish copying opcode, but don't tag memory until dependent code is called successfully
		m_OpMemory.push_back(m_Memory->GetByte(m_PC++));
	}

	if ((CompleteObjRead(true, msgFile, errFile) == false) || (IsAddressLoaded(SavedPC-1, m_OpMemory.size()) == false))  {
		// Undo things here:
		m_OpMemory.clear();
		m_OpMemory.push_back(FirstByte);		// Keep only the first byte in OpMemory for illegal opcode id
		m_PC = SavedPC;
		if (bTagMemory) m_Memory->SetDescriptor(m_PC-1, DMEM_ILLEGALCODE);
		return false;
	}


//	assert(CompleteObjRead(true, msgFile, errFile));


	SavedPC--;
	for (unsigned int i=0; i<m_OpMemory.size(); ++i) {		// CompleteObjRead will add bytes to OpMemory, so we simply have to flag memory for that many bytes.  m_PC is already incremented by CompleteObjRead
		if (bTagMemory) m_Memory->SetDescriptor(SavedPC, DMEM_CODE);
		SavedPC++;
	}
	assert(SavedPC == m_PC);		// If these aren't equal, something is wrong in the CompleteObjRead!  m_PC should be incremented for every byte added to m_OpMemory by the complete routine!
	return true;
}

bool CDisassembler::Disassemble(std::ostream *msgFile, std::ostream *errFile, std::ostream *outFile)
{
	bool RetVal = true;
	std::ostream *theOutput;
	std::fstream aOutput;
	std::string Temp;

	if (outFile) {
		theOutput = outFile;
	} else {
		aOutput.open(m_sOutputFilename.c_str(), std::ios_base::out | std::ios_base::trunc);
		if (!aOutput.is_open()) {
			if (errFile) {
				(*errFile) << "\n*** Error: Opening file \"" << m_sOutputFilename << "\" for writing...\n";
			}
			return false;
		}
		theOutput = &aOutput;
	}

	while (1) {		// Setup dummy endless loop so we can use the break
		if (msgFile) (*msgFile) << "\nPass 1 - Finding Code, Data, and Labels...\n";
		RetVal = Pass1(*theOutput, msgFile, errFile);
		if (!RetVal) break;

		if (msgFile) {
			(*msgFile) << ((LAdrDplyCnt != 0) ? '\n' : ' ') << "\nPass 2 - Disassembling to Output File...\n";
		}
		RetVal = Pass2(*theOutput, msgFile, errFile);
		if (!RetVal) break;

		if (!m_sFunctionsFilename.empty()) {
			if (*msgFile) {
				(*msgFile) << ((LAdrDplyCnt != 0) ? '\n' : ' ') << "\nPass 3 - Creating Functions Output File...\n";
			}

			RetVal = Pass3(*theOutput, msgFile, errFile);
			if (!RetVal) break;
		}

		if (msgFile) {
			(*msgFile) << "\nDisassembly Complete\n";
		}

		// Add additional Passes here and end this loop with a break
		break;
	}

	if (RetVal == false) {
		(*theOutput) << "\n*** Internal error encountered while disassembling.\n";
		if (errFile) (*errFile) << "\n*** Internal error encountered while disassembling.\n";
	}

	if (aOutput.is_open()) aOutput.close();		// Close the output file if we opened it

	return RetVal;
}

bool CDisassembler::Pass1(std::ostream& /* outFile */, std::ostream *msgFile, std::ostream *errFile)
{
	bool RetVal = true;

	// Note that Short-Circuiting will keep following process stages from being called in the even of an error!
	RetVal = RetVal && ScanEntries(msgFile, errFile);
	RetVal = RetVal && ScanBranches(msgFile, errFile);
	RetVal = RetVal && ScanData(GetExcludedPrintChars(), msgFile, errFile);

	return RetVal;
}

bool CDisassembler::Pass2(std::ostream& outFile, std::ostream *msgFile, std::ostream *errFile)
{
	bool RetVal = true;

	// Note that Short-Circuiting will keep following process stages from being called in the even of an error!
	RetVal = RetVal && WriteHeader(outFile, msgFile, errFile);
	RetVal = RetVal && WriteEquates(outFile, msgFile, errFile);
	RetVal = RetVal && WriteDisassembly(outFile, msgFile, errFile);

	return RetVal;
}

bool CDisassembler::Pass3(std::ostream& /* outFile */, std::ostream *msgFile, std::ostream *errFile)
{
	bool RetVal = true;
	std::fstream aFunctionsFile;
	bool bInFunc;
	bool bLastFlag;
	bool bBranchOutFlag;
	unsigned int nFuncAddr;
	unsigned int nSavedPC;
	TMemRange *pMemRange;
	bool bTempFlag;
	bool bTempFlag2;

	aFunctionsFile.open(m_sFunctionsFilename.c_str(), std::ios_base::out | std::ios_base::trunc);
	if (!aFunctionsFile.is_open()) {
		if (errFile) {
			(*errFile) << "\n*** Error: Opening file \"" << m_sFunctionsFilename << "\" for writing...\n";
		}
		return false;
	}

	aFunctionsFile << ";\n";
	aFunctionsFile << "; " << GetGDCLongName() << " Generated Function Information File\n";
	aFunctionsFile << ";\n";
	aFunctionsFile << ";    Control File" << ((m_sControlFileList.size() > 1) ? "s:" : ": ") << " ";
	for (unsigned int i=0; i<m_sControlFileList.size(); ++i) {
		if (i==0) {
			aFunctionsFile << m_sControlFileList.at(i) << "\n";
		} else {
			aFunctionsFile << ";                  " << m_sControlFileList.at(i) << "\n";
		}
	}
	aFunctionsFile << ";      Input File" << ((m_sInputFileList.size() > 1) ? "s:" : ": ") << " ";
	for (unsigned int i=0; i<m_sInputFileList.size(); ++i) {
		if (i==0) {
			aFunctionsFile << m_sInputFileList.at(i) << "\n";
		} else {
			aFunctionsFile << ";                  " << m_sInputFileList.at(i) << "\n";
		}
	}
	aFunctionsFile << ";     Output File:  " << m_sOutputFilename << "\n";
	aFunctionsFile << ";  Functions File:  " << m_sFunctionsFilename << "\n";
	aFunctionsFile << ";\n";

	aFunctionsFile << "; Memory Mappings:";

	aFunctionsFile << "  ROM Memory Map: ";
	if (m_ROMMemMap.IsNullRange()) {
		aFunctionsFile << "<Not Defined>\n";
	} else {
		aFunctionsFile << "\n";

		pMemRange = &m_ROMMemMap;
		while (pMemRange) {
			aFunctionsFile << ";                       "
							<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << pMemRange->GetStartAddr()
							<< std::nouppercase << std::setbase(0) << " - "
							<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << (pMemRange->GetStartAddr() + pMemRange->GetSize() - 1)
							<< std::nouppercase << std::setbase(0) << "  (Size: "
							<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << pMemRange->GetSize()
							<< std::nouppercase << std::setbase(0) << ")\n";
			pMemRange = pMemRange->GetNext();
		}
	}

	aFunctionsFile << ";                   RAM Memory Map: ";
	if (m_RAMMemMap.IsNullRange()) {
		aFunctionsFile << "<Not Defined>\n";
	} else {
		aFunctionsFile << "\n";

		pMemRange = &m_RAMMemMap;
		while (pMemRange) {
			aFunctionsFile << ";                       "
							<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << pMemRange->GetStartAddr()
							<< std::nouppercase << std::setbase(0) << " - "
							<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << (pMemRange->GetStartAddr() + pMemRange->GetSize() - 1)
							<< std::nouppercase << std::setbase(0) << "  (Size: "
							<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << pMemRange->GetSize()
							<< std::nouppercase << std::setbase(0) << ")\n";
			pMemRange = pMemRange->GetNext();
		}
	}

	aFunctionsFile << ";                    IO Memory Map: ";
	if (m_IOMemMap.IsNullRange()) {
		aFunctionsFile << "<Not Defined>\n";
	} else {
		aFunctionsFile << "\n";

		pMemRange = &m_IOMemMap;
		while (pMemRange) {
			aFunctionsFile << ";                       "
							<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << pMemRange->GetStartAddr()
							<< std::nouppercase << std::setbase(0) << " - "
							<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << (pMemRange->GetStartAddr() + pMemRange->GetSize() - 1)
							<< std::nouppercase << std::setbase(0) << "  (Size: "
							<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << pMemRange->GetSize()
							<< std::nouppercase << std::setbase(0) << ")\n";
			pMemRange = pMemRange->GetNext();
		}
	}

	aFunctionsFile << ";\n";
	aFunctionsFile << ";       Generated:  " << std::ctime(&m_StartTime);		// Note: std::ctime adds extra \n character, so no need to add our own
	aFunctionsFile << ";\n\n";

	bTempFlag = false;
	if (!m_ROMMemMap.IsNullRange()) {
		pMemRange = &m_ROMMemMap;
		while (pMemRange) {
			std::ostringstream sstrTemp;

			sstrTemp << "#ROM|" << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << pMemRange->GetStartAddr();
			sstrTemp << "|"  << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << pMemRange->GetSize();
			sstrTemp << "\n";
			m_sFunctionalOpcode = sstrTemp.str();

			aFunctionsFile << m_sFunctionalOpcode;
			pMemRange = pMemRange->GetNext();
		}
		bTempFlag = true;
	}
	if (!m_RAMMemMap.IsNullRange()) {
		pMemRange = &m_RAMMemMap;
		while (pMemRange) {
			std::ostringstream sstrTemp;

			sstrTemp << "#RAM|" << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << pMemRange->GetStartAddr();
			sstrTemp << "|"  << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << pMemRange->GetSize();
			sstrTemp << "\n";
			m_sFunctionalOpcode = sstrTemp.str();

			aFunctionsFile << m_sFunctionalOpcode;
			pMemRange = pMemRange->GetNext();
		}
		bTempFlag = true;
	}
	if (!m_IOMemMap.IsNullRange()) {
		pMemRange = &m_IOMemMap;
		while (pMemRange) {
			std::ostringstream sstrTemp;

			sstrTemp << "#IO|" << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << pMemRange->GetStartAddr();
			sstrTemp << "|"  << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << pMemRange->GetSize();
			sstrTemp << "\n";
			m_sFunctionalOpcode = sstrTemp.str();

			aFunctionsFile << m_sFunctionalOpcode;
			pMemRange = pMemRange->GetNext();
		}
		bTempFlag = true;
	}
	if (bTempFlag) aFunctionsFile << "\n";

	bTempFlag = false;
	for (m_PC = 0; ((m_PC < m_Memory->GetMemSize()) && (RetVal)); m_PC++) {
		TLabelTableMap::const_iterator itrLabels = m_LabelTable.find(m_PC);
		if (itrLabels != m_LabelTable.end()) {
			std::ostringstream sstrTemp;
			bTempFlag2 = false;
			sstrTemp << "!" << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << m_PC;
			sstrTemp << std::nouppercase << std::setbase(0) << "|";
			for (unsigned int i = 0; i < itrLabels->second.size(); ++i) {
				if (bTempFlag2) sstrTemp << ",";
				if (!itrLabels->second.at(i).empty()) {
					sstrTemp << itrLabels->second.at(i);
					bTempFlag2 = true;
				}
			}
			sstrTemp << "\n";
			m_sFunctionalOpcode = sstrTemp.str();
			if (bTempFlag2) aFunctionsFile << m_sFunctionalOpcode;
			bTempFlag = true;
		}
	}
	if (bTempFlag) aFunctionsFile << "\n";

	bInFunc = false;
	for (m_PC = 0; ((m_PC < m_Memory->GetMemSize()) && (RetVal)); ) {
		bLastFlag = false;
		bBranchOutFlag = false;

		// Check for function start/end flags:
		TAddressMap::const_iterator itrFunction = m_FunctionsTable.find(m_PC);
		if (itrFunction != m_FunctionsTable.end()) {
			switch (itrFunction->second) {
				case FUNCF_HARDSTOP:
				case FUNCF_EXITBRANCH:
				case FUNCF_SOFTSTOP:
					bInFunc = false;
					bLastFlag = true;
					break;

				case FUNCF_BRANCHOUT:
					bBranchOutFlag = true;
					break;

				case FUNCF_BRANCHIN:
					if (bInFunc) break;		// Continue if already inside a function
					// Else, fall-through and setup for new function:
				case FUNCF_ENTRY:
				case FUNCF_INDIRECT:
				case FUNCF_CALL:
				{
					std::ostringstream sstrTemp;
					sstrTemp << "@" << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << m_PC << "|";
					m_sFunctionalOpcode = sstrTemp.str();

					TLabelTableMap::const_iterator itrLabels = m_LabelTable.find(m_PC);
					if (itrLabels != m_LabelTable.end()) {
						for (unsigned int i = 0; i < itrLabels->second.size(); ++i) {
							if (i != 0) m_sFunctionalOpcode += ",";
							m_sFunctionalOpcode += FormatLabel(LC_REF, itrLabels->second.at(i), m_PC);
						}
					} else {
						m_sFunctionalOpcode += "???";
					}

					aFunctionsFile << m_sFunctionalOpcode;
					aFunctionsFile << "\n";

					nFuncAddr = m_PC;
					bInFunc = true;
					break;
				}

				default:
					assert(false);			// Unexpected Function Flag!!  Check Code!!
					RetVal = false;
					continue;
			}
		}

		nSavedPC = m_PC;
		m_sFunctionalOpcode = "";
		switch (m_Memory->GetDescriptor(m_PC)) {
			case DMEM_NOTLOADED:
				m_PC++;
				bLastFlag = bInFunc;
				bInFunc = false;
				break;
			case DMEM_LOADED:
				assert(false);		// WARNING!  All loaded code should have been evaluated.  Check override code!
				RetVal = false;
				m_PC++;
				break;
			case DMEM_DATA:
			case DMEM_PRINTDATA:
			case DMEM_CODEINDIRECT:
			case DMEM_DATAINDIRECT:
			case DMEM_ILLEGALCODE:
			{
				std::ostringstream sstrTemp;
				sstrTemp << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << m_PC << "|";
				m_sFunctionalOpcode = sstrTemp.str();

				TLabelTableMap::const_iterator itrLabels = m_LabelTable.find(m_PC);
				if (itrLabels != m_LabelTable.end()) {
					for (unsigned int i=0; i<itrLabels->second.size(); ++i) {
						if (i != 0) m_sFunctionalOpcode += ",";
						m_sFunctionalOpcode += FormatLabel(LC_REF, itrLabels->second.at(i), m_PC);
					}
				}

				sstrTemp.str(std::string());
				sstrTemp << std::uppercase << std::setfill('0') << std::setw(2) << std::setbase(16) << m_Memory->GetByte(m_PC) << "|";
				m_sFunctionalOpcode += sstrTemp.str();

				m_PC++;
				break;
			}
			case DMEM_CODE:
				RetVal = ReadNextObj(false, msgFile, errFile);
				break;
			default:
				bInFunc = false;
				RetVal = false;
				break;
		}

		if (!m_sFunctionalOpcode.empty()) {
			std::ostringstream sstrTemp;
			sstrTemp << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << (nSavedPC - nFuncAddr) << "|";
			m_sFunctionalOpcode = sstrTemp.str() + m_sFunctionalOpcode;
		}

		if (bBranchOutFlag) {
			TAddressMap::const_iterator itrFunction = m_FunctionsTable.find(m_PC);
			if (itrFunction != m_FunctionsTable.end()) {
				switch (itrFunction->second) {
					case FUNCF_ENTRY:
					case FUNCF_INDIRECT:
					case FUNCF_CALL:
						bInFunc = false;
						bLastFlag = true;
						break;
				}
			} else {
				bInFunc = false;
				bLastFlag = true;
			}
		}

		if (bLastFlag) {
			TAddressMap::const_iterator itrFunction = m_FunctionsTable.find(m_PC);
			if (itrFunction != m_FunctionsTable.end()) {
				switch (itrFunction->second) {
					case FUNCF_BRANCHIN:
						bLastFlag = false;
						bInFunc = true;
						break;
				}
			}
		}

		if ((bInFunc) || (bLastFlag)) {
			if (!m_sFunctionalOpcode.empty()) {
				aFunctionsFile << m_sFunctionalOpcode;
				aFunctionsFile << "\n";
			}
			if (bLastFlag) aFunctionsFile << "\n";
		}
	}

	aFunctionsFile.close();

	return RetVal;
}

std::string CDisassembler::FormatComments(MNEMONIC_CODE /* nMCCode */)		// The default returns the reference comment string.
{
	return FormatReferences(m_PC - m_OpMemory.size());
}

std::string CDisassembler::FormatAddress(unsigned int nAddress)
{
	std::ostringstream sstrTemp;

	sstrTemp << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << nAddress;
	return sstrTemp.str();
}

std::string CDisassembler::FormatOpBytes()
{
	std::ostringstream sstrTemp;

	for (unsigned int i=0; i<m_OpMemory.size(); ++i) {
		sstrTemp << std::uppercase << std::setfill('0') << std::setw(2) << std::setbase(16) << static_cast<unsigned int>(m_OpMemory.at(i))
					<< std::nouppercase << std::setbase(0);
		if (i < m_OpMemory.size()-1) sstrTemp << " ";
	}
	return sstrTemp.str();
}

std::string CDisassembler::FormatLabel(LABEL_CODE /* nLC */, const std::string & szLabel, unsigned int nAddress)
{
	std::ostringstream sstrTemp;

	if (szLabel.empty()) {
		sstrTemp << "L" << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << nAddress;
	} else {
		sstrTemp << szLabel;
	}

	return sstrTemp.str();
}

std::string CDisassembler::FormatReferences(unsigned int nAddress)
{
	std::string RetVal;
	bool Flag;

	RetVal = "";
	Flag = false;
	TAddressTableMap::const_iterator itrBranches = m_BranchTable.find(nAddress);
	if (itrBranches != m_BranchTable.end()) {
		if (itrBranches->second.size() != 0) {
			RetVal += "CRef: ";
			Flag = true;
			for (unsigned int i=0; i<itrBranches->second.size(); ++i) {
				std::ostringstream sstrTemp;
				sstrTemp << GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << itrBranches->second.at(i);
				RetVal += sstrTemp.str();
				if (i < itrBranches->second.size()-1) RetVal += ",";
			}
		}
	}

	if (m_LabelTable.find(nAddress) != m_LabelTable.end()) {
		TAddressTableMap::const_iterator itrLabelRef = m_LabelRefTable.find(nAddress);
		if (itrLabelRef == m_LabelRefTable.end()) {
			assert(false);		// Should also have a ref entry!
			return RetVal;
		}

		if (itrLabelRef->second.size() != 0) {
			if (Flag) RetVal += "; ";
			RetVal += "DRef: ";
			Flag = true;
			for (unsigned int i=0; i<itrLabelRef->second.size(); ++i) {
				std::ostringstream sstrTemp;
				sstrTemp << GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << itrLabelRef->second.at(i);
				RetVal += sstrTemp.str();

				if (i < itrLabelRef->second.size()-1) RetVal += ",";
			}
		}
	}

	return RetVal;
}


bool CDisassembler::WriteHeader(std::ostream& outFile, std::ostream * /* msgFile */, std::ostream * /* errFile */)
{
	TStringArray OutLine;
	TMemRange *pMemRange;

	ClearOutputLine(OutLine);
	OutLine[FC_ADDRESS] = FormatAddress(0);

	OutLine[FC_LABEL] = GetCommentStartDelim() + GetCommentEndDelim() + "\n";
	outFile << MakeOutputLine(OutLine) << "\n";
	OutLine[FC_LABEL] = GetCommentStartDelim() + " " + GetGDCLongName() + " Generated Source Code " + GetCommentEndDelim() + "\n";
	outFile << MakeOutputLine(OutLine) << "\n";
	OutLine[FC_LABEL] = GetCommentStartDelim() + GetCommentEndDelim() + "\n";
	outFile << MakeOutputLine(OutLine) << "\n";
	OutLine[FC_LABEL] = GetCommentStartDelim() + "    Control File";
	OutLine[FC_LABEL] += ((m_sControlFileList.size() > 1) ? "s: " : ":  ");
	for (unsigned int i=0; i<m_sControlFileList.size(); ++i) {
		if (i==0) {
			OutLine[FC_LABEL] += m_sControlFileList.at(i) + "  " + GetCommentEndDelim() + "\n";
		} else {
			OutLine[FC_LABEL] = GetCommentStartDelim() + "                  " + m_sControlFileList.at(i) + "  " + GetCommentEndDelim() + "\n";
		}
		outFile << MakeOutputLine(OutLine) << "\n";
	}
	OutLine[FC_LABEL] = GetCommentStartDelim() + "      Input File";
	OutLine[FC_LABEL] += ((m_sInputFileList.size() > 1) ? "s: " : ":  ");
	for (unsigned int i=0; i<m_sInputFileList.size(); ++i) {
		if (i==0) {
			OutLine[FC_LABEL] += m_sInputFileList.at(i) +  "  " + GetCommentEndDelim() + "\n";
		} else {
			OutLine[FC_LABEL] = GetCommentStartDelim() + "                  " + m_sInputFileList.at(i) + "  " + GetCommentEndDelim() + "\n";
		}
		outFile << MakeOutputLine(OutLine) << "\n";
	}
	OutLine[FC_LABEL] = GetCommentStartDelim() + "     Output File:  " + m_sOutputFilename + "  " + GetCommentEndDelim() + "\n";
	outFile << MakeOutputLine(OutLine) << "\n";
	if (!m_sFunctionsFilename.empty()) {
		OutLine[FC_LABEL] = GetCommentStartDelim() + "  Functions File:  " + m_sFunctionsFilename + "  " + GetCommentEndDelim() + "\n";
		outFile << MakeOutputLine(OutLine) << "\n";
	}
	OutLine[FC_LABEL] = GetCommentStartDelim() + GetCommentEndDelim() + "\n";
	outFile << MakeOutputLine(OutLine) << "\n";

	OutLine[FC_LABEL] = GetCommentStartDelim() + " Memory Mappings:";

	OutLine[FC_LABEL] += "  ROM Memory Map: ";
	if (m_ROMMemMap.IsNullRange()) {
		OutLine[FC_LABEL] += "<Not Defined>" + GetCommentEndDelim() + "\n";
		outFile << MakeOutputLine(OutLine) << "\n";
	} else {
		OutLine[FC_LABEL] += GetCommentEndDelim() + "\n";
		outFile << MakeOutputLine(OutLine) << "\n";

		pMemRange = &m_ROMMemMap;
		while (pMemRange) {
			std::ostringstream sstrTemp;
			sstrTemp << GetCommentStartDelim() << "                       "
			<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << pMemRange->GetStartAddr()
			<< std::nouppercase << std::setbase(0) << " - "
			<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << (pMemRange->GetStartAddr() + pMemRange->GetSize() - 1)
			<< std::nouppercase << std::setbase(0) << "  (Size: "
			<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << pMemRange->GetSize()
			<< std::nouppercase << std::setbase(0) << ")  " << GetCommentEndDelim() << "\n";
			OutLine[FC_LABEL] = sstrTemp.str();
			outFile << MakeOutputLine(OutLine) << "\n";
			pMemRange = pMemRange->GetNext();
		}
	}

	OutLine[FC_LABEL] = GetCommentStartDelim() + "                   RAM Memory Map: ";
	if (m_RAMMemMap.IsNullRange()) {
		OutLine[FC_LABEL] += "<Not Defined>" + GetCommentEndDelim() + "\n";
		outFile << MakeOutputLine(OutLine) << "\n";
	} else {
		OutLine[FC_LABEL] += GetCommentEndDelim() + "\n";
		outFile << MakeOutputLine(OutLine) << "\n";

		pMemRange = &m_RAMMemMap;
		while (pMemRange) {
			std::ostringstream sstrTemp;
			sstrTemp << GetCommentStartDelim() << "                       "
			<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << pMemRange->GetStartAddr()
			<< std::nouppercase << std::setbase(0) << " - "
			<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << (pMemRange->GetStartAddr() + pMemRange->GetSize() - 1)
			<< std::nouppercase << std::setbase(0) << "  (Size: "
			<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << pMemRange->GetSize()
			<< std::nouppercase << std::setbase(0) << ")  " << GetCommentEndDelim() << "\n";
			OutLine[FC_LABEL] = sstrTemp.str();
			outFile << MakeOutputLine(OutLine) << "\n";
			pMemRange = pMemRange->GetNext();
		}
	}

	OutLine[FC_LABEL] = GetCommentStartDelim() + "                    IO Memory Map: ";
	if (m_IOMemMap.IsNullRange()) {
		OutLine[FC_LABEL] += "<Not Defined>" + GetCommentEndDelim() + "\n";
		outFile << MakeOutputLine(OutLine) << "\n";
	} else {
		OutLine[FC_LABEL] += GetCommentEndDelim() + "\n";
		outFile << MakeOutputLine(OutLine) << "\n";

		pMemRange = &m_IOMemMap;
		while (pMemRange) {
			std::ostringstream sstrTemp;
			sstrTemp << GetCommentStartDelim() << "                       "
			<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << pMemRange->GetStartAddr()
			<< std::nouppercase << std::setbase(0) << " - "
			<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << (pMemRange->GetStartAddr() + pMemRange->GetSize() - 1)
			<< std::nouppercase << std::setbase(0) << "  (Size: "
			<< GetHexDelim() << std::uppercase << std::setfill('0') << std::setw(4) << std::setbase(16) << pMemRange->GetSize()
			<< std::nouppercase << std::setbase(0) << ")  " << GetCommentEndDelim() << "\n";
			OutLine[FC_LABEL] = sstrTemp.str();
			outFile << MakeOutputLine(OutLine) << "\n";
			pMemRange = pMemRange->GetNext();
		}
	}

	OutLine[FC_LABEL] = GetCommentStartDelim() + GetCommentEndDelim() + "\n";
	outFile << MakeOutputLine(OutLine) << "\n";
	OutLine[FC_LABEL] = GetCommentStartDelim() + "       Generated:  ";
	OutLine[FC_LABEL] += std::ctime(&m_StartTime);
	OutLine[FC_LABEL].erase(OutLine[FC_LABEL].end()-1);		// Note: std::ctime adds extra \n character
	OutLine[FC_LABEL] += GetCommentEndDelim() + "\n";
	outFile << MakeOutputLine(OutLine) << "\n";
	OutLine[FC_LABEL] = GetCommentStartDelim() + GetCommentEndDelim() + "\n";
	outFile << MakeOutputLine(OutLine) << "\n";


	OutLine[FC_LABEL].clear();
	outFile << MakeOutputLine(OutLine) << "\n";
	outFile << MakeOutputLine(OutLine) << "\n";
	return true;
}

bool CDisassembler::WritePreEquates(std::ostream& /* outFile */, std::ostream * /* msgFile */, std::ostream * /* errFile */)		// Don't do anything by default.
{
	return true;
}

bool CDisassembler::WriteEquates(std::ostream& outFile, std::ostream *msgFile, std::ostream *errFile)
{
	TStringArray OutLine;
	bool RetVal;

	m_OpMemory.clear();			// Remove bytes so all code will properly reference first and last PC addresses

	RetVal = false;
	if (WritePreEquates(outFile, msgFile, errFile)) {
		RetVal = true;
		ClearOutputLine(OutLine);
		for (m_PC = 0; m_PC < m_Memory->GetMemSize(); m_PC++) {
			assert(m_Memory->GetDescriptor(m_PC) != DMEM_LOADED);		// Find Routines didn't find and analyze all memory!!  Fix it!
			if (m_Memory->GetDescriptor(m_PC) != DMEM_NOTLOADED) continue;		// Loaded addresses will get outputted during main part

			TLabelTableMap::const_iterator itrLabels = m_LabelTable.find(m_PC);
			if (itrLabels != m_LabelTable.end()) {
				OutLine[FC_ADDRESS] = FormatAddress(m_PC);
				for (unsigned int i=0; i<itrLabels->second.size(); ++i) {
					OutLine[FC_LABEL] = FormatLabel(LC_EQUATE, itrLabels->second.at(i), m_PC);
					OutLine[FC_MNEMONIC] = FormatMnemonic(MC_EQUATE);
					OutLine[FC_OPERANDS] = FormatOperands(MC_EQUATE);
					OutLine[FC_COMMENT] = FormatComments(MC_EQUATE);
					outFile << MakeOutputLine(OutLine) << "\n";
				}
			}
		}
		RetVal = RetVal && WritePostEquates(outFile, msgFile, errFile);
	}
	return RetVal;
}

bool CDisassembler::WritePostEquates(std::ostream& /* outFile */, std::ostream * /* msgFile */, std::ostream * /* errFile */)		// Don't do anything by default
{
	return true;
}

bool CDisassembler::WritePreDisassembly(std::ostream& outFile, std::ostream * /* msgFile */, std::ostream * /* errFile */)
{
	TStringArray OutLine;

	ClearOutputLine(OutLine);
	OutLine[FC_ADDRESS] = FormatAddress(0);

	outFile << MakeOutputLine(OutLine) << "\n";
	outFile << MakeOutputLine(OutLine) << "\n";
	return true;
}

bool CDisassembler::WriteDisassembly(std::ostream& outFile, std::ostream *msgFile, std::ostream *errFile)
{
	bool RetVal;

	RetVal = false;
	if (WritePreDisassembly(outFile, msgFile, errFile)) {
		RetVal = true;

		for (m_PC = 0; ((m_PC < m_Memory->GetMemSize()) && (RetVal)); ) {	// WARNING!! WriteSection MUST increment m_PC
			switch (m_Memory->GetDescriptor(m_PC)) {
				case DMEM_NOTLOADED:
					m_PC++;
					break;
				case DMEM_LOADED:
					assert(false);		// WARNING!  All loaded code should have been evaluated.  Check override code!
					RetVal = false;
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

bool CDisassembler::WritePostDisassembly(std::ostream& /* outFile */, std::ostream * /* msgFile */, std::ostream * /* errFile */)		// Don't do anything by default
{
	return true;
}

bool CDisassembler::WritePreSection(std::ostream& /* outFile */, std::ostream * /* msgFile */, std::ostream * /* errFile */)		// Don't do anything by default
{
	return true;
}

bool CDisassembler::WriteSection(std::ostream& outFile, std::ostream *msgFile, std::ostream *errFile)
{
	bool RetVal;
	bool Done;

	RetVal = false;
	if (WritePreSection(outFile, msgFile, errFile)) {
		RetVal = true;

		Done = false;
		while ((m_PC < m_Memory->GetMemSize()) && (Done == false) && (RetVal)) {
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
					Done = true;
					break;
			}
		}

		RetVal = RetVal && WritePostSection(outFile, msgFile, errFile);
	}

	return RetVal;
}

bool CDisassembler::WritePostSection(std::ostream& /* outFile */, std::ostream * /* msgFile */, std::ostream * /* errFile */)		// Don't do anything by default
{
	return true;
}

bool CDisassembler::WritePreDataSection(std::ostream& /* outFile */, std::ostream * /* msgFile */, std::ostream * /* errFile */)		// Don't do anything by default
{
	return true;
}

bool CDisassembler::WriteDataSection(std::ostream& outFile, std::ostream *msgFile, std::ostream *errFile)
{
	bool RetVal;
	TStringArray OutLine;
	bool Done;
	unsigned int SavedPC;
	int Count;
	bool Flag;
	MEM_DESC Code;
	std::string TempLabel;
	TByteArray TempOpMemory;

	ClearOutputLine(OutLine);

	RetVal = false;
	if (WritePreDataSection(outFile, msgFile, errFile)) {
		RetVal = true;

		Done = false;
		while ((m_PC < m_Memory->GetMemSize()) && (Done == false) && (RetVal)) {
			ClearOutputLine(OutLine);
			OutLine[FC_ADDRESS] = FormatAddress(m_PC);
			TLabelTableMap::const_iterator itrLabels = m_LabelTable.find(m_PC);
			if (itrLabels != m_LabelTable.end()) {
				for (unsigned int i=1; i<itrLabels->second.size(); ++i) {
					OutLine[FC_LABEL] = FormatLabel(LC_DATA, itrLabels->second.at(i), m_PC);
					outFile << MakeOutputLine(OutLine) << "\n";
				}
				if (itrLabels->second.size()) OutLine[FC_LABEL] = FormatLabel(LC_DATA, itrLabels->second.at(0), m_PC);
			}

			SavedPC = m_PC;		// Keep a copy of the PC for this line because some calls will be incrementing our m_PC
			Code = (MEM_DESC)m_Memory->GetDescriptor(m_PC);
			if ((m_bAsciiFlag == false) && (Code == DMEM_PRINTDATA)) Code = DMEM_DATA;		// If not doing ASCII, treat print data as data
			switch (Code) {
				case DMEM_DATA:
					Count = 0;
					m_OpMemory.clear();
					Flag = false;
					while (Flag == false) {
						m_OpMemory.push_back(m_Memory->GetByte(m_PC++));
						Count++;
						// Stop on this line when we've either run out of data, hit the specified line limit, or hit another label
						if (Count >= m_nMaxNonPrint) Flag = true;
						if (m_LabelTable.find(m_PC) != m_LabelTable.end()) Flag = true;
						Code = (MEM_DESC)m_Memory->GetDescriptor(m_PC);
						if ((m_bAsciiFlag == false) && (Code == DMEM_PRINTDATA)) Code = DMEM_DATA;		// If not doing ASCII, treat print data as data
						if (Code != DMEM_DATA) Flag = true;
					}
					if (m_bDataOpBytesFlag) OutLine[FC_OPBYTES] = FormatOpBytes();
					OutLine[FC_MNEMONIC] = FormatMnemonic(MC_DATABYTE);
					OutLine[FC_OPERANDS] = FormatOperands(MC_DATABYTE);
					OutLine[FC_COMMENT] = FormatComments(MC_DATABYTE);
					outFile << MakeOutputLine(OutLine) << "\n";
					break;
				case DMEM_PRINTDATA:
					Count = 0;
					m_OpMemory.clear();
					Flag = false;
					while (Flag == false) {
						m_OpMemory.push_back(m_Memory->GetByte(m_PC++));
						Count++;
						// Stop on this line when we've either run out of data, hit the specified line limit, or hit another label
						if (Count >= m_nMaxPrint) Flag = true;
						if (m_LabelTable.find(m_PC) != m_LabelTable.end()) Flag = true;
						if (m_Memory->GetDescriptor(m_PC) != DMEM_PRINTDATA) Flag = true;
					}
					// First, print a line of the output bytes for reference:
					if (m_bAsciiBytesFlag) {
						TempLabel = OutLine[FC_LABEL];
						TempOpMemory = m_OpMemory;

						Count = TempOpMemory.size();
						while (Count) {
							m_OpMemory.clear();
							for (int i=0; ((i<Count) && (i<m_nMaxNonPrint)); ++i)
								m_OpMemory.push_back(TempOpMemory.at(TempOpMemory.size()-Count+i));
							OutLine[FC_LABEL] = GetCommentStartDelim() + " " + TempLabel +
												((!TempLabel.empty()) ? " " : "") +
												FormatOperands(MC_DATABYTE) + " " + GetCommentEndDelim();
							OutLine[FC_ADDRESS] = FormatAddress(m_PC-Count);
							OutLine[FC_MNEMONIC] = "";
							OutLine[FC_OPERANDS] = "";
							OutLine[FC_COMMENT] = "";
							outFile << MakeOutputLine(OutLine) << "\n";
							Count -= m_OpMemory.size();
						}

						OutLine[FC_LABEL] = TempLabel;
						m_OpMemory = TempOpMemory;
					}

					// Then, print the line as it should be in the ASCII equivalent:
					OutLine[FC_ADDRESS] = FormatAddress(SavedPC);
					if (m_bDataOpBytesFlag) OutLine[FC_OPBYTES] = FormatOpBytes();
					OutLine[FC_MNEMONIC] = FormatMnemonic(MC_ASCII);
					OutLine[FC_OPERANDS] = FormatOperands(MC_ASCII);
					OutLine[FC_COMMENT] = FormatComments(MC_ASCII);
					outFile << MakeOutputLine(OutLine) << "\n";
					break;
				case DMEM_CODEINDIRECT:
				case DMEM_DATAINDIRECT:
					// If the following call returns false, that means that the user (or
					//	special indirect detection logic) erroneously specified (or detected)
					//	the indirect.  So instead of throwing an error or causing problems,
					//	we will treat it as a data byte instead:
					if (RetrieveIndirect(msgFile, errFile) == false) {		// Bumps PC
						if (m_bDataOpBytesFlag) OutLine[FC_OPBYTES] = FormatOpBytes();
						OutLine[FC_MNEMONIC] = FormatMnemonic(MC_DATABYTE);
						OutLine[FC_OPERANDS] = FormatOperands(MC_DATABYTE);
						OutLine[FC_COMMENT] = FormatComments(MC_DATABYTE) + " -- Erroneous Indirect Stub";
						outFile << MakeOutputLine(OutLine) << "\n";
						break;
					}

					if (m_bDataOpBytesFlag) OutLine[FC_OPBYTES] = FormatOpBytes();
					OutLine[FC_MNEMONIC] = FormatMnemonic(MC_INDIRECT);
					OutLine[FC_OPERANDS] = FormatOperands(MC_INDIRECT);
					OutLine[FC_COMMENT] = FormatComments(MC_INDIRECT);
					outFile << MakeOutputLine(OutLine) << "\n";
					break;

				default:
					Done = true;
			}
		}

		RetVal = RetVal && WritePostDataSection(outFile, msgFile, errFile);
	}

	return RetVal;
}

bool CDisassembler::WritePostDataSection(std::ostream& /* outFile */, std::ostream * /* msgFile */, std::ostream * /* errFile */)		// Don't do anything by default
{
	return true;
}

bool CDisassembler::WritePreCodeSection(std::ostream& /* outFile */, std::ostream * /* msgFile */, std::ostream * /* errFile */)		// Don't do anything by default
{
	return true;
}

bool CDisassembler::WriteCodeSection(std::ostream& outFile, std::ostream *msgFile, std::ostream *errFile)
{
	bool RetVal;
	TStringArray OutLine;
	bool Done;
	unsigned int SavedPC;
	bool bInFunc;
	bool bLastFlag;
	bool bBranchOutFlag;

	ClearOutputLine(OutLine);

	RetVal = false;
	if (WritePreCodeSection(outFile, msgFile, errFile)) {
		RetVal = true;

		bInFunc = false;
		Done = false;
		while ((m_PC < m_Memory->GetMemSize()) && (Done == false) && (RetVal)) {
			bLastFlag = false;
			bBranchOutFlag = false;

			// Check for function start/end flags:
			TAddressMap::const_iterator itrFunction = m_FunctionsTable.find(m_PC);
			if (itrFunction != m_FunctionsTable.end()) {
				switch (itrFunction->second) {
					case FUNCF_HARDSTOP:
					case FUNCF_EXITBRANCH:
					case FUNCF_SOFTSTOP:
						bInFunc = false;
						bLastFlag = true;
						break;

					case FUNCF_BRANCHOUT:
						bBranchOutFlag = true;
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
						bInFunc = true;
						break;

					default:
						assert(false);			// Unexpected Function Flag!!  Check Code!!
						RetVal = false;
						continue;
				}
			}

			ClearOutputLine(OutLine);
			OutLine[FC_ADDRESS] = FormatAddress(m_PC);

			TLabelTableMap::const_iterator itrLabels = m_LabelTable.find(m_PC);
			if (itrLabels != m_LabelTable.end()) {
				for (unsigned int i=1; i<itrLabels->second.size(); ++i) {
					OutLine[FC_LABEL] = FormatLabel(LC_CODE, itrLabels->second.at(i), m_PC);
					outFile << MakeOutputLine(OutLine) << "\n";
				}
				if (itrLabels->second.size()) OutLine[FC_LABEL] = FormatLabel(LC_CODE, itrLabels->second.at(0), m_PC);
			}

			SavedPC = m_PC;		// Keep a copy of the PC for this line because some calls will be incrementing our m_PC
			switch (m_Memory->GetDescriptor(m_PC)) {
				case DMEM_ILLEGALCODE:
					m_OpMemory.clear();
					m_OpMemory.push_back(m_Memory->GetByte(m_PC++));
					OutLine[FC_OPBYTES] = FormatOpBytes();
					OutLine[FC_MNEMONIC] = FormatMnemonic(MC_ILLOP);
					OutLine[FC_OPERANDS] = FormatOperands(MC_ILLOP);
					OutLine[FC_COMMENT] = FormatComments(MC_ILLOP);
					outFile << MakeOutputLine(OutLine) << "\n";
					break;
				case DMEM_CODE:
					// If the following call returns false, that means that we erroneously
					//	detected code in the first pass so instead of throwing an error or
					//	causing problems, we will treat it as an illegal opcode:
					if (ReadNextObj(false, msgFile, errFile) == false) {		// Bumps PC
						// Flag it as illegal and then process it as such:
						m_Memory->SetDescriptor(SavedPC, DMEM_ILLEGALCODE);
						OutLine[FC_OPBYTES] = FormatOpBytes();
						OutLine[FC_MNEMONIC] = FormatMnemonic(MC_ILLOP);
						OutLine[FC_OPERANDS] = FormatOperands(MC_ILLOP);
						OutLine[FC_COMMENT] = FormatComments(MC_ILLOP);
						outFile << MakeOutputLine(OutLine) << "\n";
						break;
					}
					OutLine[FC_OPBYTES] = FormatOpBytes();
					OutLine[FC_MNEMONIC] = FormatMnemonic(MC_OPCODE);
					OutLine[FC_OPERANDS] = FormatOperands(MC_OPCODE);
					OutLine[FC_COMMENT] = FormatComments(MC_OPCODE);
					outFile << MakeOutputLine(OutLine) << "\n";
					break;
				default:
					Done = true;
			}

			if (bBranchOutFlag) {
				itrFunction = m_FunctionsTable.find(m_PC);
				if (itrFunction != m_FunctionsTable.end()) {
					switch (itrFunction->second) {
						case FUNCF_ENTRY:
						case FUNCF_INDIRECT:
						case FUNCF_CALL:
							bInFunc = false;
							bLastFlag = true;
							break;
					}
				} else {
					bInFunc = false;
					bLastFlag = true;
				}
			}

			if (bLastFlag) {
				itrFunction = m_FunctionsTable.find(m_PC);
				if (itrFunction != m_FunctionsTable.end()) {
					switch (itrFunction->second) {
						case FUNCF_BRANCHIN:
							bLastFlag = false;
							bInFunc = true;
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

bool CDisassembler::WritePostCodeSection(std::ostream& /* outFile */, std::ostream * /* msgFile */, std::ostream * /* errFile */)		// Don't do anything by default
{
	return true;
}

bool CDisassembler::WritePreFunction(std::ostream& outFile, std::ostream * /* msgFile */, std::ostream * /* errFile */)		// Default is separator bar
{
	TStringArray OutLine;

	ClearOutputLine(OutLine);
	OutLine[FC_ADDRESS] = FormatAddress(m_PC);
	outFile << MakeOutputLine(OutLine) << "\n";
	OutLine[FC_LABEL] = GetCommentStartDelim();
	OutLine[FC_LABEL] += " ";
	for (int i=(GetFieldWidth(FC_LABEL)+
			GetFieldWidth(FC_MNEMONIC)+
			GetFieldWidth(FC_OPERANDS)); i; i--) OutLine[FC_LABEL] += "=";
	OutLine[FC_LABEL] += " ";
	OutLine[FC_LABEL] += GetCommentEndDelim();
	outFile << MakeOutputLine(OutLine) << "\n";

	return true;
}

bool CDisassembler::WriteIntraFunctionSep(std::ostream& outFile, std::ostream * /* msgFile */, std::ostream * /* errFile */)	// Default is separator bar
{
	TStringArray OutLine;

	ClearOutputLine(OutLine);
	OutLine[FC_ADDRESS] = FormatAddress(m_PC);
	OutLine[FC_LABEL] = GetCommentStartDelim();
	OutLine[FC_LABEL] += " ";
	for (int i=(GetFieldWidth(FC_LABEL)+
			GetFieldWidth(FC_MNEMONIC)+
			GetFieldWidth(FC_OPERANDS)); i; i--) OutLine[FC_LABEL] += "-";
	OutLine[FC_LABEL] += " ";
	OutLine[FC_LABEL] += GetCommentEndDelim();
	outFile << MakeOutputLine(OutLine) << "\n";

	return true;
}

bool CDisassembler::WritePostFunction(std::ostream& outFile, std::ostream * /* msgFile */, std::ostream * /* errFile */)		// Default is separator bar
{
	TStringArray OutLine;

	ClearOutputLine(OutLine);
	OutLine[FC_ADDRESS] = FormatAddress(m_PC);
	OutLine[FC_LABEL] = GetCommentStartDelim();
	OutLine[FC_LABEL] += " ";
	for (int i=(GetFieldWidth(FC_LABEL)+
			GetFieldWidth(FC_MNEMONIC)+
			GetFieldWidth(FC_OPERANDS)); i; i--) OutLine[FC_LABEL] += "=";
	OutLine[FC_LABEL] += " ";
	OutLine[FC_LABEL] += GetCommentEndDelim();
	outFile << MakeOutputLine(OutLine) << "\n";
	ClearOutputLine(OutLine);
	OutLine[FC_ADDRESS] = FormatAddress(m_PC);
	outFile << MakeOutputLine(OutLine) << "\n";

	return true;
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

std::string CDisassembler::MakeOutputLine(TStringArray& saOutputData)
{
	std::string OutputStr;
	std::string Temp,Temp2;
	std::string Item;
	std::size_t fw;
	std::size_t pos;
	std::size_t tfw;
	std::size_t TempPos;
	std::string OpBytePart;
	std::string CommentPart;
	TStringArray WrapSave;
	int Toss;
	bool LabelBreak;
	std::string SaveLabel;

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
	LabelBreak = false;

	OutputStr = "";
	pos = 0;
	tfw = 0;
	fw = 0;
	for (unsigned int i=0; i<saOutputData.size(); ++i) {
		Item = saOutputData[i];

		tfw += fw;
		fw = GetFieldWidth((FIELD_CODE) i);

		if ((m_bAddrFlag == false) && (i == FC_ADDRESS)) {
			pos += fw;
			continue;
		}
		if ((m_bOpcodeFlag == false) && (i == FC_OPBYTES)) {
			pos += fw;
			continue;
		}

		if (i == FC_LABEL) {			// Check for Label Break:
			if (Item.find('\v') != std::string::npos) {
				while (Item.find('\v') != std::string::npos) {
					Item = Item.substr(0, Item.find('\v')) + Item.substr(Item.find('\v')+1);
				}
				SaveLabel = Item;
				Item.clear();
				LabelBreak = true;
			}
		}

		if (i == FC_OPBYTES) {			// Check OpBytes "wrap"
			if (Item.size() > fw) {
				Temp = Item.substr(0, fw);
				TempPos = Temp.find_last_of(" \t,;:.!?");
				if (TempPos != std::string::npos) {
					Toss = ((Temp.find_last_of(" \t")==TempPos) ? 1 : 0);
					Temp = Item.substr(0, TempPos + 1 - Toss);	// Throw ' ' and '\t' away
					OpBytePart = Item.substr(TempPos + Toss);
					Item = Temp;
				} else {
					OpBytePart = Item.substr(fw+1);
					Item = Item.substr(0, fw);
				}
			}
		}
		if (i == FC_COMMENT) {			// Check comment "wrap"
			Temp = GetCommentStartDelim() + " " + Item + " " + GetCommentEndDelim();
			Temp2 = GetCommentStartDelim() + "  " + GetCommentEndDelim();
			if ((Temp.size() > fw) || (Temp.find('\n') != std::string::npos)) {
				Temp = Item.substr(0, fw - Temp2.size());
				TempPos = Temp.find_last_of(" \n\t,;:.!?");
				if (TempPos != std::string::npos) {
					Toss = ((Temp.find_last_of(" \n\t")==TempPos) ? 1 : 0);
					Temp = Item.substr(0, TempPos + 1 - Toss);	// Throw ' ' and '\t' away
					CommentPart = "  " + Item.substr(TempPos+1 + Toss);
					Item = Temp;
				} else {
					CommentPart = "  " + Item.substr(fw+1-Temp2.size());
					Item = Item.substr(0, fw-Temp2.size());
				}
			}
		}

		if (i == FC_COMMENT) {
			if (!Item.empty())
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
			for (int j=0; j<m_nTabWidth; j++) Temp += " ";
			while (Item.find('\t') != std::string::npos) {
				TempPos = Item.find('\t');
				Item = Item.substr(0, TempPos) + Temp + Item.substr(TempPos+1);
			}
			while (pos < tfw) {
				OutputStr += " ";
				pos++;
			}
			OutputStr += Item;
		}
		pos += Item.size();
		if (pos >= (tfw+fw)) pos = tfw+fw-1;		// Sub 1 to force at least 1 space between fields
	}

	// Remove all extra white space on right-hand side:
	rtrim(OutputStr);

	if ((!OpBytePart.empty()) || (!CommentPart.empty()) || (LabelBreak)) {
		// Save data:
		WrapSave = saOutputData;
		if (saOutputData.size() < NUM_FIELD_CODES)
			for (unsigned int i=saOutputData.size(); i<NUM_FIELD_CODES; ++i) saOutputData.push_back(std::string());

		if (LabelBreak) {
			saOutputData[FC_OPBYTES] = "";
			saOutputData[FC_LABEL] = SaveLabel;
			saOutputData[FC_MNEMONIC] = "";
			saOutputData[FC_OPERANDS] = "";
			saOutputData[FC_COMMENT] = "";
			OutputStr = MakeOutputLine(saOutputData) + "\n" + OutputStr;	// Call recursively
		}
		if ((!OpBytePart.empty()) || (!CommentPart.empty())) {
			saOutputData[FC_OPBYTES] = OpBytePart;
			saOutputData[FC_LABEL] = "";
			saOutputData[FC_MNEMONIC] = "";
			saOutputData[FC_OPERANDS] = "";
			saOutputData[FC_COMMENT] = CommentPart;
			OutputStr += "\n";
			OutputStr += MakeOutputLine(saOutputData);		// Call recursively
		}

		// Restore data:
		saOutputData = WrapSave;
	}

	return OutputStr;
}

void CDisassembler::ClearOutputLine(TStringArray& saOutputData)
{
	saOutputData.clear();
	saOutputData.reserve(NUM_FIELD_CODES);
	for (unsigned int i=0; i<NUM_FIELD_CODES; ++i) saOutputData.push_back(std::string());
}
