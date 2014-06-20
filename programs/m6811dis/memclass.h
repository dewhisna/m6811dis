//
//	Motorola 6811 Disassembler
//	Copyright(c)1996 - 2014 by Donna Whisnant
//

/*
      MEMCLASS

      This is the C++ Memory Object Class Header File.

	  Author:  Donna Whisnant
      Date:    January 28, 1997

      Written in: Borland C++ 4.52

      Updated: June 22, 1997 to work with both MS Visual C++ 4.0/5.0
				as well as Borland C++.

		Updated:	July 21, 1997 to expand TMemRange function base.

*/

#ifndef MEMCLASS_H
#define MEMCLASS_H

#include "errmsgs.h"

class TMemRange {
protected:
   unsigned long  StartAddr;
   unsigned long  Size;
	unsigned long	UserData;
   TMemRange *Next;

public:
   TMemRange(unsigned long aStartAddr, unsigned long aSize, unsigned long aUserData = 0);
   TMemRange(TMemRange& aRange);
   virtual ~TMemRange(void);
   void PurgeRange(void);
   int AddressInRange(unsigned long aAddr);
   void Consolidate(void);
	int IsNullRange(void);
	void Invert(void);
	void Sort(void);
	void RemoveOverlaps(void);
	void Compact(void);
	void Concatenate(TMemRange* aRange);
   unsigned long GetStartAddr(void);
   void SetStartAddr(unsigned long aStartAddr);
   unsigned long GetSize(void);
   void SetSize(unsigned long aSize);
   TMemRange *GetNext(void);
   void SetNext(TMemRange *aNext);
	unsigned long GetUserData(void);
	void SetUserData(unsigned long aUserData);
   TMemRange *AddRange(unsigned long aStartAddr, unsigned long aSize, unsigned long aUserData = 0);
};


class TMappingObject {
protected:
   unsigned long LogicalAddr;
   unsigned int BlockNumber;
   unsigned long PhysicalAddr;
   TMappingObject *Next;

public:
   TMappingObject(unsigned long ALogicalAddr, unsigned int ABlockNumber, unsigned long APhysAddr);
   TMappingObject(TMappingObject& aMap);
   virtual ~TMappingObject(void);
   unsigned long GetLogicalAddr(void);
   unsigned int GetBlockNumber(void);
   unsigned long GetPhysicalAddr(void);
   void SetNext(TMappingObject *ANext);
   TMappingObject *GetNext(void);
   TMappingObject *AddMapping(unsigned long ALogicalAddr, unsigned int ABlockNumber, unsigned long APhysAddr);
};


class TMemObject {
protected:
   unsigned int NumBlocks;
   unsigned long BlockSize;

   unsigned char *Data;
   unsigned char *Descriptors;

   TMappingObject *BlockMap;

public:
   TMemObject(unsigned int NBlocks, unsigned long BSize, unsigned char FillByte, int UseDesc);
   TMemObject(TMemObject& aMem);
   TMemObject(TMemRange& aRange, unsigned char FillByte, int UseDesc);
   virtual ~TMemObject(void);
   unsigned char GetByte(unsigned long LogicalAddr);
   int SetByte(unsigned long LogicalAddr, unsigned char Byte);
   unsigned char GetDescriptor(unsigned long LogicalAddr);
   int SetDescriptor(unsigned long LogicalAddr, unsigned char Desc);
   unsigned long GetMemSize(void);
   void ClearMemory(unsigned char FillByte);
   void ClearMapping(void);
   void AddMapping(unsigned long ALogicalAddr, unsigned int ABlockNumber, unsigned long APhysAddr);
   unsigned long GetPhysicalAddr(unsigned long LogicalAddr);

private:
   void CreateMemObject(unsigned int NBlocks, unsigned long BSize, unsigned char FillByte, int UseDesc);
};

#endif	// MEMCLASS_H

