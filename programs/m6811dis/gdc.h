//
//	Motorola 6811 Disassembler
//	Copyright(c)1996 - 2014 by Donna Whisnant
//

//
//	Header for GDC -- Generic Disassembly Class
//

#ifndef _GDC_H_
#define _GDC_H_

#include "memclass.h"

#include <time.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

// ============================================================================

class ifstreamControlFile : public std::ifstream
{
public:
	ifstreamControlFile(const std::string &strFilename)
		:	std::ifstream(),
			m_strFilename(strFilename)
	{
		open(strFilename.c_str(), std::ios_base::in);
	}

	std::string getFilename() const { return m_strFilename; }

private:
	std::string m_strFilename;
};

// ============================================================================

typedef std::vector<std::string> TStringArray;
typedef std::vector<unsigned char> TByteArray;
typedef std::vector<unsigned int> TWordArray;
typedef std::vector<unsigned int> TDWordArray;
typedef std::map<unsigned int, unsigned int> TAddressMap;
typedef std::map<unsigned int, TDWordArray> TAddressTableMap;
typedef std::map<unsigned int, TStringArray> TLabelTableMap;
typedef std::map<unsigned int, std::string> TAddressLabelMap;

// The following macro provides an easy method for GetFieldWidth to round field widths to the next tab-stop to prevent tab/space mixture
#define CALC_FIELD_WIDTH(n) (((m_nTabWidth != 0) && (m_bTabsFlag)) ? ((n/m_nTabWidth)+(((n%m_nTabWidth)!=0) ? 1 : 0))*m_nTabWidth : n)

// Forward declarations
class COpcodeEntry;
class COpcodeArray;
class COpcodeTable;
class CDisassembler;

// The following bit-mask definitions define the bits in the m_Control field
//	of each COpcodeEntry object that are processor independent and must be
//	properly used in each opcode defined by all overrides (GDC's).  This is
//	because these are used to control the overall disassembly operation.
//	All other bit-fields in m_Control can be defined and used as needed by
//	each override.  You will notice that the bit-fields in these definitions
//	start on the upper-bit of the upper-byte in the 32-bit m_Control DWord.
//	It is recommended that processor specific additions are made starting
//	at the other end -- that is the lower-bit of the lower-byte...  This
//	will allow the insertion of additional processor independent bits at the
//	top of the DWord in future updates and releases without necessarily
//	running into and conflicting with those defined by overrides.  It is
//	futher suggested that the entire upper-word be reserved for processor
//	independent functions and the entire lower-word be reserved for
//	dependent functions.  Also note that the usage need not be bit-by-bit
//	only, but can be numerical values occupying certain bit positions -- with
//	proper "and/or" masks and bit-shifting these can easily be converted to/from
//	bit-field equivalents.  (Names use OCTL_xxxx form)
#define OCTL_STOP	0x80000000ul		// Discontinue Disassembly - Upper-bit = 1


// Currently, all m_Group entries are used exclusively by processor
//	dependent functions.  But, it too should follow the guidelines above
//	so that in the future, if the group field is needed by the processor
//	independent functions, then they can be added in without changing
//	anything.  (Names use OGRP_xxxx form)


//	COpcodeEntry
//		This specifies exactly one opcode for the target processor.
class COpcodeEntry
{
public:
	COpcodeEntry();
	COpcodeEntry(int nNumBytes, unsigned char *Bytes, unsigned int nGroup, unsigned int nControl, const std::string & szMnemonic, unsigned int nUserData = 0);
	COpcodeEntry(const COpcodeEntry& aEntry);

	virtual void CopyFrom(const COpcodeEntry& aEntry);

	TWordArray		m_OpcodeBytes;	// Array of opcodes for this entry
	unsigned int	m_Group;		// Group declaration
	unsigned int	m_Control;		// Flow control
	unsigned int	m_UserData;		// User defined addition data storage
	std::string		m_Mnemonic;		// Printed mnemonic

	virtual COpcodeEntry &operator=(const COpcodeEntry& aEntry);
};


//	COpcodeArray
//		This specifies an array of opcodes that have the SAME initial
//		byte.  This class is exclusively a friend of COpcodeTable.
//		COpcodeTable controls the sorting and creating of these
//		arrays.  When new opcodes are added, COpcodeTable will properly
//		create and sort (via hash maps) these arrays...
class COpcodeArray : public std::vector<COpcodeEntry>
{
public:
	COpcodeArray();
	virtual ~COpcodeArray();

};


//	COpcodeTable
//		This function manages a single table of opcodes by properly
//		adding and maintaining opcode array's.  This class is a
//		mapping of initial opcode bytes to an array of COpcodeArray.
//		When adding a new opcode, this function first checks to
//		see if an array exists, if not it creates it - then it adds
//		the opcode to the array.   It also provides a "lookup"
//		function of "FindFirst" and "FindNext"...  These work sort
//		of like the traditional GetFirstPosition and GetNextAssoc
//		routines, but here they find the "First" and "Next" opcodes
//		that have a SPECIFIC byte.  The "First" function simply
//		returns the initial index position in the array +1 (0 = NOT Found)
//		The "Next" function updates the position and sets the
//		specified COpcodeEntry to the opcode entry value.  Note that
//		since arrays are used, the First/Next operation returns back
//		the opcodes for a common byte in the SAME order they were
//		added.  Currently, there is only an "Add" function and no remove!
//		It is also the users responsibility to not add duplicate or
//		conflicting opcodes!
class COpcodeTable : public std::map<unsigned char, COpcodeArray>
{
public:
	COpcodeTable();
	virtual ~COpcodeTable();

	virtual void AddOpcode(const COpcodeEntry& nOpcode);
};


//	CDisassembler
//		This class describes a generic disassembler class to handle disassembly.
//		It contains no processor dependent information, therefore this class
//		must be overriden to handle processor dependent operations.  It is also
//		setup to use the DFC converters and memory objects so that these
//		functions are not dependent on input file types.
//
//		In the override, the constructor and destructor must be
//		defined to setup opcodes and memory for the target processor!
//
//		The function "ReadControlFile" takes an already open CFile
//		object and reads in the control file.  The base function calls the
//		function 'ParseControlLine' for every line in the input file (comment
//		lines are automatically removed).  The override (if any) should
//		return TRUE if all is OK with the line being processed, otherwise,
//		it should set the m_ParseError with an error message and return FALSE.
//		The default message, if not set, for m_ParseError is "*** Error: Unknown error in line xxx".
//		The ReadControlFile routine will automatically output the control
//		file line number information for each m_ParseError message.
//		If default functionality is desired for the line, then the override
//		should call the CDisassembler::ParseControlLine function -- that way,
//		for the normally accepted options, the override doesn't have to worry
//		about processing them.  Plus, it gives the override a chance to create
//		new behavior for each command.
//
//		When branches are encountered, they are added to the m_BranchTable
//		map -- m_BranchTable is a map of arrays -- each array is a DWORD array that
//		contains every address that referenced the branch.  So, when a new branch is
//		found, the array is created -- when a branch is re-encountered, the address
//		that referenced the branch is added...  This is NEW to this disassembler from
//		the original! -- the original used only a simple table of branch addresses.
//
//		NOTE: Code references are added to the m_BranchTable arrays and data references
//		are added to the m_LabelRefTable arrays!!  This way, not only can we track
//		who referenced a particular address, but we can track how it was referenced.
//		All overrides should conform to this by making sure they add references to the
//		right place.  AddBranch function handles code references and AddLabel handles
//		data references.  When adding labels to code, add the reference using either
//		AddBranch and then call AddLabel without the refernece or just call
//		GenAddrLabel (which calls both AddBranch and AddLabel correctly).  When adding
//		data references, use either AddLabel or call GenDataLabel (GenDataLabel also
//		handles outputting of generated labels).
//		
class CDisassembler
{
public:
	// The following define the function identification flags:
	enum FUNC_FLAGS {	// Entries <128 = Stops:
						FUNCF_HARDSTOP = 0,			// Hard Stop = RTS or RTI encountered
						FUNCF_EXITBRANCH = 1,		// Exit Branch = Branch to a specified exit address
						FUNCF_BRANCHOUT = 2,		// Execution branched out (JMP, BRA, etc)
						FUNCF_SOFTSTOP = 3,			// Soft Stop = Code execution flowed into another function definition
						// Entries >=128 = Starts:
						FUNCF_ENTRY = 128,			// Entry Point Start
						FUNCF_INDIRECT = 129,		// Indirect Code Entry Point Start
						FUNCF_CALL = 130,			// Call Start Point (JSR, BSR, etc)
						FUNCF_BRANCHIN = 131 };		// Entry from a branch into (BEQ, BNE, etc)


	// The following define the memory descriptors for the disassembler memory.
	//		0 = The memory is unused/not-loaded
	//		10 = The memory is loaded, but hasn't been processed
	//		20 = The memory is loaded, has been processed, and found to be data, but not printable ASCII data
	//		21 = The memory is loaded, has been processed, and found to be printable ASCII data
	//		30 = The memory is loaded, has been processed, and is an indirect vector to Code
	//		31 = The memory is loaded, has been processed, and is an indirect vector to Data
	//		40 = The memory is loaded, has been processed, and found to be code
	//		41 = The memory is loaded, has been processed, and should be code, but was determined to be an illegal opcode
	//	NOTE: These should not be changed -- enough room has been allowed for future expansion!
	enum MEM_DESC {	DMEM_NOTLOADED = 0,
					DMEM_LOADED = 10,
					DMEM_DATA = 20,
					DMEM_PRINTDATA = 21,
					DMEM_CODEINDIRECT = 30,
					DMEM_DATAINDIRECT = 31,
					DMEM_CODE = 40,
					DMEM_ILLEGALCODE = 41 };

	// The following define field positions for obtaining field widths from overrides.
	//	Additional entries can be added on overrides, but this order should NOT change
	//	in order to maintain backward compatibility at all times:
	#define NUM_FIELD_CODES 6
	enum FIELD_CODE { FC_ADDRESS, FC_OPBYTES, FC_LABEL, FC_MNEMONIC, FC_OPERANDS, FC_COMMENT };

	// The following define mnemonic codes for use with the FormatMnemonic pure virtual
	//	function.  These are used to specify the retrieving of "special mnemonics" for
	//	things such as equates, data bytes, ascii text, etc.  Additional entries can be
	//	added on overrides, but this order should NOT change in order to maintain backward
	//	compatibility at all times:
	enum MNEMONIC_CODE { MC_OPCODE, MC_ILLOP, MC_EQUATE, MC_DATABYTE, MC_ASCII, MC_INDIRECT };

	// The following defines the label types when formating output.  The are flags that
	//	determine where the label is being referenced.  It is used by the FormatLabel
	//	function to determine what type of delimiter to add, if any.  This enum can be
	//	added to in overrides, but the existing entries should not be changed:
	enum LABEL_CODE { LC_EQUATE, LC_DATA, LC_CODE, LC_REF };

public:
	CDisassembler();
	virtual ~CDisassembler();

	virtual unsigned int GetVersionNumber(void);			// Base function returns GDC version is most-significant word.  Overrides should call this parent to get the GDC version and then set the least-significant word with the specific disassembler version.
	virtual std::string GetGDCLongName(void) = 0;		// Pure virtual.  Defines the long name for this disassembler
	virtual std::string GetGDCShortName(void) = 0;		// Pure virtual.  Defines the short name for this disassembler

	virtual bool ReadControlFile(ifstreamControlFile& inFile, bool bLastFile = true, std::ostream *msgFile = NULL, std::ostream *errFile = NULL, int nStartLineCount = 0);	// Read already open control file 'infile', outputs messages to 'msgFile' and errors to 'errFile', nStartLineCount = initial m_nCtrlLine value
	virtual bool ParseControlLine(const std::string & szLine, TStringArray& argv, std::ostream *msgFile = NULL, std::ostream *errFile = NULL);		// Parses a line from the control file -- szLine is full line, argv is array of whitespace delimited args.  Should return false ONLY if ReadControlFile should print the ParseError string to errFile with line info
	virtual bool ReadSourceFile(const std::string & szFilename, unsigned int nLoadAddress, const std::string & szDFCLibrary, std::ostream *msgFile = NULL, std::ostream *errFile = NULL);	// Reads source file named szFilename using DFC szDFCLibrary

	virtual bool ScanEntries(std::ostream *msgFile = NULL, std::ostream *errFile = NULL);		// Scans through the entry list looking for code
	virtual bool ScanBranches(std::ostream *msgFile = NULL, std::ostream *errFile = NULL);		// Scans through the branch lists looking for code
	virtual bool ScanData(const std::string & szExcludeChars, std::ostream *msgFile = NULL, std::ostream *errFile = NULL);			// Scans through the data that remains and tags it as printable or non-printable
	virtual bool Disassemble(std::ostream *msgFile = NULL, std::ostream *errFile = NULL, std::ostream *outFile = NULL);		// Disassembles entire loaded memory and outputs info to outFile if non-NULL or opens and writes the file specified by m_sOutputFilename -- calls Pass1 and Pass2 to perform this processing

	virtual bool Pass1(std::ostream& outFile, std::ostream *msgFile = NULL, std::ostream *errFile = NULL);	// Performs Pass1 which finds code and data -- i.e. calls Scan functions
	virtual bool Pass2(std::ostream& outFile, std::ostream *msgFile = NULL, std::ostream *errFile = NULL);	// Performs Pass2 which is the actual disassemble stage
	virtual bool Pass3(std::ostream& outFile, std::ostream *msgFile = NULL, std::ostream *errFile = NULL);	// Performs Pass3 which creates the function output file

	virtual bool FindCode(std::ostream *msgFile = NULL, std::ostream *errFile = NULL);		// Iterates through memory using m_PC finding and marking code.  It should add branches and labels as necessary and return when it hits an end-of-code or runs into other code found
	virtual bool ReadNextObj(bool bTagMemory, std::ostream *msgFile = NULL, std::ostream *errFile = NULL);	// Procedure to read next object code from memory.  The memory type is flagged as code (or illegal code).  Returns True if current code legal, else False.  OpMemory = object code from memory.  CurrentOpcode = Copy of COpcodeEntry for the opcode located.  PC is automatically advanced.
	virtual bool CompleteObjRead(bool bAddLabels = true, std::ostream *msgFile = NULL, std::ostream *errFile = NULL) = 0;	// Pure Virtual that finishes the opcode reading process as is dependent on processor.  It should increment m_PC and add bytes to m_OpMemory, but it should NOT flag memory descriptors!  If the result produces an invalid opcode, the routine should return FALSE.  The ReadNextObj func will tag the memory bytes based off of m_OpMemory!  If bAddLabels = FALSE, then labels and branches aren't added -- disassemble only!

	virtual bool RetrieveIndirect(std::ostream *msgFile = NULL, std::ostream *errFile = NULL) = 0;	// Pure Virtual. Retrieves the indirect address specified by m_PC and places it m_OpMemory for later formatting.  It is a pure virtual because of length and indian notation differences

	virtual std::string FormatMnemonic(MNEMONIC_CODE nMCCode) = 0;		// Pure Virtual.  This function should provide the specified mnemonic.  For normal opcodes, MC_OPCODE is used -- for which the override should return the mnemonic in the opcode table.  This is done to provide the assembler/processor dependent module opportunity to change/set the case of the mnemonic and to provide special opcodes.
	virtual std::string FormatOperands(MNEMONIC_CODE nMCCode) = 0;		// Pure Virtual.  This function should create the operands for the current opcode if MC_OPCODE is issued.  For others, it should format the data in m_OpMemory to the specified form!
	virtual std::string FormatComments(MNEMONIC_CODE nMCCode);			// This function should create any needed comments for the disassembly output for the current opcode or other special MC_OPCODE function.  This is where "undetermined branches" and "out of source branches" can get flagged by the specific disassembler.  The built in function calls FormatReferences and creates the references in the comments.  Overrides wishing to perform the same functionallity, should also call FormatReferences.

	virtual std::string FormatAddress(unsigned int nAddress);					// This function creates the address field of the disassembly output.  Default is "xxxx" hex value.  Override for other formats.
	virtual std::string FormatOpBytes(void);							// This function creates a opcode byte string from the bytes in m_OpMemory.  If another format is desired, override it
	virtual std::string FormatLabel(LABEL_CODE nLC, const std::string & szLabel, unsigned int nAddress);	// This function modifies the specified label to be in the Lxxxx format for the nAddress if szLabel is null. If szLabel is non-null no changes are made.  This function should be overridden to add the correct suffix delimiters as needed!
	virtual std::string FormatReferences(unsigned int nAddress);				// Makes a string to place in the comment field that contains all references for the specified address

	virtual int GetFieldWidth(FIELD_CODE nFC);			// Defines the widths of each output field.  Can be overridden to alter output formatting.  To eliminate space/tab mixing, these should typically be a multiple of the tab width
	virtual std::string MakeOutputLine(TStringArray& saOutputData);		// Formats the data in saOutputData, which should have indicies corresponding to FIELD_CODE enum, to a string that can be sent to the output file
	virtual void ClearOutputLine(TStringArray& saOutputData);

	virtual bool WriteHeader(std::ostream& outFile, std::ostream *msgFile = NULL, std::ostream *errFile = NULL);		// Writes the disassembly header comments to the output file.  Override in subclasses for non-default header

	virtual bool WritePreEquates(std::ostream& outFile, std::ostream *msgFile = NULL, std::ostream *errFile = NULL);	// Writes any info needed in the disassembly file prior to the equates section.  Default is do nothing.  Override as needed
	virtual bool WriteEquates(std::ostream& outFile, std::ostream *msgFile = NULL, std::ostream *errFile = NULL);		// Writes the equates table to the disassembly file.
	virtual bool WritePostEquates(std::ostream& outFile, std::ostream *msgFile = NULL, std::ostream *errFile = NULL);	// Writes any info needed in the disassembly file after the equates section.  Default is do nothing.  Override as needed

	virtual bool WritePreDisassembly(std::ostream& outFile, std::ostream *msgFile = NULL, std::ostream *errFile = NULL);	// Writes any info needed in the disassembly file prior to the main disassembly section.  Default writes several blank lines.  Override as needed
	virtual bool WriteDisassembly(std::ostream& outFile, std::ostream *msgFile = NULL, std::ostream *errFile = NULL);	// Writes the disassembly to the output file.  Default iterates through memory and calls correct functions.  Override as needed.
	virtual bool WritePostDisassembly(std::ostream& outFile, std::ostream *msgFile = NULL, std::ostream *errFile = NULL);	// Writes any info needed in the disassembly file after the main disassembly section, such as ".end".  Default is do nothing.  Override as needed

	virtual bool WritePreSection(std::ostream& outFile, std::ostream *msgFile = NULL, std::ostream *errFile = NULL);	// Writes any info needed in the disassembly file prior to a new section.  That is, a section that is loaded and analyzed.  A part of memory containing "not_loaded" areas would cause a new section to be generated.  This should be things like "org", etc.
	virtual bool WriteSection(std::ostream& outFile, std::ostream *msgFile = NULL, std::ostream *errFile = NULL);		// Writes the current section to the disassembly file.  Override as needed.
	virtual bool WritePostSection(std::ostream& outFile, std::ostream *msgFile = NULL, std::ostream *errFile = NULL);	// Writes any info needed in the disassembly file after a new section.  Default is do nothing, override as needed.

	virtual bool WritePreDataSection(std::ostream& outFile, std::ostream *msgFile = NULL, std::ostream *errFile = NULL);	// Writes any info needed prior to a data section -- that is a section that contains only DMEM_DATA directives.  This would be things like "DSEG", etc.  Default is do nothing. Override as needed.
	virtual bool WriteDataSection(std::ostream& outFile, std::ostream *msgFile = NULL, std::ostream *errFile = NULL);		// Writes a data section.
	virtual bool WritePostDataSection(std::ostream& outFile, std::ostream *msgFile = NULL, std::ostream *errFile = NULL);	// Writes any info needed after a data section.  Default is do nothing.  Typically this would be something like "ends"

	virtual bool WritePreCodeSection(std::ostream& outFile, std::ostream *msgFile = NULL, std::ostream *errFile = NULL);	// Writes any info needed prior to a code section -- such as "CSEG", etc.  Default is do nothing.  Override as needed.
	virtual bool WriteCodeSection(std::ostream& outFile, std::ostream *msgFile = NULL, std::ostream *errFile = NULL);		// Writes a code section.
	virtual bool WritePostCodeSection(std::ostream& outFile, std::ostream *msgFile = NULL, std::ostream *errFile = NULL);	// Writes any info needed after a code section.  Default is do nothing.  Typically this would be something like "ends"

	virtual bool WritePreFunction(std::ostream& outFile, std::ostream *msgFile = NULL, std::ostream *errFile = NULL);		// Writes any info needed prior to a new function -- such as "proc".  The default simply writes a separator line "===".  Override as needed
	virtual bool WriteIntraFunctionSep(std::ostream& outFile, std::ostream *msgFile = NULL, std::ostream *errFile = NULL);	// Writes a function's intra code separation.  The default simply writes a separator line "---".  Override as needed
	virtual bool WritePostFunction(std::ostream& outFile, std::ostream *msgFile = NULL, std::ostream *errFile = NULL);	// Writes any info needed after a function -- such as "endp".  The default simply writes a separator line "===".  Override as needed

	virtual bool ValidateLabelName(const std::string & aName);		// Parses a specified label to make sure it meets label rules

	virtual bool ResolveIndirect(unsigned int nAddress, unsigned int& nResAddress, int nType) = 0;	// Pure Virtual that Resolves the indirect address specified by nAddress of type nType where nType = 0 for Code locations and 1 for Data locations.  It returns the resolved address in nResAddress and T/F for mem load status

	virtual bool IsAddressLoaded(unsigned int nAddress, int nSize);	// Checks to see if the nSize bytes starting at address nAddress are loaded.  True only if all the bytes are loaded!

	virtual bool AddLabel(unsigned int nAddress, bool bAddRef = false, unsigned int nRefAddress = 0, const std::string & szLabel = std::string());		// Sets szLabel string as the label for nAddress.  If nAddress is already set with that string, returns FALSE else returns TRUE or all OK.  If address has a label and szLabel = NULL or zero length, nothing is added!  If szLabel is NULL or zero length, a null string is added to later get resolved to Lxxxx form.  If bAddRef, then nRefAddress is added to the reference list
	virtual bool AddBranch(unsigned int nAddress, bool bAddRef = false, unsigned int nRefAddress = 0);	// Adds nAddress to the branch table with nRefAddress as the referring address.  Returns T if all ok, F if branch address is outside of loaded space.  If nAddRef is false, a branch is added without a reference

	virtual void GenDataLabel(unsigned int nAddress, unsigned int nRefAddress, const std::string & szLabel = std::string(), std::ostream *msgFile = NULL, std::ostream *errFile = NULL);	// Calls AddLabel to create a label for nAddress -- unlike calling direct, this function outputs the new label to msgFile if specified...
	virtual void GenAddrLabel(unsigned int nAddress, unsigned int nRefAddress, const std::string & szLabel = std::string(), std::ostream *msgFile = NULL, std::ostream *errFile = NULL);	// Calls AddLabel to create a label for nAddress, then calls AddBranch to add a branch address -- unlike calling direct, this function outputs the new label to msgFile if specified and displays "out of source" errors for the branches to errFile...
	virtual void OutputGenLabel(unsigned int nAddress, std::ostream *msgFile);		// Outputs a generated label to the msgFile -- called by GenAddrLabel and GenDataLabel
	virtual std::string GenLabel(unsigned int nAddress);		// Creates a default generated label.  Base form is Lxxxx, but can be overridden in derived classes for other formats

	virtual std::string GetExcludedPrintChars(void) = 0;	// Pure Virtual. Returns a list of characters excluded during datascan for printable scan
	virtual std::string GetHexDelim(void) = 0;				// Pure Virtual. Returns hexadecimal delimiter for specific assembler.  Typically "0x" or "$"
	virtual std::string GetCommentStartDelim(void) = 0;		// Pure Virtual. Returns start of comment delimiter string
	virtual std::string	GetCommentEndDelim(void) = 0;		// Pure Virtual. Returns end of comment delimiter string

	// Output Flags and Control Values:
	bool	m_bAddrFlag;		// CmdFileToggle, TRUE = Write addresses on disassembly
	bool	m_bOpcodeFlag;		// CmdFileToggle, TRUE = Write opcode listing as comment in disassembly file
	bool	m_bAsciiFlag;		// CmdFileToggle, TRUE = Convert printable data to ASCII strings
	bool	m_bSpitFlag;		// CmdFileToggle, Code-Seeking/Code-Dump Mode, TRUE = Code-Dump, FALSE = Code-Seek
	bool	m_bTabsFlag;		// CmdFileToggle, TRUE = Use tabs in output file.  Default is TRUE.
	bool	m_bAsciiBytesFlag;	// CmdFileToggle, TRUE = Outputs byte values of ASCII text as comment prior to ascii line (default)  FALSE = ASCII line only
	bool	m_bDataOpBytesFlag;	// CmdFileToggle, TRUE = Outputs byte values for data fields in OpBytes field for both printable and non-printable data, FALSE = no opbytes -- Default is FALSE

	int		m_nMaxNonPrint;		// CmdFile Value, Maximum number of non-printable characters per line
	int		m_nMaxPrint;		// CmdFile Value, Maximum number of printable characters per line
	int		m_nBase;			// CmdFile Value, Default number base for control file input
	int		m_nTabWidth;		// CmdFile Value, Width of tabs for output, default is 4

	std::string	m_sDefaultDFC;		// CmdFile Value, Default DFC Library -- used with single input file and on multi-file when not specified
	unsigned int	m_nLoadAddress;		// CmdFile Value, Load address for input file -- used only with single input file!
	std::string	m_sInputFilename;	// CmdFile Value, Filename for the input file -- used only with single input file!
	std::string	m_sOutputFilename;	// CmdFile Value, Filename for the disassembly output file
	std::string	m_sFunctionsFilename;	// CmdFile Value, Filename for the function output file
	TStringArray	m_sControlFileList;	// This list is appended with each control file read.  The only purpose for this list is for generating comments in the output file
	TStringArray	m_sInputFileList;	// This list is appended with each source file read (even with the single input name).  The only purpose for this list is for generating comments in the output file

	std::string m_sFunctionalOpcode;	// Functions File export opcode.  Cleared by ReadNextObj and set by CompleteObjRead.

	time_t	m_StartTime;		// Set to system time at instantiated

protected:
	int				m_nCtrlLine;	// Line Count while processing control file
	int				m_nFilesLoaded;	// Count of number of files successfully loaded by the control file
	COpcodeTable	m_Opcodes;		// Table of opcodes for this disassembler
	std::string		m_ParseError;	// Set during the control file parsing to indicate error messages

	TAddressMap	m_EntryTable;	// Table of Entry values (start addresses) from control file
	TAddressMap	m_FunctionsTable;	// Table of start-of and end-of functions
	TAddressMap	m_FuncExitAddresses;	// Table of address that are equivalent to function exit like RTS or RTI.  Any JMP or BRA or execution into one of these addresses will equate to a function exit
	TAddressTableMap m_BranchTable;	// Table mapping branch addresses encountered with the address that referenced it in disassembly.
	TLabelTableMap m_LabelTable;	// Table of labels both specified by the user and from disassembly.  Entry is pointer to array of labels.  A null entry equates back to Lxxxx.  First entry is default for "Get" function.
	TAddressTableMap m_LabelRefTable;	// Table of reference addresses for labels.  User specified labels have no reference added.
	TAddressLabelMap m_CodeIndirectTable;	// Table of indirect code vectors with labels specified by the user and from disassembly
	TAddressLabelMap m_DataIndirectTable;	// Table of indirect data vectors with labels specified by the user and from disassembly

	TMemObject		*m_Memory;		// Memory object for the processor.  Any overrides must define allocate/deallocate this appropriately
	unsigned int	m_PC;			// Program counter
	TByteArray		m_OpMemory;		// Array to hold the opcode currently being processed
	COpcodeEntry	m_CurrentOpcode;	// Copy of COpcodeEntry object of the opcode last located in the ReadNextObj function

	TMemRange		m_ROMMemMap;	// Mapping of ROM memory
	TMemRange		m_RAMMemMap;	// Mapping of RAM memory
	TMemRange		m_IOMemMap;		// Mapping of Memory Mapped I/O

private:
	typedef std::map<std::string, unsigned int> TParseCmdMap;
	TParseCmdMap	ParseCmds;		// Used internally to store control file commands
	TParseCmdMap	ParseCmdsOP1;	// Used internally to store control file commands
	TParseCmdMap	ParseCmdsOP2;	// Used internally to store control file commands
	TParseCmdMap	ParseCmdsOP3;	// Used internally to store control file commands
	TParseCmdMap	ParseCmdsOP4;	// Used internally to store control file commands
	TParseCmdMap	ParseCmdsOP5;	// Used internally to store control file commands

	int LAdrDplyCnt;
};

#endif	// _GDC_H_
