//
//	Motorola 6811 Disassembler
//	Copyright(c)1996 - 2014 by Donna Whisnant
//

/*
	SFileDFC

		This is the C++ Module that contains routines to read and write Motorola
		S-Files, by defining a new derived class DFC_SFILE from the base
		class DataFileConverter.  This module makes use of the MEMCLASS type for
		memory interface.

		Author:	Donna Whisnant
		Date:	February 10, 2000

		Written in: MS Visual C++ 5.0

*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "sfiledfc.h"



// Motorola format:
//		Sxccaa..aadd....kk
//		where:	S is the record character "S"
//				x=record mode or type (See below)
//				cc=byte count after cc -- unlike Intel where the byte count is ONLY
//							the number of data bytes, this is the count of the
//							number of bytes in the address field, plus the number of
//							bytes in the data field, plus the 1 byte checksum
//				aa..aa=offset address -- Can be 2, 3, or 4 bytes depending on the 'x' type
//				dd....=data bytes
//				kk=checksum = 1's complement of the sum of all bytes in the record
//							starting with 'cc' through the last 'dd'...
//							Total sum (including checkbyte) should equal 0xFF
//
//			Record mode/types:
//				0 = Header character -- this is just a file signon that can contain
//						any desired data...  GM PTP's, for example, use "HDR" in the
//						data field...  The Address field is 2 bytes (4 characters) and
//						is usually 0000 except for Motorola DSP's where it indicates
//						the DSP Memory Space for the s-records that follow, where
//						0001=X, 0002=Y, 0003=L, 0004=P, and 0005=E (DSP only!)
//						This field is optional
//				1 = Data Record with a 2-byte address (aaaa = start address for data in this record)
//				2 = Data Record with a 3-byte address (aaaaaa = start address for data in this record)
//				3 = Data Record with a 4-byte address (aaaaaaaa = start address for data in this record)
//				7 = EOF Record with a 4-byte address (aaaaaaaa = Executable start address for this s-file)
//				8 = EOF Record with a 3-byte address (aaaaaa = Executable start address for this s-file)
//				9 = EOF Record with a 2-byte address (aaaa = Executable start address for this s-file)
//
//		Extensions:
//			S19 = 16-Bit addressing files (ones using S1 and S9 records)
//			S28 = 24-Bit addressing files (ones using S2 and S8 records)
//			S37 = 32-Bit addressing files (ones using S3 and S7 records)
//


// Intel format:
//    ccaaaammdd....kk
//    where: cc=byte count
//           aaaa=offset address
//           mm=mode byte
//           dd=date bytes (there are 'cc' many of 'dd' entries)
//           kk=checksum = sum of ALL hex pairs as bytes (including cc, aaaa, mm, and dd)
//                            Sum (including checkbyte) should equal zero.
//       mm=00 -> normal line
//       mm=01 -> end of file
//       mm=02 -> SBA entry


// RetrieveFileMapping:
//
//    This function reads in an already opened text BINARY file pointed to by
//    'aFile' and generates a TMemRange object that encapsulates the file's
//    contents, offsetted by 'NewBase' (allowing loading of different files
//    to different base addresses).
//
// Author:  Donna Whisnant
// Date:    June 28, 1997
int TDFC_SFile::RetrieveFileMapping(std::istream *aFile, unsigned long NewBase, TMemRange *aRange) const
{
   int   RetVal;

   try {
      RetVal = _ReadDataFile(aFile, NewBase, NULL, aRange, 0);
   }
   catch (EXCEPTION_ERROR*) {
      throw;
   }
   return RetVal;
}


// ReadDataFile:
//
//
// Author:  Donna Whisnant
// Date:    January 29, 1997
int TDFC_SFile::ReadDataFile(std::istream *aFile, unsigned long NewBase, TMemObject *aMemory, unsigned char aDesc) const
{
   int   RetVal;

   try {
      RetVal = _ReadDataFile(aFile, NewBase, aMemory, NULL, aDesc);
   }
   catch (EXCEPTION_ERROR*) {
      throw;
   }
   return RetVal;
}


// _ReadDataFile:
//
//    This function reads in an already opened text BINARY file pointed to by
//    'aFile' into the MemObject pointed to by 'aMemory' offsetted by
//    'NewBase' (allowing loading of different files to different base
//    address) and setting the corresponding Memory Descriptors to 'aDesc'...
//
//    This function has been updated to a composite function that handles both
//    the reading of the file into memory and simply reading the Mapping.
//    This is a private function called by the regular ReadDataFile and
//    RetrieveFileMapping.  If aMemory is null then memory is not filled.  If
//    aRange is null, the mapping is not generated.  If either or both is
//    non-null, then the corresponding operation is performed.
//
// Author:  Donna Whisnant
// Date:    January 29, 1997
int TDFC_SFile::_ReadDataFile(std::istream *aFile, unsigned long NewBase, TMemObject *aMemory, TMemRange *aRange, unsigned char aDesc) const
{
   int            RetVal;           // Return Value
   int            LineCount;        // Line Counter
   char           Buff[256];        // Line Buffer
   char           *BufPtr;          // Index into buffer
   unsigned int   SBA;              // Segment Base Address
   unsigned int   NBytes;           // Number of bytes supposed to be on line
   unsigned int   OffsetAddr;       // Offset Address read on line
   unsigned int   i;                // Loop counter
   unsigned int   chksum;           // Running Checksum
   unsigned long  CurrAddr;         // Current Memory Address
   unsigned int   Mode;             // Mode Byte
   unsigned int   TempByte;         // Hex to Decimal conversion storage
   int            EndReached;       // End reached Flag
   int            FirstRange;       // Flag for first range
   int            NeedRangeUpdate;  // Flag for needing to update the range capture
   int            NeedToWriteRange; // Flag for needing to write the range
   unsigned long  StartAddr;        // Current StartAddr of the current Range
   unsigned long  CurrSize;         // Current Size of the current Range

   RetVal=1;
   SBA=0;
   LineCount=0;
   EndReached=0;
   aFile->seekg(0L, std::ios_base::beg);
   FirstRange = 1;
   NeedToWriteRange = 0;
   NeedRangeUpdate = 1; // Force the first update
	StartAddr = 0;
   CurrAddr = 0;
   CurrSize = 0;
   if (aRange!=NULL) {
      // Initialize the range and setup incase there are no real bytes
      aRange->PurgeRange();
      aRange->SetStartAddr(0);
      aRange->SetSize(0);
   }
   while ((!aFile->eof()) && (aFile->peek() != EOF) && (!EndReached)) {
      aFile->getline(Buff, sizeof(Buff));
      if (!aFile->good()) THROW_EXCEPTION_ERROR(EXCEPTION_ERROR::ERR_UNEXPECTED_EOF, 0, "");
      LineCount++;
      if (Buff[0]==':') {
         sscanf(&Buff[1], "%2X%4X%2X", &NBytes, &OffsetAddr, &Mode);
         chksum=NBytes+(OffsetAddr/256)+(OffsetAddr%256)+Mode;

         if (CurrAddr!=((((unsigned long)SBA)*16)+(unsigned long)OffsetAddr+NewBase))
            NeedRangeUpdate=1;

         if (NeedRangeUpdate) {
            // If we need to update the Range do so, else only update the StartAddr
            if (NeedToWriteRange) {
               // If this is not the first range, add a range
               if ((aRange!=NULL) && (!FirstRange)) {
                  try {
                     aRange->AddRange(StartAddr, CurrSize);
                  }
                  catch (EXCEPTION_ERROR*) {
                     throw;
                  }
               }
               // Else, set the first range
               if ((aRange!=NULL) && (FirstRange)) {
                  aRange->SetStartAddr(StartAddr);
                  aRange->SetSize(CurrSize);
                  FirstRange = 0;
               }
               // Signify we don't need to write it, since we just did
               NeedToWriteRange = 0;
            }

            // Set start of next range, and initialize variables
            StartAddr = (((unsigned long)SBA)*16)+(unsigned long)OffsetAddr+NewBase;
            CurrAddr = StartAddr;
            CurrSize = 0;
            NeedRangeUpdate=0;
         }

         BufPtr=&Buff[9];
         switch (Mode) {
			case 0:  for (i=0; i<NBytes; ++i) {
                        sscanf(BufPtr, "%2X", &TempByte);
                        chksum+=TempByte;
                        if (aMemory) {
                           if (!aMemory->SetByte(CurrAddr, (unsigned char)TempByte)) {
                              THROW_EXCEPTION_ERROR(EXCEPTION_ERROR::ERR_OVERFLOW, 0, "");
                           }
                           if (aMemory->GetDescriptor(CurrAddr)!=0) RetVal=0;
                           aMemory->SetDescriptor(CurrAddr, aDesc);
                        }
                        CurrAddr++;
                        CurrSize++;
                        BufPtr+=2;
                        // Any time we add a byte, make sure the range gets updated
                        NeedToWriteRange = 1;
                     }
                     break;
            case 1:  EndReached=1;
                     break;
            case 2:  sscanf(BufPtr, "%4X", &SBA);
                     chksum+=(SBA/256)+(SBA%256);
                     BufPtr+=4;
                     break;
         }
         sscanf(BufPtr, "%2X", &TempByte);
         chksum+=TempByte;
         if ((chksum%256)!=0) {
            THROW_EXCEPTION_ERROR(EXCEPTION_ERROR::ERR_CHECKSUM, LineCount, "");
         }
      }
   }
   // Make a final test to see if the range needs to be saved,
   //    This is necessary to save the last range operated upon
   //    in case a pre-mature EOF is encountered.
   if (NeedToWriteRange) {
      // If this is not the first range, add a range
      if ((aRange!=NULL) && (!FirstRange)) {
         try {
            aRange->AddRange(StartAddr, CurrSize);
         }
         catch (EXCEPTION_ERROR*) {
            throw;
         }
      }
      // Else, set the first range
      if ((aRange!=NULL) && (FirstRange)) {
         aRange->SetStartAddr(StartAddr);
         aRange->SetSize(CurrSize);
         FirstRange = 0;
      }
   }
   if (((aFile->eof()) || (aFile->peek() == EOF)) &&
       (!EndReached)) THROW_EXCEPTION_ERROR(EXCEPTION_ERROR::ERR_UNEXPECTED_EOF, 0, "");
   return(RetVal);
}


// WriteDataFile:
//
//    This function writes to an already opened text BINARY file pointed to by
//    'aFile' from the MemObject pointed to by 'aMemory' and whose Descriptor
//    has a match specified by 'aDesc'.  Matching is done by 'anding' the
//    aDesc value with the descriptor in memory.  If the result is non-zero,
//    the location is written.  Unless, aDesc=0, in which case ALL locations
//    are written regardless of the actual descriptor.  If the aDesc validator
//    computes out as false, then the data is filled according to the FillMode.
//
// Author:  Donna Whisnant
// Date:    January 29, 1997
int TDFC_SFile::WriteDataFile(std::ostream *aFile, TMemRange *aRange, unsigned long NewBase, TMemObject *aMemory,
					 unsigned char aDesc, int UsePhysicalAddr, unsigned int FillMode) const
{
   TMemRange      *CurrRange;       // Range Spanning variable
   int            RetVal;           // Return Value
   int            LineCount;        // Line Counter
   char           TempStr[10];      // Temporary Conversion String
   char           Buff[256];        // Line Buffer
   unsigned int   SBA;              // Segment Base Address
   unsigned int   NBytes;           // Number of bytes supposed to be on line
   unsigned int   OffsetAddr = 0;   // Offset Address read on line
   unsigned int   i;                // Loop counter
   unsigned int   chksum;           // Running Checksum
   unsigned long  BytesLeft;        // Remaining number of bytes to check/write
   unsigned long  CurrAddr;         // Current Logical Memory Address
   unsigned long  RealAddr = 0;     // Current Written Address equivalent
   unsigned int   LowestOffset;     // Lowest Load offset for the file
   unsigned int   LowestFound;      // Flag for finding lowest offset
   unsigned int   TempByte;         // Temp byte used for calculation
   int            NeedNewOffset;    // Flag for needing to write a new offset
   char           WriteBuff[256];   // Used for printing to the output stream
   int            NoFill;           // Flag for no-byte and no-fill

   srand( (unsigned)time( NULL ) );

   CurrRange=aRange;
   RetVal=1;
   SBA=0;
   LineCount=0;
   LowestOffset=0x0000;
   LowestFound=0;

   while (CurrRange!=NULL) {
      CurrAddr=CurrRange->GetStartAddr();
      BytesLeft=CurrRange->GetSize();

      while (BytesLeft) {
         strcpy(Buff, "");
         NBytes=0;
         chksum=0;
         NeedNewOffset=1;
         while ((BytesLeft) && (NeedNewOffset)) {     // Find first location to write next
            if ((aDesc==0) || (aDesc & aMemory->GetDescriptor(CurrAddr)) ||
                 ((FillMode!=DFC_FILL_TYPE(NO_FILL)) &&
                  (FillMode!=DFC_FILL_TYPE(CONDITIONAL_FILL_WITH)) &&
                  (FillMode!=DFC_FILL_TYPE(CONDITIONAL_FILL_WITH_RANDOM)))) {
               if (UsePhysicalAddr) {
                  RealAddr=aMemory->GetPhysicalAddr(CurrAddr)+NewBase;
               } else {
                  RealAddr=CurrAddr+NewBase;
               }
               OffsetAddr=(unsigned int)(RealAddr&0xFFFF);
               if (!LowestFound) {
                  LowestOffset=OffsetAddr;
                  LowestFound=1;
               }
               if (((RealAddr/65536UL)*0x1000UL)!=SBA) {
                  SBA=(unsigned int)((RealAddr/65536UL)*0x1000UL);
                  if (((RealAddr/65536UL)*0x1000UL)!=(unsigned long)(SBA))
							THROW_EXCEPTION_ERROR(EXCEPTION_ERROR::ERR_OVERFLOW, 0, "");
                  TempByte=(256-(((SBA/256)+(SBA%256)+4)%256))%256;
                  sprintf(WriteBuff, ":02000002%04X%02X\r\n", SBA, TempByte);
                  aFile->write(WriteBuff, strlen(WriteBuff));
                  if (!aFile->good()) THROW_EXCEPTION_ERROR(EXCEPTION_ERROR::ERR_WRITEFAILED, 0 ,"");
                  LineCount++;
               }
               NeedNewOffset=0;
            } else {
               BytesLeft--;
               CurrAddr++;
            }
         }

		 for (i=0; ((i<32) && (!NeedNewOffset) && (BytesLeft)); ++i) {
            if ((aDesc==0) || (aDesc & aMemory->GetDescriptor(CurrAddr))) {
               TempByte=aMemory->GetByte(CurrAddr);
               NoFill=0;   // Fill, cause we have a byte
            } else {
               NoFill=0;   // Default to Fill
               switch GET_DFC_FILL_TYPE(FillMode) {
                  case DFC_FILL_TYPE(ALWAYS_FILL_WITH):
                           TempByte=GET_DFC_FILL_BYTE(FillMode);
                           break;
                  case DFC_FILL_TYPE(ALWAYS_FILL_WITH_RANDOM):
                           TempByte=(unsigned char)(rand()%256);
                           break;
                  default: TempByte=0x00;
                           NoFill=1;      // No-Fill
                           break;
               }
            }
            if (!NoFill) {    // If we have a byte, use it
               sprintf(TempStr, "%02X", TempByte);
               strcat(Buff, TempStr);
               chksum+=TempByte;
               NBytes++;
            }
            BytesLeft--;
            CurrAddr++;
            RealAddr++;
            if ((aDesc==0) || (aDesc & aMemory->GetDescriptor(CurrAddr)) ||
                 ((FillMode!=DFC_FILL_TYPE(NO_FILL)) &&
                  (FillMode!=DFC_FILL_TYPE(CONDITIONAL_FILL_WITH)) &&
                  (FillMode!=DFC_FILL_TYPE(CONDITIONAL_FILL_WITH_RANDOM)))) {
               if (UsePhysicalAddr) {
                  if (RealAddr!=(aMemory->GetPhysicalAddr(CurrAddr)+NewBase)) {
                     NeedNewOffset=1;
                  }
               }
               if (((RealAddr/65536UL)*0x1000UL)!=SBA) {
                  NeedNewOffset=1;
               } else {
                  if ((RealAddr&0xFFFF)==0) NeedNewOffset=1;
               }
            } else {
               NeedNewOffset=1;
            }
         }

         if (NBytes!=0) {
            chksum+=NBytes;
            chksum+=(OffsetAddr/256)+(OffsetAddr%256);
            chksum=(256-(chksum%256))%256;
            sprintf(WriteBuff, ":%02X%04X00%s%02X\r\n", NBytes, OffsetAddr, Buff, chksum);
            aFile->write(WriteBuff, strlen(WriteBuff));
            if (!aFile->good()) THROW_EXCEPTION_ERROR(EXCEPTION_ERROR::ERR_WRITEFAILED, 0 ,"");
            LineCount++;
         }
      }

      CurrRange=CurrRange->GetNext();
   }

   TempByte=(256-(((LowestOffset/256)+(LowestOffset%256)+1)%256))%256;
   sprintf(WriteBuff, ":00%04X01%02X\r\n", LowestOffset, TempByte);
   aFile->write(WriteBuff, strlen(WriteBuff));
   if (!aFile->good()) THROW_EXCEPTION_ERROR(EXCEPTION_ERROR::ERR_WRITEFAILED, 0 ,"");
   return(RetVal);
}

