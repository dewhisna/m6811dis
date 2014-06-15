// funcdesc.cpp: implementation of the CFuncDesc, CFuncDescFile, and CFuncObject classes.
//
// $Log: funcdesc.cpp,v $
// Revision 1.4  2003/09/13 05:38:03  dewhisna
// Added Read/Write File support.  Added output options on output line creation
// for function diff and added returning of match percentage.  Added callback
// function for read operation feedback.  Enhanced symbol map manipulation.
//
// Revision 1.3  2003/02/09 06:59:56  dewhisna
// Split comparison logic from Function File I/O
//
// Revision 1.2  2002/09/15 22:54:02  dewhisna
// Added Symbol Table comparison support
//
// Revision 1.1  2002/07/01 05:50:00  dewhisna
// Initial Revision
//
//

//
//	Format of Function Output File:
//
//		Any line beginning with a ";" is considered to be a commment line and is ignored
//
//		Memory Mapping:
//			#type|addr|size
//			   |    |    |____  Size of Mapped area (hex)
//			   |    |_________  Absolute Address of Mapped area (hex)
//			   |______________  Type of Mapped area (One of following: ROM, RAM, IO)
//
//		Label Definitions:
//			!addr|label
//			   |    |____ Label(s) for this address(comma separated)
//			   |_________ Absolute Address (hex)
//
//		Start of New Function:
//			@xxxx|name
//			   |    |____ Name(s) of Function (comma separated list)
//			   |_________ Absolute Address of Function Start relative to overall file (hex)
//
//		Mnemonic Line (inside function):
//			xxxx|xxxx|label|xxxxxxxxxx|xxxxxx|xxxx|DST|SRC|mnemonic|operands
//			  |    |    |        |        |     |   |   |     |        |____  Disassembled operand output (ascii)
//			  |    |    |        |        |     |   |   |     |_____________  Disassembled mnemonic (ascii)
//			  |    |    |        |        |     |   |___|___________________  See below for SRC/DST format
//			  |    |    |        |        |     |___________________________  Addressing/Data bytes from operand portion of instruction (hex)
//			  |    |    |        |        |_________________________________  Opcode bytes from instruction (hex)
//			  |    |    |        |__________________________________________  All bytes from instruction (hex)
//			  |    |    |___________________________________________________  Label(s) for this address (comma separated list)
//			  |    |________________________________________________________  Absolute address of instruction (hex)
//			  |_____________________________________________________________  Relative address of instruction to the function (hex)
//
//		Data Byte Line (inside function):
//			xxxx|xxxx|label|xx
//			  |    |    |    |____  Data Byte (hex)
//			  |    |    |_________  Label(s) for this address (comma separated)
//			  |    |______________  Absolute address of data byte (hex)
//			  |___________________  Relative address of data byte to the function (hex)
//
//		SRC and DST entries:
//			#xxxx			Immediate Data Value (xxxx=hex value)
//			C@xxxx			Absolute Code Address (xxxx=hex address)
//			C^n(xxxx)		Relative Code Address (n=signed offset in hex, xxxx=resolved absolute address in hex)
//			C&xx(r)			Register Code Offset (xx=hex offset, r=register number or name), ex: jmp 2,x -> "C$02(x)"
//			D@xxxx			Absolute Data Address (xxxx=hex address)
//			D^n(xxxx)		Relative Data Address (n=signed offset in hex, xxxx=resolved absolute address in hex)
//			D&xx(r)			Register Data Offset (xx=hex offset, r=register number or name), ex: ldaa 1,y -> "D$01(y)"
//
//			If any of the above also includes a mask, then the following will be added:
//			,Mxx			Value mask (xx=hex mask value)
//
//		Note: The address sizes are consistent with the particular process.  For example, an HC11 will
//			use 16-bit addresses (or 4 hex digits).  The size of immediate data entries, offsets, and masks will
//			reflect the actual value.  A 16-bit immediate value, offset, or mask, will be outputted as 4 hex
//			digits, but an 8-bit immediate value, offset, or mask, will be outputted as only 2 hex digits.
//

#include "stdafx.h"
//#include "funcanal.h"
#include "funcdesc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

const static char m_strUnexpectedError[] = "Unexpected Error";
const static char m_strSyntaxError[] = "Syntax Error or Unexpected Entry";
const static char m_strOutOfMemoryError[] = "Out of Memory or Object Creation Error";


//////////////////////////////////////////////////////////////////////
// Static Functions
//////////////////////////////////////////////////////////////////////

// ParseLine : Parses a line from the file and returns an array of
//				items from the specified separator.
static void ParseLine(LPCTSTR pszLine, TCHAR cSepChar, CStringArray &argv)
{
	CString strLine = pszLine;
	CString strTemp;
	int pos;

	argv.RemoveAll();

	strLine += cSepChar;

	while (!strLine.IsEmpty()) {
		pos = strLine.Find(cSepChar);
		if (pos != -1) {
			strTemp = strLine.Left(pos);
			strTemp.TrimLeft();
			strTemp.TrimRight();
			strLine = strLine.Mid(pos+1);
		} else {
			strTemp = strLine;
			strTemp.TrimLeft();
			strTemp.TrimRight();
			strLine = "";
		}
		argv.Add(strTemp);
	}
}

// ReadFileString : Reads a line from an open CFile object and
//					fills in a CString object and returns a BOOL
//					of EOF status.  Is basically identical to
//					the CStdioFile ReadString function, but can
//					be any CFile derived object:
static BOOL ReadFileString(CFile &rFile, CString &rString)
{
	if (rFile.IsKindOf(RUNTIME_CLASS(CStdioFile))) return ((CStdioFile&)rFile).ReadString(rString);

	rString.Empty();

	TCHAR c = 0;
	UINT nBytesRead;
	while (((nBytesRead = rFile.Read(&c, sizeof(c))) != 0) && (c != 10)) {
		if (c != 10) rString += c;
	}
	if ((!rString.IsEmpty()) && (rString.GetAt(rString.GetLength()-1) == 13)) rString = rString.Left(rString.GetLength()-1);
	return ((nBytesRead != 0) || (!rString.IsEmpty()));
}

// WriteFileString : Writes a line to an open CFile object using
//					LF -> CRLF conversion as defined for text
//					files.  Is basically identical to the
//					CStdioFile WriteString function, but can be
//					any CFile derived object:
static void WriteFileString(CFile &rFile, LPCTSTR pszString)
{
	if (rFile.IsKindOf(RUNTIME_CLASS(CStdioFile))) {
		((CStdioFile&)rFile).WriteString(pszString);
		return;
	}

	CString strString = pszString;
	int pos;

	while ((pos = strString.Find('\n')) != -1) {
		if (pos != 0) rFile.Write(LPCTSTR(strString), pos);
		rFile.Write("\r\n", 2);
		strString = strString.Mid(pos+1);
	}
	if (!strString.IsEmpty()) rFile.Write(LPCTSTR(strString), strString.GetLength());
}


//////////////////////////////////////////////////////////////////////
// Global Functions
//////////////////////////////////////////////////////////////////////

// PadString : Pads a string with spaces up to the specified length.
CString PadString(LPCTSTR pszString, int nWidth)
{
	CString strRetVal = pszString;

	while (strRetVal.GetLength() < nWidth) strRetVal += ' ';
	return strRetVal;
}


//////////////////////////////////////////////////////////////////////
// CFuncObject Class
//////////////////////////////////////////////////////////////////////

CFuncObject::CFuncObject(CFuncDescFile &ParentFuncFile, CFuncDesc &ParentFunc, CStringArray &argv)
	:	m_ParentFuncFile(ParentFuncFile),
		m_ParentFunc(ParentFunc)
{
	ASSERT(argv.GetSize() >= 4);
	CStringArray strarrLabels;
	int i;
	CString strTemp;

	if (argv.GetSize() >= 1) m_dwRelFuncAddress = strtoul(argv[0], NULL, 16);
	if (argv.GetSize() >= 2) m_dwAbsAddress = strtoul(argv[1], NULL, 16);
	if (argv.GetSize() >= 3) {
		ParseLine(argv[2], ',', strarrLabels);
		for (i=0; i<strarrLabels.GetSize(); i++) AddLabel(strarrLabels.GetAt(i));
	}
	if (argv.GetSize() >= 4) {
		strTemp = argv[3];
		for (i=0; i<strTemp.GetLength()/2; i++) m_Bytes.Add((BYTE)strtoul(strTemp.Mid(i*2, 2), NULL, 16));
	}
}

CFuncObject::~CFuncObject()
{

}

BOOL CFuncObject::IsExactMatch(CFuncObject *pObj)
{
	int i;

	if (pObj == NULL) return FALSE;
	if (m_Bytes.GetSize() != pObj->m_Bytes.GetSize()) return FALSE;
	for (i=0; i<m_Bytes.GetSize(); i++) if (m_Bytes.GetAt(i) != pObj->m_Bytes.GetAt(i)) return FALSE;

	return TRUE;
}

BOOL CFuncObject::AddLabel(LPCTSTR pszLabel)
{
	CString strTemp;
	int i;

	if (pszLabel == NULL) return FALSE;
	strTemp = pszLabel;
	if (strTemp.IsEmpty()) return FALSE;

	// If we have the label already, return:
	for (i=0; i<m_LabelTable.GetSize(); i++) {
		if (strTemp.CompareNoCase(m_LabelTable.GetAt(i)) == 0) return FALSE;
	}
	m_LabelTable.Add(strTemp);
	return TRUE;
}

DWORD CFuncObject::GetRelFuncAddress()
{
	return m_dwRelFuncAddress;
}

DWORD CFuncObject::GetAbsAddress()
{
	return m_dwAbsAddress;
}

int CFuncObject::GetLabelCount()
{
	return m_LabelTable.GetSize();
}

CString CFuncObject::GetLabel(int nIndex)
{
	return m_LabelTable.GetAt(nIndex);
}

CString CFuncObject::GetBytes()
{
	CString strRetVal = "";
	CString strTemp;
	int i;

	for (i=0; i<m_Bytes.GetSize(); i++) {
		strTemp.Format("%02X", m_Bytes.GetAt(i));
		strRetVal +=  strTemp;
	}
	return strRetVal;
}

int CFuncObject::GetByteCount()
{
	return m_Bytes.GetSize();
}

void CFuncObject::GetSymbols(CStringArray &aSymbolArray)
{
	//
	// This function, and its override, fills in the passed in array as follows:
	//		Lxxxxxx			-- Label for THIS object, "xxxxxx = Label"
	//		RSCxxxxxx		-- Referenced by THIS object -- Source, Code, "xxxxxx = Label"
	//		RSDxxxxxx		-- Referenced by THIS object -- Source, Data, "xxxxxx = Label"
	//		RDCxxxxxx		-- Referenced by THIS object -- Destination, Code, "xxxxxx = Label"
	//		RDDxxxxxx		-- Referenced by THIS object -- Destination, Data, "xxxxxx = Label"
	//

	CString strTemp;

	aSymbolArray.RemoveAll();

	if (m_ParentFuncFile.AddrHasLabel(m_dwAbsAddress)) {
		strTemp = m_ParentFuncFile.GetPrimaryLabel(m_dwAbsAddress);
		if (!strTemp.IsEmpty()) {
			aSymbolArray.Add("L" + strTemp);
		}
	}
}

int CFuncObject::GetFieldWidth(FIELD_CODE nFC)
{
	switch (nFC) {
		case FC_ADDRESS:	return CALC_FIELD_WIDTH(7);
		case FC_OPBYTES:	return CALC_FIELD_WIDTH(11);
		case FC_LABEL:		return CALC_FIELD_WIDTH(13);
		case FC_MNEMONIC:	return CALC_FIELD_WIDTH(7);
		case FC_OPERANDS:	return CALC_FIELD_WIDTH(21);
		case FC_COMMENT:	return CALC_FIELD_WIDTH(60);
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////
// CFuncAsmInstObject Class
//////////////////////////////////////////////////////////////////////

CFuncAsmInstObject::CFuncAsmInstObject(CFuncDescFile &ParentFuncFile, CFuncDesc &ParentFunc, CStringArray &argv)
	:	CFuncObject(ParentFuncFile, ParentFunc, argv)
{
	ASSERT(argv.GetSize() >= 10);
	int i;
	CString strTemp;

	if (argv.GetSize() >= 5) {
		strTemp = argv[4];
		for (i=0; i<strTemp.GetLength()/2; i++) m_OpCodeBytes.Add((BYTE)strtoul(strTemp.Mid(i*2, 2), NULL, 16));
	}
	if (argv.GetSize() >= 6) {
		strTemp = argv[5];
		for (i=0; i<strTemp.GetLength()/2; i++) m_OperandBytes.Add((BYTE)strtoul(strTemp.Mid(i*2, 2), NULL, 16));
	}
	if (argv.GetSize() >= 7) m_strDstOperand = argv[6];
	if (argv.GetSize() >= 8) m_strSrcOperand = argv[7];
	if (argv.GetSize() >= 9) m_strOpCodeText = argv[8];
	if (argv.GetSize() >= 10) m_strOperandText = argv[9];
}

CFuncAsmInstObject::~CFuncAsmInstObject()
{

}

CString CFuncAsmInstObject::ExportToDiff()
{
	CString strRetVal;
	CString strTemp;
	CString strTemp2;
	int i;
	DWORD nFuncStartAddr = m_ParentFunc.GetMainAddress();
	DWORD nFuncSize = m_ParentFunc.GetFuncSize();
	DWORD dwTemp;
	signed long snTemp;
	TMemRange zFuncRange(nFuncStartAddr, nFuncSize, 0);
	int pos;

	strRetVal.Format("C|%ld|", GetByteCount());
	strRetVal += GetOpCodeBytes();

	for (i=0; i<2; i++) {
		switch (i) {
			case 0:
				strTemp = m_strDstOperand;
				break;
			case 1:
				strTemp = m_strSrcOperand;
				break;
		}

		if (strTemp.IsEmpty()) continue;

		strRetVal += "|";

		switch (strTemp.GetAt(0)) {
			case '#':				// Immediate Data
				strRetVal += strTemp;
				break;
			case 'C':				// Code addressing
				if (strTemp.GetLength() < 2) continue;
				switch (strTemp.GetAt(1)) {
					case '@':		// Absolute
						dwTemp = strtoul(strTemp.Mid(2), NULL, 16);
						if (m_ParentFuncFile.AddrHasLabel(dwTemp)) {
							// If the address has a user-defined file-level label, use it instead:
							strRetVal += "C=" + m_ParentFuncFile.GetPrimaryLabel(dwTemp);
						} else {
							// If there isn't a label, see if this absolute address lies inside the function:
							if (zFuncRange.AddressInRange(dwTemp)) {
								// If so, convert and treat as a relative address:
								snTemp = dwTemp - (GetAbsAddress() + GetByteCount());
								strTemp2.Format("C^%c%lX",
											((snTemp != labs(snTemp)) ? '-' : '+'),
											labs(snTemp));
								strRetVal += strTemp2;
							} else {
								// Otherwise, it is an outside reference to something unknown:
								strRetVal += "C?";
							}
						}
						// If there is a mask (or other appendage) add it:
						pos = strTemp.Find(',');
						if (pos != -1) strRetVal += strTemp.Mid(pos);
						break;
					case '^':		// Relative
						// Relative addressing is always relative to the "next byte" after the instruction:
						if (strTemp.GetLength() < 3) continue;
						pos = strTemp.Find('(', 3);
						if (pos != -1) {
							strTemp2 = strTemp.Mid(3, pos-3);
							pos = strTemp2.Find(')');
							if (pos != -1) strTemp2 = strTemp2.Left(pos);
						} else {
							strTemp2 = strTemp.Mid(3);
							pos = strTemp2.Find(',');
							if (pos != -1) strTemp2 = strTemp2.Left(pos);
						}
						snTemp = strtoul(strTemp2, NULL, 16);
						if (strTemp.GetAt(2) == '-') snTemp = 0 - snTemp;
						dwTemp = GetAbsAddress() + GetByteCount() + snTemp;		// Resolve address
						if (m_ParentFuncFile.AddrHasLabel(dwTemp)) {
							// If the address has a user-defined file-level label, use it instead:
							strRetVal += "C=" + m_ParentFuncFile.GetPrimaryLabel(dwTemp);
						} else {
							// If there isn't a label, see if this relative address lies inside the function:
							if (zFuncRange.AddressInRange(dwTemp)) {
								// If so, continue to treat it as a relative address:
								strTemp2.Format("C^%c%lX",
											((snTemp != labs(snTemp)) ? '-' : '+'),
											labs(snTemp));
								strRetVal += strTemp2;
							} else {
								// Otherwise, it is an outside reference to something unknown:
								strRetVal += "C?";
							}
						}
						// If there is a mask (or other appendage) add it:
						pos = strTemp.Find(',');
						if (pos != -1) strRetVal += strTemp.Mid(pos);
						break;
					case '&':		// Reg. Offset
						strRetVal += strTemp;
						break;
				}
				break;
			case 'D':				// Data addressing
				if (strTemp.GetLength() < 2) continue;
				switch (strTemp.GetAt(1)) {
					case '@':		// Absolute
						dwTemp = strtoul(strTemp.Mid(2), NULL, 16);
						if (m_ParentFuncFile.AddrHasLabel(dwTemp)) {
							// If the address has a user-defined file-level label, use it instead:
							strRetVal += "D=" + m_ParentFuncFile.GetPrimaryLabel(dwTemp);
						} else {
							// If there isn't a label, see if this absolute address lies inside the function:
							if (zFuncRange.AddressInRange(dwTemp)) {
								// If so, convert and treat as a relative address:
								snTemp = dwTemp - (GetAbsAddress() + GetByteCount());
								strTemp2.Format("D^%c%lX",
											((snTemp != labs(snTemp)) ? '-' : '+'),
											labs(snTemp));
								strRetVal += strTemp2;
							} else {
								// See if it is an I/O or NON-ROM/RAM location:
								if ((m_ParentFuncFile.IsIOMemAddr(dwTemp)) ||
									(!m_ParentFuncFile.IsROMMemAddr(dwTemp) && !m_ParentFuncFile.IsRAMMemAddr(dwTemp))) {
									// If so, treat create a label to reference it as it is significant:
									strTemp2.Format("D=L%04lX", dwTemp);
									strRetVal += strTemp2;
								} else {
									// Otherwise, it is an outside reference to something unknown in RAM/ROM
									//		and is probably a variable in memory that can move:
									strRetVal += "D?";
								}
							}
						}
						// If there is a mask (or other appendage) add it:
						pos = strTemp.Find(',');
						if (pos != -1) strRetVal += strTemp.Mid(pos);
						break;
					case '^':		// Relative
						// Relative addressing is always relative to the "next byte" after the instruction:
						if (strTemp.GetLength() < 3) continue;
						pos = strTemp.Find('(', 3);
						if (pos != -1) {
							strTemp2 = strTemp.Mid(3, pos-3);
							pos = strTemp2.Find(')');
							if (pos != -1) strTemp2 = strTemp2.Left(pos);
						} else {
							strTemp2 = strTemp.Mid(3);
							pos = strTemp2.Find(',');
							if (pos != -1) strTemp2 = strTemp2.Left(pos);
						}
						snTemp = strtoul(strTemp2, NULL, 16);
						if (strTemp.GetAt(2) == '-') snTemp = 0 - snTemp;
						dwTemp = GetAbsAddress() + GetByteCount() + snTemp;		// Resolve address
						if (m_ParentFuncFile.AddrHasLabel(dwTemp)) {
							// If the address has a user-defined file-level label, use it instead:
							strRetVal += "D=" + m_ParentFuncFile.GetPrimaryLabel(dwTemp);
						} else {
							// If there isn't a label, see if this relative address lies inside the function:
							if (zFuncRange.AddressInRange(dwTemp)) {
								// If so, continue to treat it as a relative address:
								strTemp2.Format("D^%c%lX",
											((snTemp != labs(snTemp)) ? '-' : '+'),
											labs(snTemp));
								strRetVal += strTemp2;
							} else {
								// See if it is an I/O or NON-ROM/RAM location:
								if ((m_ParentFuncFile.IsIOMemAddr(dwTemp)) ||
									(!m_ParentFuncFile.IsROMMemAddr(dwTemp) && !m_ParentFuncFile.IsRAMMemAddr(dwTemp))) {
									// If so, treat create a label to reference it as it is significant:
									strTemp2.Format("D=L%04lX", dwTemp);
									strRetVal += strTemp2;
								} else {
									// Otherwise, it is an outside reference to something unknown in RAM/ROM
									//		and is probably a variable in memory that can move:
									strRetVal += "D?";
								}
							}
						}
						// If there is a mask (or other appendage) add it:
						pos = strTemp.Find(',');
						if (pos != -1) strRetVal += strTemp.Mid(pos);
						break;
					case '&':		// Reg. Offset
						strRetVal += strTemp;
						break;
				}
				break;
		}

	}

	return strRetVal;
}

void CFuncAsmInstObject::ExportToDiff(CStringArray &anArray)
{
	anArray.Add(ExportToDiff());
}

CString CFuncAsmInstObject::CreateOutputLine(DWORD nOutputOptions)
{
	CString strRetVal = "";
	CString strTemp;

	if (nOutputOptions & OO_ADD_ADDRESS) {
		strTemp.Format("%04lX ", m_dwAbsAddress);
		strRetVal += PadString(strTemp, GetFieldWidth(FC_ADDRESS));
	}

	strTemp = m_ParentFunc.GetPrimaryLabel(m_dwAbsAddress);
	strRetVal += PadString(strTemp + ((!strTemp.IsEmpty()) ? ": " : " "), GetFieldWidth(FC_LABEL));
	strRetVal += PadString(m_strOpCodeText + " ", GetFieldWidth(FC_MNEMONIC));
	strRetVal += PadString(m_strOperandText, GetFieldWidth(FC_OPERANDS));

	return strRetVal;
}

void CFuncAsmInstObject::GetSymbols(CStringArray &aSymbolArray)
{
	CString strTemp;
	CString strTemp2;
	CString strLabelPrefix;
	int i;
	DWORD dwTemp;
	signed long snTemp;
	int pos;

	// Call parent to build initial list:
	CFuncObject::GetSymbols(aSymbolArray);

	for (i=0; i<2; i++) {
		switch (i) {
			case 0:
				strTemp = m_strDstOperand;
				strLabelPrefix = "RD";
				break;
			case 1:
				strTemp = m_strSrcOperand;
				strLabelPrefix = "RS";
				break;
		}

		if (strTemp.IsEmpty()) continue;

		switch (strTemp.GetAt(0)) {
			case '#':				// Immediate Data
				break;
			case 'C':				// Code addressing
				if (strTemp.GetLength() < 2) continue;
				switch (strTemp.GetAt(1)) {
					case '@':		// Absolute
						dwTemp = strtoul(strTemp.Mid(2), NULL, 16);
						if (m_ParentFuncFile.AddrHasLabel(dwTemp)) {
							// If the address has a user-defined file-level label, use it instead:
							aSymbolArray.Add(strLabelPrefix + "C" + m_ParentFuncFile.GetPrimaryLabel(dwTemp));
						} else {
							// Else, build a label:
							strTemp2.Format("L%04lX", dwTemp);
							aSymbolArray.Add(strLabelPrefix + "C" + strTemp2);
						}
						break;
					case '^':		// Relative
						// Relative addressing is always relative to the "next byte" after the instruction:
						if (strTemp.GetLength() < 3) continue;
						pos = strTemp.Find('(', 3);
						if (pos != -1) {
							strTemp2 = strTemp.Mid(3, pos-3);
							pos = strTemp2.Find(')');
							if (pos != -1) strTemp2 = strTemp2.Left(pos);
						} else {
							strTemp2 = strTemp.Mid(3);
							pos = strTemp2.Find(',');
							if (pos != -1) strTemp2 = strTemp2.Left(pos);
						}
						snTemp = strtoul(strTemp2, NULL, 16);
						if (strTemp.GetAt(2) == '-') snTemp = 0 - snTemp;
						dwTemp = GetAbsAddress() + GetByteCount() + snTemp;		// Resolve address
						if (m_ParentFuncFile.AddrHasLabel(dwTemp)) {
							// If the address has a user-defined file-level label, use it instead:
							aSymbolArray.Add(strLabelPrefix + "C" + m_ParentFuncFile.GetPrimaryLabel(dwTemp));
						} else {
							// Else, build a label:
							strTemp2.Format("L%04lX", dwTemp);
							aSymbolArray.Add(strLabelPrefix + "C" + strTemp2);
						}
						break;
					case '&':		// Reg. Offset
						break;
				}
				break;
			case 'D':				// Data addressing
				if (strTemp.GetLength() < 2) continue;
				switch (strTemp.GetAt(1)) {
					case '@':		// Absolute
						dwTemp = strtoul(strTemp.Mid(2), NULL, 16);
						if (m_ParentFuncFile.AddrHasLabel(dwTemp)) {
							// If the address has a user-defined file-level label, use it instead:
							aSymbolArray.Add(strLabelPrefix + "D" + m_ParentFuncFile.GetPrimaryLabel(dwTemp));
						} else {
							// Else, build a label:
							strTemp2.Format("L%04lX", dwTemp);
							aSymbolArray.Add(strLabelPrefix + "D" + strTemp2);
						}
						break;
					case '^':		// Relative
						// Relative addressing is always relative to the "next byte" after the instruction:
						if (strTemp.GetLength() < 3) continue;
						pos = strTemp.Find('(', 3);
						if (pos != -1) {
							strTemp2 = strTemp.Mid(3, pos-3);
							pos = strTemp2.Find(')');
							if (pos != -1) strTemp2 = strTemp2.Left(pos);
						} else {
							strTemp2 = strTemp.Mid(3);
							pos = strTemp2.Find(',');
							if (pos != -1) strTemp2 = strTemp2.Left(pos);
						}
						snTemp = strtoul(strTemp2, NULL, 16);
						if (strTemp.GetAt(2) == '-') snTemp = 0 - snTemp;
						dwTemp = GetAbsAddress() + GetByteCount() + snTemp;		// Resolve address
						if (m_ParentFuncFile.AddrHasLabel(dwTemp)) {
							// If the address has a user-defined file-level label, use it instead:
							aSymbolArray.Add(strLabelPrefix + "D" + m_ParentFuncFile.GetPrimaryLabel(dwTemp));
						} else {
							// Else, build a label:
							strTemp2.Format("L%04lX", dwTemp);
							aSymbolArray.Add(strLabelPrefix + "D" + strTemp2);
						}
						break;
					case '&':		// Reg. Offset
						break;
				}
				break;
		}
	}
}

CString CFuncAsmInstObject::GetOpCodeBytes()
{
	CString strRetVal = "";
	CString strTemp;
	int i;

	for (i=0; i<m_OpCodeBytes.GetSize(); i++) {
		strTemp.Format("%02X", m_OpCodeBytes.GetAt(i));
		strRetVal +=  strTemp;
	}
	return strRetVal;
}

int CFuncAsmInstObject::GetOpCodeByteCount()
{
	return m_OpCodeBytes.GetSize();
}

CString CFuncAsmInstObject::GetOperandBytes()
{
	CString strRetVal = "";
	CString strTemp;
	int i;

	for (i=0; i<m_OperandBytes.GetSize(); i++) {
		strTemp.Format("%02X", m_OperandBytes.GetAt(i));
		strRetVal +=  strTemp;
	}
	return strRetVal;
}

int CFuncAsmInstObject::GetOperandByteCount()
{
	return m_OperandBytes.GetSize();
}


//////////////////////////////////////////////////////////////////////
// CFuncDataByteObject Class
//////////////////////////////////////////////////////////////////////
CFuncDataByteObject::CFuncDataByteObject(CFuncDescFile &ParentFuncFile, CFuncDesc &ParentFunc, CStringArray &argv)
	:	CFuncObject(ParentFuncFile, ParentFunc, argv)
{

}

CFuncDataByteObject::~CFuncDataByteObject()
{

}

CString CFuncDataByteObject::ExportToDiff()
{
	CString strRetVal;

	strRetVal.Format("D|%ld|", GetByteCount());
	strRetVal += GetBytes();
	return strRetVal;
}

void CFuncDataByteObject::ExportToDiff(CStringArray &anArray)
{
	anArray.Add(ExportToDiff());
}

CString CFuncDataByteObject::CreateOutputLine(DWORD nOutputOptions)
{
	CString strRetVal = "";
	CString strTemp;
	CString strTemp2;
	int i;

	if (nOutputOptions & OO_ADD_ADDRESS) {
		strTemp.Format("%04lX ", m_dwAbsAddress);
		strRetVal += PadString(strTemp, GetFieldWidth(FC_ADDRESS));
	}

	strTemp = m_ParentFunc.GetPrimaryLabel(m_dwAbsAddress);
	strRetVal += PadString(strTemp + ((!strTemp.IsEmpty()) ? ": " : " "), GetFieldWidth(FC_LABEL));
	strRetVal += PadString(".data ", GetFieldWidth(FC_MNEMONIC));
	strTemp2 = "";
	for (i=0; i<m_Bytes.GetSize(); i++) {
		if (i!=0) {
			strTemp.Format(", 0x%02X", m_Bytes.GetAt(i));
		} else {
			strTemp.Format("0x%02X", m_Bytes.GetAt(i));
		}
		strTemp2 += strTemp;
	}
	strRetVal += PadString(strTemp2, GetFieldWidth(FC_OPERANDS));

	return strRetVal;
}


//////////////////////////////////////////////////////////////////////
// CFuncDesc Class
//////////////////////////////////////////////////////////////////////

CFuncDesc::CFuncDesc(DWORD nAddress, LPCTSTR pszNames)
{
	CStringArray argv;
	int i;

	m_dwFunctionSize = 0;
	m_dwMainAddress = nAddress;

	ParseLine(pszNames, ',', argv);
	for (i=0; i<argv.GetSize(); i++) AddName(nAddress, argv[i]);
}

CFuncDesc::~CFuncDesc()
{
	FreeAll();
}

void CFuncDesc::FreeAll()
{
	CFuncObject *pFuncObject;
	int i;
	POSITION pos;
	DWORD anAddress;
	CStringArray *asArray;

	// Release function objects:
	for (i=0; i<GetSize(); i++) {
		pFuncObject = GetAt(i);
		if (pFuncObject) delete pFuncObject;
	}
	RemoveAll();

	// Release names table:
	pos = m_FuncNameTable.GetStartPosition();
	while (pos) {
		m_FuncNameTable.GetNextAssoc(pos, anAddress, asArray);
		if (asArray != NULL) delete asArray;
	}
	m_FuncNameTable.RemoveAll();

	// Release label table:
	pos = m_LabelTable.GetStartPosition();
	while (pos) {
		m_LabelTable.GetNextAssoc(pos, anAddress, asArray);
		if (asArray != NULL) delete asArray;
	}
	m_LabelTable.RemoveAll();
}

BOOL CFuncDesc::AddName(DWORD nAddress, LPCTSTR pszLabel)
{
	CStringArray *pNameList;
	CString strTemp;
	int i;

	if (pszLabel == NULL) return FALSE;
	strTemp = pszLabel;
	if (strTemp.IsEmpty()) return FALSE;
	if (m_FuncNameTable.Lookup(nAddress, pNameList)) {
		if (pNameList == NULL) {
			ASSERT(FALSE);			// We should never encounter a null entry -- check adding!
			return FALSE;
		}
		// If we have the label already, return:
		for (i=0; i<pNameList->GetSize(); i++) {
			if (strTemp.CompareNoCase(pNameList->GetAt(i)) == 0) return FALSE;
		}
	} else {
		pNameList = new CStringArray;
		if (pNameList == NULL) AfxThrowMemoryException();
		m_FuncNameTable.SetAt(nAddress, pNameList);
	}
	ASSERT(pNameList != NULL);
	pNameList->Add(strTemp);
	return TRUE;
}

CString CFuncDesc::GetMainName()
{
	CStringArray *pNameList;
	CString strRetVal;

	strRetVal.Format("L%0lX", m_dwMainAddress);

	if (m_FuncNameTable.Lookup(m_dwMainAddress, pNameList)) {
		if (pNameList == NULL) {
			ASSERT(FALSE);			// We should never encounter a null entry -- check adding!
			return strRetVal;
		}
		if (pNameList->GetSize() < 1)  return strRetVal;
		if (pNameList->GetAt(0).Compare("???") == 0) return strRetVal;
		return pNameList->GetAt(0);
	}

	return strRetVal;
}

DWORD CFuncDesc::GetMainAddress()
{
	return m_dwMainAddress;
}

BOOL CFuncDesc::AddLabel(DWORD nAddress, LPCTSTR pszLabel)
{
	CStringArray *pLabelList;
	CString strTemp;
	int i;

	if (pszLabel == NULL) return FALSE;
	strTemp = pszLabel;
	if (strTemp.IsEmpty()) return FALSE;
	if (m_LabelTable.Lookup(nAddress, pLabelList)) {
		if (pLabelList == NULL) {
			ASSERT(FALSE);			// We should never encounter a null entry -- check adding!
			return FALSE;
		}
		// If we have the label already, return:
		for (i=0; i<pLabelList->GetSize(); i++) {
			if (strTemp.CompareNoCase(pLabelList->GetAt(i)) == 0) return FALSE;
		}
	} else {
		pLabelList = new CStringArray;
		if (pLabelList == NULL) AfxThrowMemoryException();
		m_LabelTable.SetAt(nAddress, pLabelList);
	}
	ASSERT(pLabelList != NULL);
	pLabelList->Add(strTemp);
	return TRUE;
}

BOOL CFuncDesc::AddrHasLabel(DWORD nAddress)
{
	CStringArray *pLabelList;

	if (m_LabelTable.Lookup(nAddress, pLabelList)) return TRUE;
	return FALSE;
}

CString CFuncDesc::GetPrimaryLabel(DWORD nAddress)
{
	CStringArray *pLabelList;

	if (m_LabelTable.Lookup(nAddress, pLabelList)) {
		if (pLabelList == NULL) {
			ASSERT(FALSE);			// We should never encounter a null entry -- check adding!
			return "";
		}
		if (pLabelList->GetSize()<1) return "";
		return pLabelList->GetAt(0);
	}
	return "";
}

CStringArray *CFuncDesc::GetLabelList(DWORD nAddress)
{
	CStringArray *pLabelList;

	if (m_LabelTable.Lookup(nAddress, pLabelList)) return pLabelList;

	return NULL;
}

DWORD CFuncDesc::GetFuncSize()
{
//	CFuncObject *pFuncObject;
//	int i;
//	DWORD nRetVal;
//
//	nRetVal = 0;
//
//	// Add byte counts to get size:
//	for (i=0; i<GetSize(); i++) {
//		pFuncObject = GetAt(i);
//		if (pFuncObject) nRetVal += pFuncObject->GetByteCount();
//	}
//
//	return nRetVal;

	// For efficiency, the above has been replaced with the following:
	return m_dwFunctionSize;
}

CString CFuncDesc::ExportToDiff()
{
	CFuncObject *pFuncObject;
	int i;
	CString strRetVal;

	strRetVal = "";

	// Concatenate output from each member object to create composite:
	for (i=0; i<GetSize(); i++) {
		pFuncObject = GetAt(i);
		if (pFuncObject) strRetVal += pFuncObject->ExportToDiff() + "\n";
	}

	return strRetVal;
}

void CFuncDesc::ExportToDiff(CStringArray &anArray)
{
	CFuncObject *pFuncObject;
	int i;

	anArray.RemoveAll();

	// Add each item to array to create overall:
	for (i=0; i<GetSize(); i++) {
		pFuncObject = GetAt(i);
		if (pFuncObject) pFuncObject->ExportToDiff(anArray);
	}
}

int CFuncDesc::Add(CFuncObject *anObj)
{
	int i;
	DWORD nObjAddress;

	if (anObj) {
		nObjAddress = anObj->GetAbsAddress();
		for (i=0; i<anObj->GetLabelCount(); i++) {
			AddLabel(nObjAddress, anObj->GetLabel(i));
		}
		m_dwFunctionSize += anObj->GetByteCount();
		return CTypedPtrArray<CObArray, CFuncObject *>::Add(anObj);
	}

	return -1;
}


//////////////////////////////////////////////////////////////////////
// CFuncDescFile Class
//////////////////////////////////////////////////////////////////////

CFuncDescFile::CFuncDescFile()
	:	m_ROMMemMap(0, 0, 0),
		m_RAMMemMap(0, 0, 0),
		m_IOMemMap(0, 0, 0)
{
	ParseCmdsOP1.SetAt("ROM", 0);
	ParseCmdsOP1.SetAt("RAM", 1);
	ParseCmdsOP1.SetAt("IO", 2);

	m_pfnProgressCallback = NULL;
	m_lParamProgressCallback = 0;
}

CFuncDescFile::~CFuncDescFile()
{
	FreeAll();
}

void CFuncDescFile::FreeAll()
{
	POSITION pos;
	DWORD anAddress;
	CStringArray *asArray;
	CFuncDesc *pFuncDesc;
	int i;

	// Release Functions:
	for (i=0; i<m_Functions.GetSize(); i++) {
		pFuncDesc = m_Functions.GetAt(i);
		if (pFuncDesc) delete pFuncDesc;
	}
	m_Functions.RemoveAll();

	// Release label table:
	pos = m_LabelTable.GetStartPosition();
	while (pos) {
		m_LabelTable.GetNextAssoc(pos, anAddress, asArray);
		if (asArray != NULL) delete asArray;
	}
	m_LabelTable.RemoveAll();
}

BOOL CFuncDescFile::ReadFuncDescFile(CFile& inFile, CFile *msgFile, CFile *errFile, int nStartLineCount)
{
	CString strLine;
	CString strTemp;
	int nLineCount = nStartLineCount;
	CFuncDesc *pCurrentFunction = NULL;
	CFuncObject *pFuncObj;
	BOOL bRetVal = TRUE;
	CString strError = m_strUnexpectedError;
	WORD nParseVal;
	DWORD nTemp;
	DWORD nAddress;
	DWORD nSize;
	BOOL bTemp;
	CStringArray argv;
	int i,j;
	TMemRange *pMemRange;
	CDWordArray SortedList;
	CStringArray *pStrArray;
	POSITION mpos;

#define BUSY_CALLBACK_RATE 50
#define BUSY_CALLBACK_RATE2 10

	if (msgFile) {
		strTemp = "Reading Function Definition File \"" + inFile.GetFileName() + "\"...\n";
		WriteFileString(*msgFile, strTemp);
	}

	m_strFilePathName = inFile.GetFilePath();
	m_strFileName = inFile.GetFileName();

	while ((bRetVal) && (ReadFileString(inFile, strLine))) {
		nLineCount++;
		strLine.TrimLeft();
		strLine.TrimRight();

		if ((m_pfnProgressCallback) && ((nLineCount % BUSY_CALLBACK_RATE) == 0))
			m_pfnProgressCallback(0, 1, FALSE, m_lParamProgressCallback);

		// Empty lines are ignored:
		if (strLine.IsEmpty()) continue;

		switch (strLine.GetAt(0)) {
			case ';':			// Comments
				// Ignore comment lines:
				break;

			case '#':			// Memory Mapping
				pCurrentFunction = NULL;

				ParseLine(strLine.Mid(1), '|', argv);
				if (argv.GetSize() != 3) {
					strError = m_strSyntaxError;
					bRetVal = FALSE;
					break;
				}

				if (!ParseCmdsOP1.Lookup(argv[0], nParseVal)) {
					strError = m_strSyntaxError;
					bRetVal = FALSE;
					break;
				}

				nAddress = strtoul(argv[1], NULL, 16);
				nSize = strtoul(argv[2], NULL, 16);

				switch (nParseVal) {
					case 0:		// ROM
						m_ROMMemMap.AddRange(nAddress, nSize, 0);
						m_ROMMemMap.Compact();
						m_ROMMemMap.RemoveOverlaps();
						m_ROMMemMap.Sort();
						bTemp = TRUE;
						if (errFile) {
							for (nTemp = nAddress; ((nTemp < (nAddress + nSize)) && (bTemp)); nTemp++) {
								if (m_RAMMemMap.AddressInRange(nTemp)) {
									bTemp = FALSE;
									WriteFileString(*errFile, "*** Warning: Specified ROM Mapping conflicts with RAM Mapping\n");
								} else {
									if (m_IOMemMap.AddressInRange(nTemp)) {
										bTemp = FALSE;
										WriteFileString(*errFile, "*** Warning: Specified ROM Mapping conflicts with IO Mapping\n");
									}
								}
							}
						}
						break;
					case 1:		// RAM
						m_RAMMemMap.AddRange(nAddress, nSize, 0);
						m_RAMMemMap.Compact();
						m_RAMMemMap.RemoveOverlaps();
						m_RAMMemMap.Sort();
						bTemp = TRUE;
						if (errFile) {
							for (nTemp = nAddress; ((nTemp < (nAddress + nSize)) && (bTemp)); nTemp++) {
								if (m_ROMMemMap.AddressInRange(nTemp)) {
									bTemp = FALSE;
									WriteFileString(*errFile, "*** Warning: Specified RAM Mapping conflicts with ROM Mapping\n");
								} else {
									if (m_IOMemMap.AddressInRange(nTemp)) {
										bTemp = FALSE;
										WriteFileString(*errFile, "*** Warning: Specified RAM Mapping conflicts with IO Mapping\n");
									}
								}
							}
						}
						break;
					case 2:		// IO
						m_IOMemMap.AddRange(nAddress, nSize, 0);
						m_IOMemMap.Compact();
						m_IOMemMap.RemoveOverlaps();
						m_IOMemMap.Sort();
						bTemp = TRUE;
						if (errFile) {
							for (nTemp = nAddress; ((nTemp < (nAddress + nSize)) && (bTemp)); nTemp++) {
								if (m_ROMMemMap.AddressInRange(nTemp)) {
									bTemp = FALSE;
									WriteFileString(*errFile, "*** Warning: Specified IO Mapping conflicts with ROM Mapping\n");
								} else {
									if (m_RAMMemMap.AddressInRange(nTemp)) {
										bTemp = FALSE;
										WriteFileString(*errFile, "*** Warning: Specified IO Mapping conflicts with RAM Mapping\n");
									}
								}
							}
						}
						break;
				}
				break;

			case '!':			// Label
				pCurrentFunction = NULL;

				ParseLine(strLine.Mid(1), '|', argv);
				if (argv.GetSize() != 2) {
					strError = m_strSyntaxError;
					bRetVal = FALSE;
					break;
				}

				nAddress = strtoul(argv[0], NULL, 16);
				ParseLine(argv[1], ',', argv);

				for (i=0; i<argv.GetSize(); i++) {
					AddLabel(nAddress, argv[i]);
				}
				break;

			case '@':			// New Function declaration:
				pCurrentFunction = NULL;

				ParseLine(strLine.Mid(1), '|', argv);
				if (argv.GetSize() != 2) {
					strError = m_strSyntaxError;
					bRetVal = FALSE;
					break;
				}

				nAddress = strtoul(argv[0], NULL, 16);

				pCurrentFunction = new CFuncDesc(nAddress, argv[1]);
				if (pCurrentFunction == NULL) {
					strError = m_strOutOfMemoryError;
					bRetVal = FALSE;
					break;
				}
				m_Functions.Add(pCurrentFunction);
				break;

			default:
				// See if we are in the middle of a function declaration:
				if (pCurrentFunction == NULL) {
					// If we aren't in a function, it's a syntax error:
					strError = m_strSyntaxError;
					bRetVal = FALSE;
					break;
				}

				// If we are in a function, parse entry:
				if (isxdigit(strLine.GetAt(0))) {
					ParseLine(strLine, '|', argv);
					if ((argv.GetSize() != 4) &&
						(argv.GetSize() != 10)) {
						strError = m_strSyntaxError;
						bRetVal = FALSE;
						break;
					}

					pFuncObj = NULL;
					if (argv.GetSize() == 4) {
						pFuncObj = new CFuncDataByteObject(*this, *pCurrentFunction, argv);
					} else {
						pFuncObj = new CFuncAsmInstObject(*this, *pCurrentFunction, argv);
					}
					pCurrentFunction->Add(pFuncObj);

				} else {
					strError = m_strSyntaxError;
					bRetVal = FALSE;
				}
				break;
		}
	}

	if ((bRetVal) && (msgFile)) {
		WriteFileString(*msgFile, "\n");

		WriteFileString(*msgFile, "    Memory Mappings:\n");
		WriteFileString(*msgFile, "        ROM Memory Map:");
		if (m_ROMMemMap.IsNullRange()) {
			WriteFileString(*msgFile, " <Not Defined>\n");
		} else {
			WriteFileString(*msgFile, "\n");
			pMemRange = &m_ROMMemMap;
			while (pMemRange) {
				strTemp.Format("            0x%04lX - 0x%04lX  (Size: 0x%04lX)\n",
							pMemRange->GetStartAddr(),
							pMemRange->GetStartAddr() + pMemRange->GetSize() - 1,
							pMemRange->GetSize());
				WriteFileString(*msgFile, strTemp);
				pMemRange = pMemRange->GetNext();
			}
		}
		WriteFileString(*msgFile, "        RAM Memory Map:");
		if (m_RAMMemMap.IsNullRange()) {
			WriteFileString(*msgFile, " <Not Defined>\n");
		} else {
			WriteFileString(*msgFile, "\n");
			pMemRange = &m_RAMMemMap;
			while (pMemRange) {
				strTemp.Format("            0x%04lX - 0x%04lX  (Size: 0x%04lX)\n",
							pMemRange->GetStartAddr(),
							pMemRange->GetStartAddr() + pMemRange->GetSize() - 1,
							pMemRange->GetSize());
				WriteFileString(*msgFile, strTemp);
				pMemRange = pMemRange->GetNext();
			}
		}
		WriteFileString(*msgFile, "         IO Memory Map:");
		if (m_IOMemMap.IsNullRange()) {
			WriteFileString(*msgFile, " <Not Defined>\n");
		} else {
			WriteFileString(*msgFile, "\n");
			pMemRange = &m_IOMemMap;
			while (pMemRange) {
				strTemp.Format("            0x%04lX - 0x%04lX  (Size: 0x%04lX)\n",
							pMemRange->GetStartAddr(),
							pMemRange->GetStartAddr() + pMemRange->GetSize() - 1,
							pMemRange->GetSize());
				WriteFileString(*msgFile, strTemp);
				pMemRange = pMemRange->GetNext();
			}
		}
		WriteFileString(*msgFile, "\n");

		strTemp.Format("    %ld Function%s defined%s\n",
									m_Functions.GetSize(),
									((m_Functions.GetSize() != 1) ? "s" : ""),
									((m_Functions.GetSize() != 0) ? ":" : ""));
		WriteFileString(*msgFile, strTemp);
		for (i=0; i<m_Functions.GetSize(); i++) {
			strTemp.Format("        %04lX -> %s\n", m_Functions.GetAt(i)->GetMainAddress(), LPCTSTR(m_Functions.GetAt(i)->GetMainName()));
			WriteFileString(*msgFile, strTemp);

			if ((m_pfnProgressCallback) && ((i % BUSY_CALLBACK_RATE2) == 0))
				m_pfnProgressCallback(0, 1, FALSE, m_lParamProgressCallback);
		}
		WriteFileString(*msgFile, "\n");


		strTemp.Format("    %ld Unique Label%s Defined%s\n",
									m_LabelTable.GetCount(),
									((m_LabelTable.GetCount() != 1) ? "s" : ""),
									((m_LabelTable.GetCount() != 0) ? ":" : ""));
		WriteFileString(*msgFile, strTemp);
		SortedList.RemoveAll();
		mpos = m_LabelTable.GetStartPosition();
		while (mpos) {
			m_LabelTable.GetNextAssoc(mpos, nAddress, pStrArray);
			bTemp = FALSE;
			for (i=0; ((i<SortedList.GetSize()) && (bTemp == FALSE)); i++) {
				if (nAddress <= SortedList.GetAt(i)) {
					SortedList.InsertAt(i, nAddress);
					bTemp = TRUE;
				}
			}
			if (bTemp == FALSE) SortedList.Add(nAddress);
		}
		for (i=0; i<SortedList.GetSize(); i++) {
			m_LabelTable.Lookup(SortedList.GetAt(i), pStrArray);
			strTemp.Format("        0x%04lX=", SortedList.GetAt(i));
			WriteFileString(*msgFile, strTemp);
			if (pStrArray) {
				for (j=0; j<pStrArray->GetSize(); j++) {
					if (j != 0) WriteFileString(*msgFile, ",");
					WriteFileString(*msgFile, pStrArray->GetAt(j));
				}
			}
			WriteFileString(*msgFile, "\n");

			if ((m_pfnProgressCallback) && ((i % BUSY_CALLBACK_RATE2) == 0))
				m_pfnProgressCallback(0, 1, FALSE, m_lParamProgressCallback);
		}
		WriteFileString(*msgFile, "\n");
	}

	if ((bRetVal == FALSE) && (errFile)) {
		strTemp.Format("*** Error: %s : on line %ld of file\n           \"%s\"\n",
							LPCTSTR(strError), nLineCount, LPCTSTR(inFile.GetFilePath()));
		WriteFileString(*errFile, strTemp);
	}

	return bRetVal;
}

BOOL CFuncDescFile::AddLabel(DWORD nAddress, LPCTSTR pszLabel)
{
	CStringArray *pLabelList;
	CString strTemp;
	int i;

	if (pszLabel == NULL) return FALSE;
	strTemp = pszLabel;
	if (strTemp.IsEmpty()) return FALSE;
	if (m_LabelTable.Lookup(nAddress, pLabelList)) {
		if (pLabelList == NULL) {
			ASSERT(FALSE);			// We should never encounter a null entry -- check adding!
			return FALSE;
		}
		// If we have the label already, return:
		for (i=0; i<pLabelList->GetSize(); i++) {
			if (strTemp.CompareNoCase(pLabelList->GetAt(i)) == 0) return FALSE;
		}
	} else {
		pLabelList = new CStringArray;
		if (pLabelList == NULL) AfxThrowMemoryException();
		m_LabelTable.SetAt(nAddress, pLabelList);
	}
	ASSERT(pLabelList != NULL);
	pLabelList->Add(strTemp);
	return TRUE;
}

BOOL CFuncDescFile::AddrHasLabel(DWORD nAddress)
{
	CStringArray *pLabelList;

	if (m_LabelTable.Lookup(nAddress, pLabelList)) return TRUE;
	return FALSE;
}

CString CFuncDescFile::GetPrimaryLabel(DWORD nAddress)
{
	CStringArray *pLabelList;

	if (m_LabelTable.Lookup(nAddress, pLabelList)) {
		if (pLabelList == NULL) {
			ASSERT(FALSE);			// We should never encounter a null entry -- check adding!
			return "";
		}
		if (pLabelList->GetSize()<1) return "";
		return pLabelList->GetAt(0);
	}
	return "";
}

CStringArray *CFuncDescFile::GetLabelList(DWORD nAddress)
{
	CStringArray *pLabelList;

	if (m_LabelTable.Lookup(nAddress, pLabelList)) return pLabelList;

	return NULL;
}

int CFuncDescFile::GetFuncCount()
{
	return m_Functions.GetSize();
}

CFuncDesc *CFuncDescFile::GetFunc(int nIndex)
{
	return m_Functions.GetAt(nIndex);
}

CString CFuncDescFile::GetFuncPathName()
{
	return m_strFilePathName;
}

CString CFuncDescFile::GetFuncFileName()
{
	return m_strFileName;
}

BOOL CFuncDescFile::IsROMMemAddr(DWORD nAddress)
{
	return m_ROMMemMap.AddressInRange(nAddress);
}

BOOL CFuncDescFile::IsRAMMemAddr(DWORD nAddress)
{
	return m_RAMMemMap.AddressInRange(nAddress);
}

BOOL CFuncDescFile::IsIOMemAddr(DWORD nAddress)
{
	return m_IOMemMap.AddressInRange(nAddress);
}


//////////////////////////////////////////////////////////////////////
// CFuncDescFileArray Class
//////////////////////////////////////////////////////////////////////

CFuncDescFileArray::CFuncDescFileArray()
{
	m_pfnProgressCallback = NULL;
	m_lParamProgressCallback = 0;
}

CFuncDescFileArray::~CFuncDescFileArray()
{
	FreeAll();
}

void CFuncDescFileArray::FreeAll()
{
	CFuncDescFile *pFuncDescFileObj;
	int i;

	// Release function objects:
	for (i=0; i<GetSize(); i++) {
		pFuncDescFileObj = GetAt(i);
		if (pFuncDescFileObj) delete pFuncDescFileObj;
	}
	RemoveAll();
}

int CFuncDescFileArray::GetFuncCount()
{
	int nRetVal = 0;
	int i;
	CFuncDescFile *pFuncDescFile;

	for (i=0; i<GetSize(); i++) {
		pFuncDescFile = GetAt(i);
		if (pFuncDescFile == NULL) continue;
		nRetVal += pFuncDescFile->GetFuncCount();
	}

	return nRetVal;
}

double CFuncDescFileArray::CompareFunctions(FUNC_COMPARE_METHOD nMethod,
											int nFile1Ndx, int nFile1FuncNdx,
											int nFile2Ndx, int nFile2FuncNdx,
											BOOL bBuildEditScript)
{
	CFuncDescFile *pFuncDescFile1Obj;
	CFuncDescFile *pFuncDescFile2Obj;

	pFuncDescFile1Obj = GetAt(nFile1Ndx);
	pFuncDescFile2Obj = GetAt(nFile2Ndx);

	return ::CompareFunctions(nMethod, pFuncDescFile1Obj, nFile1FuncNdx, pFuncDescFile2Obj, nFile2FuncNdx, bBuildEditScript);
}

CString CFuncDescFileArray::DiffFunctions(FUNC_COMPARE_METHOD nMethod,
									int nFile1Ndx, int nFile1FuncNdx,
									int nFile2Ndx, int nFile2FuncNdx,
									DWORD nOutputOptions,
									double &nMatchPercent,
									CSymbolMap *pSymbolMap)
{
	CFuncDescFile *pFuncDescFile1Obj;
	CFuncDescFile *pFuncDescFile2Obj;

	pFuncDescFile1Obj = GetAt(nFile1Ndx);
	pFuncDescFile2Obj = GetAt(nFile2Ndx);

	return ::DiffFunctions(nMethod, pFuncDescFile1Obj, nFile1FuncNdx, pFuncDescFile2Obj, nFile2FuncNdx,
							nOutputOptions, nMatchPercent, pSymbolMap);
}


//////////////////////////////////////////////////////////////////////
// CSymbolMap Class
//////////////////////////////////////////////////////////////////////

CSymbolMap::CSymbolMap()
{

}

CSymbolMap::~CSymbolMap()
{
	FreeAll();
}

void CSymbolMap::FreeAll()
{
	CStringArray *pArray;
	CString strTemp;
	POSITION pos;

	pos = m_LeftSideCodeSymbols.GetStartPosition();
	while (pos) {
		m_LeftSideCodeSymbols.GetNextAssoc(pos, strTemp, pArray);
		if (pArray != NULL) delete pArray;
	}
	m_LeftSideCodeSymbols.RemoveAll();

	pos = m_RightSideCodeSymbols.GetStartPosition();
	while (pos) {
		m_RightSideCodeSymbols.GetNextAssoc(pos, strTemp, pArray);
		if (pArray != NULL) delete pArray;
	}
	m_RightSideCodeSymbols.RemoveAll();

	pos = m_LeftSideDataSymbols.GetStartPosition();
	while (pos) {
		m_LeftSideDataSymbols.GetNextAssoc(pos, strTemp, pArray);
		if (pArray != NULL) delete pArray;
	}
	m_LeftSideDataSymbols.RemoveAll();

	pos = m_RightSideDataSymbols.GetStartPosition();
	while (pos) {
		m_RightSideDataSymbols.GetNextAssoc(pos, strTemp, pArray);
		if (pArray != NULL) delete pArray;
	}
	m_RightSideDataSymbols.RemoveAll();
}

BOOL CSymbolMap::IsEmpty()
{
	if (m_LeftSideCodeSymbols.GetCount() != 0) return FALSE;
	if (m_RightSideCodeSymbols.GetCount() != 0) return FALSE;
	if (m_LeftSideDataSymbols.GetCount() != 0) return FALSE;
	if (m_RightSideDataSymbols.GetCount() != 0) return FALSE;

	return TRUE;
}

void CSymbolMap::AddObjectMapping(CFuncObject &aLeftObject, CFuncObject &aRightObject)
{
	CStringArray arrLeftSymbols;
	CStringArray arrRightSymbols;
	int i;
	int j;
	CString strTemp1;
	CString strTemp2;
	BOOL bFlag;
	BOOL bFlag2;
	int nMode;		// 0 = Source, 1 = Destination
	int nType;		// 0 = Code, 1 = Data

	aLeftObject.GetSymbols(arrLeftSymbols);
	aRightObject.GetSymbols(arrRightSymbols);

	// Since we are comparing functions, any "L" entries are automatically "Code" entries:
	for (i=0; i<arrLeftSymbols.GetSize(); i++) {
		strTemp1 = arrLeftSymbols.GetAt(i);
		if ((strTemp1.IsEmpty()) || (strTemp1.GetAt(0) != 'L')) continue;
		if (strTemp1.GetLength() < 2) continue;
		bFlag = FALSE;
		for (j=0; j<arrRightSymbols.GetSize(); j++) {
			strTemp2 = arrRightSymbols.GetAt(j);
			if ((strTemp2.IsEmpty()) || (strTemp2.GetAt(0) != 'L')) continue;
			if (strTemp2.GetLength() < 2) continue;
			AddLeftSideCodeSymbol(strTemp1.Mid(1), strTemp2.Mid(1));
			bFlag = TRUE;
		}
		if (!bFlag) AddLeftSideCodeSymbol(strTemp1.Mid(1), "");
	}

	for (i=0; i<arrRightSymbols.GetSize(); i++) {
		strTemp1 = arrRightSymbols.GetAt(i);
		if ((strTemp1.IsEmpty()) || (strTemp1.GetAt(0) != 'L')) continue;
		if (strTemp1.GetLength() < 2) continue;
		bFlag = FALSE;
		for (j=0; j<arrLeftSymbols.GetSize(); j++) {
			strTemp2 = arrLeftSymbols.GetAt(j);
			if ((strTemp2.IsEmpty()) || (strTemp2.GetAt(0) != 'L')) continue;
			if (strTemp2.GetLength() < 2) continue;
			AddRightSideCodeSymbol(strTemp1.Mid(1), strTemp2.Mid(1));
			bFlag = TRUE;
		}
		if (!bFlag) AddRightSideCodeSymbol(strTemp1.Mid(1), "");
	}

	// Add Left Source/Destination entries:
	for (i=0; i<arrLeftSymbols.GetSize(); i++) {
		strTemp1 = arrLeftSymbols.GetAt(i);
		if ((strTemp1.IsEmpty()) || (strTemp1.GetAt(0) != 'R')) continue;
		if (strTemp1.GetLength() < 4) continue;
		switch (strTemp1.GetAt(1)) {
			case 'S':
				nMode = 0;
				break;
			case 'D':
				nMode = 1;
				break;
			default:
				continue;
		}
		switch (strTemp1.GetAt(2)) {
			case 'C':
				nType = 0;
				break;
			case 'D':
				nType = 1;
				break;
			default:
				continue;
		}
		bFlag = FALSE;
		for (j=0; j<arrRightSymbols.GetSize(); j++) {
			strTemp2 = arrRightSymbols.GetAt(j);
			if ((strTemp2.IsEmpty()) || (strTemp2.GetAt(0) != 'R')) continue;
			if (strTemp2.GetLength() < 4) continue;
			bFlag2 = FALSE;
			switch (strTemp2.GetAt(1)) {
				case 'S':
					if (nMode == 0) {
						bFlag2 = TRUE;
					}
					break;
				case 'D':
					if (nMode == 1) {
						bFlag2 = TRUE;
					}
					break;
				default:
					continue;
			}
			if (bFlag2) {
				switch (strTemp2.GetAt(2)) {
					case 'C':
						if (nType == 0) {
							AddLeftSideCodeSymbol(strTemp1.Mid(3), strTemp2.Mid(3));
							bFlag = TRUE;
						}
						break;
					case 'D':
						if (nType == 1) {
							AddLeftSideDataSymbol(strTemp1.Mid(3), strTemp2.Mid(3));
							bFlag = TRUE;
						}
						break;
					default:
						continue;
				}
			}
		}
		if (!bFlag) {
			switch (nType) {
				case 0:
					AddLeftSideCodeSymbol(strTemp1.Mid(3), "");
					break;
				case 1:
					AddLeftSideDataSymbol(strTemp1.Mid(3), "");
					break;
			}
		}
	}

	// Add Right Source/Destination entries:
	for (i=0; i<arrRightSymbols.GetSize(); i++) {
		strTemp1 = arrRightSymbols.GetAt(i);
		if ((strTemp1.IsEmpty()) || (strTemp1.GetAt(0) != 'R')) continue;
		if (strTemp1.GetLength() < 4) continue;
		switch (strTemp1.GetAt(1)) {
			case 'S':
				nMode = 0;
				break;
			case 'D':
				nMode = 1;
				break;
			default:
				continue;
		}
		switch (strTemp1.GetAt(2)) {
			case 'C':
				nType = 0;
				break;
			case 'D':
				nType = 1;
				break;
			default:
				continue;
		}
		bFlag = FALSE;
		for (j=0; j<arrLeftSymbols.GetSize(); j++) {
			strTemp2 = arrLeftSymbols.GetAt(j);
			if ((strTemp2.IsEmpty()) || (strTemp2.GetAt(0) != 'R')) continue;
			if (strTemp2.GetLength() < 4) continue;
			bFlag2 = FALSE;
			switch (strTemp2.GetAt(1)) {
				case 'S':
					if (nMode == 0) {
						bFlag2 = TRUE;
					}
					break;
				case 'D':
					if (nMode == 1) {
						bFlag2 = TRUE;
					}
					break;
				default:
					continue;
			}
			if (bFlag2) {
				switch (strTemp2.GetAt(2)) {
					case 'C':
						if (nType == 0) {
							AddRightSideCodeSymbol(strTemp1.Mid(3), strTemp2.Mid(3));
							bFlag = TRUE;
						}
						break;
					case 'D':
						if (nType == 1) {
							AddRightSideDataSymbol(strTemp1.Mid(3), strTemp2.Mid(3));
							bFlag = TRUE;
						}
						break;
					default:
						continue;
				}
			}
		}
		if (!bFlag) {
			switch (nType) {
				case 0:
					AddRightSideCodeSymbol(strTemp1.Mid(3), "");
					break;
				case 1:
					AddRightSideDataSymbol(strTemp1.Mid(3), "");
					break;
			}
		}
	}
}

void CSymbolMap::GetLeftSideCodeSymbolList(CStringArray &anArray)
{
	GetSymbolList(m_LeftSideCodeSymbols, anArray);
}

void CSymbolMap::GetRightSideCodeSymbolList(CStringArray &anArray)
{
	GetSymbolList(m_RightSideCodeSymbols, anArray);
}

void CSymbolMap::GetLeftSideDataSymbolList(CStringArray &anArray)
{
	GetSymbolList(m_LeftSideDataSymbols, anArray);
}

void CSymbolMap::GetRightSideDataSymbolList(CStringArray &anArray)
{
	GetSymbolList(m_RightSideDataSymbols, anArray);
}

void CSymbolMap::GetSymbolList(CMap<CString, LPCTSTR, CStringArray*, CStringArray*> &mapSymbolArrays,
								CStringArray &anArray)
{
	POSITION pos;
	CString strTemp;
	CStringArray *pArray;
	int i;

	anArray.RemoveAll();

	pos = mapSymbolArrays.GetStartPosition();
	while (pos) {
		mapSymbolArrays.GetNextAssoc(pos, strTemp, pArray);
		for (i=0; i<anArray.GetSize(); i++) {
			if (strTemp < anArray.GetAt(i)) break;
		}
		anArray.InsertAt(i, strTemp);
	}
}

DWORD CSymbolMap::GetLeftSideCodeHitList(LPCTSTR aSymbol, CStringArray &aSymbolArray, CDWordArray &aHitCountArray)
{
	return GetHitList(m_LeftSideCodeSymbols, aSymbol, aSymbolArray, aHitCountArray);
}

DWORD CSymbolMap::GetRightSideCodeHitList(LPCTSTR aSymbol, CStringArray &aSymbolArray, CDWordArray &aHitCountArray)
{
	return GetHitList(m_RightSideCodeSymbols, aSymbol, aSymbolArray, aHitCountArray);
}

DWORD CSymbolMap::GetLeftSideDataHitList(LPCTSTR aSymbol, CStringArray &aSymbolArray, CDWordArray &aHitCountArray)
{
	return GetHitList(m_LeftSideDataSymbols, aSymbol, aSymbolArray, aHitCountArray);
}

DWORD CSymbolMap::GetRightSideDataHitList(LPCTSTR aSymbol, CStringArray &aSymbolArray, CDWordArray &aHitCountArray)
{
	return GetHitList(m_RightSideDataSymbols, aSymbol, aSymbolArray, aHitCountArray);
}

DWORD CSymbolMap::GetHitList(CMap<CString, LPCTSTR, CStringArray*, CStringArray*> &mapSymbolArrays,
								LPCTSTR aSymbol, CStringArray &aSymbolArray, CDWordArray &aHitCountArray)
{
	CStringArray *pArray;
	CMap<CString, LPCTSTR, DWORD, DWORD> mapSymbols;
	int i;
	CString strTemp;
	POSITION pos;
	DWORD nTemp;
	DWORD nTotal = 0;

	aSymbolArray.RemoveAll();
	aHitCountArray.RemoveAll();

	if (mapSymbolArrays.Lookup(aSymbol, pArray)) {
		if (pArray) {
			for (i=0; i<pArray->GetSize(); i++) {
				strTemp = pArray->GetAt(i);
				if (mapSymbols.Lookup(strTemp, nTemp)) {
					mapSymbols.SetAt(strTemp, nTemp+1);
				} else {
					mapSymbols.SetAt(strTemp, 1);
				}
			}

			pos = mapSymbols.GetStartPosition();
			while (pos) {
				mapSymbols.GetNextAssoc(pos, strTemp, nTemp);

				for (i=0; i<aHitCountArray.GetSize(); i++) {
					if (nTemp > aHitCountArray.GetAt(i)) break;
					if (nTemp == aHitCountArray.GetAt(i)) {
						if (aSymbolArray.GetAt(i).IsEmpty()) break;
						if ((strTemp < aSymbolArray.GetAt(i)) &&
							(!strTemp.IsEmpty())) break;
					}
				}
				aSymbolArray.InsertAt(i, strTemp);
				aHitCountArray.InsertAt(i, nTemp);
			}

			nTotal = pArray->GetSize();
		}
	}

	return nTotal;
}

/*
void CSymbolMap::SortStringArray(CStringArray &aStringArray)
{
	// Sort an array of strings.  Here, we'll just use bubble sorting
	//	as the number of string entries should be relatively small:
	CString strTemp;
	int i;
	int j;

	for (i=0; i<aStringArray.GetSize()-1; i++) {
		for (j=i+1; j<aStringArray.GetSize(); j++) {
			if (aStringArray.GetAt(i).Compare(aStringArray.GetAt(j)) > 0) {
				strTemp = aStringArray.GetAt(j);
				aStringArray.SetAt(j, aStringArray.GetAt(i));
				aStringArray.SetAt(i, strTemp);
			}
		}
	}
}
*/

/*
void CSymbolMap::SortStringDWordArray(CStringArray &aStringArray, CDWordArray &aDWordArray)
{
	// Sort an array of strings along with an array of numbers.  Since the numbers
	//	are "hit counts", we want them to have precedence in the sort, so we will
	//	sort but numbers and then by string.  We'll just use bubble sorting
	//	as the number of entries should be relatively small:
	CString strTemp;
	DWORD nTemp;
	int i;
	int j;

	ASSERT(aStringArray.GetSize() == aDWordArray.GetSize());	// The two arrays must have same number of elements!

	for (i=0; i<aStringArray.GetSize()-1; i++) {
		for (j=i+1; j<aStringArray.GetSize(); j++) {
			if ((aDWordArray.GetAt(i) < aDWordArray.GetAt(j)) ||
				((aDWordArray.GetAt(i) == aDWordArray.GetAt(j)) && 
				 ((aStringArray.GetAt(i).Compare(aStringArray.GetAt(j)) > 0) || (aStringArray.GetAt(i).IsEmpty())))) {
				strTemp = aStringArray.GetAt(j);
				aStringArray.SetAt(j, aStringArray.GetAt(i));
				aStringArray.SetAt(i, strTemp);
				nTemp = aDWordArray.GetAt(j);
				aDWordArray.SetAt(j, aDWordArray.GetAt(i));
				aDWordArray.SetAt(i, nTemp);
			}
		}
	}
}
*/

void CSymbolMap::AddLeftSideCodeSymbol(LPCTSTR aLeftSymbol, LPCTSTR aRightSymbol)
{
	CStringArray *pArray;

	if (m_LeftSideCodeSymbols.Lookup(aLeftSymbol, pArray)) {
		if (pArray == NULL) {
			ASSERT(FALSE);			// We should never encounter a null entry -- check adding!
			return;
		}
	} else {
		pArray = new CStringArray;
		if (pArray == NULL) AfxThrowMemoryException();
		m_LeftSideCodeSymbols.SetAt(aLeftSymbol, pArray);
	}
	ASSERT(pArray != NULL);
	pArray->Add(aRightSymbol);
}

void CSymbolMap::AddRightSideCodeSymbol(LPCTSTR aRightSymbol, LPCTSTR aLeftSymbol)
{
	CStringArray *pArray;

	if (m_RightSideCodeSymbols.Lookup(aRightSymbol, pArray)) {
		if (pArray == NULL) {
			ASSERT(FALSE);			// We should never encounter a null entry -- check adding!
			return;
		}
	} else {
		pArray = new CStringArray;
		if (pArray == NULL) AfxThrowMemoryException();
		m_RightSideCodeSymbols.SetAt(aRightSymbol, pArray);
	}
	ASSERT(pArray != NULL);
	pArray->Add(aLeftSymbol);
}

void CSymbolMap::AddLeftSideDataSymbol(LPCTSTR aLeftSymbol, LPCTSTR aRightSymbol)
{
	CStringArray *pArray;

	if (m_LeftSideDataSymbols.Lookup(aLeftSymbol, pArray)) {
		if (pArray == NULL) {
			ASSERT(FALSE);			// We should never encounter a null entry -- check adding!
			return;
		}
	} else {
		pArray = new CStringArray;
		if (pArray == NULL) AfxThrowMemoryException();
		m_LeftSideDataSymbols.SetAt(aLeftSymbol, pArray);
	}
	ASSERT(pArray != NULL);
	pArray->Add(aRightSymbol);
}

void CSymbolMap::AddRightSideDataSymbol(LPCTSTR aRightSymbol, LPCTSTR aLeftSymbol)
{
	CStringArray *pArray;

	if (m_RightSideDataSymbols.Lookup(aRightSymbol, pArray)) {
		if (pArray == NULL) {
			ASSERT(FALSE);			// We should never encounter a null entry -- check adding!
			return;
		}
	} else {
		pArray = new CStringArray;
		if (pArray == NULL) AfxThrowMemoryException();
		m_RightSideDataSymbols.SetAt(aRightSymbol, pArray);
	}
	ASSERT(pArray != NULL);
	pArray->Add(aLeftSymbol);
}

