//
//	M6811GDC -- This is the class definition for the M6811 Disassembler GDC module
//

//
// $Log: m6811gdc.h,v $
// Revision 1.1  2002/05/24 02:38:59  dewhisna
// Initial Revision Version 1.2 (Beta 1)
//
//

#ifndef _M6811GDC_H
#define _M6811GDC_H

#include "gdc.h"

#define OCTL_MASK 0xFF			// Mask for our control byte
#define MAKEOCTL(d, s) (((d & 0x0F) << 4) | (s & 0x0F))
#define OCTL_SRC (m_CurrentOpcode.m_Control & 0x0F)
#define OCTL_DST ((m_CurrentOpcode.m_Control >> 4) & 0x0F)

#define OGRP_MASK 0xFF			// Mask for our group byte
#define MAKEOGRP(d, s) (((d & 0x0F) << 4) | (s & 0x0F))
#define OGRP_SRC (m_CurrentOpcode.m_Group & 0x0F)
#define OGRP_DST ((m_CurrentOpcode.m_Group >> 4) & 0x0F)

class CM6811Disassembler : public CDisassembler
{
public:
	CM6811Disassembler();

	virtual DWORD GetVersionNumber(void);
	virtual CString GetGDCLongName(void);
	virtual CString GetGDCShortName(void);

	virtual DWORD ResolveIndirect(DWORD nAddress, DWORD& nResAddress, int nType);
	virtual CString GetExcludedPrintChars(void);
	virtual CString GetHexDelim(void);
	virtual CString GetCommentStartDelim(void);
	virtual CString	GetCommentEndDelim(void);
	virtual BOOL CompleteObjRead(BOOL bAddLabels = TRUE, CStdioFile *msgFile = NULL, CStdioFile *errFile = NULL);
	virtual BOOL RetrieveIndirect(CStdioFile *msgFile = NULL, CStdioFile *errFile = NULL);

	virtual CString FormatLabel(LABEL_CODE nLC, LPCTSTR szLabel, DWORD nAddress);
	virtual CString FormatMnemonic(MNEMONIC_CODE nMCCode);
	virtual CString FormatOperands(MNEMONIC_CODE nMCCode);
	virtual CString FormatComments(MNEMONIC_CODE nMCCode);

	virtual BOOL WritePreSection(CStdioFile& outFile, CStdioFile *msgFile = NULL, CStdioFile *errFile = NULL);

private:
	BOOL MoveOpcodeArgs(DWORD nGroup);
	BOOL DecodeOpcode(DWORD nGroup, DWORD nControl, BOOL bAddLabels, CStdioFile *msgFile, CStdioFile *errFile);
	void CreateOperand(DWORD nGroup, CString& sOpStr);
	BOOL CheckBranchOutside(DWORD nGroup);
	CString LabelDeref2(DWORD nAddress);
	CString LabelDeref4(DWORD nAddress);

	int OpPointer;
	DWORD StartPC;

	int SectionCount;

	BOOL CBDef;
};


#endif	// _M6811GDC_H
