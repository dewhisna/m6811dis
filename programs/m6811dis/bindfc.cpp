//
//	Motorola 6811 Disassembler
//	Copyright(c)1996 - 2014 by Donna Whisnant
//

/*
      BINARYDFC --- BINARY Data File Converter Class

      This is the C++ Module that contains routines to read and write binary
      data files, by defining a new derived class DFC_BINARY from the base
      class DataFileConverter.  This module makes use of the MEMCLASS type for
      memory interface.

	  Author:  Donna Whisnant
      Date:    March 7, 1997

      Written in: Borland C++ 4.52

      Updated: June 23, 1997 to work with both MS Visual C++ 4.0/5.0
				as well as Borland C++.

      Updated: June 29, 1997 to use iostream instead of run-time
            library so that external DLL calls will function
            correctly.
*/

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <time.h>
#include "bindfc.h"


// RetrieveFileMapping:
//
//    This function reads in an already opened BINARY file pointed to by
//    'aFile' and generates a TMemRange object that encapsulates the file's
//    contents, offsetted by 'NewBase' (allowing loading of different files
//    to different base addresses).
//
// Author:  Donna Whisnant
// Date:    June 28, 1997
int TDFC_Binary::RetrieveFileMapping(std::istream *aFile, unsigned long NewBase, TMemRange *aRange) const
{
	if (aRange) {
		aRange->PurgeRange();
		aRange->SetStartAddr(NewBase);
		aFile->seekg(0L, std::ios_base::end);
		aRange->SetSize(aFile->tellg());
		if (aRange->GetSize() == static_cast<unsigned long>(-1))
			aRange->SetSize(0);
		aFile->seekg(0L, std::ios_base::beg);
	}
   return 1;
}


// ReadDataFile:
//
//    This function reads in an already opened BINARY file pointed to by
//    'aFile' into the MemObject pointed to by 'aMemory' offsetted by
//    'NewBase' (allowing loading of different files to different base
//    address and setting the corresponding Memory Descriptors to 'aDesc'...
//
// Author:  Donna Whisnant
// Date:    March 7, 1997
int TDFC_Binary::ReadDataFile(std::istream *aFile, unsigned long NewBase, TMemObject *aMemory, unsigned char aDesc) const
{
	int            RetVal;           // Return Value
	unsigned char  aByte;

	unsigned long  CurrAddr;         // Current Memory Address

	RetVal=1;

	CurrAddr = NewBase;
	aFile->seekg(0L, std::ios_base::beg);
	while ((!aFile->eof()) && (aFile->peek() != EOF) && (aFile->good())) {
		aByte = aFile->get();
		if (!aMemory->SetByte(CurrAddr, aByte)) {
			THROW_EXCEPTION_ERROR(EXCEPTION_ERROR::ERR_OVERFLOW, 0, "");
		}
		if (aMemory->GetDescriptor(CurrAddr)!=0) RetVal=0;
		aMemory->SetDescriptor(CurrAddr, aDesc);
		CurrAddr++;
	}

	return(RetVal);
}


// WriteDataFile:
//
//    This function writes to an already opened BINARY file pointed to by
//    'aFile' from the MemObject pointed to by 'aMemory' and whose Descriptor
//    has a match specified by 'aDesc'.  Matching is done by 'anding' the
//    aDesc value with the descriptor in memory.  If the result is non-zero,
//    the location is written.  Unless, aDesc=0, in which case ALL locations
//    are written regardless of the actual descriptor.  If the aDesc validator
//    computes out as false, then the data is filled according to the FillMode.
//
// Author:  Donna Whisnant
// Date:    January 29, 1997
int TDFC_Binary::WriteDataFile(std::ostream *aFile, TMemRange *aRange, unsigned long NewBase, TMemObject *aMemory,
					 unsigned char aDesc, int UsePhysicalAddr, unsigned int FillMode) const
{
   TMemRange      *CurrRange;       // Range Spanning variable
   int            RetVal;           // Return Value
   unsigned long  BytesLeft;        // Remaining number of bytes to check/write
   unsigned long  CurrAddr;         // Current Logical Memory Address
   unsigned long  RealAddr = 0;     // Current Written Address equivalent
   int            NeedNewOffset;


   srand( (unsigned)time( NULL ) );

   CurrRange=aRange;
   RetVal=1;
   NeedNewOffset = 1;
   while (CurrRange!=NULL) {
      CurrAddr=CurrRange->GetStartAddr();
      BytesLeft=CurrRange->GetSize();

      while (BytesLeft) {
         while (BytesLeft && NeedNewOffset) {
            if ((aDesc==0) || (aDesc & aMemory->GetDescriptor(CurrAddr)) || (FillMode!=DFC_FILL_TYPE(NO_FILL))) {
               if (UsePhysicalAddr) {
                  RealAddr=aMemory->GetPhysicalAddr(CurrAddr)+NewBase;
               } else {
                  RealAddr=CurrAddr+NewBase;
               }
               NeedNewOffset=0;
            } else {
               BytesLeft--;
               CurrAddr++;
            }
         }

         while (BytesLeft && !NeedNewOffset) {
            if ((aDesc==0) || (aDesc & aMemory->GetDescriptor(CurrAddr))) {
               aFile->put(aMemory->GetByte(CurrAddr));
               if (!aFile->good()) THROW_EXCEPTION_ERROR(EXCEPTION_ERROR::ERR_WRITEFAILED, 0, "");
            } else {
               switch GET_DFC_FILL_TYPE(FillMode) {
                  case DFC_FILL_TYPE(ALWAYS_FILL_WITH):
                  case DFC_FILL_TYPE(CONDITIONAL_FILL_WITH):
                           aFile->put(GET_DFC_FILL_BYTE(FillMode));
                           if (!aFile->good()) THROW_EXCEPTION_ERROR(EXCEPTION_ERROR::ERR_WRITEFAILED, 0, "");
                           break;
                  case DFC_FILL_TYPE(ALWAYS_FILL_WITH_RANDOM):
                  case DFC_FILL_TYPE(CONDITIONAL_FILL_WITH_RANDOM):
                           aFile->put((unsigned char)(rand()%256));
                           if (!aFile->good()) THROW_EXCEPTION_ERROR(EXCEPTION_ERROR::ERR_WRITEFAILED, 0, "");
                           break;
                  default: aFile->put((unsigned char)0x00);
                           if (!aFile->good()) THROW_EXCEPTION_ERROR(EXCEPTION_ERROR::ERR_WRITEFAILED, 0, "");
                           break;
               }
            }

            BytesLeft--;
            CurrAddr++;
            RealAddr++;
            if ((aDesc==0) || (aDesc & aMemory->GetDescriptor(CurrAddr)) || (FillMode!=DFC_FILL_TYPE(NO_FILL))) {
               if (UsePhysicalAddr) {
                  if (RealAddr!=(aMemory->GetPhysicalAddr(CurrAddr)+NewBase)) {
                     NeedNewOffset=1;
                  }
               }
            } else {
               NeedNewOffset=1;
            }
         }
      }

      CurrRange=CurrRange->GetNext();
   }

   return(RetVal);
}

