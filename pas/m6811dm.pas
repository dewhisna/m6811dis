{

      6811 Disassembler V1.1

Copyright(c)1996 by Donald Whisnant

For use on IBM PC Compatible
Written in Turbo Pascal V7.0

Version 1.1: This version has been extended to handle multiple input
                files by having it read the input files as they are
                specified in the control file.  HOWEVER, this means
                that the control files MUST BE MODIFIED to have the
                "load" address specified BEFORE each of the specified
                input files.  This way, unique load addresses can
                be specified for each file.
             Also, a couple of "hidden bugs" were fixed.

}

program M6811DM;
uses Dos;

{$I-}
{$M 16384 131072 655360}

{
   Object Code File Format:

       The object code data file is a text file where each line in the
   text file denotes one opcode entry.  Each line has the following
   syntax.  End the file with an entry where nn=0.

      nn op1 op2 ... opn grp control mnemonic

   Where:
      nn       = number of bytes for opcode
      op1..opn = opcode data bytes
      grp      = opcode operator groups
      control  = algorithm control code
      mnemonic = Opcode Assembly Mnemonic

    #Bytes: (nn)
         0 = END OF DATA FILE
         1 = 1 byte opcode
         2 = 2 byte opcode
         etc.  (Range limited in program.  See const: MaxOpcodeBytes)

    Opcodes: (op1..opn)
         valid range = 0-255 (byte)

    Groups: (Grp) : xy
      Where x (msnb = destination = FIRST operand)
            y (lsnb = source = LAST operand)
         0 = opcode only, nothing follows
         1 = 8-bit absolute address follows only
         2 = 16-bit absolute address follows only
         3 = 8-bit relative address follows only
         4 = 16-bit relative address follows only
         5 = 8-bit data follows only
         6 = 16-bit data follows only
         7 = 8-bit absolute address followed by 8-bit mask
         8 = 8-bit X offset address followed by 8-bit mask
         9 = 8-bit Y offset address followed by 8-bit mask
         A = 8-bit X offset address
         B = 9-bit Y offset address

    Control: (Algorithm control) : xy
      Where x (msnb = destination = FIRST operand)
            y (lsnb = source = LAST operand)
         0 = Disassemble as code, and continue disassembly
         1 = Disassemble as code, but discontinue disassembly
               (i.e. finished with routine. ex: RTS)
         2 = Disassemble as code, with data addr label,
                and continue disassembly (on opcodes with post-byte,
                label generation is dependent upon post-byte value.)
         3 = Disassemble as code, with data addr label,
                but discontinue disassembly (on opcodes with post-byte,
                label generation is dependent upon post-byte value.)
         4 = Disassemble as undeterminable branch address (Comment code),
               and continue disassembly
         5 = Disassemble as undeterminable branch address (Comment code),
               but discontinue disassembly (i.e. finished with routine, ex:JMP)
         6 = Disassemble as determinable branch, add branch addr and label,
               and continue disassembly
         7 = Disassemble as determinable branch, add branch addr and label,
               but discontinue disassembly (i.e. finished with routine, ex:JMP)
         8 = Disassemble as conditionally undeterminable branch address (Comment code),
               and continue disassembly, with data addr label,
               (on opcodes with post-byte, label generation is dependent
               upon post-byte value. conditional upon post-byte)
         9 = Disassemble as conditionally undeterminable branch address (Comment code),
               but discontinue disassembly (i.e. finished with routine,
               ex:JMP), with data addr label
               (on opcodes with post-byte, label generation is dependent
               upon post-byte value. conditional upon post-byte)

    Mnemonic:
         Any valid string
}

const
     DtaDelim   = 39;        { Specify '' as delimiter for data literals }
     LblDelim   = ':';       { Delimiter between labels and code }
     LbleDelim  = '';        { Delimiter between labels and equates }
     ComDelim   = ';';       { Comment delimiter }
     HexDelim   = '0x';      { Hex. delimiter }
     MaxOpcodes = 500;       { Allow up to 500 opcodes }
     MaxEntries = 64;        { Allow up to 64 designated entries }
     MaxBranches = 4096;     { Allow up to 4k branch addresses }
     MaxLbls     = 4096;     { Allow up to 4k labels }
     MaxIndirect = 512;      { Allow up to 1/2k indirect entries }
     BufSize     = 2048;     { Allow a 2k program read buffer for speed }
     MnemSize    = 7;        { Allow up to 7 chars for Mnemonics }
     LBLSize     = 6;        { Allow up to 6 chars for labels }
     OpcodesFilename = 'M6811DIS.OP';       { Filename of opcodes file }
     MaxOpcodeBytes = 2;     { Allow a maximum of 2 object bytes per opcode }
     NumCommands = 9;
     ValidCommands : array[1..NumCommands] of string[10]
                   = ('ENTRY',
                      'LOAD',
                      'INPUT',
                      'OUTPUT',
                      'LABEL',
                      'ADDRESSES',
                      'INDIRECT',
                      'OPCODES',
                      'ASCII');
     HexValue  : array[0..15] of string[1]
               = ('0','1','2','3','4','5','6','7','8','9',
                  'A','B','C','D','E','F');
     PrintAbleLo = 32;       { Define the highest and lowest ASCII }
     PrintAbleHi = 126;      {   characters that are printable }
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
      Mnemonic  :  String[MnemSize];   {   for single byte opcodes }
      Grp       :  Integer;     { Store name and group declaration }
      NBytes    :  Integer;     {   and # of bytes for opcode }
      OpCode    :  array[1..MaxOpcodeBytes] of Byte;  { Storage for opcode }
      Control   :  Integer;     { Storage for the control code }
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

var
   HiMemPagePtr : BytePtr;         { Memory allocated as 2 32k chunks }
   LoMemPagePtr : BytePtr;
   HiMemTypePtr : BytePtr;         { Memory type flag alloc as 2 32k chunks }
   LoMemTypePtr : BytePtr;         { 0=Data(noprint) 1=Data(print) 2=code
                                     3=Indirect Address }

   HpStatus     : Pointer;         { Heap status storage pointer }

   NumOpcodes : Integer;           { Number of opcodes used }
   ObjectCode : array[0..MaxOpcodes] of ObjCodesType;  { Storage for opcode data }
   ObjectCodeNdx : array[0..255] of ObjCodeNdxType;    { Index for opcodes }
   CurrentCode : Integer;    { Pointer to opcode in progress }
   OpMemory   : array[1..OpMemorySize] of Byte;  { Buffer for the bytes of opcode }
   CurrentLegal : Boolean;        { T=current opcode is legal, F=illegal }

   NumEntries : integer;                    { number of entries used }
   Entry      : array[1..MaxEntries] of word;    { Entry location storage }
   NumBranches: integer;                    { number of branches used }
   BranchTable: array[1..MaxBranches] of word;   { Branch location storage }

   OpcodesFile: Text;   { File variable to read opcodes from }

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
   Overlay    : Boolean;     { files overlayed each other }

   PC         : Word;        { Running Program Counter (for scanning) }
   PCOld      : Word;        { Trailing PC counter }

   Addresses  : Boolean;     { Write addresses on disassembly (toggle) }
   DisOpCode  : Boolean;     { Disassemble opcode listing as comment }
                             {  in disassembly file (toggle) }
   DisCodeFlg : Boolean;     { Disassembling flag = True when actually }
                             {  writing code/data to disassembly file  }
                             {  verses just header comments }
   AsciiFlg   : Boolean;     { Disassembling flag, True=convert printable data to .ascii }

   LAdrDplyCnt : Integer;    { Label address display counter }

   CBDef       : Boolean;    { Condition undeterminable Branch defined flag }


{ Function to convert to string to uppercase }
function UpString(Old : String): String;
var
   Temp : String;
   I    : Integer;
begin
     Temp:='';
     For I:=1 to Length(Old) do
       Temp:=Temp+UpCase(Old[I]);
     UpString:=Temp;
end;

{ Function to convert integer to string }
function StringIt(IVal : Integer): String;
var
   Temp : String;
begin
     Str(IVal,Temp);
     StringIt:=Temp;
end;

{ Function to convert a word into a Hex string }
function HexString(HVal: Word; HSize: Integer): String;
var
   Temp : String;
   I    : Integer;
   TVal : Word;
   SVal : Word;
begin
     Temp:='';
     TVal:=HVal;
     For I:=1 to HSize do
       begin
         SVal:=(TVal mod 16);
         TVal:=(TVal div 16);
         Temp:=HexValue[SVal]+Temp;
       end;
     HexString:=Temp;
end;

{ Function to find a string seperated by white space, returning both
the new seperated string, and a modified old string in ctrlinput (without
the seperated string)  }
function StrSpace: String;
var
   Spos : Integer;
   Tpos : Integer;
   NStr : String;
   TStr : String;
begin
     NStr:='';
     TStr:=CtrlInput;
     Spos:=Pos(' ',TStr);
     Tpos:=Pos(chr(9),TStr);
     while ((Spos=1) or (Tpos=1)) do
       begin
         TStr:=Copy(TStr,2,Length(TStr)-1);
         Spos:=Pos(' ',TStr);
         Tpos:=Pos(chr(9),TStr);
       end;
     if ((Spos=0) and (Tpos=0)) then
       begin
         NStr:=TStr;
         TStr:='';
       end
     else
       begin
         If Spos=0 then Spos:=Tpos;
         If Tpos=0 then Tpos:=Spos;
         If Tpos<Spos then Spos:=Tpos;
         NStr:=Copy(TStr,1,Spos-1);
         TStr:=Copy(TStr,Spos,Length(TStr)-Spos+1);
       end;
     StrSpace:=NStr;
     CtrlInput:=TStr;
end;

{ function to convert a hexadecimal string into a word.  If string
is longer that 4 nybbles, only least significant 4 are used.
incorrect string values are ignored. }
function Hex2Dec(HStr: String): Word;
var
   TVal : Word;
   TStr : String;
   I    : Integer;
   AVal : Word;
begin
     TStr:=HStr;
     TVal:=0;
     For I:=1 to Length(HStr) do
       begin
         AVal:=0;
         If ((TStr[I]>='0') and (TStr[I]<='9')) then
           AVal:=Ord(TStr[I])-48;
         If ((TStr[I]>='A') and (TStr[I]<='F')) then
           AVal:=Ord(TStr[I])-55;
         TVal:=TVal*16+AVal;
       end;
     Hex2Dec:=TVal;
end;

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
{     
  if ((((CAddr<LoadOffset) or (CAddr>LoadOffset+FSize-1)) and
               (Wrap=False)) or
          ((CAddr<LoadOffset) and (CAddr>=LoadOffset+FSize) and
              (Wrap=True))) then
          CheckAddrByte:=False else CheckAddrByte:=True;
}
  if (ReadType(CAddr)=$FF) then
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

{ Procedure to read the opcodes file }
procedure ReadOpcodes;
var
   OpSize : Integer;    { Variable to store # of opcode bytes }
   OpCodeValue : Integer;    { Variable to read opcode value }
   I : Integer;    { Misc Counter }
begin
     Write('Reading Opcodes File...');
     Assign(OpcodesFile,OpcodesFilename);
     Reset(OpcodesFile);
     If (IOResult<>0) then
       begin
         Writeln; Writeln;
         Writeln('*** Error: Opening Opcodes File: ',OpcodesFilename);
         Halt;
       end;

     NumOpcodes:=0;     { Initialize number of opcodes }
     For I:=0 to 255 do { Initialize opcode index table }
       begin
         ObjectCodeNdx[I].Legal:=False;
         ObjectCodeNdx[I].Start:=0;
       end;

     ObjectCode[0].Mnemonic:='???';    { Define 'illegal' opcode }
     ObjectCode[0].Grp:=0;
     ObjectCode[0].NBytes:=0;
     For I:=1 to MaxOpcodeBytes do
       ObjectCode[0].OpCode[I]:=0;
     ObjectCode[0].Control:=1;

     Read(OpcodesFile,OpSize);
     While OpSize<>0 do           { Read until EOF flag }
     begin
          NumOpcodes:=NumOpcodes+1;
          Read(OpcodesFile,OpCodeValue);    { Read primary opcode value }
          If ObjectCodeNdx[OpCodeValue].Legal=False then
            begin
              ObjectCodeNdx[OpCodeValue].Legal:=True;
              ObjectCodeNdx[OpCodeValue].Start:=NumOpcodes;
            end;
          ObjectCode[NumOpcodes].NBytes:=OpSize;
          ObjectCode[NumOpcodes].OpCode[1]:=OpCodeValue;

          For I:=2 to OpSize do
            begin
              Read(OpcodesFile,OpCodeValue);  { Read next opcode value }
              ObjectCode[NumOpcodes].OpCode[I]:=OpCodeValue;
            end;

          Read(OpCodesFile,ObjectCode[NumOpcodes].Grp);
          Read(OpCodesFile,ObjectCode[NumOpcodes].Control);
          Readln(OpCodesFile,CtrlInput);
          ObjectCode[NumOpcodes].Mnemonic:=StrSpace;
          Read(OpcodesFile,OpSize);
     end;

     If (IOResult<>0) then
       begin
         Writeln; Writeln;
         Writeln('*** Error: Reading Opcodes File: ',OpcodesFilename);
         Halt;
       end;

     Close(OpCodesFile);
     Writeln(NumOpcodes,' opcodes read.');
end;

{ procedure to read binary file }
procedure ReadSource(aName: string);
var
   NumRead: Word;
   AddrCnt: Word;
   I      : Integer;
   LocalFSize : Word;
begin
     Write('Reading Source File: ',aName,'...');
     AddrCnt:=LoadOffset;
     LocalFSize:=0;
     Overlay:=False;
     Assign(PgmFile,aName);
     Reset(PgmFile,1);
     If (IOResult<>0) then
       begin
         Writeln; Writeln;
         Writeln('*** Error: Opening Source File: ',aName);
         Halt;
       end;
     repeat
       BlockRead(PgmFile,Buf,SizeOf(Buf),NumRead);
       if NumRead>0 then
         begin
           For I:=1 to NumRead do
             begin
               If (ReadType(AddrCnt)<>$FF) then Overlay:=True;
               WriteByte(AddrCnt,Buf[I]);
               WriteType(AddrCnt,0);    { Tag as "loaded" assume data }
               AddrCnt:=AddrCnt+1;
               FSize:=FSize+1;
               LocalFSize:=LocalFSize+1;
               If AddrCnt=0 then Wrap:=True;
             end;
         end;
     until (NumRead=0);
     Close(PgmFile);
     Writeln('File Size: '+HexDelim,HexString(LocalFSize,4));
     If ((Wrap) and (AddrCnt<>0)) then
     begin
       Writeln;
       Writeln('*** Warning: Memory wrap around encountered...');
       Writeln('             Check Load Offset/File Length.');
       Writeln;
     end;
     If Overlay then
     begin
       Writeln;
       Writeln('*** Warning: Source file overlayed previously loaded file...');
       Writeln('             Check Load Offset/File Length.');
       Writeln;
     end;
end;

{ procedure to parse commands from control file }
procedure ParseControl;
var
   I : Integer;
   VString : String;
   TempString : String;
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
               2: begin
                    LoadOffset:=Hex2Dec(StrSpace);
                    Writeln('        Load Address: '+HexDelim,HexString(LoadOffset,4));
                  end;
               3: begin
                    TempString:=StrSpace;
                    If SrcFlag then PgmFilename:=PgmFilename+',';
                    PgmFilename:=PgmFilename+TempString;
                    ReadSource(TempString);   { Read the file in }
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
     PgmFilename:='';
     DstFlag:=False;
     NumEntries:=0;
     NumBranches:=0;
     NumLbls:=0;
     NumIndirect:=0;
     LoadOffset:=0;
     Addresses:=False;
     DisOpCode:=False;
     AsciiFlg:=False;
     Writeln('Reading Control File...');
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
     If ((NumEntries=0) and (NumIndirect=0)) then
       begin
         Entry[1]:=LoadOffset;
         NumEntries:=1;
       end;
     Writeln;
     Writeln('Total File Size: '+HexDelim,HexString(FSize,4));
     Writeln('        ',NumEntries,' Entry Points: ');
     For I:=1 to NumEntries do
       Writeln('                '+HexDelim,HexString(Entry[I],4));
     Writeln('        Source File: ',PgmFilename);
     Writeln('        Destination File: ',OutputFilename);
     Writeln('        ',NumLbls,' Labels Defined:');
     For I:=1 to NumLbls do
       Writeln('                '+HexDelim,HexString(Lbl[I].Addr,4),'=',Lbl[I].Name,LblDelim);
     If Addresses then Writeln('Writing program counter addresses to disassembly file.');
     Writeln; Writeln('Checking Entry Points...');
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
     If ReadType(PC)=2 then TFlg:=True;     { Terminate if we run into code }
     While TFlg=False do
     begin
       ReadNextObj;     { Read next object code from memory, tagging bytes }
       If (ObjectCode[CurrentCode].Control mod 2)=1 then TFlg:=True;
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
     For PC:=$0000 to $FFFF do
       if ReadType(PC)=0 then
         if (AsciiFlg and
             (ReadByte(PC)>=PrintAbleLo) and
             (ReadByte(PC)<=PrintAbleHi) and
             (ReadByte(PC)<>DtaDelim) and
             (ReadByte(PC)<>ord('\'))) then WriteType(PC,1);
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
   HasWrittenOrg: Boolean;
   IsDone: Boolean;
   TempPC: Word;
   FirstTime: Boolean;
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

     PC:=$0000;
     OutputLine:=''; WriteLine;
     OutputLine:=Tab+'.area'+Tab+'CODE1'+Tab+'(ABS)';
     WriteLine;
     HasWrittenOrg:=False;

     DisCodeFlg:=True;
     IsDone:=False;
     FirstTime:=True;
     While IsDone=False do
       begin
         While ((IsDone=False) and (ReadType(PC)=$FF)) do
         begin
           PC:=PC+1;
           If PC=0 then IsDone:=True;
           HasWrittenOrg:=False;
         end;
         
         If ((HasWrittenOrg=False) and (not IsDone)) then
         begin
           If (not FirstTime) then
           begin
             OutputLine:=''; WriteLine;
             OutputLine:=''; WriteLine;
           end else
             FirstTime:=False;
           OutputLine:=Tab+'.org'+Tab+HexDelim+HexString(PC,4);
           WriteLine;
           OutputLine:=''; WriteLine;
           HasWrittenOrg:=True;
         end;

         If (not IsDone) then
         begin
           OutputLine:='';
           CBDef:=True;
           TempPC:=PC;
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
                If PC=0 then IsDone:=True;
                While ((ReadType(PC)=0) and (J<MaxNonPrint) and
                       (CheckLbl(ReadPage)=False) and (not IsDone)) do
                  begin
                    OutputLine:=OutputLine+','+HexDelim+HexString(ReadByte(PC),2);
                    PC:=PC+1; J:=J+1;
                    LblPage.Addr:=PC;
                    If PC=0 then IsDone:=True;
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
                If PC=0 then IsDone:=True;
                While ((ReadType(PC)=1) and (J<MaxPrint) and
                       (CheckLbl(ReadPage)=False) and (not IsDone)) do
                  begin
                    OutputLine:=OutputLine+chr(ReadByte(PC));
                    PC:=PC+1; J:=J+1;
                    LblPage.Addr:=PC;
                    If PC=0 then IsDone:=True;
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
           $FF: HasWrittenOrg:=False;
           end;
           WriteLine;
           If (PC<TempPC) then IsDone:=True;  { Check for wrap around }
         end;
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
var
   I : Word;
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

     For I:=$0000 to $FFFF do
      begin
       WriteByte(I,0);       { Place 0 data byte }
       WriteType(I,$FF);     { Tag all locations as unloaded }
      end;
     LAdrDplyCnt:=0;
     Wrap:=False;
     FSize:=0;
     Writeln;
end;

{ Deinitialization procedure }
procedure Deinit;
begin
     Release(HpStatus);      { Return the heap to original conditions }
end;

begin
     Writeln('M6811 Disassembler V1.1');
     Writeln('Copyright(c)1996 by Donald Whisnant');
     Writeln;
     If ParamCount<1 then
       begin
         Writeln('Usage: M6811DM <control filename>');
         Halt;
       end;
     Init;
     ControlFilename:=UpString(ParamStr(1));
     ReadControlFile;
     ReadOpcodes;
{     ReadSource;  }
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
