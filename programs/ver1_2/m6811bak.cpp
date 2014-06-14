//
//		6811 Disassembler V1.2
//
//	Copyright(c)1996,1997,1998 by Donald Whisnant
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
//	for multiple input modules
//
//
//   Object Code File Format:
//
//	   The object code data file is a text file where each line in the
//   text file denotes one opcode entry.  Each line has the following
//   syntax.  End the file with an entry where nn=0.
//
//	  nn op1 op2 ... opn grp control mnemonic
//
//   Where:
//      nn       = number of bytes for opcode
//      op1..opn = opcode data bytes
//      grp      = opcode operator groups
//      control  = algorithm control code
//      mnemonic = Opcode Assembly Mnemonic
//
//    #Bytes: (nn)
//			0 = END OF DATA FILE
//			1 = 1 byte opcode
//			2 = 2 byte opcode
//			etc.  (Range limited in program.  See const: MaxOpcodeBytes)
//
//	Opcodes: (op1..opn)
//		 valid range = 0-255 (byte)
//
//	Groups: (Grp) : xy
//	  Where x (msnb = destination = FIRST operand)
//			y (lsnb = source = LAST operand)
//
//			0 = opcode only, nothing follows
//			1 = 8-bit absolute address follows only
//			2 = 16-bit absolute address follows only
//			3 = 8-bit relative address follows only
//			4 = 16-bit relative address follows only
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
//			0 = Disassemble as code, and continue disassembly
//			1 = Disassemble as code, but discontinue disassembly
//					(i.e. finished with routine. ex: RTS)
//			2 = Disassemble as code, with data addr label,
//					and continue disassembly (on opcodes with post-byte,
//					label generation is dependent upon post-byte value.)
//			3 = Disassemble as code, with data addr label,
//					but discontinue disassembly (on opcodes with post-byte,
//					label generation is dependent upon post-byte value.)
//			4 = Disassemble as undeterminable branch address (Comment code),
//					and continue disassembly
//			5 = Disassemble as undeterminable branch address (Comment code),
//					but discontinue disassembly (i.e. finished with routine, ex:JMP)
//			6 = Disassemble as determinable branch, add branch addr and label,
//					and continue disassembly
//			7 = Disassemble as determinable branch, add branch addr and label,
//					but discontinue disassembly (i.e. finished with routine, ex:JMP)
//			8 = Disassemble as conditionally undeterminable branch address (Comment code),
//					and continue disassembly, with data addr label,
//					(on opcodes with post-byte, label generation is dependent
//					upon post-byte value. conditional upon post-byte)
//			9 = Disassemble as conditionally undeterminable branch address (Comment code),
//					but discontinue disassembly (i.e. finished with routine,
//					ex:JMP), with data addr label
//					(on opcodes with post-byte, label generation is dependent
//					upon post-byte value. conditional upon post-byte)
//
//	Mnemonic:
//		 Any valid string

#include <stdlib.h>
#include <stdio.h>
#include "stdafx.h"
#include "m6811dis.h"

#define DtaDelim	39			// Specify ' as delimiter for data literals
#define LblcDelim	":"			// Delimiter between labels and code
#define LbleDelim	""			// Delimiter between labels and equates
#define	ComDelim	";"			// Comment delimiter
#define HexDelim	"0x"		// Hex. delimiter





//	 MaxOpcodes = 500;       { Allow up to 500 opcodes }
//	 MaxEntries = 32;        { Allow up to 32 designated entries }
//	 MaxBranches = 4096;     { Allow up to 4k branch addresses }
//	 MaxLbls     = 4096;     { Allow up to 4k labels }
//	 MaxIndirect = 512;      { Allow up to 1/2k indirect entries }
//	 BufSize     = 2048;     { Allow a 2k program read buffer for speed }
//	 MnemSize    = 7;        { Allow up to 7 chars for Mnemonics }
//	 LBLSize     = 6;        { Allow up to 6 chars for labels }
//	 MaxOpcodeBytes = 2;     { Allow a maximum of 2 object bytes per opcode }
//	 NumCommands = 10;
//	 ValidCommands : array[1..NumCommands] of string[10]
//				   = (	'ENTRY',
//						'LOAD',
//						'INPUT',
//						'OUTPUT',
//						'LABEL',
//						'ADDRESSES',
//						'INDIRECT',
//						'OPCODES',
//						'ASCII',
//						'SPIT');

//     HexValue  : array[0..15] of string[1]
//               = ('0','1','2','3','4','5','6','7','8','9',
//                  'A','B','C','D','E','F');


//     PrintAbleLo = 32;       { Define the highest and lowest ASCII }
//     PrintAbleHi = 126;      {   characters that are printable }




     OpMemorySize = 7;       { Size of opcode memory - set to maximum }
                             {   possible number of bytes an object can take }
     OpMaxSize   = 2;        { Set to number of maximum bytes for opcode
                                 itself, excluding post-bytes and operands }
     Tab = ^I;               { Tab character for output, works best a tstp=5 }
     MaxNonPrint = 8;        { Maximum non-printable characters per line }
     MaxPrint = 40;          { Maximum printable characters per line }

type
    CmdType = (ReadPage, WritePage);     { Memory Operation declaration }

    ObjCodesType = Record       { Object Code Record Storage Declaration }
      NBytes    :  Integer;     {   and # of bytes for opcode }
      OpCode    :  array[1..MaxOpcodeBytes] of Byte;  { Storage for opcode }
      Grp       :  Integer;     { Store name and group declaration }
      Control   :  Integer;     { Storage for the control code }
      Mnemonic  :  String[MnemSize];   {   for single byte opcodes }
    end;

    ObjCodeNdxType = Record     { Typing for object code index }
      Legal     :  Boolean;     { If first byte is legal opcode, then true }
      Start     :  Integer;     { Starting location in table of opcodes }
    end;

    LblTableType = Record       { Label storage definition }
      Name       : String[LblSize];
      Addr       : Word;
    end;

    MemoryDataType = array[0..32767] of Byte;    { Memory Page declaration }
    BytePtr = ^MemoryDataType;

const
   NumOpcodes = 308;                            { Number of opcodes used }

   ObjectCode : array[0..NumOpcodes] of ObjCodesType   { Storage for opcode data }
                = ((NBytes: 0; OpCode: (0, 0); Grp: 0; Control: 1; Mnemonic: '???'),
                   (NBytes: 1; OpCode: (0, 0); Grp: 0; Control: 0; Mnemonic: 'test'),
                   (NBytes: 1; OpCode: (1, 0); Grp: 0; Control: 0; Mnemonic: 'nop'),
                   (NBytes: 1; OpCode: (2, 0); Grp: 0; Control: 0; Mnemonic: 'idiv'),
                   (NBytes: 1; OpCode: (3, 0); Grp: 0; Control: 0; Mnemonic: 'fdiv'),
                   (NBytes: 1; OpCode: (4, 0); Grp: 0; Control: 0; Mnemonic: 'lsrd'),
                   (NBytes: 1; OpCode: (5, 0); Grp: 0; Control: 0; Mnemonic: 'lsld'),
                   (NBytes: 1; OpCode: (6, 0); Grp: 0; Control: 0; Mnemonic: 'tap'),
                   (NBytes: 1; OpCode: (7, 0); Grp: 0; Control: 0; Mnemonic: 'tpa'),
                   (NBytes: 1; OpCode: (8, 0); Grp: 0; Control: 0; Mnemonic: 'inx'),
                   (NBytes: 1; OpCode: (9, 0); Grp: 0; Control: 0; Mnemonic: 'dex'),
                   (NBytes: 1; OpCode: (10, 0); Grp: 0; Control: 0; Mnemonic: 'clv'),
                   (NBytes: 1; OpCode: (11, 0); Grp: 0; Control: 0; Mnemonic: 'sev'),
                   (NBytes: 1; OpCode: (12, 0); Grp: 0; Control: 0; Mnemonic: 'clc'),
                   (NBytes: 1; OpCode: (13, 0); Grp: 0; Control: 0; Mnemonic: 'sec'),
                   (NBytes: 1; OpCode: (14, 0); Grp: 0; Control: 0; Mnemonic: 'cli'),
                   (NBytes: 1; OpCode: (15, 0); Grp: 0; Control: 0; Mnemonic: 'sei'),
                   (NBytes: 1; OpCode: (16, 0); Grp: 0; Control: 0; Mnemonic: 'sba'),
                   (NBytes: 1; OpCode: (17, 0); Grp: 0; Control: 0; Mnemonic: 'cba'),
                   (NBytes: 1; OpCode: (18, 0); Grp: 115; Control: 38; Mnemonic: 'brset'),
                   (NBytes: 1; OpCode: (19, 0); Grp: 115; Control: 38; Mnemonic: 'brclr'),
                   (NBytes: 1; OpCode: (20, 0); Grp: 7; Control: 2; Mnemonic: 'bset'),
                   (NBytes: 1; OpCode: (21, 0); Grp: 7; Control: 2; Mnemonic: 'bclr'),
                   (NBytes: 1; OpCode: (22, 0); Grp: 0; Control: 0; Mnemonic: 'tab'),
                   (NBytes: 1; OpCode: (23, 0); Grp: 0; Control: 0; Mnemonic: 'tba'),
                   (NBytes: 2; OpCode: (24, 8); Grp: 0; Control: 0; Mnemonic: 'iny'),
                   (NBytes: 2; OpCode: (24, 9); Grp: 0; Control: 0; Mnemonic: 'dey'),
                   (NBytes: 2; OpCode: (24, 28); Grp: 9; Control: 0; Mnemonic: 'bset'),
                   (NBytes: 2; OpCode: (24, 29); Grp: 9; Control: 0; Mnemonic: 'bclr'),
                   (NBytes: 2; OpCode: (24, 30); Grp: 147; Control: 6; Mnemonic: 'brset'),
                   (NBytes: 2; OpCode: (24, 31); Grp: 147; Control: 6; Mnemonic: 'brclr'),
                   (NBytes: 2; OpCode: (24, 48); Grp: 0; Control: 0; Mnemonic: 'tsy'),
                   (NBytes: 2; OpCode: (24, 53); Grp: 0; Control: 0; Mnemonic: 'tys'),
                   (NBytes: 2; OpCode: (24, 56); Grp: 0; Control: 0; Mnemonic: 'puly'),
                   (NBytes: 2; OpCode: (24, 58); Grp: 0; Control: 0; Mnemonic: 'aby'),
                   (NBytes: 2; OpCode: (24, 60); Grp: 0; Control: 0; Mnemonic: 'pshy'),
                   (NBytes: 2; OpCode: (24, 96); Grp: 11; Control: 0; Mnemonic: 'neg'),
                   (NBytes: 2; OpCode: (24, 99); Grp: 11; Control: 0; Mnemonic: 'com'),
                   (NBytes: 2; OpCode: (24, 100); Grp: 11; Control: 0; Mnemonic: 'lsr'),
                   (NBytes: 2; OpCode: (24, 102); Grp: 11; Control: 0; Mnemonic: 'ror'),
                   (NBytes: 2; OpCode: (24, 103); Grp: 11; Control: 0; Mnemonic: 'asr'),
                   (NBytes: 2; OpCode: (24, 104); Grp: 11; Control: 0; Mnemonic: 'lsl'),
                   (NBytes: 2; OpCode: (24, 105); Grp: 11; Control: 0; Mnemonic: 'rol'),
                   (NBytes: 2; OpCode: (24, 106); Grp: 11; Control: 0; Mnemonic: 'dec'),
                   (NBytes: 2; OpCode: (24, 108); Grp: 11; Control: 0; Mnemonic: 'inc'),
                   (NBytes: 2; OpCode: (24, 109); Grp: 11; Control: 0; Mnemonic: 'tst'),
                   (NBytes: 2; OpCode: (24, 110); Grp: 11; Control: 5; Mnemonic: 'jmp'),
                   (NBytes: 2; OpCode: (24, 111); Grp: 11; Control: 0; Mnemonic: 'clr'),
                   (NBytes: 2; OpCode: (24, 140); Grp: 6; Control: 0; Mnemonic: 'cpy'),
                   (NBytes: 2; OpCode: (24, 143); Grp: 0; Control: 0; Mnemonic: 'xgdy'),
                   (NBytes: 2; OpCode: (24, 156); Grp: 1; Control: 2; Mnemonic: 'cpy'),
                   (NBytes: 2; OpCode: (24, 160); Grp: 11; Control: 0; Mnemonic: 'suba'),
                   (NBytes: 2; OpCode: (24, 161); Grp: 11; Control: 0; Mnemonic: 'cmpa'),
                   (NBytes: 2; OpCode: (24, 162); Grp: 11; Control: 0; Mnemonic: 'sbca'),
                   (NBytes: 2; OpCode: (24, 163); Grp: 11; Control: 0; Mnemonic: 'subd'),
                   (NBytes: 2; OpCode: (24, 164); Grp: 11; Control: 0; Mnemonic: 'anda'),
                   (NBytes: 2; OpCode: (24, 165); Grp: 11; Control: 0; Mnemonic: 'bita'),
                   (NBytes: 2; OpCode: (24, 166); Grp: 11; Control: 0; Mnemonic: 'ldaa'),
                   (NBytes: 2; OpCode: (24, 167); Grp: 11; Control: 0; Mnemonic: 'staa'),
                   (NBytes: 2; OpCode: (24, 168); Grp: 11; Control: 0; Mnemonic: 'eora'),
                   (NBytes: 2; OpCode: (24, 169); Grp: 11; Control: 0; Mnemonic: 'adca'),
                   (NBytes: 2; OpCode: (24, 170); Grp: 11; Control: 0; Mnemonic: 'oraa'),
                   (NBytes: 2; OpCode: (24, 171); Grp: 11; Control: 0; Mnemonic: 'adda'),
                   (NBytes: 2; OpCode: (24, 172); Grp: 11; Control: 0; Mnemonic: 'cpy'),
                   (NBytes: 2; OpCode: (24, 173); Grp: 11; Control: 4; Mnemonic: 'jsr'),
                   (NBytes: 2; OpCode: (24, 174); Grp: 11; Control: 0; Mnemonic: 'lds'),
                   (NBytes: 2; OpCode: (24, 175); Grp: 11; Control: 0; Mnemonic: 'sts'),
                   (NBytes: 2; OpCode: (24, 188); Grp: 2; Control: 2; Mnemonic: 'cpy'),
                   (NBytes: 2; OpCode: (24, 206); Grp: 6; Control: 0; Mnemonic: 'ldy'),
                   (NBytes: 2; OpCode: (24, 222); Grp: 1; Control: 2; Mnemonic: 'ldy'),
                   (NBytes: 2; OpCode: (24, 223); Grp: 1; Control: 2; Mnemonic: 'sty'),
                   (NBytes: 2; OpCode: (24, 224); Grp: 11; Control: 0; Mnemonic: 'subb'),
                   (NBytes: 2; OpCode: (24, 225); Grp: 11; Control: 0; Mnemonic: 'cmpb'),
                   (NBytes: 2; OpCode: (24, 226); Grp: 11; Control: 0; Mnemonic: 'sbcb'),
                   (NBytes: 2; OpCode: (24, 227); Grp: 11; Control: 0; Mnemonic: 'addd'),
                   (NBytes: 2; OpCode: (24, 228); Grp: 11; Control: 0; Mnemonic: 'andb'),
                   (NBytes: 2; OpCode: (24, 229); Grp: 11; Control: 0; Mnemonic: 'bitb'),
                   (NBytes: 2; OpCode: (24, 230); Grp: 11; Control: 0; Mnemonic: 'ldab'),
                   (NBytes: 2; OpCode: (24, 231); Grp: 11; Control: 0; Mnemonic: 'stab'),
                   (NBytes: 2; OpCode: (24, 232); Grp: 11; Control: 0; Mnemonic: 'eorb'),
                   (NBytes: 2; OpCode: (24, 233); Grp: 11; Control: 0; Mnemonic: 'adcb'),
                   (NBytes: 2; OpCode: (24, 234); Grp: 11; Control: 0; Mnemonic: 'orab'),
                   (NBytes: 2; OpCode: (24, 235); Grp: 11; Control: 0; Mnemonic: 'addb'),
                   (NBytes: 2; OpCode: (24, 236); Grp: 11; Control: 0; Mnemonic: 'ldd'),
                   (NBytes: 2; OpCode: (24, 237); Grp: 11; Control: 0; Mnemonic: 'std'),
                   (NBytes: 2; OpCode: (24, 238); Grp: 11; Control: 0; Mnemonic: 'ldy'),
                   (NBytes: 2; OpCode: (24, 239); Grp: 11; Control: 0; Mnemonic: 'sty'),
                   (NBytes: 2; OpCode: (24, 254); Grp: 2; Control: 2; Mnemonic: 'ldy'),
                   (NBytes: 2; OpCode: (24, 255); Grp: 2; Control: 2; Mnemonic: 'sty'),
                   (NBytes: 1; OpCode: (25, 0); Grp: 0; Control: 0; Mnemonic: 'daa'),
                   (NBytes: 2; OpCode: (26, 131); Grp: 6; Control: 0; Mnemonic: 'cpd'),
                   (NBytes: 2; OpCode: (26, 147); Grp: 1; Control: 2; Mnemonic: 'cpd'),
                   (NBytes: 2; OpCode: (26, 163); Grp: 10; Control: 0; Mnemonic: 'cpd'),
                   (NBytes: 2; OpCode: (26, 172); Grp: 10; Control: 0; Mnemonic: 'cpy'),
                   (NBytes: 2; OpCode: (26, 179); Grp: 2; Control: 2; Mnemonic: 'cpd'),
                   (NBytes: 2; OpCode: (26, 238); Grp: 10; Control: 0; Mnemonic: 'ldy'),
                   (NBytes: 2; OpCode: (26, 239); Grp: 10; Control: 0; Mnemonic: 'sty'),
                   (NBytes: 1; OpCode: (27, 0); Grp: 0; Control: 0; Mnemonic: 'aba'),
                   (NBytes: 1; OpCode: (28, 0); Grp: 8; Control: 0; Mnemonic: 'bset'),
                   (NBytes: 1; OpCode: (29, 0); Grp: 8; Control: 0; Mnemonic: 'bclr'),
                   (NBytes: 1; OpCode: (30, 0); Grp: 131; Control: 6; Mnemonic: 'brset'),
                   (NBytes: 1; OpCode: (31, 0); Grp: 131; Control: 6; Mnemonic: 'brclr'),
                   (NBytes: 1; OpCode: (32, 0); Grp: 3; Control: 7; Mnemonic: 'bra'),
                   (NBytes: 1; OpCode: (33, 0); Grp: 3; Control: 6; Mnemonic: 'brn'),
                   (NBytes: 1; OpCode: (34, 0); Grp: 3; Control: 6; Mnemonic: 'bhi'),
                   (NBytes: 1; OpCode: (35, 0); Grp: 3; Control: 6; Mnemonic: 'bls'),
                   (NBytes: 1; OpCode: (36, 0); Grp: 3; Control: 6; Mnemonic: 'bcc'),
                   (NBytes: 1; OpCode: (37, 0); Grp: 3; Control: 6; Mnemonic: 'bcs'),
                   (NBytes: 1; OpCode: (38, 0); Grp: 3; Control: 6; Mnemonic: 'bne'),
                   (NBytes: 1; OpCode: (39, 0); Grp: 3; Control: 6; Mnemonic: 'beq'),
                   (NBytes: 1; OpCode: (40, 0); Grp: 3; Control: 6; Mnemonic: 'bvc'),
                   (NBytes: 1; OpCode: (41, 0); Grp: 3; Control: 6; Mnemonic: 'bvs'),
                   (NBytes: 1; OpCode: (42, 0); Grp: 3; Control: 6; Mnemonic: 'bpl'),
                   (NBytes: 1; OpCode: (43, 0); Grp: 3; Control: 6; Mnemonic: 'bmi'),
                   (NBytes: 1; OpCode: (44, 0); Grp: 3; Control: 6; Mnemonic: 'bge'),
                   (NBytes: 1; OpCode: (45, 0); Grp: 3; Control: 6; Mnemonic: 'blt'),
                   (NBytes: 1; OpCode: (46, 0); Grp: 3; Control: 6; Mnemonic: 'bgt'),
                   (NBytes: 1; OpCode: (47, 0); Grp: 3; Control: 6; Mnemonic: 'ble'),
                   (NBytes: 1; OpCode: (48, 0); Grp: 0; Control: 0; Mnemonic: 'tsx'),
                   (NBytes: 1; OpCode: (49, 0); Grp: 0; Control: 0; Mnemonic: 'ins'),
                   (NBytes: 1; OpCode: (50, 0); Grp: 0; Control: 0; Mnemonic: 'pula'),
                   (NBytes: 1; OpCode: (51, 0); Grp: 0; Control: 0; Mnemonic: 'pulb'),
                   (NBytes: 1; OpCode: (52, 0); Grp: 0; Control: 0; Mnemonic: 'des'),
                   (NBytes: 1; OpCode: (53, 0); Grp: 0; Control: 0; Mnemonic: 'txs'),
                   (NBytes: 1; OpCode: (54, 0); Grp: 0; Control: 0; Mnemonic: 'psha'),
                   (NBytes: 1; OpCode: (55, 0); Grp: 0; Control: 0; Mnemonic: 'pshb'),
                   (NBytes: 1; OpCode: (56, 0); Grp: 0; Control: 0; Mnemonic: 'pulx'),
                   (NBytes: 1; OpCode: (57, 0); Grp: 0; Control: 1; Mnemonic: 'rts'),
                   (NBytes: 1; OpCode: (58, 0); Grp: 0; Control: 0; Mnemonic: 'abx'),
                   (NBytes: 1; OpCode: (59, 0); Grp: 0; Control: 1; Mnemonic: 'rti'),
                   (NBytes: 1; OpCode: (60, 0); Grp: 0; Control: 0; Mnemonic: 'pshx'),
                   (NBytes: 1; OpCode: (61, 0); Grp: 0; Control: 0; Mnemonic: 'mul'),
                   (NBytes: 1; OpCode: (62, 0); Grp: 0; Control: 0; Mnemonic: 'wai'),
                   (NBytes: 1; OpCode: (63, 0); Grp: 0; Control: 0; Mnemonic: 'swi'),
                   (NBytes: 1; OpCode: (64, 0); Grp: 0; Control: 0; Mnemonic: 'nega'),
                   (NBytes: 1; OpCode: (67, 0); Grp: 0; Control: 0; Mnemonic: 'coma'),
                   (NBytes: 1; OpCode: (68, 0); Grp: 0; Control: 0; Mnemonic: 'lsra'),
                   (NBytes: 1; OpCode: (70, 0); Grp: 0; Control: 0; Mnemonic: 'rora'),
                   (NBytes: 1; OpCode: (71, 0); Grp: 0; Control: 0; Mnemonic: 'asra'),
                   (NBytes: 1; OpCode: (72, 0); Grp: 0; Control: 0; Mnemonic: 'lsla'),
                   (NBytes: 1; OpCode: (73, 0); Grp: 0; Control: 0; Mnemonic: 'rola'),
                   (NBytes: 1; OpCode: (74, 0); Grp: 0; Control: 0; Mnemonic: 'deca'),
                   (NBytes: 1; OpCode: (76, 0); Grp: 0; Control: 0; Mnemonic: 'inca'),
                   (NBytes: 1; OpCode: (77, 0); Grp: 0; Control: 0; Mnemonic: 'tsta'),
                   (NBytes: 1; OpCode: (79, 0); Grp: 0; Control: 0; Mnemonic: 'clra'),
                   (NBytes: 1; OpCode: (80, 0); Grp: 0; Control: 0; Mnemonic: 'negb'),
                   (NBytes: 1; OpCode: (83, 0); Grp: 0; Control: 0; Mnemonic: 'comb'),
                   (NBytes: 1; OpCode: (84, 0); Grp: 0; Control: 0; Mnemonic: 'lsrb'),
                   (NBytes: 1; OpCode: (86, 0); Grp: 0; Control: 0; Mnemonic: 'rorb'),
                   (NBytes: 1; OpCode: (87, 0); Grp: 0; Control: 0; Mnemonic: 'asrb'),
                   (NBytes: 1; OpCode: (88, 0); Grp: 0; Control: 0; Mnemonic: 'lslb'),
                   (NBytes: 1; OpCode: (89, 0); Grp: 0; Control: 0; Mnemonic: 'rolb'),
                   (NBytes: 1; OpCode: (90, 0); Grp: 0; Control: 0; Mnemonic: 'decb'),
                   (NBytes: 1; OpCode: (92, 0); Grp: 0; Control: 0; Mnemonic: 'incb'),
                   (NBytes: 1; OpCode: (93, 0); Grp: 0; Control: 0; Mnemonic: 'tstb'),
                   (NBytes: 1; OpCode: (95, 0); Grp: 0; Control: 0; Mnemonic: 'clrb'),
                   (NBytes: 1; OpCode: (96, 0); Grp: 10; Control: 0; Mnemonic: 'neg'),
                   (NBytes: 1; OpCode: (99, 0); Grp: 10; Control: 0; Mnemonic: 'com'),
                   (NBytes: 1; OpCode: (100, 0); Grp: 10; Control: 0; Mnemonic: 'lsr'),
                   (NBytes: 1; OpCode: (102, 0); Grp: 10; Control: 0; Mnemonic: 'ror'),
                   (NBytes: 1; OpCode: (103, 0); Grp: 10; Control: 0; Mnemonic: 'asr'),
                   (NBytes: 1; OpCode: (104, 0); Grp: 10; Control: 0; Mnemonic: 'lsl'),
                   (NBytes: 1; OpCode: (105, 0); Grp: 10; Control: 0; Mnemonic: 'rol'),
                   (NBytes: 1; OpCode: (106, 0); Grp: 10; Control: 0; Mnemonic: 'dec'),
                   (NBytes: 1; OpCode: (108, 0); Grp: 10; Control: 0; Mnemonic: 'inc'),
                   (NBytes: 1; OpCode: (109, 0); Grp: 10; Control: 0; Mnemonic: 'tst'),
                   (NBytes: 1; OpCode: (110, 0); Grp: 10; Control: 5; Mnemonic: 'jmp'),
                   (NBytes: 1; OpCode: (111, 0); Grp: 10; Control: 0; Mnemonic: 'clr'),
                   (NBytes: 1; OpCode: (112, 0); Grp: 2; Control: 2; Mnemonic: 'neg'),
                   (NBytes: 1; OpCode: (115, 0); Grp: 2; Control: 2; Mnemonic: 'com'),
                   (NBytes: 1; OpCode: (116, 0); Grp: 2; Control: 2; Mnemonic: 'lsr'),
                   (NBytes: 1; OpCode: (118, 0); Grp: 2; Control: 2; Mnemonic: 'ror'),
                   (NBytes: 1; OpCode: (119, 0); Grp: 2; Control: 2; Mnemonic: 'asr'),
                   (NBytes: 1; OpCode: (120, 0); Grp: 2; Control: 2; Mnemonic: 'lsl'),
                   (NBytes: 1; OpCode: (121, 0); Grp: 2; Control: 2; Mnemonic: 'rol'),
                   (NBytes: 1; OpCode: (122, 0); Grp: 2; Control: 2; Mnemonic: 'dec'),
                   (NBytes: 1; OpCode: (124, 0); Grp: 2; Control: 2; Mnemonic: 'inc'),
                   (NBytes: 1; OpCode: (125, 0); Grp: 2; Control: 2; Mnemonic: 'tst'),
                   (NBytes: 1; OpCode: (126, 0); Grp: 2; Control: 7; Mnemonic: 'jmp'),
                   (NBytes: 1; OpCode: (127, 0); Grp: 2; Control: 2; Mnemonic: 'clr'),
                   (NBytes: 1; OpCode: (128, 0); Grp: 5; Control: 0; Mnemonic: 'suba'),
                   (NBytes: 1; OpCode: (129, 0); Grp: 5; Control: 0; Mnemonic: 'cmpa'),
                   (NBytes: 1; OpCode: (130, 0); Grp: 5; Control: 0; Mnemonic: 'sbca'),
                   (NBytes: 1; OpCode: (131, 0); Grp: 6; Control: 0; Mnemonic: 'subd'),
                   (NBytes: 1; OpCode: (132, 0); Grp: 5; Control: 0; Mnemonic: 'anda'),
                   (NBytes: 1; OpCode: (133, 0); Grp: 5; Control: 0; Mnemonic: 'bita'),
                   (NBytes: 1; OpCode: (134, 0); Grp: 5; Control: 0; Mnemonic: 'ldaa'),
                   (NBytes: 1; OpCode: (136, 0); Grp: 5; Control: 0; Mnemonic: 'eora'),
                   (NBytes: 1; OpCode: (137, 0); Grp: 5; Control: 0; Mnemonic: 'adca'),
                   (NBytes: 1; OpCode: (138, 0); Grp: 5; Control: 0; Mnemonic: 'oraa'),
                   (NBytes: 1; OpCode: (139, 0); Grp: 5; Control: 0; Mnemonic: 'adda'),
                   (NBytes: 1; OpCode: (140, 0); Grp: 6; Control: 0; Mnemonic: 'cpx'),
                   (NBytes: 1; OpCode: (141, 0); Grp: 3; Control: 6; Mnemonic: 'bsr'),
                   (NBytes: 1; OpCode: (142, 0); Grp: 6; Control: 0; Mnemonic: 'lds'),
                   (NBytes: 1; OpCode: (143, 0); Grp: 0; Control: 0; Mnemonic: 'xgdx'),
                   (NBytes: 1; OpCode: (144, 0); Grp: 1; Control: 2; Mnemonic: 'suba'),
                   (NBytes: 1; OpCode: (145, 0); Grp: 1; Control: 2; Mnemonic: 'cmpa'),
                   (NBytes: 1; OpCode: (146, 0); Grp: 1; Control: 2; Mnemonic: 'sbca'),
                   (NBytes: 1; OpCode: (147, 0); Grp: 1; Control: 2; Mnemonic: 'subd'),
                   (NBytes: 1; OpCode: (148, 0); Grp: 1; Control: 2; Mnemonic: 'anda'),
                   (NBytes: 1; OpCode: (149, 0); Grp: 1; Control: 2; Mnemonic: 'bita'),
                   (NBytes: 1; OpCode: (150, 0); Grp: 1; Control: 2; Mnemonic: 'ldaa'),
                   (NBytes: 1; OpCode: (151, 0); Grp: 1; Control: 2; Mnemonic: 'staa'),
                   (NBytes: 1; OpCode: (152, 0); Grp: 1; Control: 2; Mnemonic: 'eora'),
                   (NBytes: 1; OpCode: (153, 0); Grp: 1; Control: 2; Mnemonic: 'adca'),
                   (NBytes: 1; OpCode: (154, 0); Grp: 1; Control: 2; Mnemonic: 'oraa'),
                   (NBytes: 1; OpCode: (155, 0); Grp: 1; Control: 2; Mnemonic: 'adda'),
                   (NBytes: 1; OpCode: (156, 0); Grp: 1; Control: 2; Mnemonic: 'cpx'),
                   (NBytes: 1; OpCode: (157, 0); Grp: 1; Control: 6; Mnemonic: 'jsr'),
                   (NBytes: 1; OpCode: (158, 0); Grp: 1; Control: 2; Mnemonic: 'lds'),
                   (NBytes: 1; OpCode: (159, 0); Grp: 1; Control: 2; Mnemonic: 'sts'),
                   (NBytes: 1; OpCode: (160, 0); Grp: 10; Control: 0; Mnemonic: 'suba'),
                   (NBytes: 1; OpCode: (161, 0); Grp: 10; Control: 0; Mnemonic: 'cmpa'),
                   (NBytes: 1; OpCode: (162, 0); Grp: 10; Control: 0; Mnemonic: 'sbca'),
                   (NBytes: 1; OpCode: (163, 0); Grp: 10; Control: 0; Mnemonic: 'subd'),
                   (NBytes: 1; OpCode: (164, 0); Grp: 10; Control: 0; Mnemonic: 'anda'),
                   (NBytes: 1; OpCode: (165, 0); Grp: 10; Control: 0; Mnemonic: 'bita'),
                   (NBytes: 1; OpCode: (166, 0); Grp: 10; Control: 0; Mnemonic: 'ldaa'),
                   (NBytes: 1; OpCode: (167, 0); Grp: 10; Control: 0; Mnemonic: 'staa'),
                   (NBytes: 1; OpCode: (168, 0); Grp: 10; Control: 0; Mnemonic: 'eora'),
                   (NBytes: 1; OpCode: (169, 0); Grp: 10; Control: 0; Mnemonic: 'adca'),
                   (NBytes: 1; OpCode: (170, 0); Grp: 10; Control: 0; Mnemonic: 'oraa'),
                   (NBytes: 1; OpCode: (171, 0); Grp: 10; Control: 0; Mnemonic: 'adda'),
                   (NBytes: 1; OpCode: (172, 0); Grp: 10; Control: 0; Mnemonic: 'cpx'),
                   (NBytes: 1; OpCode: (173, 0); Grp: 10; Control: 4; Mnemonic: 'jsr'),
                   (NBytes: 1; OpCode: (174, 0); Grp: 10; Control: 0; Mnemonic: 'lds'),
                   (NBytes: 1; OpCode: (175, 0); Grp: 10; Control: 0; Mnemonic: 'sts'),
                   (NBytes: 1; OpCode: (176, 0); Grp: 2; Control: 2; Mnemonic: 'suba'),
                   (NBytes: 1; OpCode: (177, 0); Grp: 2; Control: 2; Mnemonic: 'cmpa'),
                   (NBytes: 1; OpCode: (178, 0); Grp: 2; Control: 2; Mnemonic: 'sbca'),
                   (NBytes: 1; OpCode: (179, 0); Grp: 2; Control: 2; Mnemonic: 'subd'),
                   (NBytes: 1; OpCode: (180, 0); Grp: 2; Control: 2; Mnemonic: 'anda'),
                   (NBytes: 1; OpCode: (181, 0); Grp: 2; Control: 2; Mnemonic: 'bita'),
                   (NBytes: 1; OpCode: (182, 0); Grp: 2; Control: 2; Mnemonic: 'ldaa'),
                   (NBytes: 1; OpCode: (183, 0); Grp: 2; Control: 2; Mnemonic: 'staa'),
                   (NBytes: 1; OpCode: (184, 0); Grp: 2; Control: 2; Mnemonic: 'eora'),
                   (NBytes: 1; OpCode: (185, 0); Grp: 2; Control: 2; Mnemonic: 'adca'),
                   (NBytes: 1; OpCode: (186, 0); Grp: 2; Control: 2; Mnemonic: 'oraa'),
                   (NBytes: 1; OpCode: (187, 0); Grp: 2; Control: 2; Mnemonic: 'adda'),
                   (NBytes: 1; OpCode: (188, 0); Grp: 2; Control: 2; Mnemonic: 'cpx'),
                   (NBytes: 1; OpCode: (189, 0); Grp: 2; Control: 6; Mnemonic: 'jsr'),
                   (NBytes: 1; OpCode: (190, 0); Grp: 2; Control: 2; Mnemonic: 'lds'),
                   (NBytes: 1; OpCode: (191, 0); Grp: 2; Control: 2; Mnemonic: 'sts'),
                   (NBytes: 1; OpCode: (192, 0); Grp: 5; Control: 0; Mnemonic: 'subb'),
                   (NBytes: 1; OpCode: (193, 0); Grp: 5; Control: 0; Mnemonic: 'cmpb'),
                   (NBytes: 1; OpCode: (194, 0); Grp: 5; Control: 0; Mnemonic: 'sbcb'),
                   (NBytes: 1; OpCode: (195, 0); Grp: 6; Control: 0; Mnemonic: 'addd'),
                   (NBytes: 1; OpCode: (196, 0); Grp: 5; Control: 0; Mnemonic: 'andb'),
                   (NBytes: 1; OpCode: (197, 0); Grp: 5; Control: 0; Mnemonic: 'bitb'),
                   (NBytes: 1; OpCode: (198, 0); Grp: 5; Control: 0; Mnemonic: 'ldab'),
                   (NBytes: 1; OpCode: (200, 0); Grp: 5; Control: 0; Mnemonic: 'eorb'),
                   (NBytes: 1; OpCode: (201, 0); Grp: 5; Control: 0; Mnemonic: 'adcb'),
                   (NBytes: 1; OpCode: (202, 0); Grp: 5; Control: 0; Mnemonic: 'orab'),
                   (NBytes: 1; OpCode: (203, 0); Grp: 5; Control: 0; Mnemonic: 'addb'),
                   (NBytes: 1; OpCode: (204, 0); Grp: 6; Control: 0; Mnemonic: 'ldd'),
                   (NBytes: 2; OpCode: (205, 163); Grp: 11; Control: 0; Mnemonic: 'cpd'),
                   (NBytes: 2; OpCode: (205, 172); Grp: 11; Control: 0; Mnemonic: 'cpx'),
                   (NBytes: 2; OpCode: (205, 238); Grp: 11; Control: 0; Mnemonic: 'ldx'),
                   (NBytes: 2; OpCode: (205, 239); Grp: 11; Control: 0; Mnemonic: 'stx'),
                   (NBytes: 1; OpCode: (206, 0); Grp: 6; Control: 0; Mnemonic: 'ldx'),
                   (NBytes: 1; OpCode: (207, 0); Grp: 0; Control: 0; Mnemonic: 'stop'),
                   (NBytes: 1; OpCode: (208, 0); Grp: 1; Control: 2; Mnemonic: 'subb'),
                   (NBytes: 1; OpCode: (209, 0); Grp: 1; Control: 2; Mnemonic: 'cmpb'),
                   (NBytes: 1; OpCode: (210, 0); Grp: 1; Control: 2; Mnemonic: 'sbcb'),
                   (NBytes: 1; OpCode: (211, 0); Grp: 1; Control: 2; Mnemonic: 'addd'),
                   (NBytes: 1; OpCode: (212, 0); Grp: 1; Control: 2; Mnemonic: 'andb'),
                   (NBytes: 1; OpCode: (213, 0); Grp: 1; Control: 2; Mnemonic: 'bitb'),
                   (NBytes: 1; OpCode: (214, 0); Grp: 1; Control: 2; Mnemonic: 'ldab'),
                   (NBytes: 1; OpCode: (215, 0); Grp: 1; Control: 2; Mnemonic: 'stab'),
                   (NBytes: 1; OpCode: (216, 0); Grp: 1; Control: 2; Mnemonic: 'eorb'),
                   (NBytes: 1; OpCode: (217, 0); Grp: 1; Control: 2; Mnemonic: 'adcb'),
                   (NBytes: 1; OpCode: (218, 0); Grp: 1; Control: 2; Mnemonic: 'orab'),
                   (NBytes: 1; OpCode: (219, 0); Grp: 1; Control: 2; Mnemonic: 'addb'),
                   (NBytes: 1; OpCode: (220, 0); Grp: 1; Control: 2; Mnemonic: 'ldd'),
                   (NBytes: 1; OpCode: (221, 0); Grp: 1; Control: 2; Mnemonic: 'std'),
                   (NBytes: 1; OpCode: (222, 0); Grp: 1; Control: 2; Mnemonic: 'ldx'),
                   (NBytes: 1; OpCode: (223, 0); Grp: 1; Control: 2; Mnemonic: 'stx'),
                   (NBytes: 1; OpCode: (224, 0); Grp: 10; Control: 0; Mnemonic: 'subb'),
                   (NBytes: 1; OpCode: (225, 0); Grp: 10; Control: 0; Mnemonic: 'cmpb'),
                   (NBytes: 1; OpCode: (226, 0); Grp: 10; Control: 0; Mnemonic: 'sbcb'),
                   (NBytes: 1; OpCode: (227, 0); Grp: 10; Control: 0; Mnemonic: 'addd'),
                   (NBytes: 1; OpCode: (228, 0); Grp: 10; Control: 0; Mnemonic: 'andb'),
                   (NBytes: 1; OpCode: (229, 0); Grp: 10; Control: 0; Mnemonic: 'bitb'),
                   (NBytes: 1; OpCode: (230, 0); Grp: 10; Control: 0; Mnemonic: 'ldab'),
                   (NBytes: 1; OpCode: (231, 0); Grp: 10; Control: 0; Mnemonic: 'stab'),
                   (NBytes: 1; OpCode: (232, 0); Grp: 10; Control: 0; Mnemonic: 'eorb'),
                   (NBytes: 1; OpCode: (233, 0); Grp: 10; Control: 0; Mnemonic: 'adcb'),
                   (NBytes: 1; OpCode: (234, 0); Grp: 10; Control: 0; Mnemonic: 'orab'),
                   (NBytes: 1; OpCode: (235, 0); Grp: 10; Control: 0; Mnemonic: 'addb'),
                   (NBytes: 1; OpCode: (236, 0); Grp: 10; Control: 0; Mnemonic: 'ldd'),
                   (NBytes: 1; OpCode: (237, 0); Grp: 10; Control: 0; Mnemonic: 'std'),
                   (NBytes: 1; OpCode: (238, 0); Grp: 10; Control: 0; Mnemonic: 'ldx'),
                   (NBytes: 1; OpCode: (239, 0); Grp: 10; Control: 0; Mnemonic: 'stx'),
                   (NBytes: 1; OpCode: (240, 0); Grp: 2; Control: 2; Mnemonic: 'subb'),
                   (NBytes: 1; OpCode: (241, 0); Grp: 2; Control: 2; Mnemonic: 'cmpb'),
                   (NBytes: 1; OpCode: (242, 0); Grp: 2; Control: 2; Mnemonic: 'sbcb'),
                   (NBytes: 1; OpCode: (243, 0); Grp: 2; Control: 2; Mnemonic: 'addd'),
                   (NBytes: 1; OpCode: (244, 0); Grp: 2; Control: 2; Mnemonic: 'andb'),
                   (NBytes: 1; OpCode: (245, 0); Grp: 2; Control: 2; Mnemonic: 'bitb'),
                   (NBytes: 1; OpCode: (246, 0); Grp: 2; Control: 2; Mnemonic: 'ldab'),
                   (NBytes: 1; OpCode: (247, 0); Grp: 2; Control: 2; Mnemonic: 'stab'),
                   (NBytes: 1; OpCode: (248, 0); Grp: 2; Control: 2; Mnemonic: 'eorb'),
                   (NBytes: 1; OpCode: (249, 0); Grp: 2; Control: 2; Mnemonic: 'adcb'),
                   (NBytes: 1; OpCode: (250, 0); Grp: 2; Control: 2; Mnemonic: 'orab'),
                   (NBytes: 1; OpCode: (251, 0); Grp: 2; Control: 2; Mnemonic: 'addb'),
                   (NBytes: 1; OpCode: (252, 0); Grp: 2; Control: 2; Mnemonic: 'ldd'),
                   (NBytes: 1; OpCode: (253, 0); Grp: 2; Control: 2; Mnemonic: 'std'),
                   (NBytes: 1; OpCode: (254, 0); Grp: 2; Control: 2; Mnemonic: 'ldx'),
                   (NBytes: 1; OpCode: (255, 0); Grp: 2; Control: 2; Mnemonic: 'stx'));


var
   HiMemPagePtr : BytePtr;         { Memory allocated as 2 32k chunks }
   LoMemPagePtr : BytePtr;
   HiMemTypePtr : BytePtr;         { Memory type flag alloc as 2 32k chunks }
   LoMemTypePtr : BytePtr;         { 0=Data(noprint) 1=Data(print) 2=code
                                     3=Indirect Address }

   HpStatus     : Pointer;         { Heap status storage pointer }

   ObjectCodeNdx : array[0..255] of ObjCodeNdxType;    { Index for opcodes }
   CurrentCode : Integer;    { Pointer to opcode in progress }
   OpMemory   : array[1..OpMemorySize] of Byte;  { Buffer for the bytes of opcode }
   CurrentLegal : Boolean;        { T=current opcode is legal, F=illegal }

   NumEntries : integer;                    { number of entries used }
   Entry      : array[1..MaxEntries] of word;    { Entry location storage }
   NumBranches: integer;                    { number of branches used }
   BranchTable: array[1..MaxBranches] of word;   { Branch location storage }

   LoadOffset : Word;   { Program load offset }

   OutputFile : Text;   { File to write disassembly to }
   OutputFilename : String;  { Filename to write disassembly to }
   OutputLine : String;      { File Output Buffer }

   ControlFile : Text;  { File to read control info from }
   ControlFilename : String; { Filename to read control info from }

   PgmFile    : File;   { File to read binary program file from }
   PgmFilename: String; { Filename to read binary program file from }
   Buf        : array[1..BufSize] of Byte;     { Read Buffer }

   SrcFlag,DstFlag : Boolean;     { Must have at least source/dest in control }
   CtrlInput  : String; { Input buffer to read from control }

   NumLbls    : Integer;     { Number of labels on file - Storage in local proc }
   LblPage    : LblTableType;     { Label passing }
   Lbl : array[1..MaxLbls] of LblTableType;   { Storage for Labels }

   NumIndirect : Integer;    { Number of indirect addresses on file }
   Ind : array[1..MaxIndirect] of LblTableType;       { Store as labels }
   IndPage     : LblTableType;    { Indirect passing variable }

   FSize      : Word;        { Binary File Size }
   Wrap       : Boolean;     { Read Wrap around flag }

   PC         : Word;        { Running Program Counter (for scanning) }
   PCOld      : Word;        { Trailing PC counter }

   Addresses  : Boolean;     { Write addresses on disassembly (toggle) }
   DisOpCode  : Boolean;     { Disassemble opcode listing as comment }
                             {  in disassembly file (toggle) }
   DisCodeFlg : Boolean;     { Disassembling flag = True when actually }
                             {  writing code/data to disassembly file  }
                             {  verses just header comments }
   AsciiFlg   : Boolean;     { Disassembling flag, True=convert printable data to .ascii }
   SpitFlg    : Boolean;     { Code-seeking/Code-Dump flag, false=code-seeking, true=spit }

   LAdrDplyCnt : Integer;    { Label address display counter }

   CBDef       : Boolean;    { Condition undeterminable Branch defined flag }



{ Function to read a byte from any memory location }
function ReadByte(Address : Word): Byte;
begin
     If (Address<32768) then
         ReadByte:=LoMemPagePtr^[Address]
     else
         ReadByte:=HiMemPagePtr^[Address-32768];
end;

{ procedure to write a byte to any memory location }
procedure WriteByte(Address : Word; Dta : Byte);
begin
     If (Address<32768) then
         LoMemPagePtr^[Address]:=Dta
     else
         HiMemPagePtr^[Address-32768]:=Dta;
end;

{ Function to read a byte from any memory typing location }
function ReadType(Address : Word): Byte;
begin
     If (Address<32768) then
         ReadType:=LoMemTypePtr^[Address]
     else
         ReadType:=HiMemTypePtr^[Address-32768];
end;

{ procedure to write a byte to any memory typing location }
procedure WriteType(Address : Word; Dta : Byte);
begin
     If (Address<32768) then
         LoMemTypePtr^[Address]:=Dta
     else
         HiMemTypePtr^[Address-32768]:=Dta;
end;

{ function to calc 8-bit offset }
function C8Bit(Ofst : Byte): Integer;
begin
     if Ofst>127 then C8Bit:=-(256-Ofst)
                 else C8Bit:=Ofst;
end;

{ function to calc 16-bit offset }
function C16Bit(Ofst : Word): Word;
begin
     C16Bit:=Ofst;      { mod 65536 wrap around should take care of sign }
end;

{ function to handle labels }
function CheckLbl(LblCmd: CmdType): Boolean;
var
   I   : Integer;
begin
     case LblCmd of
     ReadPage: begin
                 For I:=1 to NumLbls do
                   if LblPage.Addr=Lbl[I].Addr then
                      begin
                        LblPage:=Lbl[I];
                        CheckLbl:=True;
                        exit;
                      end;
                 CheckLbl:=False;
               end;
     WritePage: begin
                  If CheckLbl(ReadPage)=True then
                    begin
                      CheckLbl:=False;
                      exit;
                    end;
                  If NumLbls=MaxLbls then
                    begin
                      CheckLbl:=True;
                      Writeln('    *** Warning: Label buffer full.');
                      LAdrDplyCnt:=0;
                      exit;
                    end;
                  NumLbls:=NumLbls+1;
                  Lbl[NumLbls]:=LblPage;
                  CheckLbl:=True;
                end;
     end;
end;

{ function to handle indirect address }
function CheckInd(LblCmd: CmdType): Boolean;
var
   I   : Integer;
begin
     case LblCmd of
     ReadPage: begin
                 For I:=1 to NumIndirect do
                   if IndPage.Addr=Ind[I].Addr then
                      begin
                        IndPage:=Ind[I];
                        CheckInd:=True;
                        exit;
                      end;
                 CheckInd:=False;
               end;
     WritePage: begin
                  If CheckInd(ReadPage)=True then
                    begin
                      CheckInd:=False;
                      exit;
                    end;
                  If NumIndirect=MaxIndirect then
                    begin
                      CheckInd:=True;
                      Writeln('    *** Warning: Indirect buffer full.');
                      exit;
                    end;
                  NumIndirect:=NumIndirect+1;
                  Ind[NumIndirect]:=IndPage;
                  CheckInd:=True;
                end;
     end;
end;

{ Function to check an address byte to see if it lies with our program }
function CheckAddrByte(CAddr: Word): Boolean;
begin
     if ((((CAddr<LoadOffset) or (CAddr>LoadOffset+FSize-1)) and
               (Wrap=False)) or
          ((CAddr<LoadOffset) and (CAddr>=LoadOffset+FSize) and
              (Wrap=True))) then
          CheckAddrByte:=False else CheckAddrByte:=True;
end;

{ Function to check an address word to see if it lies with our program }
function CheckAddr(CAddr: Word): Boolean;
begin
     CheckAddr:=(CheckAddrByte(CAddr) and CheckAddrByte(CAddr+1));
end;

{ Function to check a branch }
function CheckBranch(BAddr: Word): Boolean;
var
   I : Integer;
begin
     For I:=1 to NumBranches do
       If BranchTable[I]=BAddr then
          begin
            CheckBranch:=True;
            exit;
          end;
     CheckBranch:=False;
end;

{ Procedure to add a branch to branch table }
procedure AddBranch(BAddr: Word);
begin
     If CheckBranch(BAddr)=False then
       begin
         If NumBranches=MaxBranches then
           begin
             Writeln('    *** Warning: Branch buffer full.');
             LAdrDplyCnt:=0;
             exit;
           end;
         NumBranches:=NumBranches+1;
         BranchTable[NumBranches]:=BAddr;
         if CheckAddrByte(BAddr)=False then
           begin
            Writeln;
            Writeln('     *** Warning:  Branch Ref: '+HexDelim,HexString(BAddr,4),
                    ' is outside of Loaded Source File.');
            NumBranches:=NumBranches-1;
            LAdrDplyCnt:=0;
           end;
       end;
end;

{ procedure to generate data label }
procedure GenDataLabel(DLblAddr: Word);
var
   TLbl : Boolean;
begin
     LblPage.Addr:=DLblAddr;
     LblPage.Name:='L'+HexString(LblPage.Addr,4);
     TLbl:=CheckLbl(WritePage);    { Add a 16bit DATA label }
     If TLbl=True then
      begin
        Write(Copy(LblPage.Name+'        ',1,7));
        Inc(LAdrDplyCnt);
        if LAdrDplyCnt>=9 then
          begin
            Writeln;
            LAdrDplyCnt:=0;
          end;
      end;
end;

{ procedure to generate addr label and branch }
procedure GenAddrLabel(ALblAddr: Word);
begin
     GenDataLabel(ALblAddr);         { Add a 16bit ADDR Label }
     AddBranch(LblPage.Addr);        { Add a det. branch }
end;



{ procedure to parse commands from control file }
procedure ParseControl;
var
   I : Integer;
   VString : String;
   CFlg : Boolean;
   TFlg : Boolean;
begin
     VString:=StrSpace;
     If ((VString[1]<>';') and (VString<>'')) then
       begin
         CFlg:=False;
         for I:=1 to NumCommands do
           if ValidCommands[I]=VString then
             begin
               CFlg:=True;
               Case I of
               1: if NumEntries>MaxEntries then
                    begin
                      Writeln; Writeln;
                      Writeln('*** Warning: Too many ENTRY commands.');
                    end
                  else
                    begin
                      NumEntries:=NumEntries+1;
                      Entry[NumEntries]:=Hex2Dec(StrSpace);
                    end;
               2: LoadOffset:=Hex2Dec(StrSpace);
               3: begin
                    PgmFilename:=StrSpace;
                    SrcFlag:=True;
                  end;
               4: begin
                    OutputFilename:=StrSpace;
                    DstFlag:=True;
                  end;
               5: begin
                    LblPage.Addr:=Hex2Dec(StrSpace);
                    LblPage.Name:=StrSpace;
                    If CheckLbl(WritePage)=False then
                       begin
                         Writeln; Writeln;
                         Writeln('*** Warning: Duplicate Label Definition.');
                       end;
                  end;
               6: Addresses:=True;
               7: begin
                    IndPage.Addr:=Hex2Dec(StrSpace);
                    IndPAge.Name:=StrSpace; { Define indirect address store }
                    TFlg:=CheckInd(WritePage);
                  end;
               8: DisOpCode:=True;
               9: AsciiFlg:=True;
               10: SpitFlg:=True;
               end;
             end;
         If CFlg=False then
           begin
             Writeln; Writeln;
             Writeln('*** Warning: Unrecognized command in control file.');
           end;
       end;
end;




{ Procedure to read commands from the control file }
procedure ReadControlFile;
var
   I : Integer;
begin
     SrcFlag:=False;
     DstFlag:=False;
     NumEntries:=0;
     NumBranches:=0;
     NumLbls:=0;
     NumIndirect:=0;
     LoadOffset:=0;
     Addresses:=False;
     DisOpCode:=False;
     AsciiFlg:=False;
     SpitFlg:=False;
     Write('Reading Control File...');
     Assign(ControlFile,ControlFilename);
     Reset(ControlFile);
     If (IOResult<>0) then
       begin
         Writeln; Writeln;
         Writeln('*** Error: Opening Control File: ',ControlFilename);
         Halt;
       end;
     While not EOF(ControlFile) do
       begin
         Readln(ControlFile,CtrlInput);
         CtrlInput:=UpString(CtrlInput);    { Convert to upper case }
         ParseControl;
       end;
     close(ControlFile);

     If ((SrcFlag=False) or (DstFlag=False)) then
       begin
         Writeln; Writeln;
         Writeln('*** Error: Input and Output files MUST be specified in control file.');
         Halt;
       end;
     If ((NumEntries=0) and (NumIndirect=0) and (not SpitFlg)) then
       begin
         Entry[1]:=LoadOffset;
         NumEntries:=1;
       end;
     Writeln;
     Writeln('        Load Address: '+HexDelim,HexString(LoadOffset,4));
     Writeln('        ',NumEntries,' Entry Points: ');
     For I:=1 to NumEntries do
       Writeln('                '+HexDelim,HexString(Entry[I],4));
     Writeln('        Source File: ',PgmFilename);
     Writeln('        Destination File: ',OutputFilename);
     Writeln('        ',NumLbls,' Labels Defined:');
     For I:=1 to NumLbls do
       Writeln('                '+HexDelim,HexString(Lbl[I].Addr,4),'=',Lbl[I].Name,LblDelim);
     If Addresses then Writeln('Writing program counter addresses to disassembly file.');
end;

{ procedure to read binary file }
procedure ReadSource;
var
   NumRead: Word;
   AddrCnt: Word;
   I      : Word;
begin
     For I:=$0000 to $FFFF do
      begin
       WriteByte(I,0);       { Place 0 data byte }
       if (SpitFlg) then
          WriteType(I, 2)       { If "spitting" assume all is code }
       else
          WriteType(I,0);       { First assume everything is data }
      end;

     Write('Reading Source File...');
     AddrCnt:=LoadOffset;
     Wrap:=False;
     FSize:=0;
     Assign(PgmFile,PgmFilename);
     Reset(PgmFile,1);
     If (IOResult<>0) then
       begin
         Writeln; Writeln;
         Writeln('*** Error: Opening Source File: ',PgmFilename);
         Halt;
       end;
     repeat
       BlockRead(PgmFile,Buf,SizeOf(Buf),NumRead);
       if NumRead>0 then
         begin
           If Wrap then
             begin
               Writeln; Writeln;
               Writeln('*** Warning: Memory wrap around encountered, Check Load Offset/File Length.');
             end;
           For I:=1 to NumRead do
             begin
               WriteByte(AddrCnt,Buf[I]);
               AddrCnt:=AddrCnt+1;
               FSize:=FSize+1;
               If AddrCnt=0 then Wrap:=True;
             end;
         end;
     until (NumRead=0);
     Close(PgmFile);
     Writeln('File Size: '+HexDelim,HexString(FSize,4));
     For I:=1 to NumEntries do
       if CheckAddrByte(Entry[I])=False then
         Writeln('     *** Warning:  Entry Point: '+HexDelim,HexString(Entry[I],4),
                 ' is outside of Loaded Source File.');
end;

{ procedure to create address/branch labels from indirect specifiers
in control file (which are now loaded into Ind array) }
{ uPU dependent! -- Indian dependent }
{ Updated for 6811 }
procedure CompileIndirect;
var
   I : Integer;
begin
     Writeln('Compiling Indirect Branch Table as specified in Control File...');
     I:=1;
     while (I<=NumIndirect) do
       begin
        if (CheckAddr(Ind[I].Addr)=True) then
        begin
         WriteType(Ind[I].Addr,3);     { Mark memory as indirect vector }
         WriteType(Ind[I].Addr+1,3);
         LblPage.Name:=Ind[I].Name;
         LblPage.Addr:=(ReadByte(Ind[I].Addr)*256+ReadByte(Ind[I].Addr+1));
         If CheckLbl(WritePage) then
            begin
              Writeln('   ['+HexDelim,HexString(Ind[I].Addr,4),'] -> '+HexDelim,
                              HexString(LblPage.Addr,4),' = ',
                              LblPage.Name);
              AddBranch(LblPage.Addr);      { Make it a branch also }
            end;
        end;
        I:=I+1;
       end;
end;

{ Procedure to finish reading opcode from memory.  This procedure reads
operand bytes, placing them in OpMemory.  Plus branch/labels and data/labels
are generated (according to function).
Note: This routine IS uPU dependent! -- Indian and Group/Control dependent }
{ Updated for 6811 }
procedure CompleteRead;
var
   I: Integer;
   TAddr : Word;
   OpPointer: Word;
begin
     CBDef:=True;
     For I:=OpMaxSize+1 to OpMemorySize do
       OpMemory[I]:=ReadByte(PC+I-OpMaxSize-1);     { Xfer to buffer }
{ Handle Destination Argument first: }
     Case (ObjectCode[CurrentCode].Grp and $F0) of
     $10,$30,$50,
     $A0,$B0: begin
                WriteType(PC,2);
                PC:=PC+1;
              end;
     $20,$40,$60,
     $70,$80,$90: begin
                    WriteType(PC,2); WriteType(PC+1,2);
                    PC:=PC+2;
                  end;
     end;
{ Handle Source Argument Last: }
     Case (ObjectCode[CurrentCode].Grp and $0F) of
     1,3,5,
     10,11: begin
              WriteType(PC,2);
              PC:=PC+1;
            end;
     2,4,6,
     7,8,9: begin
              WriteType(PC,2); WriteType(PC+1,2);
              PC:=PC+2;
            end;
     end;

     OpPointer:=OpMaxSize+1;

{ Handle Destination Argument First: }
     Case (ObjectCode[CurrentCode].Grp and $F0) of
     $10: begin                        { absolute 8-bit, assume msb=$00 }
            Case (ObjectCode[CurrentCode].Control and $F0) of
            $20,$30: GenDataLabel(OpMemory[OpPointer]);
            $60,$70: GenAddrLabel(OpMemory[OpPointer]);
            end;
            OpPointer:=OpPointer+1;
          end;
     $20: begin                        { 16-bit Absolute }
            Case (ObjectCode[CurrentCode].Control and $F0) of
            $20,$30: GenDataLabel(OpMemory[OpPointer]*256+OpMemory[OpPointer+1]);
            $60,$70: GenAddrLabel(OpMemory[OpPointer]*256+OpMemory[OpPointer+1]);
            end;
            OpPointer:=OpPointer+2;
          end;
     $30: begin                        { 8-Bit Rel }
            Case (ObjectCode[CurrentCode].Control and $F0) of
            $20,$30: GenDataLabel(PC+C8Bit(OpMemory[OpPointer]));
            $60,$70: GenAddrLabel(PC+C8Bit(OpMemory[OpPointer]));
            end;
            OpPointer:=OpPointer+1;
          end;
     $40: begin                        { 16-Bit Rel }
            Case (ObjectCode[CurrentCode].Control and $F0) of
            $20,$30: GenDataLabel(PC+C16Bit(OpMemory[OpPointer]*256+
                                      OpMemory[OpPointer+1]));
            $60,$70: GenAddrLabel(PC+C16Bit(OpMemory[OpPointer]*256+
                                      OpMemory[OpPointer+1]));
            end;
            OpPointer:=OpPointer+2;
          end;
     $50: OpPointer:=OpPointer+1;      { Immediate 8-bit data = no label }
     $60: OpPointer:=OpPointer+2;      { Immediate 16-bit data = no label }
     $70: begin                        { 8-bit Absolute address, and mask }
            Case (ObjectCode[CurrentCode].Control and $F0) of
            $20,$30: GenDataLabel(OpMemory[OpPointer]);
            $60,$70: GenAddrLabel(OpMemory[OpPointer]);
            end;
            OpPointer:=OpPointer+2;
          end;
     $80,$90: OpPointer:=OpPointer+2;      { 8-bit offset and 8-bit mask }
     $A0,$B0: OpPointer:=OpPointer+1;      { 8-bit offset }
     end;

{ Handle Source Argument Last: }
     Case (ObjectCode[CurrentCode].Grp and $0F) of
     1: begin                          { absolute 8-bit, assume msb=$00 }
            Case (ObjectCode[CurrentCode].Control and $0F) of
            2,3: GenDataLabel(OpMemory[OpPointer]);
            6,7: GenAddrLabel(OpMemory[OpPointer]);
            end;
            OpPointer:=OpPointer+1;
          end;
     2: begin                          { 16-bit Absolute }
            Case (ObjectCode[CurrentCode].Control and $0F) of
            2,3: GenDataLabel(OpMemory[OpPointer]*256+OpMemory[OpPointer+1]);
            6,7: GenAddrLabel(OpMemory[OpPointer]*256+OpMemory[OpPointer+1]);
            end;
            OpPointer:=OpPointer+2;
          end;
     3: begin                          { 8-Bit Rel }
            Case (ObjectCode[CurrentCode].Control and $0F) of
            2,3: GenDataLabel(PC+C8Bit(OpMemory[OpPointer]));
            6,7: GenAddrLabel(PC+C8Bit(OpMemory[OpPointer]));
            end;
            OpPointer:=OpPointer+1;
          end;
     4: begin                           { 16-Bit Rel }
            Case (ObjectCode[CurrentCode].Control and $0F) of
            2,3: GenDataLabel(PC+C16Bit(OpMemory[OpPointer]*256+
                                      OpMemory[OpPointer+1]));
            6,7: GenAddrLabel(PC+C16Bit(OpMemory[OpPointer]*256+
                                      OpMemory[OpPointer+1]));
            end;
            OpPointer:=OpPointer+2;
          end;
     5: OpPointer:=OpPointer+1;        { Immediate 8-bit data = no label }
     6: OpPointer:=OpPointer+2;        { Immediate 16-bit data = no label }
     7: begin                          { 8-bit Absolute address, and mask }
            Case (ObjectCode[CurrentCode].Control and $0F) of
            2,3: GenDataLabel(OpMemory[OpPointer]);
            6,7: GenAddrLabel(OpMemory[OpPointer]);
            end;
            OpPointer:=OpPointer+2;
          end;
     8,9: OpPointer:=OpPointer+2;        { 8-bit offset and 8-bit mask }
     10,11: OpPointer:=OpPointer+1;      { 8-bit offset }
     end;
end;

{ procedure to read next object code from memory.  The memory type is
flagged as code, unless invalid object code is encountered.
CurrentLegal = Status of object code.  OpMemory = object code from memory
CurrentCode = Point into ObjectCode array for current object code
PC is automatically advanced
Note: This procedure is processor independent! }
procedure ReadNextObj;
var
   I : Integer;
   J : Integer;
   TFlg : Boolean;
   SFlg : Boolean;
begin
     CurrentLegal:=ObjectCodeNdx[ReadByte(PC)].Legal;
     I:=ObjectCodeNdx[ReadByte(PC)].Start;
     If ((CurrentLegal=True) and
         (ObjectCode[I].NBytes<>1)) then
       begin
         TFlg:=False;
         while ((I<=NumOpcodes) and (TFlg=False) and (CurrentLegal=True)) do
           begin
             SFlg:=False;
             J:=1;
             While ((J<=ObjectCode[I].NBytes) and (SFlg=False)) do
               begin
                 If ObjectCode[I].OpCode[J]<>ReadByte(PC+J-1) then
                   begin
                     SFlg:=True;
                     If J=1 then CurrentLegal:=False;
                   end;
                 J:=J+1;
               end;
             If SFlg=False then TFlg:=True else I:=I+1;
           end;
       end;

{ At end of test, I=pointer to ObjectCode array, CurrentLegal=Valid Status }

     CurrentCode:=I;

     If CurrentLegal=False then
       begin
         OpMemory[1]:=ReadByte(PC);
         if (SpitFlg) then
            WriteType(PC,2)
         else
            WriteType(PC,0);    { If not valid opcode, flag as data, error }
         PC:=PC+1;
         CurrentCode:=0;
         exit;
       end;

     For J:=1 to ObjectCode[I].NBytes do
       begin
         OpMemory[J]:=ReadByte(PC);
         WriteType(PC,2);         { Flag byte as code }
         PC:=PC+1;
       end;

     CompleteRead;      { Call uPU dependent routine to complete opcode read }
end;

{ Procedure to read code from PC until already-encountered-code is
    re-encountered or until terminating branch jump algorithm control
    code is encountered.
Note: The procedure is uPU independent! }
procedure FindCode;
var
   TFlg : Boolean;
begin
     TFlg:=False;
     If ((ReadType(PC)=2) and (not SpitFlg)) then TFlg:=True;     { Terminate if we run into code }
     While TFlg=False do
     begin
       ReadNextObj;     { Read next object code from memory, tagging bytes }
       If ((not SpitFlg) and ((ObjectCode[CurrentCode].Control mod 2)=1)) then TFlg:=True;
                        { Terminate if control code is odd = discontinue }
       If ReadType(PC)=2 then TFlg:=True;   { Terminate if we run into code }
       If CheckAddrByte(PC)=False then TFlg:=True;    { Terminate if outside }
     end;
end;

{ Procedure to scan for code, starting with entries }
procedure ScanEntries;
var
   ECnt  : Integer;
begin
     ECnt:=1;
     while ECnt<=NumEntries do
       begin
         PC:=Entry[ECnt];    { Start scanning at entry }
         FindCode;
         ECnt:=ECnt+1;
       end;
end;

{ Procedure to scan for code, starting with branch addresses }
procedure ScanBranches;
var
   BCnt  : Integer;
begin
     BCnt:=1;
     while BCnt<=NumBranches do
       begin
         PC:=BranchTable[BCnt];   { Start scanning at branch }
         FindCode;
         BCnt:=BCnt+1;
       end;
end;

{ Procedure to scan for data, throughout defined program space }
procedure ScanData;
begin
     PC:=LoadOffset;
     Repeat
       if ReadType(PC)=0 then
         if (AsciiFlg and
             (ReadByte(PC)>=PrintAbleLo) and
             (ReadByte(PC)<=PrintAbleHi) and
             (ReadByte(PC)<>DtaDelim) and
             (ReadByte(PC)<>ord('\'))) then WriteType(PC,1);
       PC:=PC+1;
     until PC=LoadOffset+FSize;
end;

{ Procedure to send a line to the output file }
procedure SendToOutput;
begin
     If Addresses then
        OutputLine:=HexString(PCOld,4)+Tab+OutputLine;
     Writeln(OutputFile,OutputLine);
     If IOResult<>0 then
       begin
         Writeln;
         Writeln('*** Error Writing Output File: ',OutputFilename);
         Close(OutputFile);
         Halt;
       end;
end;

{ Procedure to write a line to the output file }
procedure WriteLine;
var
   I : Word;
   BCnt : Integer;
   Temp : String;
begin
     Temp:=OutputLine;
     If ((DisCodeFlg=True) and
         ((DisOpCode=True) or (ReadType(PCOld)=1)) and
         (ReadType(PCOld)<>0)) then
       begin
         BCnt:=0;
         For I:=PCOld to PC-1 do
           begin
             If CheckAddrByte(I)=True then
               If BCnt=0 then
                 OutputLine:=ComDelim+' '+HexString(I,4)+': '+HexString(ReadByte(I),2)
                else
                 OutputLine:=OutputLine+','+HexString(ReadByte(I),2);
             Inc(BCnt);
             If BCnt=MaxNonPrint then
               begin
                 SendToOutput;
                 BCnt:=0;
               end;
           end;
         If BCnt<>0 then SendToOutput;
       end;
     OutputLine:=Temp;
     SendToOutput;
     PCOld:=PC;
end;

{ Procedure to Disassemble object code into output file
Note: This procedure IS uPU dependent }
{ ASSEMBLER dependent! }
procedure Disassemble;
var
   I,J : Word;
   Ct  : Byte;
   Mt  : Boolean;
   Rn  : Byte;
   Cn  : ShortInt;
   Stemp : String;
   OpPointer : Word;
begin
     PC:=0;
     PCOld:=0;
     DisCodeFlg:=False;
     Assign(OutputFile,OutputFilename);
     Rewrite(OutputFile);
     If IOResult<>0 then
       begin
         Writeln;
         Writeln('*** Error Opening Output File: ',OutputFilename);
         Halt;
       end;

{ Write Header }
     OutputLine:=ComDelim; WriteLine;
     OutputLine:=ComDelim+'  M6811 Disassembler Generated Source Code'; Writeline;
     OutputLine:=ComDelim; WriteLine;
     OutputLine:=ComDelim+'  For User Control File: '+ControlFilename; WriteLine;
     OutputLine:=ComDelim+'           Program File: '+PgmFilename; WriteLine;
     OutputLine:=ComDelim+'  Disassembly into File: '+OutputFilename; WriteLine;
     OutputLine:=ComDelim; WriteLine;
     OutputLine:=''; WriteLine;
     OutputLine:=''; WriteLine;

     For PC:=$0000 to $FFFF do
       If (CheckAddrByte(PC)=False) then
         begin
           LblPage.Addr:=PC;
           If (CheckLbl(ReadPage)=True) then
             begin
               OutputLine:=LblPage.Name+LbleDelim+Tab+'='+Tab+
                           HexDelim+HexString(LblPage.Addr,4);
               WriteLine;
             end;
         end;

     PC:=LoadOffset;
     OutputLine:=''; WriteLine;
     OutputLine:=Tab+'.area'+Tab+'CODE1'+Tab+'(ABS)';
     WriteLine;
     OutputLine:=Tab+'.org'+Tab+HexDelim+HexString(LoadOffset,4);
     WriteLine;
     OutputLine:=''; WriteLine;

     DisCodeFlg:=True;
     While CheckAddrByte(PC)=True do
       begin
         OutputLine:='';
         CBDef:=True;
         Case ReadType(PC) of
         0: begin
              LblPage.Addr:=PC;
              If CheckLbl(ReadPage)=True then
                 OutputLine:=OutputLine+LblPage.Name+LblDelim+Tab
                else
                 OutputLine:=OutputLine+Tab;
              OutputLine:=OutputLine+'.byte'+Tab+HexDelim+HexString(ReadByte(PC),2);
              PC:=PC+1; J:=1;
              LblPage.Addr:=PC;
              While ((ReadType(PC)=0) and (J<MaxNonPrint) and
                     (CheckLbl(ReadPage)=False)) do
                begin
                  OutputLine:=OutputLine+','+HexDelim+HexString(ReadByte(PC),2);
                  PC:=PC+1; J:=J+1;
                  LblPage.Addr:=PC;
                end;
            end;
         1: begin
              LblPage.Addr:=PC;
              If CheckLbl(ReadPage)=True then
                 OutputLine:=OutputLine+LblPage.Name+LblDelim+Tab
                else
                 OutputLine:=OutputLine+Tab;
              OutputLine:=OutputLine+'.ascii'+Tab+chr(DtaDelim)+chr(ReadByte(PC));
              PC:=PC+1; J:=1;
              LblPage.Addr:=PC;
              While ((ReadType(PC)=1) and (J<MaxPrint) and
                     (CheckLbl(ReadPage)=False)) do
                begin
                  OutputLine:=OutputLine+chr(ReadByte(PC));
                  PC:=PC+1; J:=J+1;
                  LblPage.Addr:=PC;
                end;
              OutputLine:=OutputLine+chr(DtaDelim);
            end;
         2: begin
              LblPage.Addr:=PC;
              If CheckLbl(ReadPage)=True then
                 OutputLine:=OutputLine+LblPage.Name+LblDelim+Tab
                else
                 OutputLine:=OutputLine+Tab;
              ReadNextObj;        { Get object code }
              OutputLine:=OutputLine+ObjectCode[CurrentCode].Mnemonic+Tab;
              if (not CurrentLegal) then OutputLine:=OutputLine+'0x'+HexString(OpMemory[1],2);
              OpPointer:=OpMaxSize+1;
{ Handle Destination }
              Case (ObjectCode[CurrentCode].Grp and $F0) of
              $10: begin
                     OutputLine:=OutputLine+'*';
                     LblPage.Addr:=OpMemory[OpPointer];
                     If CheckLbl(ReadPage)=True then
                       OutputLine:=OutputLine+LblPage.Name
                     else
                       OutputLine:=OutputLine+HexDelim+
                           HexString(OpMemory[OpPointer],2);
                     OpPointer:=OpPointer+1;
                     OutputLine:=OutputLine+',';
                   end;
              $20: begin
                     LblPage.Addr:=OpMemory[OpPointer]*256+OpMemory[OpPointer+1];
                     If CheckLbl(ReadPage)=True then
                       OutputLine:=OutputLine+LblPage.Name
                     else
                       OutputLine:=OutputLine+HexDelim+HexString(LblPage.Addr,4);
                     OpPointer:=OpPointer+2;
                     OutputLine:=OutputLine+',';
                   end;
              $30: begin
                     LblPage.Addr:=C8Bit(OpMemory[OpPointer])+PC;
                     If CheckLbl(ReadPage)=True then
                       OutputLine:=OutputLine+LblPage.Name
                      else
                       OutputLine:=OutputLine+HexDelim+HexString(LblPage.Addr,4);
                     OpPointer:=OpPointer+1;
                     OutputLine:=OutputLine+',';
                   end;
              $40: begin
                     LblPage.Addr:=C16Bit(OpMemory[OpPointer]*256+OpMemory[OpPointer+1])+PC;
                     If CheckLbl(ReadPage)=True then
                        OutputLine:=OutputLine+LblPage.Name
                      else
                        OutputLine:=OutputLine+HexDelim+HexString(LblPage.Addr,4);
                     OpPointer:=OpPointer+2;
                     OutputLine:=OutputLine+',';
                   end;
              $50: begin
                     OutputLine:=OutputLine+'#'+HexDelim+
                                    HexString(OpMemory[OpPointer],2);
                     OpPointer:=OpPointer+1;
                     OutputLine:=OutputLine+',';
                   end;
              $60: begin
                     OutputLine:=OutputLine+'#'+HexDelim+
                                    HexString(OpMemory[OpPointer]*256+
                                              OpMemory[OpPointer+1],4);
                     OpPointer:=OpPointer+2;
                     OutputLine:=OutputLine+',';
                   end;
              $70: begin
                     OutputLine:=OutputLine+'*';
                     LblPage.Addr:=OpMemory[OpPointer];
                     If CheckLbl(ReadPage)=True then
                       OutputLine:=OutputLine+LblPage.Name
                     else
                       OutputLine:=OutputLine+HexDelim+
                           HexString(OpMemory[OpPointer],2);
                     OpPointer:=OpPointer+1;
                     OutputLine:=OutputLine+',';
                     OutputLine:=OutputLine+'#'+HexDelim+
                                    HexString(OpMemory[OpPointer],2);
                     OpPointer:=OpPointer+1;
                     OutputLine:=OutputLine+',';
                   end;
              $80: begin
                     OutputLine:=OutputLine+HexDelim+
                                    HexString(OpMemory[OpPointer],2);
                     OpPointer:=OpPointer+1;
                     OutputLine:=OutputLine+',x,';
                     OutputLine:=OutputLine+'#'+HexDelim+
                                    HexString(OpMemory[OpPointer],2);
                     OpPointer:=OpPointer+1;
                     OutputLine:=OutputLine+',';
                   end;
              $90: begin
                     OutputLine:=OutputLine+HexDelim+
                                    HexString(OpMemory[OpPointer],2);
                     OpPointer:=OpPointer+1;
                     OutputLine:=OutputLine+',y,';
                     OutputLine:=OutputLine+'#'+HexDelim+
                                    HexString(OpMemory[OpPointer],2);
                     OpPointer:=OpPointer+1;
                     OutputLine:=OutputLine+',';
                   end;
              $A0: begin
                     OutputLine:=OutputLine+HexDelim+
                                    HexString(OpMemory[OpPointer],2);
                     OpPointer:=OpPointer+1;
                     OutputLine:=OutputLine+',x,';
                   end;
              $B0: begin
                     OutputLine:=OutputLine+HexDelim+
                                    HexString(OpMemory[OpPointer],2);
                     OpPointer:=OpPointer+1;
                     OutputLine:=OutputLine+',y,';
                   end;
              end;
{ Handle Source }
              Case (ObjectCode[CurrentCode].Grp and $0F) of
              1: begin
                   OutputLine:=OutputLine+'*';
                   LblPage.Addr:=OpMemory[OpPointer];
                   If CheckLbl(ReadPage)=True then
                     OutputLine:=OutputLine+LblPage.Name
                   else
                     OutputLine:=OutputLine+HexDelim+
                         HexString(OpMemory[OpPointer],2);
                   OpPointer:=OpPointer+1;
                 end;
              2: begin
                   LblPage.Addr:=OpMemory[OpPointer]*256+OpMemory[OpPointer+1];
                   If CheckLbl(ReadPage)=True then
                     OutputLine:=OutputLine+LblPage.Name
                   else
                     OutputLine:=OutputLine+HexDelim+HexString(LblPage.Addr,4);
                   OpPointer:=OpPointer+2;
                 end;
              3: begin
                   LblPage.Addr:=C8Bit(OpMemory[OpPointer])+PC;
                   If CheckLbl(ReadPage)=True then
                     OutputLine:=OutputLine+LblPage.Name
                   else
                     OutputLine:=OutputLine+HexDelim+HexString(LblPage.Addr,4);
                   OpPointer:=OpPointer+1;
                 end;
              4: begin
                   LblPage.Addr:=C16Bit(OpMemory[OpPointer]*256+OpMemory[OpPointer+1])+PC;
                   If CheckLbl(ReadPage)=True then
                     OutputLine:=OutputLine+LblPage.Name
                   else
                     OutputLine:=OutputLine+HexDelim+HexString(LblPage.Addr,4);
                   OpPointer:=OpPointer+2;
                 end;
              5: begin
                   OutputLine:=OutputLine+'#'+HexDelim+
                                  HexString(OpMemory[OpPointer],2);
                   OpPointer:=OpPointer+1;
                 end;
              6: begin
                   OutputLine:=OutputLine+'#'+HexDelim+
                                  HexString(OpMemory[OpPointer]*256+
                                            OpMemory[OpPointer+1],4);
                   OpPointer:=OpPointer+2;
                 end;
              7: begin
                   OutputLine:=OutputLine+'*';
                   LblPage.Addr:=OpMemory[OpPointer];
                   If CheckLbl(ReadPage)=True then
                     OutputLine:=OutputLine+LblPage.Name
                   else
                     OutputLine:=OutputLine+HexDelim+
                         HexString(OpMemory[OpPointer],2);
                   OpPointer:=OpPointer+1;
                   OutputLine:=OutputLine+',';
                   OutputLine:=OutputLine+'#'+HexDelim+
                                  HexString(OpMemory[OpPointer],2);
                   OpPointer:=OpPointer+1;
                 end;
              8: begin
                   OutputLine:=OutputLine+HexDelim+
                                  HexString(OpMemory[OpPointer],2);
                   OpPointer:=OpPointer+1;
                   OutputLine:=OutputLine+',x,';
                   OutputLine:=OutputLine+'#'+HexDelim+
                                  HexString(OpMemory[OpPointer],2);
                   OpPointer:=OpPointer+1;
                 end;
              9: begin
                   OutputLine:=OutputLine+HexDelim+
                                  HexString(OpMemory[OpPointer],2);
                   OpPointer:=OpPointer+1;
                   OutputLine:=OutputLine+',y,';
                   OutputLine:=OutputLine+'#'+HexDelim+
                                  HexString(OpMemory[OpPointer],2);
                   OpPointer:=OpPointer+1;
                 end;
              10: begin
                     OutputLine:=OutputLine+HexDelim+
                                    HexString(OpMemory[OpPointer],2);
                     OpPointer:=OpPointer+1;
                     OutputLine:=OutputLine+',x';
                   end;
              11: begin
                     OutputLine:=OutputLine+HexDelim+
                                    HexString(OpMemory[OpPointer],2);
                     OpPointer:=OpPointer+1;
                     OutputLine:=OutputLine+',y';
                   end;
              end;

              Case (ObjectCode[CurrentCode].Control and $F0) of
              $40,$50: OutputLine:=OutputLine+Tab+Tab+
                        ComDelim+' Undetermined Branch Address';
              $80,$90: If CBDef=False then
                        OutputLine:=OutputLine+Tab+Tab+
                          ComDelim+' Undetermined Branch Address';
              end;
              Case (ObjectCode[CurrentCode].Control and $0F) of
              4,5: OutputLine:=OutputLine+Tab+Tab+
                    ComDelim+' Undetermined Branch Address';
              8,9: If CBDef=False then
                    OutputLine:=OutputLine+Tab+Tab+
                      ComDelim+' Undetermined Branch Address';
              end;
            end;
         3: begin
              LblPage.Addr:=PC;
              If CheckLbl(ReadPage)=True then
                 OutputLine:=OutputLine+LblPage.Name+LblDelim+Tab
                else
                 OutputLine:=OutputLine+Tab;
              LblPage.Addr:=ReadByte(PC)*256+ReadByte(PC+1);
              PC:=PC+2;
              If CheckLbl(ReadPage)=True then
                 OutputLine:=OutputLine+'.word'+TAB+LblPage.Name
                else
                 OutputLine:=OutputLine+',word'+TAB+HexDelim+
                   HexString(LblPage.Addr,4);
            end;
         end;
         WriteLine;
       end;

     DisCodeFlg:=False;
     OutputLine:=ComDelim+Tab+'.end';        { Write end of file }
     Writeline;
     Close(OutputFile);
     Writeln;
     Writeln('Disassembly Complete');
end;

{ Initialization procedure }
procedure Init;
begin
     Write('Initializing...');
     Mark(HpStatus);         { Get the heap state to free it later }
     If MaxAvail < SizeOf(MemoryDataType) then
       begin
         Writeln; Writeln;
         Writeln('Not enough memory');
         Halt;
       end
      else
       New(LoMemPagePtr);
     If MaxAvail < SizeOf(MemoryDataType) then
       begin
         Writeln; Writeln;
         Writeln('Not enough memory');
         Halt;
       end
      else
       New(HiMemPagePtr);
     If MaxAvail < SizeOf(MemoryDataType) then
       begin
         Writeln; Writeln;
         Writeln('Not enough memory');
         Halt;
       end
      else
       New(LoMemTypePtr);
     If MaxAvail < SizeOf(MemoryDataType) then
       begin
         Writeln; Writeln;
         Writeln('Not enough memory');
         Halt;
       end
      else
       New(HiMemTypePtr);

     LAdrDplyCnt:=0;
     Writeln;
end;

{ Deinitialization procedure }
procedure Deinit;
begin
     Release(HpStatus);      { Return the heap to original conditions }
end;

begin
     Writeln('M6811 Disassembler V1.1');
     Writeln('Copyright(c)1996,1997 by Donald Whisnant');
     Writeln;
     If ParamCount<1 then
       begin
         Writeln('Usage: M6811DIS <control filename>');
         Halt;
       end;
     Init;
     ControlFilename:=UpString(ParamStr(1));
     ReadControlFile;
     ReadOpcodes;
     ReadSource;
     CompileIndirect;
     Writeln;
     Writeln('Pass 1 - Finding Code, Data, and Labels...');
     ScanEntries;
     ScanBranches;
     ScanData;
     Writeln;
     Writeln('Pass 2 - Disassembling to Output File...');
     Disassemble;
     Deinit;
end.
