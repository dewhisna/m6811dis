//
//	M6811GDC -- This is the implementation for the M6811 Disassembler GDC module
//
//
//	Groups: (Grp) : xy
//	  Where x (msnb = destination = FIRST operand)
//			y (lsnb = source = LAST operand)
//
//			0 = opcode only, nothing follows
//			1 = 8-bit absolute address follows only
//			2 = 16-bit absolute address follows only
//			3 = 8-bit relative address follows only
//			4 = 16-bit relative address follows only		(Not used on HC11)
//			5 = 8-bit data follows only
//			6 = 16-bit data follows only
//			7 = 8-bit absolute address followed by 8-bit mask
//			8 = 8-bit X offset address followed by 8-bit mask
//			9 = 8-bit Y offset address followed by 8-bit mask
//			A = 8-bit X offset address
//			B = 8-bit Y offset address
//
//	Control: (Algorithm control) : xy
//	  Where x (msnb = destination = FIRST operand)
//			y (lsnb = source = LAST operand)
//
//			0 = Disassemble as code
//
//			1 = Disassemble as code, with data addr label
//					(on opcodes with post-byte, label generation is dependent upon post-byte value.)
//
//			2 = Disassemble as undeterminable branch address (Comment code)
//
//			3 = Disassemble as determinable branch, add branch addr and label
//
//			4 = Disassemble as conditionally undeterminable branch address (Comment code),
//					with data addr label, (on opcodes with post-byte, label generation is dependent
//					upon post-byte value. conditional upon post-byte)
//
//	This version setup for compatibility with the AS6811 assembler
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

//
// $Log: m6811gdc.cpp,v $
// Revision 1.2  2003/01/26 06:24:27  dewhisna
// Added function identification and exportation capabilities
//
// Revision 1.1  2002/05/24 02:38:58  dewhisna
// Initial Revision Version 1.2 (Beta 1)
//

#include "stdafx.h"

#include "m6811gdc.h"
#include "gdc.h"

#define DataDelim	"'"			// Specify ' as delimiter for data literals
#define LblcDelim	":"			// Delimiter between labels and code
#define LbleDelim	""			// Delimiter between labels and equates
#define	ComDelimS	";"			// Comment Start delimiter
#define ComDelimE	""			// Comment End delimiter
#define HexDelim	"0x"		// Hex. delimiter

#define VERSION 0x114				// M6811 Disassembler Version number 1.20

// ----------------------------------------------------------------------------
//	CM6811Disassembler
// ----------------------------------------------------------------------------
CM6811Disassembler::CM6811Disassembler()
	: CDisassembler()
{
//                = ((NBytes: 0; OpCode: (0, 0); Grp: 0; Control: 1; Mnemonic: '???'),
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x00", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "test"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x01", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "nop"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x02", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "idiv"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x03", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "fdiv"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x04", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "lsrd"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x05", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "lsld"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x06", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "tap"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x07", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "tpa"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x08", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "inx"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x09", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "dex"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x0A", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "clv"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x0B", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "sev"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x0C", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "clc"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x0D", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "sec"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x0E", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "cli"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x0F", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "sei"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x10", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "sba"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x11", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "cba"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x12", MAKEOGRP(0x7, 0x3), MAKEOCTL(0x1, 0x3), "brset"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x13", MAKEOGRP(0x7, 0x3), MAKEOCTL(0x1, 0x3), "brclr"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x14", MAKEOGRP(0x0, 0x7), MAKEOCTL(0x0, 0x1), "bset"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x15", MAKEOGRP(0x0, 0x7), MAKEOCTL(0x0, 0x1), "bclr"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x16", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "tab"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x17", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "tba"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\x08", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "iny"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\x09", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "dey"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\x1C", MAKEOGRP(0x0, 0x9), MAKEOCTL(0x0, 0x0), "bset"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\x1D", MAKEOGRP(0x0, 0x9), MAKEOCTL(0x0, 0x0), "bclr"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\x1E", MAKEOGRP(0x9, 0x3), MAKEOCTL(0x0, 0x3), "brset"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\x1F", MAKEOGRP(0x9, 0x3), MAKEOCTL(0x0, 0x3), "brclr"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\x30", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "tsy"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\x35", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "tys"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\x38", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "puly"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\x3A", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "aby"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\x3C", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "pshy"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\x60", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "neg"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\x63", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "com"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\x64", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "lsr"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\x66", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "ror"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\x67", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "asr"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\x68", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "lsl"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\x69", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "rol"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\x6A", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "dec"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\x6C", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "inc"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\x6D", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "tst"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\x6E", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x2) | OCTL_STOP, "jmp"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\x6F", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "clr"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\x8C", MAKEOGRP(0x0, 0x6), MAKEOCTL(0x0, 0x0), "cpy"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\x8F", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "xgdy"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\x9C", MAKEOGRP(0x0, 0x1), MAKEOCTL(0x0, 0x1), "cpy"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xA0", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "suba"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xA1", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "cmpa"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xA2", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "sbca"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xA3", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "subd"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xA4", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "anda"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xA5", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "bita"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xA6", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "ldaa"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xA7", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "staa"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xA8", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "eora"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xA9", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "adca"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xAA", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "oraa"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xAB", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "adda"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xAC", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "cpy"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xAD", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x2), "jsr"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xAE", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "lds"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xAF", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "sts"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xBC", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "cpy"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xCE", MAKEOGRP(0x0, 0x6), MAKEOCTL(0x0, 0x0), "ldy"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xDE", MAKEOGRP(0x0, 0x1), MAKEOCTL(0x0, 0x1), "ldy"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xDF", MAKEOGRP(0x0, 0x1), MAKEOCTL(0x0, 0x1), "sty"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xE0", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "subb"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xE1", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "cmpb"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xE2", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "sbcb"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xE3", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "addd"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xE4", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "andb"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xE5", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "bitb"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xE6", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "ldab"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xE7", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "stab"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xE8", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "eorb"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xE9", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "adcb"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xEA", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "orab"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xEB", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "addb"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xEC", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "ldd"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xED", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "std"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xEE", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "ldy"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xEF", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "sty"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xFE", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "ldy"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x18\xFF", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "sty"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x19", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "daa"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x1A\x83", MAKEOGRP(0x0, 0x6), MAKEOCTL(0x0, 0x0), "cpd"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x1A\x93", MAKEOGRP(0x0, 0x1), MAKEOCTL(0x0, 0x1), "cpd"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x1A\xA3", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "cpd"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x1A\xAC", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "cpy"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x1A\xB3", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "cpd"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x1A\xEE", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "ldy"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\x1A\xEF", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "sty"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x1B", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "aba"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x1C", MAKEOGRP(0x0, 0x8), MAKEOCTL(0x0, 0x0), "bset"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x1D", MAKEOGRP(0x0, 0x8), MAKEOCTL(0x0, 0x0), "bclr"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x1E", MAKEOGRP(0x8, 0x3), MAKEOCTL(0x0, 0x3), "brset"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x1F", MAKEOGRP(0x8, 0x3), MAKEOCTL(0x0, 0x3), "brclr"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x20", MAKEOGRP(0x0, 0x3), MAKEOCTL(0x0, 0x3) | OCTL_STOP, "bra"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x21", MAKEOGRP(0x0, 0x3), MAKEOCTL(0x0, 0x3), "brn"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x22", MAKEOGRP(0x0, 0x3), MAKEOCTL(0x0, 0x3), "bhi"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x23", MAKEOGRP(0x0, 0x3), MAKEOCTL(0x0, 0x3), "bls"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x24", MAKEOGRP(0x0, 0x3), MAKEOCTL(0x0, 0x3), "bcc"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x25", MAKEOGRP(0x0, 0x3), MAKEOCTL(0x0, 0x3), "bcs"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x26", MAKEOGRP(0x0, 0x3), MAKEOCTL(0x0, 0x3), "bne"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x27", MAKEOGRP(0x0, 0x3), MAKEOCTL(0x0, 0x3), "beq"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x28", MAKEOGRP(0x0, 0x3), MAKEOCTL(0x0, 0x3), "bvc"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x29", MAKEOGRP(0x0, 0x3), MAKEOCTL(0x0, 0x3), "bvs"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x2A", MAKEOGRP(0x0, 0x3), MAKEOCTL(0x0, 0x3), "bpl"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x2B", MAKEOGRP(0x0, 0x3), MAKEOCTL(0x0, 0x3), "bmi"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x2C", MAKEOGRP(0x0, 0x3), MAKEOCTL(0x0, 0x3), "bge"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x2D", MAKEOGRP(0x0, 0x3), MAKEOCTL(0x0, 0x3), "blt"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x2E", MAKEOGRP(0x0, 0x3), MAKEOCTL(0x0, 0x3), "bgt"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x2F", MAKEOGRP(0x0, 0x3), MAKEOCTL(0x0, 0x3), "ble"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x30", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "tsx"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x31", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "ins"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x32", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "pula"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x33", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "pulb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x34", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "des"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x35", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "txs"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x36", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "psha"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x37", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "pshb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x38", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "pulx"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x39", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0) | OCTL_STOP, "rts"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x3A", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "abx"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x3B", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0) | OCTL_STOP, "rti"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x3C", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "pshx"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x3D", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "mul"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x3E", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "wai"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x3F", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "swi"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x40", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "nega"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x43", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "coma"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x44", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "lsra"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x46", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "rora"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x47", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "asra"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x48", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "lsla"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x49", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "rola"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x4A", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "deca"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x4C", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "inca"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x4D", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "tsta"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x4F", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "clra"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x50", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "negb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x53", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "comb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x54", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "lsrb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x56", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "rorb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x57", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "asrb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x58", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "lslb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x59", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "rolb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x5A", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "decb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x5C", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "incb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x5D", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "tstb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x5F", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "clrb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x60", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "neg"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x63", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "com"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x64", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "lsr"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x66", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "ror"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x67", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "asr"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x68", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "lsl"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x69", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "rol"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x6A", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "dec"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x6C", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "inc"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x6D", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "tst"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x6E", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x2) | OCTL_STOP, "jmp"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x6F", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "clr"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x70", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "neg"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x73", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "com"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x74", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "lsr"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x76", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "ror"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x77", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "asr"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x78", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "lsl"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x79", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "rol"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x7A", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "dec"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x7C", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "inc"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x7D", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "tst"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x7E", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x3) | OCTL_STOP, "jmp"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x7F", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "clr"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x80", MAKEOGRP(0x0, 0x5), MAKEOCTL(0x0, 0x0), "suba"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x81", MAKEOGRP(0x0, 0x5), MAKEOCTL(0x0, 0x0), "cmpa"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x82", MAKEOGRP(0x0, 0x5), MAKEOCTL(0x0, 0x0), "sbca"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x83", MAKEOGRP(0x0, 0x6), MAKEOCTL(0x0, 0x0), "subd"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x84", MAKEOGRP(0x0, 0x5), MAKEOCTL(0x0, 0x0), "anda"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x85", MAKEOGRP(0x0, 0x5), MAKEOCTL(0x0, 0x0), "bita"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x86", MAKEOGRP(0x0, 0x5), MAKEOCTL(0x0, 0x0), "ldaa"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x88", MAKEOGRP(0x0, 0x5), MAKEOCTL(0x0, 0x0), "eora"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x89", MAKEOGRP(0x0, 0x5), MAKEOCTL(0x0, 0x0), "adca"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x8A", MAKEOGRP(0x0, 0x5), MAKEOCTL(0x0, 0x0), "oraa"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x8B", MAKEOGRP(0x0, 0x5), MAKEOCTL(0x0, 0x0), "adda"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x8C", MAKEOGRP(0x0, 0x6), MAKEOCTL(0x0, 0x0), "cpx"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x8D", MAKEOGRP(0x0, 0x3), MAKEOCTL(0x0, 0x3), "bsr"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x8E", MAKEOGRP(0x0, 0x6), MAKEOCTL(0x0, 0x0), "lds"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x8F", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "xgdx"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x90", MAKEOGRP(0x0, 0x1), MAKEOCTL(0x0, 0x1), "suba"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x91", MAKEOGRP(0x0, 0x1), MAKEOCTL(0x0, 0x1), "cmpa"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x92", MAKEOGRP(0x0, 0x1), MAKEOCTL(0x0, 0x1), "sbca"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x93", MAKEOGRP(0x0, 0x1), MAKEOCTL(0x0, 0x1), "subd"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x94", MAKEOGRP(0x0, 0x1), MAKEOCTL(0x0, 0x1), "anda"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x95", MAKEOGRP(0x0, 0x1), MAKEOCTL(0x0, 0x1), "bita"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x96", MAKEOGRP(0x0, 0x1), MAKEOCTL(0x0, 0x1), "ldaa"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x97", MAKEOGRP(0x0, 0x1), MAKEOCTL(0x0, 0x1), "staa"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x98", MAKEOGRP(0x0, 0x1), MAKEOCTL(0x0, 0x1), "eora"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x99", MAKEOGRP(0x0, 0x1), MAKEOCTL(0x0, 0x1), "adca"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x9A", MAKEOGRP(0x0, 0x1), MAKEOCTL(0x0, 0x1), "oraa"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x9B", MAKEOGRP(0x0, 0x1), MAKEOCTL(0x0, 0x1), "adda"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x9C", MAKEOGRP(0x0, 0x1), MAKEOCTL(0x0, 0x1), "cpx"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x9D", MAKEOGRP(0x0, 0x1), MAKEOCTL(0x0, 0x3), "jsr"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x9E", MAKEOGRP(0x0, 0x1), MAKEOCTL(0x0, 0x1), "lds"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\x9F", MAKEOGRP(0x0, 0x1), MAKEOCTL(0x0, 0x1), "sts"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xA0", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "suba"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xA1", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "cmpa"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xA2", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "sbca"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xA3", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "subd"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xA4", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "anda"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xA5", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "bita"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xA6", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "ldaa"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xA7", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "staa"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xA8", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "eora"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xA9", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "adca"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xAA", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "oraa"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xAB", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "adda"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xAC", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "cpx"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xAD", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x2), "jsr"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xAE", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "lds"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xAF", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "sts"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xB0", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "suba"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xB1", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "cmpa"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xB2", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "sbca"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xB3", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "subd"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xB4", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "anda"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xB5", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "bita"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xB6", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "ldaa"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xB7", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "staa"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xB8", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "eora"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xB9", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "adca"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xBA", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "oraa"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xBB", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "adda"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xBC", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "cpx"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xBD", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x3), "jsr"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xBE", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "lds"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xBF", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "sts"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xC0", MAKEOGRP(0x0, 0x5), MAKEOCTL(0x0, 0x0), "subb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xC1", MAKEOGRP(0x0, 0x5), MAKEOCTL(0x0, 0x0), "cmpb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xC2", MAKEOGRP(0x0, 0x5), MAKEOCTL(0x0, 0x0), "sbcb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xC3", MAKEOGRP(0x0, 0x6), MAKEOCTL(0x0, 0x0), "addd"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xC4", MAKEOGRP(0x0, 0x5), MAKEOCTL(0x0, 0x0), "andb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xC5", MAKEOGRP(0x0, 0x5), MAKEOCTL(0x0, 0x0), "bitb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xC6", MAKEOGRP(0x0, 0x5), MAKEOCTL(0x0, 0x0), "ldab"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xC8", MAKEOGRP(0x0, 0x5), MAKEOCTL(0x0, 0x0), "eorb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xC9", MAKEOGRP(0x0, 0x5), MAKEOCTL(0x0, 0x0), "adcb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xCA", MAKEOGRP(0x0, 0x5), MAKEOCTL(0x0, 0x0), "orab"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xCB", MAKEOGRP(0x0, 0x5), MAKEOCTL(0x0, 0x0), "addb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xCC", MAKEOGRP(0x0, 0x6), MAKEOCTL(0x0, 0x0), "ldd"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\xCD\xA3", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "cpd"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\xCD\xAC", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "cpx"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\xCD\xEE", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "ldx"));
	m_Opcodes.AddOpcode(COpcodeEntry(2, (unsigned char *)"\xCD\xEF", MAKEOGRP(0x0, 0xB), MAKEOCTL(0x0, 0x0), "stx"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xCE", MAKEOGRP(0x0, 0x6), MAKEOCTL(0x0, 0x0), "ldx"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xCF", MAKEOGRP(0x0, 0x0), MAKEOCTL(0x0, 0x0), "stop"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xD0", MAKEOGRP(0x0, 0x1), MAKEOCTL(0x0, 0x1), "subb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xD1", MAKEOGRP(0x0, 0x1), MAKEOCTL(0x0, 0x1), "cmpb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xD2", MAKEOGRP(0x0, 0x1), MAKEOCTL(0x0, 0x1), "sbcb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xD3", MAKEOGRP(0x0, 0x1), MAKEOCTL(0x0, 0x1), "addd"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xD4", MAKEOGRP(0x0, 0x1), MAKEOCTL(0x0, 0x1), "andb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xD5", MAKEOGRP(0x0, 0x1), MAKEOCTL(0x0, 0x1), "bitb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xD6", MAKEOGRP(0x0, 0x1), MAKEOCTL(0x0, 0x1), "ldab"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xD7", MAKEOGRP(0x0, 0x1), MAKEOCTL(0x0, 0x1), "stab"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xD8", MAKEOGRP(0x0, 0x1), MAKEOCTL(0x0, 0x1), "eorb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xD9", MAKEOGRP(0x0, 0x1), MAKEOCTL(0x0, 0x1), "adcb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xDA", MAKEOGRP(0x0, 0x1), MAKEOCTL(0x0, 0x1), "orab"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xDB", MAKEOGRP(0x0, 0x1), MAKEOCTL(0x0, 0x1), "addb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xDC", MAKEOGRP(0x0, 0x1), MAKEOCTL(0x0, 0x1), "ldd"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xDD", MAKEOGRP(0x0, 0x1), MAKEOCTL(0x0, 0x1), "std"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xDE", MAKEOGRP(0x0, 0x1), MAKEOCTL(0x0, 0x1), "ldx"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xDF", MAKEOGRP(0x0, 0x1), MAKEOCTL(0x0, 0x1), "stx"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xE0", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "subb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xE1", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "cmpb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xE2", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "sbcb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xE3", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "addd"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xE4", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "andb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xE5", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "bitb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xE6", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "ldab"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xE7", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "stab"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xE8", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "eorb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xE9", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "adcb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xEA", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "orab"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xEB", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "addb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xEC", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "ldd"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xED", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "std"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xEE", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "ldx"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xEF", MAKEOGRP(0x0, 0xA), MAKEOCTL(0x0, 0x0), "stx"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xF0", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "subb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xF1", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "cmpb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xF2", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "sbcb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xF3", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "addd"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xF4", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "andb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xF5", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "bitb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xF6", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "ldab"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xF7", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "stab"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xF8", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "eorb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xF9", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "adcb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xFA", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "orab"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xFB", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "addb"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xFC", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "ldd"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xFD", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "std"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xFE", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "ldx"));
	m_Opcodes.AddOpcode(COpcodeEntry(1, (unsigned char *)"\xFF", MAKEOGRP(0x0, 0x2), MAKEOCTL(0x0, 0x1), "stx"));

	m_Memory = new TMemObject(1, 0x10000ul, 0x00, TRUE);			// 64K of memory available to processor
	if (m_Memory == NULL) AfxThrowMemoryException();
	m_Memory->AddMapping(0ul, 0, 0ul);								// Mapped as one block

	SectionCount = 0;
	CBDef = TRUE;
}

DWORD CM6811Disassembler::GetVersionNumber()
{
	return (CDisassembler::GetVersionNumber() | VERSION);			// Get GDC version and append our version to it
}

CString CM6811Disassembler::GetGDCLongName() {
	return "M6811 Disassembler GDC";
}

CString CM6811Disassembler::GetGDCShortName() {
	return "M6811GDC";
}

DWORD CM6811Disassembler::ResolveIndirect(DWORD nAddress, DWORD& nResAddress, int nType)
{
	DWORD	aVector;

	// In the HC11, we can assume that all indirect addresses are 2-bytes in length
	//	and stored in little endian format.
	aVector = 0;
	if ((IsAddressLoaded(nAddress, 2) == FALSE) ||			// Not only must it be loaded, but we must have never examined it before!
		(m_Memory->GetDescriptor(nAddress) != DMEM_LOADED) ||
		(m_Memory->GetDescriptor(nAddress+1) != DMEM_LOADED)) {
		nResAddress = 0;
		return FALSE;
	}
	aVector = m_Memory->GetByte(nAddress) * 256ul + m_Memory->GetByte(nAddress + 1);
	nResAddress = aVector;
	m_Memory->SetDescriptor(nAddress, DMEM_CODEINDIRECT + nType);		// Flag the addresses as being the proper vector type
	m_Memory->SetDescriptor(nAddress+1, DMEM_CODEINDIRECT + nType);
	return TRUE;
}

CString CM6811Disassembler::GetExcludedPrintChars()
{
	return "\\" DataDelim;
}

CString CM6811Disassembler::GetHexDelim()
{
	return HexDelim;
}

CString CM6811Disassembler::GetCommentStartDelim()
{
	return ComDelimS;
}

CString CM6811Disassembler::GetCommentEndDelim()
{
	return ComDelimE;
}

BOOL CM6811Disassembler::CompleteObjRead(BOOL bAddLabels, CStdioFile *msgFile, CStdioFile *errFile)
{
	// Procedure to finish reading opcode from memory.  This procedure reads operand bytes,
	//	placing them in m_OpMemory.  Plus branch/labels and data/labels are generated (according to function).

	BOOL A,B;
	int i;
	CString strTemp;
	CStringArray *pLabelList;

	CBDef = TRUE;								// On HC11 all conditionally defined are defined, unlike 6809

	OpPointer = m_OpMemory.GetSize();			// Get position of start of operands following opcode
	StartPC = m_PC - m_OpMemory.GetSize();		// Get PC of first byte of this instruction for branch references

	// Note: We'll start with the absolute address.  Since we don't know what the relative address is
	//		to the start of the function, then the function outputting function (which should know it),
	//		will have to add it in:
	m_sFunctionalOpcode.Format("%04lX|", StartPC);

	// Move Bytes to m_OpMemory:
	A = MoveOpcodeArgs(OGRP_DST);
	B = MoveOpcodeArgs(OGRP_SRC);
	if ((A == FALSE) || (B == FALSE)) return FALSE;

	if (m_LabelTable.Lookup(StartPC, pLabelList)) {
		ASSERT(pLabelList != NULL);			// Shouldn't have null list!
		if (pLabelList == NULL) return FALSE;
		for (i=0; i<pLabelList->GetSize(); i++) {
			if (i != 0) m_sFunctionalOpcode += ",";
			m_sFunctionalOpcode += FormatLabel(LC_REF, pLabelList->GetAt(i), StartPC);
		}
	}

	m_sFunctionalOpcode += "|";

	for (i=0; i<m_OpMemory.GetSize(); i++) {
		strTemp.Format("%02X", m_OpMemory.GetAt(i));
		m_sFunctionalOpcode += strTemp;
	}

	m_sFunctionalOpcode += "|";

	for (i=0; i<OpPointer; i++) {
		strTemp.Format("%02X", m_OpMemory.GetAt(i));
		m_sFunctionalOpcode += strTemp;
	}

	m_sFunctionalOpcode += "|";

	for (i=OpPointer; i<m_OpMemory.GetSize(); i++) {
		strTemp.Format("%02X", m_OpMemory.GetAt(i));
		m_sFunctionalOpcode += strTemp;
	}

	m_sFunctionalOpcode += "|";

	// Decode Operands:
	A = DecodeOpcode(OGRP_DST, OCTL_DST, bAddLabels, msgFile, errFile);

	m_sFunctionalOpcode += "|";

	B = DecodeOpcode(OGRP_SRC, OCTL_SRC, bAddLabels, msgFile, errFile);
	if ((A == FALSE) || (B == FALSE)) return FALSE;

	m_sFunctionalOpcode += "|";
	m_sFunctionalOpcode += FormatMnemonic(MC_OPCODE);
	m_sFunctionalOpcode += "|";
	m_sFunctionalOpcode += FormatOperands(MC_OPCODE);

	// See if this is the end of a function.  Note: These preempt all previously
	//		decoded function tags -- such as call, etc:
	if ((m_CurrentOpcode.m_Mnemonic.CompareNoCase("rts") == 0) ||
		(m_CurrentOpcode.m_Mnemonic.CompareNoCase("rti") == 0)) {
		m_FunctionsTable.SetAt(StartPC, FUNCF_HARDSTOP);
	}

	return TRUE;
}

BOOL CM6811Disassembler::RetrieveIndirect(CStdioFile *msgFile, CStdioFile *errFile)
{
	MEM_DESC b1d, b2d;
	CString Temp;

	m_OpMemory.RemoveAll();
	m_OpMemory.Add(m_Memory->GetByte(m_PC));
	m_OpMemory.Add(m_Memory->GetByte(m_PC+1));
	// All HC11 indirects are Motorola format and are 2-bytes in length, regardless:
	b1d = (MEM_DESC)m_Memory->GetDescriptor(m_PC);
	b2d = (MEM_DESC)m_Memory->GetDescriptor(m_PC+1);
	if (((b1d != DMEM_CODEINDIRECT) && (b1d != DMEM_DATAINDIRECT)) ||
		((b2d != DMEM_CODEINDIRECT) && (b2d != DMEM_DATAINDIRECT))) {
		m_PC += 2;
		return FALSE;			// Memory should be tagged as indirect!
	}
	if (b1d != b2d) {
		m_PC += 2;
		return FALSE;			// Memory should be tagged as a correct pair!
	}

	if (((b1d == DMEM_CODEINDIRECT) && (m_CodeIndirectTable.Lookup(m_PC, Temp) == FALSE)) ||
		((b1d == DMEM_DATAINDIRECT) && (m_DataIndirectTable.Lookup(m_PC, Temp) == FALSE))) {
		m_PC += 2;
		return FALSE;			// Location must exist in one of the two tables!
	}
	m_PC += 2;
	return TRUE;
}


BOOL CM6811Disassembler::MoveOpcodeArgs(DWORD nGroup)
{
	switch (nGroup) {
		case 0x2:
		case 0x4:
		case 0x6:
		case 0x7:
		case 0x8:
		case 0x9:
			m_OpMemory.Add(m_Memory->GetByte(m_PC++));
			// Fall through and do it again:
		case 0x1:
		case 0x3:
		case 0x5:
		case 0xA:
		case 0xB:
			m_OpMemory.Add(m_Memory->GetByte(m_PC++));
			break;
		case 0x0:
			break;
		default:
			return FALSE;
	}
	return TRUE;
}

BOOL CM6811Disassembler::DecodeOpcode(DWORD nGroup, DWORD nControl, BOOL bAddLabels, CStdioFile *msgFile, CStdioFile *errFile)
{
	BOOL RetVal = TRUE;

	CString strTemp;
	CString strTemp2;
	DWORD nTargetAddr;
	DWORD nDummy;

	strTemp = "";
	nTargetAddr = 0xFFFFFFFF;

	switch (nGroup) {
		case 0x0:								// No operand
			break;
		case 0x1:								// absolute 8-bit, assume msb=$00
			switch (nControl) {
				case 1:
					if (bAddLabels) GenDataLabel(m_OpMemory[OpPointer], StartPC, NULL, msgFile, errFile);
					strTemp.Format("D@%02lX", (unsigned long)m_OpMemory[OpPointer]);
					break;
				case 3:
					if (bAddLabels) GenAddrLabel(m_OpMemory[OpPointer], StartPC, NULL, msgFile, errFile);
					strTemp.Format("C@%02lX", (unsigned long)m_OpMemory[OpPointer]);
					nTargetAddr = (unsigned long)m_OpMemory[OpPointer];
					break;
				default:
					RetVal = FALSE;
			}
			OpPointer++;
			break;
		case 0x2:								// 16-bit Absolute
			switch (nControl) {
				case 1:
					if (bAddLabels) GenDataLabel(m_OpMemory[OpPointer]*256+m_OpMemory[OpPointer+1], StartPC, NULL, msgFile, errFile);
					strTemp.Format("D@%04lX", (unsigned long)(m_OpMemory[OpPointer]*256+m_OpMemory[OpPointer+1]));
					break;
				case 3:
					if (bAddLabels) GenAddrLabel(m_OpMemory[OpPointer]*256+m_OpMemory[OpPointer+1], StartPC, NULL, msgFile, errFile);
					strTemp.Format("C@%04lX", (unsigned long)(m_OpMemory[OpPointer]*256+m_OpMemory[OpPointer+1]));
					nTargetAddr = (unsigned long)(m_OpMemory[OpPointer]*256+m_OpMemory[OpPointer+1]);
					break;
				default:
					RetVal = FALSE;
			}
			OpPointer += 2;
			break;
		case 0x3:								// 8-bit Relative
			switch (nControl) {
				case 1:
					if (bAddLabels) GenDataLabel(m_PC+(signed long)(signed char)(m_OpMemory[OpPointer]), StartPC, NULL, msgFile, errFile);
					strTemp.Format("D^%c%02lX(%04lX)",
											((((signed long)(signed char)(m_OpMemory[OpPointer])) !=
											labs((signed long)(signed char)(m_OpMemory[OpPointer]))) ? '-' : '+'),
											labs((signed long)(signed char)(m_OpMemory[OpPointer])),
											m_PC+(signed long)(signed char)(m_OpMemory[OpPointer]));
					break;
				case 3:
					if (bAddLabels) GenAddrLabel(m_PC+(signed long)(signed char)(m_OpMemory[OpPointer]), StartPC, NULL, msgFile, errFile);
					strTemp.Format("C^%c%02lX(%04lX)",
											((((signed long)(signed char)(m_OpMemory[OpPointer])) !=
											labs((signed long)(signed char)(m_OpMemory[OpPointer]))) ? '-' : '+'),
											labs((signed long)(signed char)(m_OpMemory[OpPointer])),
											m_PC+(signed long)(signed char)(m_OpMemory[OpPointer]));
					nTargetAddr = m_PC+(signed long)(signed char)(m_OpMemory[OpPointer]);
					break;
				default:
					RetVal = FALSE;
			}
			OpPointer++;
			break;
		case 0x4:								// 16-bit Relative
			switch (nControl) {
				case 1:
					if (bAddLabels) GenDataLabel(m_PC+(signed long)(signed short)(m_OpMemory[OpPointer]*256+m_OpMemory[OpPointer+1]), StartPC, NULL, msgFile, errFile);
					strTemp.Format("D^%c%04lX(%04lX)",
											((((signed long)(signed short)(m_OpMemory[OpPointer]*256+m_OpMemory[OpPointer+1])) !=
											labs((signed long)(signed short)(m_OpMemory[OpPointer]*256+m_OpMemory[OpPointer+1]))) ? '-' : '+'),
											labs((signed long)(signed short)(m_OpMemory[OpPointer]*256+m_OpMemory[OpPointer+1])),
											m_PC+(signed long)(signed short)(m_OpMemory[OpPointer]*256+m_OpMemory[OpPointer+1]));
					break;
				case 3:
					if (bAddLabels) GenAddrLabel(m_PC+(signed long)(signed short)(m_OpMemory[OpPointer]*256+m_OpMemory[OpPointer+1]), StartPC, NULL, msgFile, errFile);
					strTemp.Format("C^%c%04lX(%04lX)",
											((((signed long)(signed short)(m_OpMemory[OpPointer]*256+m_OpMemory[OpPointer+1])) !=
											labs((signed long)(signed short)(m_OpMemory[OpPointer]*256+m_OpMemory[OpPointer+1]))) ? '-' : '+'),
											labs((signed long)(signed short)(m_OpMemory[OpPointer]*256+m_OpMemory[OpPointer+1])),
											m_PC+(signed long)(signed short)(m_OpMemory[OpPointer]*256+m_OpMemory[OpPointer+1]));
					nTargetAddr = m_PC+(signed long)(signed short)(m_OpMemory[OpPointer]*256+m_OpMemory[OpPointer+1]);
					break;
				default:
					RetVal = FALSE;
			}
			OpPointer += 2;
			break;
		case 0x5:								// Immediate 8-bit data -- no label
			strTemp.Format("#%02lX", (unsigned long)m_OpMemory[OpPointer]);
			OpPointer++;
			break;
		case 0x6:								// Immediate 16-bit data -- no label
			strTemp.Format("#%04lX", (unsigned long)(m_OpMemory[OpPointer]*256+m_OpMemory[OpPointer+1]));
			OpPointer += 2;
			break;
		case 0x7:								// 8-bit Absolute address, and mask
			switch (nControl) {
				case 1:
					if (bAddLabels) GenDataLabel(m_OpMemory[OpPointer], StartPC, NULL, msgFile, errFile);
					strTemp.Format("D@%02lX", (unsigned long)m_OpMemory[OpPointer]);
					break;
				case 3:
					if (bAddLabels) GenAddrLabel(m_OpMemory[OpPointer], StartPC, NULL, msgFile, errFile);
					strTemp.Format("C@%02lX", (unsigned long)m_OpMemory[OpPointer]);
					nTargetAddr = (unsigned long)m_OpMemory[OpPointer];
					break;
				default:
					RetVal = FALSE;
			}
			strTemp2.Format(",M%02lX", (unsigned long)m_OpMemory[OpPointer+1]);
			strTemp += strTemp2;
			OpPointer += 2;
			break;
		case 0x8:								// 8-bit offset and 8-bit mask (x)
			switch (nControl) {
				case 0:
				case 1:
					strTemp.Format("D&%02lX(x)", (unsigned long)m_OpMemory[OpPointer]);
					break;

				case 2:
				case 3:
				case 4:
					strTemp.Format("C&%02lX(x)", (unsigned long)m_OpMemory[OpPointer]);
					break;
			}
			strTemp2.Format(",M%02lX", (unsigned long)m_OpMemory[OpPointer+1]);
			strTemp += strTemp2;
			OpPointer += 2;
			break;
		case 0x9:								// 8-bit offset and 8-bit mask (y)
			switch (nControl) {
				case 0:
				case 1:
					strTemp.Format("D&%02lX(y)", (unsigned long)m_OpMemory[OpPointer]);
					break;

				case 2:
				case 3:
				case 4:
					strTemp.Format("C&%02lX(y)", (unsigned long)m_OpMemory[OpPointer]);
					break;
			}
			strTemp2.Format(",M%02lX", (unsigned long)m_OpMemory[OpPointer+1]);
			strTemp += strTemp2;
			OpPointer += 2;
			break;
		case 0xA:								// 8-bit offset (x)
			switch (nControl) {
				case 0:
				case 1:
					strTemp.Format("D&%02lX(x)", (unsigned long)m_OpMemory[OpPointer]);
					break;

				case 2:
				case 3:
				case 4:
					strTemp.Format("C&%02lX(x)", (unsigned long)m_OpMemory[OpPointer]);
					break;
			}
			OpPointer++;
			break;
		case 0xB:								// 8-bit offset (y)
			switch (nControl) {
				case 0:
				case 1:
					strTemp.Format("D&%02lX(y)", (unsigned long)m_OpMemory[OpPointer]);
					break;

				case 2:
				case 3:
				case 4:
					strTemp.Format("C&%02lX(y)", (unsigned long)m_OpMemory[OpPointer]);
					break;
			}
			OpPointer++;
			break;
		default:
			RetVal = FALSE;
	}

	m_sFunctionalOpcode += strTemp;

	// See if this is a function branch reference that needs to be added:
	if (nTargetAddr != 0xFFFFFFFF) {
		if ((m_CurrentOpcode.m_Mnemonic.CompareNoCase("jsr") == 0) ||
			(m_CurrentOpcode.m_Mnemonic.CompareNoCase("bsr") == 0)) {
			// Add these only if it is replacing a lower priority value:
			if (!m_FunctionsTable.Lookup(nTargetAddr, nDummy)) {
				m_FunctionsTable.SetAt(nTargetAddr, FUNCF_CALL);
			} else {
				if (nDummy > FUNCF_CALL) m_FunctionsTable.SetAt(nTargetAddr, FUNCF_CALL);
			}

			// See if this is the end of a function:
			if (m_FuncExitAddresses.Lookup(nTargetAddr, nDummy) != 0) {
				m_FunctionsTable.SetAt(StartPC, FUNCF_EXITBRANCH);
			}
		}

		if ((m_CurrentOpcode.m_Mnemonic.CompareNoCase("brset") == 0) ||
			(m_CurrentOpcode.m_Mnemonic.CompareNoCase("brclr") == 0) ||
			(m_CurrentOpcode.m_Mnemonic.CompareNoCase("brn") == 0) ||
			(m_CurrentOpcode.m_Mnemonic.CompareNoCase("bhi") == 0) ||
			(m_CurrentOpcode.m_Mnemonic.CompareNoCase("bls") == 0) ||
			(m_CurrentOpcode.m_Mnemonic.CompareNoCase("bcc") == 0) ||
			(m_CurrentOpcode.m_Mnemonic.CompareNoCase("bcs") == 0) ||
			(m_CurrentOpcode.m_Mnemonic.CompareNoCase("bne") == 0) ||
			(m_CurrentOpcode.m_Mnemonic.CompareNoCase("beq") == 0) ||
			(m_CurrentOpcode.m_Mnemonic.CompareNoCase("bvc") == 0) ||
			(m_CurrentOpcode.m_Mnemonic.CompareNoCase("bvs") == 0) ||
			(m_CurrentOpcode.m_Mnemonic.CompareNoCase("bpl") == 0) ||
			(m_CurrentOpcode.m_Mnemonic.CompareNoCase("bmi") == 0) ||
			(m_CurrentOpcode.m_Mnemonic.CompareNoCase("bge") == 0) ||
			(m_CurrentOpcode.m_Mnemonic.CompareNoCase("blt") == 0) ||
			(m_CurrentOpcode.m_Mnemonic.CompareNoCase("bgt") == 0) ||
			(m_CurrentOpcode.m_Mnemonic.CompareNoCase("ble") == 0) ||
			(m_CurrentOpcode.m_Mnemonic.CompareNoCase("bra") == 0) ||
			(m_CurrentOpcode.m_Mnemonic.CompareNoCase("jmp") == 0)) {
			// Add these only if there isn't a function tag here:
			if (!m_FunctionsTable.Lookup(nTargetAddr, nDummy)) m_FunctionsTable.SetAt(nTargetAddr, FUNCF_BRANCHIN);
		}
	}

	// See if this is the end of a function:
	if ((m_CurrentOpcode.m_Mnemonic.CompareNoCase("jmp") == 0) ||
		(m_CurrentOpcode.m_Mnemonic.CompareNoCase("bra") == 0)) {
		if (nTargetAddr != 0xFFFFFFFF) {
			if (m_FuncExitAddresses.Lookup(nTargetAddr, nDummy) != 0) {
				m_FunctionsTable.SetAt(StartPC, FUNCF_EXITBRANCH);
			} else {
				m_FunctionsTable.SetAt(StartPC, FUNCF_BRANCHOUT);
			}
		} else {
			// Non-Deterministic branches get tagged as a branchout:
			//m_FunctionsTable.SetAt(StartPC, FUNCF_BRANCHOUT);

			// Non-Deterministic branches get tagged as a hardstop (usually it exits the function):
			m_FunctionsTable.SetAt(StartPC, FUNCF_HARDSTOP);
		}
	}

	return RetVal;
}

void CM6811Disassembler::CreateOperand(DWORD nGroup, CString& sOpStr)
{
	CString Temp;

	switch (nGroup) {
		case 0x1:								// absolute 8-bit, assume msb=$00
			sOpStr += "*";
			sOpStr += LabelDeref2(m_OpMemory[OpPointer]);
			OpPointer++;
			break;
		case 0x2:								// 16-bit Absolute
			sOpStr += LabelDeref4(m_OpMemory[OpPointer]*256+m_OpMemory[OpPointer+1]);
			OpPointer += 2;
			break;
		case 0x3:								// 8-bit Relative
			sOpStr += LabelDeref4(m_PC+(signed long)(signed char)(m_OpMemory[OpPointer]));
			OpPointer++;
			break;
		case 0x4:								// 16-bit Relative
			sOpStr += LabelDeref4(m_PC+(signed long)(signed short)(m_OpMemory[OpPointer]*256+m_OpMemory[OpPointer+1]));
			OpPointer += 2;
			break;
		case 0x5:								// Immediate 8-bit data
			Temp.Format("#%s%02X", LPCTSTR(GetHexDelim()), m_OpMemory[OpPointer]);
			sOpStr += Temp;
			OpPointer++;
			break;
		case 0x6:								// Immediate 16-bit data
			Temp.Format("#%s%04X", LPCTSTR(GetHexDelim()), m_OpMemory[OpPointer]*256+m_OpMemory[OpPointer+1]);
			sOpStr += Temp;
			OpPointer += 2;
			break;
		case 0x7:								// 8-bit Absolute address, and mask
			sOpStr += "*";
			sOpStr += LabelDeref2(m_OpMemory[OpPointer]);
			sOpStr += ",";
			Temp.Format("#%s%02X", LPCTSTR(GetHexDelim()), m_OpMemory[OpPointer+1]);
			sOpStr += Temp;
			OpPointer += 2;
			break;
		case 0x8:								// 8-bit offset and 8-bit mask (x)
			Temp.Format("%s%02X,x,#%s%02X", LPCTSTR(GetHexDelim()), m_OpMemory[OpPointer],
											LPCTSTR(GetHexDelim()), m_OpMemory[OpPointer+1]);
			sOpStr += Temp;
			OpPointer += 2;
			break;
		case 0x9:								// 8-bit offset and 8-bit mask (y)
			Temp.Format("%s%02X,y,#%s%02X", LPCTSTR(GetHexDelim()), m_OpMemory[OpPointer],
											LPCTSTR(GetHexDelim()), m_OpMemory[OpPointer+1]);
			sOpStr += Temp;
			OpPointer += 2;
			break;
		case 0xA:								// 8-bit offset (x)
			Temp.Format("%s%02X,x", LPCTSTR(GetHexDelim()), m_OpMemory[OpPointer]);
			sOpStr += Temp;
			OpPointer++;
			break;
		case 0xB:								// 8-bit offset (y)
			Temp.Format("%s%02X,y", LPCTSTR(GetHexDelim()), m_OpMemory[OpPointer]);
			sOpStr += Temp;
			OpPointer++;
			break;
	}
}

BOOL CM6811Disassembler::CheckBranchOutside(DWORD nGroup)
{
	BOOL RetVal = TRUE;
	DWORD Address;

	switch (nGroup) {
		case 0x1:								// absolute 8-bit, assume msb=$00
			Address = m_OpMemory[OpPointer];
			RetVal = IsAddressLoaded(Address, 1);
			OpPointer++;
			break;
		case 0x2:								// 16-bit Absolute
			Address = m_OpMemory[OpPointer]*256+m_OpMemory[OpPointer+1];
			RetVal = IsAddressLoaded(Address, 1);
			OpPointer += 2;
			break;
		case 0x3:								// 8-bit Relative
			Address = m_PC+(signed long)(signed char)(m_OpMemory[OpPointer]);
			RetVal = IsAddressLoaded(Address, 1);
			OpPointer++;
			break;
		case 0x4:								// 16-bit Relative
			Address = m_PC+(signed long)(signed short)(m_OpMemory[OpPointer]*256+m_OpMemory[OpPointer+1]);
			RetVal = IsAddressLoaded(Address, 1);
			OpPointer += 2;
			break;
	}
	return RetVal;
}

CString CM6811Disassembler::FormatLabel(LABEL_CODE nLC, LPCTSTR szLabel, DWORD nAddress)
{
	CString Temp = CDisassembler::FormatLabel(nLC, szLabel, nAddress);	// Call parent

	switch (nLC) {
		case LC_EQUATE:
			Temp += LbleDelim;
			break;
		case LC_DATA:
		case LC_CODE:
			Temp += LblcDelim;
			break;
	}
	return Temp;
}


CString CM6811Disassembler::FormatMnemonic(MNEMONIC_CODE nMCCode)
{
	switch (nMCCode) {
		case MC_OPCODE:
			return m_CurrentOpcode.m_Mnemonic;
		case MC_ILLOP:
			return "???";
		case MC_EQUATE:
			return "=";
		case MC_DATABYTE:
			return ".byte";
		case MC_ASCII:
			return ".ascii";
		case MC_INDIRECT:
			return ".word";
	}
	return "???";
}

CString CM6811Disassembler::FormatOperands(MNEMONIC_CODE nMCCode)
{
	CString OpStr;
	CString Temp;
	int i;
	DWORD Address;

	OpStr = "";
	switch (nMCCode) {
		case MC_ILLOP:
		case MC_DATABYTE:
			for (i=0; i<m_OpMemory.GetSize(); i++) {
				Temp.Format("%s%02X", LPCTSTR(GetHexDelim()), m_OpMemory.GetAt(i));
				OpStr += Temp;
				if (i < m_OpMemory.GetSize()-1) OpStr += ",";
			}
			break;
		case MC_ASCII:
			OpStr += DataDelim;
			for (i=0; i<m_OpMemory.GetSize(); i++) {
				Temp.Format("%c", m_OpMemory.GetAt(i));
				OpStr += Temp;
			}
			OpStr += DataDelim;
			break;
		case MC_EQUATE:
			OpStr.Format("%s%04lX", LPCTSTR(GetHexDelim()), m_PC);
			break;
		case MC_INDIRECT:
			if (m_OpMemory.GetSize() != 2) {
				ASSERT(FALSE);			// Check code and make sure RetrieveIndirect got called and called correctly!
				break;
			}
			Address = m_OpMemory.GetAt(0)*256+m_OpMemory.GetAt(1);
			if (m_CodeIndirectTable.Lookup(m_PC-2, Temp) == FALSE) {
				if (m_DataIndirectTable.Lookup(m_PC-2, Temp) == FALSE) {
					Temp = "";
				}
			}
			OpStr = FormatLabel(LC_REF, Temp, Address);
			break;
		case MC_OPCODE:
			OpPointer = m_CurrentOpcode.m_OpcodeBytes.GetSize();	// Get position of start of operands following opcode
			StartPC = m_PC - m_OpMemory.GetSize();		// Get PC of first byte of this instruction for branch references

			CreateOperand(OGRP_DST, OpStr);				// Handle Destination operand first
			if (OGRP_DST != 0x0) OpStr += ",";
			CreateOperand(OGRP_SRC, OpStr);				// Handle Source operand last
			break;
	}

	return OpStr;
}

CString CM6811Disassembler::FormatComments(MNEMONIC_CODE nMCCode)
{
	CString RetVal;
	BOOL Flag;
	DWORD Address;
	CString Temp;

	RetVal = "";

	// Handle Undetermined Branch:
	switch (nMCCode) {
		case MC_OPCODE:
			if ((OCTL_SRC == 0x2) || (OCTL_DST == 0x2) ||
				((CBDef == FALSE) && ((OCTL_SRC == 0x4) || (OCTL_DST == 0x4)))) {
				RetVal = "Undetermined Branch Address";
			}
			break;
	}

	// Handle out-of-source branch:
	Flag = FALSE;
	switch (nMCCode) {
		case MC_OPCODE:
			OpPointer = m_CurrentOpcode.m_OpcodeBytes.GetSize();	// Get position of start of operands following opcode
			StartPC = m_PC - m_OpMemory.GetSize();		// Get PC of first byte of this instruction for branch references
			switch (OCTL_DST) {
				case 0x2:
				case 0x3:
				case 0x4:
					if (CheckBranchOutside(OGRP_DST) == FALSE) Flag = TRUE;
					break;
			}
			switch (OCTL_SRC) {
				case 0x2:
				case 0x3:
				case 0x4:
					if (CheckBranchOutside(OGRP_SRC) == FALSE) Flag = TRUE;
					break;
			}
			break;
		case MC_INDIRECT:
			Address = m_OpMemory.GetAt(0)*256+m_OpMemory.GetAt(1);
			if (m_CodeIndirectTable.Lookup(m_PC-2, Temp)) {
				if (IsAddressLoaded(Address, 1) == FALSE) Flag = TRUE;
			}
			break;
	}

	if (Flag) {
		if (RetVal.GetLength()) RetVal += ", ";
		RetVal += "Branch Outside Loaded Source(s)";
	}

	// Add general refernce stuff:
	if (RetVal.GetLength()) RetVal += "\t";
	RetVal += CDisassembler::FormatComments(nMCCode);		// Call parent and add default comments... i.e references
	return RetVal;
}


BOOL CM6811Disassembler::WritePreSection(CStdioFile& outFile, CStdioFile *msgFile, CStdioFile *errFile)
{
	CStringArray OutLine;
	int i;

	for (i=0; i<NUM_FIELD_CODES; i++) OutLine.Add("");
	OutLine[FC_ADDRESS] = FormatAddress(m_PC);
	OutLine[FC_MNEMONIC] = ".area";
	OutLine[FC_OPERANDS].Format("CODE%d\t(ABS)", ++SectionCount);
	outFile.WriteString(MakeOutputLine(OutLine));
	outFile.WriteString("\n");

	OutLine[FC_MNEMONIC] = ".org";
	OutLine[FC_OPERANDS].Format("%s%04lX", LPCTSTR(GetHexDelim()), m_PC);
	outFile.WriteString(MakeOutputLine(OutLine));
	outFile.WriteString("\n");

	OutLine[FC_MNEMONIC] = "";
	OutLine[FC_OPERANDS] = "";
	outFile.WriteString(MakeOutputLine(OutLine));
	outFile.WriteString("\n");
	return TRUE;
}

CString CM6811Disassembler::LabelDeref2(DWORD nAddress)
{
	CString Temp;
	CStringArray *pLabelList;

	if (m_LabelTable.Lookup(nAddress, pLabelList)) {
		ASSERT(pLabelList != NULL);
		if (pLabelList->GetSize()) {
			Temp = FormatLabel(LC_REF, pLabelList->GetAt(0), nAddress);
		} else {
			Temp = FormatLabel(LC_REF, NULL, nAddress);
		}
	} else {
		Temp.Format("%s%02lX", LPCTSTR(GetHexDelim()), nAddress);
	}
	return Temp;
}

CString CM6811Disassembler::LabelDeref4(DWORD nAddress)
{
	CString Temp;
	CStringArray *pLabelList;

	if (m_LabelTable.Lookup(nAddress, pLabelList)) {
		ASSERT(pLabelList != NULL);
		if (pLabelList->GetSize()) {
			Temp = FormatLabel(LC_REF, pLabelList->GetAt(0), nAddress);
		} else {
			Temp = FormatLabel(LC_REF, NULL, nAddress);
		}
	} else {
		Temp.Format("%s%04lX", LPCTSTR(GetHexDelim()), nAddress);
	}
	return Temp;
}
