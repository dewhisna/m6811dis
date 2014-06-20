//
//	Motorola 6811 Disassembler
//	Copyright(c)1996 - 2014 by Donna Whisnant
//

//
//	M6811GDC -- This is the class definition for the M6811 Disassembler GDC module
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

	virtual unsigned int GetVersionNumber(void);
	virtual std::string GetGDCLongName(void);
	virtual std::string GetGDCShortName(void);

	virtual bool ResolveIndirect(unsigned int nAddress, unsigned int& nResAddress, int nType);
	virtual std::string GetExcludedPrintChars(void);
	virtual std::string GetHexDelim(void);
	virtual std::string GetCommentStartDelim(void);
	virtual std::string	GetCommentEndDelim(void);
	virtual bool CompleteObjRead(bool bAddLabels = true, std::ostream *msgFile = NULL, std::ostream *errFile = NULL);
	virtual bool RetrieveIndirect(std::ostream *msgFile = NULL, std::ostream *errFile = NULL);

	virtual std::string FormatLabel(LABEL_CODE nLC, const std::string & szLabel, unsigned int nAddress);
	virtual std::string FormatMnemonic(MNEMONIC_CODE nMCCode);
	virtual std::string FormatOperands(MNEMONIC_CODE nMCCode);
	virtual std::string FormatComments(MNEMONIC_CODE nMCCode);

	virtual bool WritePreSection(std::ostream& outFile, std::ostream *msgFile = NULL, std::ostream *errFile = NULL);

private:
	bool MoveOpcodeArgs(unsigned int nGroup);
	bool DecodeOpcode(unsigned int nGroup, unsigned int nControl, bool bAddLabels, std::ostream *msgFile, std::ostream *errFile);
	void CreateOperand(unsigned int nGroup, std::string& sOpStr);
	bool CheckBranchOutside(unsigned int nGroup);
	std::string LabelDeref2(unsigned int nAddress);
	std::string LabelDeref4(unsigned int nAddress);

	unsigned int OpPointer;
	unsigned int StartPC;

	int SectionCount;

	bool CBDef;
};


#endif	// _M6811GDC_H
