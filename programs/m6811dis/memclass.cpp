//
//	Motorola 6811 Disassembler
//	Copyright(c)1996 - 2014 by Donna Whisnant
//

/*
      MEMCLASS

      This is the C++ object definition for a Memory Object.  The Memory
      object is an object that encompasses the whole of defined memory to
      be used for file conversions, disassemblers, etc.

	  Author:  Donna Whisnant
      Date:    January 28, 1997

      Written in: Borland C++ 4.52

      Updated: June 22, 1997 to work with both MS Visual C++ 4.0/5.0
				as well as Borland C++.

		Updated:	July 21, 1997 to expand TMemRange function base.

      Description:

         Each memory object represents the entire memory being acted upon,
      and typically, only one memory object needs to be created for an
      application.
         The memory is divided into pages (for paged memory applications)
      and each page has a descriptor byte that the application can use
      for processing -- useful for things such as "is_loaded", "is_data",
      "is_code", etc.

      Note:  All Addresses and All Block Numbers are ZERO ORIGINATED!
               Size numbers (such as NBlocks and BlockSize) are ONE ORIGINATED!

*/

#include "memclass.h"
#include "errmsgs.h"

#ifndef NULL
#define NULL 0
#endif

// The following are the function declarations for TMemRange

// TMemRange -- Constructor
//    The following constructs a TMemRange object by setting the StartAddr and
//    Size, and nullifying the next pointer.
TMemRange::TMemRange(unsigned long aStartAddr, unsigned long aSize, unsigned long aUserData)
{
   StartAddr = aStartAddr;
   Size = aSize;
	UserData = aUserData;
   Next=NULL;
}

// TMemRange -- Duplicating Constructor
//    This function is sorta like 'strdup'.  It creates a new MemRange object
//    and copies itself to it and then calls itself recursively to duplicate
//    the objects linked with the Next pointer.
//    Supports:   ERR_OUT_OF_MEMORY throwback
TMemRange::TMemRange(TMemRange& aRange)
{
   StartAddr=aRange.StartAddr;
   Size=aRange.Size;
	UserData=aRange.UserData;
   Next=NULL;

	try {
		if (aRange.Next!=NULL) {
      	Next=new TMemRange(*aRange.Next);
         if (Next==NULL) THROW_MEMORY_EXCEPTION_ERROR(0);
      }
   }
   catch (EXCEPTION_ERROR*) {
		PurgeRange();
      throw;
   }
}

// TMemRange -- Destructor
//    The following destructs a TMemRange object by checking to see if Next
//    points to a valid pointer to a linked list of MemRange Objects.  If it
//    does, it recursively destroys additionally allocated MemRange Objects.
TMemRange::~TMemRange(void)
{
   PurgeRange();
}

// TMemRange -- PurgeRange
//    The following destructs all children of the called range.  This
//    is useful in re-using (and re-initializing) existing TMemRange objects.
//    This function is used by the destructor to destroy children as well
//    when an object is removed.  The difference is that this function allows
//    us to destroy the children without killing the parent.
void TMemRange::PurgeRange(void)
{
	StartAddr = 0;
	Size = 0;
	UserData = 0;

   if (Next!=NULL) {
      delete Next;
      Next=NULL;
   }
}

// TMemRange -- AddressInRange
//    The following determines if a specified address exists within the
//    ranges specified by the current object (or its children).
//    This is useful for programs needing to identify addresses that are
//    in a range without necessarily needing to use descriptor bytes or
//    other methods of address referal.
//    The routine returns a '1' (or TRUE) if the address lies inside the
//    linked list of range (where this object is the head) or '0' (or
//    FALSE) if it does not.
int TMemRange::AddressInRange(unsigned long aAddr)
{
   if ((aAddr>=StartAddr) && (aAddr<(StartAddr+Size)))
      return(1);
   else if (Next!=NULL) return (Next->AddressInRange(aAddr));
   return (0);
}


// TMemRange -- Consolidate
//    The following function spans through the TMemRange and finds the
//    lowest and highest addresses.  It then purges the TMemRange and
//    sets its size and length to the corresponding values.
void TMemRange::Consolidate(void)
{
   unsigned long  MinAddr, MaxAddr;
   TMemRange      *aRange;

   MinAddr = StartAddr;
   MaxAddr = StartAddr+Size;

   aRange = Next;
   while (aRange) {
      if (aRange->StartAddr < MinAddr) MinAddr = aRange->StartAddr;
      if ((aRange->StartAddr+aRange->Size) > MaxAddr)
         MaxAddr = aRange->StartAddr+aRange->Size;
      aRange = aRange->Next;
   }
   PurgeRange();
   StartAddr = MinAddr;
   Size = MaxAddr - MinAddr;
}

// IsNullRange
//		The following function will test the range to see if it
//		is null and return a TRUE if it is or FALSE if not.  A
//		null range is defined as one whose size records are all
//		zero.
int TMemRange::IsNullRange(void)
{
	if (Size != 0) return 0;
	if (Next != NULL) return (Next->IsNullRange());
	return 1;
}

//	Invert
//		The following is the converse of Consolidate.  It transforms
//		the range list to its inverse within the bounds of the
//		given range.
//		Note: The UserData is lost from the original Range... (obviously)
//    Supports:   ERR_OUT_OF_MEMORY throwback
void TMemRange::Invert(void)
{
	TMemRange*	TempRange;
	TMemRange	NewRange(0, 0, 0);

	TMemRange*	ptRange;

	unsigned long	StartA;
	unsigned long	Ending;

	// Duplicate ourselves:
	try {
		TempRange = new TMemRange(*this);
	}
	catch (EXCEPTION_ERROR*) {
		throw;
	}

	// clean and sort the copy
	try {
		TempRange->Compact();
		TempRange->RemoveOverlaps();
		TempRange->Sort();
	}
	catch (EXCEPTION_ERROR*) {
		delete TempRange;
		throw;
	}

	// Get our bounds from the consolidated version
	Ending = 0;

	ptRange = TempRange;
	while (ptRange != NULL) {
		StartA = ptRange->StartAddr;

		if (StartA > Ending) {
			try {
				NewRange.AddRange(Ending, StartA-Ending);	// Add range between zones
			}
			catch (EXCEPTION_ERROR*) {
				delete TempRange;
				throw;
			}
		}
		Ending = StartA + ptRange->Size;			// Move Ending

		ptRange=ptRange->Next;
	}

	// Purge us and fill ourselves with the new inverted range
	PurgeRange();

	// Copy the new range structure to this range structure
	if (NewRange.Next != NULL) {
		// Copy the first record
		StartAddr = NewRange.Next->StartAddr;
		Size = NewRange.Next->Size;
		UserData = NewRange.Next->UserData;
		// And concatenate the rest
		try {
			Concatenate(NewRange.Next->Next);
		}
		catch (EXCEPTION_ERROR*) {
			throw;
		}
	}

	// Clean memory:
	NewRange.PurgeRange();
	TempRange->PurgeRange();
	delete TempRange;
}

//	Sort
//		The following sorts the range in ascending format.  Useful
//		for traversing, etc.  If two ranges have the same starting
//		address, then the shorter is placed ahead of the longer.
//		Since range lists are very short, we'll use a simple
//		bubble sort.
//    Supports:   ERR_OUT_OF_MEMORY throwback
void TMemRange::Sort(void)
{
	TMemRange	TempRange(0, 0, 0);
	TMemRange	Swap(0, 0, 0);

	TMemRange*	TempA;
	TMemRange*	TempB;
	TMemRange*	TempX;
	TMemRange*	TempY;
	unsigned long	StartA, EndA;
	unsigned long	StartB, EndB;
	int	Flag;

	try {
		TempRange.Concatenate(this);
	}
	catch (EXCEPTION_ERROR*) {
		throw;
	}

	TempA = &TempRange;
	TempB = TempA->Next;

	while (TempB != NULL) {
		TempX=TempB;
		TempY=TempB->Next;

		StartA = TempB->StartAddr;
		EndA = StartA+TempB->Size;
		Flag = 0;

		while ((!Flag) && (TempY != NULL)) {
			StartB = TempY->StartAddr;
			EndB = StartB+TempY->Size;

			if ( (StartB<StartA)  ||
				   ((StartB==StartA) && (EndB<EndA)) ) {
				Flag = 1;
			} else {
				TempX=TempX->Next;
				TempY=TempY->Next;
			}
		}

		if (!Flag) {
			// If none was found greater than this A/B pair, advance -- loop to end
			TempA=TempA->Next;
			TempB=TempB->Next;
		} else {
			// We found one, lets swap it
			Swap.StartAddr = TempY->StartAddr;
			Swap.Size = TempY->Size;
			Swap.UserData = TempY->UserData;
			TempY->StartAddr = TempB->StartAddr;
			TempY->Size = TempB->Size;
			TempY->UserData = TempB->UserData;
			TempB->StartAddr = Swap.StartAddr;
			TempB->Size = Swap.Size;
			TempB->UserData = Swap.UserData;
			// Reloop, but don't advance A/B pair -- this way all pairs are tested
		}
	}

	// Purge us and fill ourselves with the new sorted range
	PurgeRange();

	// Copy the new range structure to this range structure
	if (TempRange.Next != NULL) {
		// Copy the first record
		StartAddr = TempRange.Next->StartAddr;
		Size = TempRange.Next->Size;
		UserData = TempRange.Next->UserData;
		// And concatenate the rest
		try {
			Concatenate(TempRange.Next->Next);
		}
		catch (EXCEPTION_ERROR*) {
			throw;
		}
	}

	// Clean memory:
	TempRange.PurgeRange();
}

//	RemoveOverlaps
//		The following function traverses the range and removes
//		overlaps.  If overlaps exist, the offending range
//		entries are replaced by a single range spanning the same
//		total range.  Note that the order of the ranges will
//		change if overlaps exist and are not deterministic.
//		Also, the UserData value must be IDENTICAL on the
//		offending ranges, otherwise, they are not changed.
//    Supports:   ERR_OUT_OF_MEMORY throwback
void TMemRange::RemoveOverlaps(void)
{
	TMemRange	TempRange(0, 0, 0);

	TMemRange*	TempA;
	TMemRange*	TempB;
	TMemRange*	TempX;
	TMemRange*	TempY;
	unsigned long	StartA, EndA;
	unsigned long	StartB, EndB;
	int	Flag;

	try {
		TempRange.Concatenate(this);
	}
	catch (EXCEPTION_ERROR*) {
		throw;
	}

	TempA = &TempRange;
	TempB = TempA->Next;

	while (TempB != NULL) {
		TempX=TempB;
		TempY=TempB->Next;

		StartA = TempB->StartAddr;
		EndA = StartA+TempB->Size;
		Flag = 0;

		while ((!Flag) && (TempY != NULL)) {
			StartB = TempY->StartAddr;
			EndB = StartB+TempY->Size;

			if ((TempB->UserData == TempY->UserData) &&
			    ( ((StartA>=StartB) && (StartA<EndB)) ||
				   ((EndA>=StartB) && (EndA<EndB)) ) ) {
				Flag = 1;
			} else {
				TempX=TempX->Next;
				TempY=TempY->Next;
			}
		}

		if (!Flag) {
			// If no overlap with this A/B pair, advance -- continue till end
			TempA=TempA->Next;
			TempB=TempB->Next;
		} else {
			if (StartB<StartA) StartA=StartB;
			if (EndB>EndA) EndA=EndB;
			// Consolidate to remove overlap:
			TempB->StartAddr = StartA;
			TempB->Size = EndA-StartA;
			// Remove extraneous range:
			TempX->Next = TempY->Next;
			TempY->Next = NULL;		// Nullify to avoid false delete
			delete TempY;
			TempY = TempX->Next;
			// Reloop, but don't advance A/B pair -- this way all pairs are tested
		}
	}

	// Purge us and fill ourselves with the new compressed range
	PurgeRange();

	// Copy the new range structure to this range structure
	if (TempRange.Next != NULL) {
		// Copy the first record
		StartAddr = TempRange.Next->StartAddr;
		Size = TempRange.Next->Size;
		UserData = TempRange.Next->UserData;
		// And concatenate the rest
		try {
			Concatenate(TempRange.Next->Next);
		}
		catch (EXCEPTION_ERROR*) {
			throw;
		}
	}

	// Clean memory:
	TempRange.PurgeRange();
}

//	Compact
//		This function removes any size-zero entries within
//		the range, unless of course there is only one range (this one)
//		and it has a zero size.
void TMemRange::Compact(void)
{
	TMemRange*	TempA;
	TMemRange*	TempB;

	TempA = this;
	TempB = Next;
	while (TempB != NULL) {
		if (TempB->Size == 0) {
			TempA->Next = TempB->Next;
			TempB->Next = NULL;		// Nullify to avoid false delete
			delete TempB;
			TempB = TempA->Next;
		} else {
			TempA = TempA->Next;
			TempB = TempB->Next;
		}
	}
	// If this one is 0, and we have more after it, rotate data up
	if ((Size == 0) && (Next != NULL)) {
		TempA = Next;
		StartAddr = TempA->StartAddr;
		Size = TempA->Size;
		UserData = TempA->UserData;
		Next = TempA->Next;
		TempA->Next = NULL;			// Nullify to avoid false delete
		delete TempA;
	}
}

//	Concatenate
//		The following traverses the passed in range and appends
//		it to the end of this range.
//    Supports:   ERR_OUT_OF_MEMORY throwback
void TMemRange::Concatenate(TMemRange* aRange)
{
	TMemRange*	TempRange;

	TempRange = aRange;
	while (TempRange != NULL) {
		try {
			if (TempRange->Size != 0) {
				AddRange(TempRange->StartAddr, TempRange->Size,
												TempRange->UserData);
			}
		}
		catch (EXCEPTION_ERROR*) {
			throw;
		}
		TempRange = TempRange->Next;
	}
}

// GetStartAddr
//    This function returns the protected StartAddr element.
unsigned long TMemRange::GetStartAddr(void)
{
   return(StartAddr);
}

// SetStartAddr
//    This function sets the protected StartAddr element
void TMemRange::SetStartAddr(unsigned long aStartAddr)
{
   StartAddr=aStartAddr;
}

// GetSize
//    This function returns the protected Size element.
unsigned long TMemRange::GetSize(void)
{
   return(Size);
}

// SetSize
//    This function sets the protected Size element.
void TMemRange::SetSize(unsigned long aSize)
{
   Size=aSize;
}

// GetNext
//    Returns the next pointer
TMemRange *TMemRange::GetNext(void)
{
   return(Next);
}

// SetNext
//    Sets the next pointer
void TMemRange::SetNext(TMemRange *aNext)
{
   Next=aNext;
}

//	GetUserData
//		This function returns the user-set data.  User-set data
//	is just to allow additional storage for the user without
//	having to allocate additional ranges -- sorta like user
//	data in listboxes, etc.  Useful to correlate ranges with
//	other objects and events.
unsigned long TMemRange::GetUserData(void)
{
	return (UserData);
}

//	SetUserData
//		This function sets the user-set data. User-set data
//	is just to allow additional storage for the user without
//	having to allocate additional ranges -- sorta like user
//	data in listboxes, etc.  Useful to correlate ranges with
//	other objects and events.
void TMemRange::SetUserData(unsigned long aUserData)
{
	UserData = aUserData;
}

// AddRange
//    This function allocates a new Range object based on the passed
//    parameters and then follows through the chain of Nexts recursively
//    to add it to the TAIL.
//    Supports:   ERR_OUT_OF_MEMORY throwback
TMemRange *TMemRange::AddRange(unsigned long aStartAddr, unsigned long aSize, unsigned long aUserData)
{
   TMemRange         *RetVal;

   RetVal=NULL;
   if (Next!=NULL) {
      try {
         RetVal=Next->AddRange(aStartAddr, aSize, aUserData);
      }
      catch (EXCEPTION_ERROR*) {
         throw;
      }
   } else {
      RetVal=new TMemRange(aStartAddr, aSize, aUserData);
      if (RetVal!=NULL) {
         Next=RetVal;
      } else {
         THROW_MEMORY_EXCEPTION_ERROR(0);
      }
   }
   return(RetVal);
}


// The following are the function declarations for TMappingObject

// TMappingObject -- Constructor
//    The following constructs a TMappingObject by setting the LogicalAddr,
//    BlockNumber, and Physical Address, and nullifying the next pointer.
TMappingObject::TMappingObject(unsigned long ALogicalAddr, unsigned int ABlockNumber, unsigned long APhysAddr)
{
   LogicalAddr = ALogicalAddr;
   BlockNumber = ABlockNumber;
   PhysicalAddr = APhysAddr;
   Next=NULL;
}

// TMappingObject -- Duplication Constructor
//    Works sorta like 'strdup'.  It allocates memory for a new TMappingObject,
//    copies itself to the new object, and returns a pointer to the new one.
//    Also, this function is recursive -- if the object this function is called
//    for has a valid next pointer, it calls the Duplication Constructor with
//		the next to duplicate the nexts in the newly created object.
//    Supports:   ERR_OUT_OF_MEMORY throwback.
TMappingObject::TMappingObject(TMappingObject& aMap)
{
   LogicalAddr = aMap.LogicalAddr;
   BlockNumber = aMap.BlockNumber;
   PhysicalAddr = aMap.PhysicalAddr;
   Next=NULL;

   try {
		if (aMap.Next!=NULL) {
      	Next=new TMappingObject(*aMap.Next);
         if (Next==NULL) THROW_MEMORY_EXCEPTION_ERROR(0);
      }
   }
   catch (EXCEPTION_ERROR*) {
		if (Next!=NULL) {
			delete Next;
			Next=NULL;
		}
      throw;
   }
}

// TMappingObject -- Destructor
//    The following destructs a TMappingObject by checking to see if Next
//    points to a valid pointer to a linked list of MappingObjects.  If it
//    does, it recursively destroys additionally allocated MappingObjects.
TMappingObject::~TMappingObject(void)
{
   if (Next!=NULL) {
      delete Next;
      Next=NULL;
   }
}

// GetLogicalAddr
//    Returns the protected LogicalAddr element.
unsigned long TMappingObject::GetLogicalAddr(void)
{
   return LogicalAddr;
}

// GetBlockNumber
//    Returns the protected BlockNumber element.
unsigned int TMappingObject::GetBlockNumber(void)
{
   return BlockNumber;
}

// GetPhysicalAddr
//    Returns the protected PhysicalAddr element.
unsigned long TMappingObject::GetPhysicalAddr(void)
{
   return PhysicalAddr;
}

// SetNext
//    Sets the Next pointer.
void TMappingObject::SetNext(TMappingObject *ANext)
{
   Next = ANext;
}

// GetNext
//    Returns the protected Next pointer element.
TMappingObject *TMappingObject::GetNext(void)
{
   return Next;
}


// AddMapping
//    This function allocates a new TMappingObject based on the passed
//    parameters and then follows through the chain of Nexts recursively
//    to add it to the TAIL.
//    Supports:   ERR_OUT_OF_MEMORY throwback
TMappingObject *TMappingObject::AddMapping(unsigned long ALogicalAddr, unsigned int ABlockNumber, unsigned long APhysAddr)
{
   TMappingObject    *RetVal;

   RetVal=NULL;
   if (Next!=NULL) {
      try {
         RetVal=Next->AddMapping(ALogicalAddr, ABlockNumber, APhysAddr);
      }
      catch (EXCEPTION_ERROR*) {
         throw;
      }
   } else {
      RetVal=new TMappingObject(ALogicalAddr, ABlockNumber, APhysAddr);
      if (RetVal!=NULL) {
         Next=RetVal;
      } else {
         THROW_MEMORY_EXCEPTION_ERROR(0);
      }
   }
   return(RetVal);
}


// The following are the function declarations for TMemObject

// TMemObject -- Constructor
//    The following constructs a TMemObject by setting NumBlocks and BlockSize,
//    allocates the necessary memory for the object, and calls ClearMemory to
//    set all memory elements to FillByte with a descriptor of 0.  And it
//    nullifies the BlockMap pointer.
//    Supports:   ERR_OUT_OF_MEMORY throwback.
TMemObject::TMemObject(unsigned int NBlocks, unsigned long BSize, unsigned char FillByte, int UseDesc)
{
   try {
      CreateMemObject(NBlocks, BSize, FillByte, UseDesc);
   }
   catch (EXCEPTION_ERROR*) {
      throw;
   }
}

// TMemObject -- Duplicating Constructor
//    Works sorta like 'strdup'.  It allocates memory for a new TMemObject,
//    copies itself to the new object, and returns a pointer to the new one.
//    If BlockMap is a valid BlockMap pointer, it calls the MappingObject
//		Duplicating Contructor to duplicate the mapping as well.
//    Supports:   ERR_OUT_OF_MEMORY throwback.
TMemObject::TMemObject(TMemObject& aMem)
{
   unsigned long i;

   try {
      CreateMemObject(aMem.NumBlocks, aMem.BlockSize, 0x00,
            ((aMem.Descriptors!=NULL) ? 1 : 0));
   }
   catch (EXCEPTION_ERROR*) {
      throw;
   }

   for (i=0; (i<GetMemSize()); ++i) {
      if (aMem.Data)
   		Data[i]=aMem.Data[i];
      if (aMem.Descriptors)
         Descriptors[i]=aMem.Descriptors[i];
   }

   try {
      if (aMem.BlockMap!=NULL) BlockMap=new TMappingObject(*aMem.BlockMap);
   }
   catch (EXCEPTION_ERROR*) {
		// Cleanup on error
	   ClearMapping();              // Dispose of all mapping objects
	   if (Descriptors) delete[] Descriptors;		// Free Allocated memory
	   if (Data) delete[] Data;
      throw;
   }
}


// TMemObject -- Constructor From Range
//    This constructor creates a copy of the passed in range,
//    consolidates it to find the actual bounds of the range,
//    and then creates a Memory Object capable of holding
//    the entire range.  The Memory Object will consist of
//    a single block of the calculated size and a mapping
//    object will be created and attached to the memory
//    in accordance with the range.
TMemObject::TMemObject(TMemRange& aRange, unsigned char FillByte, int UseDesc)
{
   TMemRange         *TempRange;

   try {
      TempRange = new TMemRange(aRange);
   }
   catch (EXCEPTION_ERROR*) {
      throw;
   }

   if (TempRange) {
      TempRange->Consolidate();
      try {
         // Create the memory and setup the mapping
         CreateMemObject(1,TempRange->GetSize(), FillByte, UseDesc);
      }
      catch (EXCEPTION_ERROR*) {
			// Cleanup on error
			delete TempRange;
         throw;
      }
		try {
         AddMapping(TempRange->GetStartAddr(), 0, TempRange->GetStartAddr());
		}
		catch (EXCEPTION_ERROR*) {
			// Cleanup if we encounter an error
		   ClearMapping();               // Dispose of all mapping objects
		   if (Descriptors) delete[] Descriptors;		// Free Allocated memory
		   if (Data) delete[] Data;
			throw;
		}
   }

   delete TempRange;
}

// TMemObject -- Destructor
//    The following destructs a TMemObject by calling ClearMapping to
//    destroy all associated mapping objects and then it frees all memory
//    allocated for the memory object.
TMemObject::~TMemObject(void)
{
   ClearMapping();               // Dispose of all mapping objects
   if (Descriptors) delete[] Descriptors;		// Free Allocated memory
   if (Data) delete[] Data;
}

// CreateMemObject
//    This function creates a TMemObject by allocating and initializing
//    necessary memory.  It is here in its own function so that all the
//    various overloaded constructors can call it.
void TMemObject::CreateMemObject(unsigned int NBlocks, unsigned long BSize, unsigned char FillByte, int UseDesc)
{
   Data = new unsigned char[NBlocks*BSize];
   if (Data == NULL) THROW_MEMORY_EXCEPTION_ERROR(0);     // Report Error if out of memory
   if (UseDesc) {
	  Descriptors = new unsigned char[NBlocks*BSize];
      if (Descriptors == NULL) {
         delete[] Data;
         THROW_MEMORY_EXCEPTION_ERROR(0);		// Report Error if out of memory
      }
   } else Descriptors = NULL;
   NumBlocks=NBlocks;
   BlockSize=BSize;
   BlockMap=NULL;
   ClearMemory(FillByte);
}

// GetByte
//    Returns the memory data byte addressed by the passed in Logical Address,
//    based on the current Block Mapping.
unsigned char TMemObject::GetByte(unsigned long LogicalAddr)
{
   TMappingObject *Temp_Ptr;

   if (Data) {
      Temp_Ptr = BlockMap;
      while (Temp_Ptr != NULL) {
         if ((LogicalAddr>=Temp_Ptr->GetLogicalAddr()) &&
             (LogicalAddr<(Temp_Ptr->GetLogicalAddr()+BlockSize))) {
            return (Data[(LogicalAddr-Temp_Ptr->GetLogicalAddr())+(Temp_Ptr->GetBlockNumber()*BlockSize)]);
         }
         Temp_Ptr=Temp_Ptr->GetNext();
      }
   }
   return (0);
}

// SetByte
//    Sets the memory data byte addressed by the passed in Logical Address,
//    based on the current Block Mapping to the passed in Byte.
int TMemObject::SetByte(unsigned long LogicalAddr, unsigned char Byte)
{
   TMappingObject *Temp_Ptr;

   if (Data) {
      Temp_Ptr = BlockMap;
      while (Temp_Ptr != NULL) {
         if ((LogicalAddr>=Temp_Ptr->GetLogicalAddr()) &&
             (LogicalAddr<(Temp_Ptr->GetLogicalAddr()+BlockSize))) {
            Data[(LogicalAddr-Temp_Ptr->GetLogicalAddr())+(Temp_Ptr->GetBlockNumber()*BlockSize)]=Byte;
            return(1);
         }
         Temp_Ptr=Temp_Ptr->GetNext();
      }
   }
   return(0);
}

// GetDescriptor
//    Returns the memory descriptor byte addressed by the passed in Logical Address,
//    based on the current Block Mapping.
unsigned char TMemObject::GetDescriptor(unsigned long LogicalAddr)
{
   TMappingObject *Temp_Ptr;

   if (Descriptors) {
      Temp_Ptr = BlockMap;
      while (Temp_Ptr != NULL) {
         if ((LogicalAddr>=Temp_Ptr->GetLogicalAddr()) &&
             (LogicalAddr<(Temp_Ptr->GetLogicalAddr()+BlockSize))) {
            return (Descriptors[(LogicalAddr-Temp_Ptr->GetLogicalAddr())+(Temp_Ptr->GetBlockNumber()*BlockSize)]);
         }
         Temp_Ptr=Temp_Ptr->GetNext();
      }
   }
   return (0);
}

// SetDescriptor
//    Sets the memory descriptor byte addressed by the passed in Logical Address,
//    based on the current Block Mapping to the passed in Desc.
int TMemObject::SetDescriptor(unsigned long LogicalAddr, unsigned char Desc)
{
   TMappingObject *Temp_Ptr;

   if (Descriptors) {
      Temp_Ptr = BlockMap;
      while (Temp_Ptr != NULL) {
         if ((LogicalAddr>=Temp_Ptr->GetLogicalAddr()) &&
             (LogicalAddr<(Temp_Ptr->GetLogicalAddr()+BlockSize))) {
            Descriptors[(LogicalAddr-Temp_Ptr->GetLogicalAddr())+(Temp_Ptr->GetBlockNumber()*BlockSize)]=Desc;
            return(1);
         }
         Temp_Ptr=Temp_Ptr->GetNext();
      }
   }
   return(0);
}

// GetMemSize
//    Returns the size of the memory allocated for data bytes.
unsigned long TMemObject::GetMemSize(void)
{
   return(NumBlocks*BlockSize);
}

// ClearMemory
//    Sets all allocated data bytes to the specified FillByte and sets
//    all corresponding descriptor bytes to 0.
void TMemObject::ClearMemory(unsigned char FillByte)
{
   unsigned long i;

   for (i=0; (i<GetMemSize()); ++i) {
      if (Data)
         Data[i]=FillByte;
      if (Descriptors)
         Descriptors[i]=0;
   }
}

// ClearMapping
//    If BlockMap is not NULL, it destroys the object (and its child
//    objects) that it points to, and sets BlockMap to NULL.
void TMemObject::ClearMapping(void)
{
   if (BlockMap != NULL) {
      delete BlockMap;
      BlockMap=NULL;
   }
}

// AddMapping
//    Adds a mapping object at the TAIL of the BlockMap IF the defined object
//    is within range of the allocated Memory space.
//    Supports:   ERR_OUT_OF_MEMORY throwback
//                ERR_OUT_OF_RANGE throwback (IF MAPTHROW is defined)
//                ERR_MAPPING_OVERLAP throwback (IF MAPTHROW is defined)
void TMemObject::AddMapping(unsigned long ALogicalAddr, unsigned int ABlockNumber, unsigned long APhysAddr)
{
   TMappingObject *Temp_Ptr;

#ifdef MAPTHROW
   if (ABlockNumber>=NumBlocks) THROW_EXCEPTION_ERROR(EXCEPTION_ERROR::ERR_OUT_OF_RANGE, 0, "");
#endif
   if (ABlockNumber>=NumBlocks) return;
#ifdef MAPTHROW
   Temp_Ptr = BlockMap;
   while (Temp_Ptr != NULL) {
      if ((ALogicalAddr>=Temp_Ptr->GetLogicalAddr()) &&
          (ALogicalAddr<(Temp_Ptr->GetLogicalAddr()+BlockSize))) {
          break;
      }
      Temp_Ptr=Temp_Ptr->GetNext();
   }
   if (Temp_Ptr != NULL) THROW_EXCEPTION_ERROR(EXCEPTION_ERROR::ERR_MAPPING_OVERLAP, 0, "");
#endif
   if (BlockMap==NULL) {
      Temp_Ptr = new TMappingObject(ALogicalAddr, ABlockNumber, APhysAddr);
      if (Temp_Ptr == NULL) THROW_MEMORY_EXCEPTION_ERROR(0);    // Return error if no memory
      BlockMap=Temp_Ptr;
   } else {
      try {
         BlockMap->AddMapping(ALogicalAddr, ABlockNumber, APhysAddr);
      }
      catch (EXCEPTION_ERROR*) {
         throw;
      }
   }
}

// GetPhysicalAddr
//    Looks the LogicalAddr up in the defined BlockMap and returns the
//    equivalent PhysicalAddr.
unsigned long TMemObject::GetPhysicalAddr(unsigned long LogicalAddr)
{
   TMappingObject *Temp_Ptr;

   Temp_Ptr = BlockMap;
   while (Temp_Ptr != NULL) {
      if ((LogicalAddr>=Temp_Ptr->GetLogicalAddr()) &&
          (LogicalAddr<(Temp_Ptr->GetLogicalAddr()+BlockSize))) {
         return (Temp_Ptr->GetPhysicalAddr()+(LogicalAddr-Temp_Ptr->GetLogicalAddr()));
      }
      Temp_Ptr=Temp_Ptr->GetNext();
   }
   return (0x0000);
}

