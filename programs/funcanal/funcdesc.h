// funcdesc.h: interface for the CFuncDesc, CFuncDescFile, and CFuncObject classes.
//
// $Log: funcdesc.h,v $
// Revision 1.4  2003/09/13 05:38:48  dewhisna
// Added Read/Write File support.  Added output options on output line creation
// for function diff and added returning of match percentage.  Added callback
// function for read operation feedback.  Enhanced symbol map manipulation.
//
// Revision 1.3  2003/02/09 06:59:57  dewhisna
// Split comparison logic from Function File I/O
//
// Revision 1.2  2002/09/15 22:54:03  dewhisna
// Added Symbol Table comparison support
//
// Revision 1.1  2002/07/01 05:50:01  dewhisna
// Initial Revision
//
//

#if !defined(AFX_FUNCDESC_H__E6CF1B55_6B17_4909_A17F_B572ED0ED0E6__INCLUDED_)
#define AFX_FUNCDESC_H__E6CF1B55_6B17_4909_A17F_B572ED0ED0E6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <MemClass.h>
#include "funccomp.h"

// Foward Declarations:
class CFuncDesc;
class CFuncDescFile;
class CSymbolMap;


#define CALC_FIELD_WIDTH(x) x

//////////////////////////////////////////////////////////////////////
// Global Functions
//////////////////////////////////////////////////////////////////////

extern CString PadString(LPCTSTR pszString, int nWidth);


//////////////////////////////////////////////////////////////////////
// Callback Functions
//////////////////////////////////////////////////////////////////////

// Function Analysis Progress Callback -- Used in functions that take
//		a while to do processing (like reading a file).  Passes
//		current progress position, maximum progress value, a flag
//		indicating if the user/client can currently cancel operation,
//		and a client defined lParam.  Returns True/False Cancel Status.
//		True=Cancel, False=Continue.  If progress indexes aren't supported
//		then nProgressPos will always be 0 and nProgressMax always 1.
typedef BOOL (CALLBACK * LPFUNCANALPROGRESSCALLBACK)(int nProgressPos, int nProgressMax, BOOL bAllowCancel, LPARAM lParam);


//////////////////////////////////////////////////////////////////////
// CFuncObject Class
//////////////////////////////////////////////////////////////////////
//		This specifies a pure virtual base class for defining
//		Function Objects:
class CFuncObject : public CObject
{
public:
	CFuncObject(CFuncDescFile &ParentFuncFile, CFuncDesc &ParentFunc, CStringArray &argv);
	virtual ~CFuncObject();

	virtual BOOL IsExactMatch(CFuncObject *pObj);

	virtual BOOL AddLabel(LPCTSTR pszLabel);

	DWORD	GetRelFuncAddress();
	DWORD	GetAbsAddress();
	int		GetLabelCount();
	CString	GetLabel(int nIndex);
	CString	GetBytes();
	int		GetByteCount();

	virtual CString ExportToDiff() = 0;
	virtual void ExportToDiff(CStringArray &anArray) = 0;

	virtual CString CreateOutputLine(DWORD nOutputOptions = 0) = 0;

	virtual void GetSymbols(CStringArray &aSymbolArray);	// Returns specially encoded symbol array (see implementation comments)

	// The following define field positions for obtaining field widths from overrides.
	//	Additional entries can be added on overrides, but this order should NOT change
	//	in order to maintain backward compatibility at all times:
	#define NUM_FIELD_CODES 6
	enum FIELD_CODE { FC_ADDRESS, FC_OPBYTES, FC_LABEL, FC_MNEMONIC, FC_OPERANDS, FC_COMMENT };

	static int GetFieldWidth(FIELD_CODE nFC);

protected:
	CFuncDescFile &m_ParentFuncFile;
	CFuncDesc &m_ParentFunc;

	DWORD	m_dwRelFuncAddress;			// Address of object relative to function start
	DWORD	m_dwAbsAddress;				// Absolute Address of object
	CStringArray	m_LabelTable;		// Labels assigned to this object
	CByteArray	m_Bytes;				// Array of bytes for this object
};

//////////////////////////////////////////////////////////////////////
// CFuncAsmInstObject Class
//////////////////////////////////////////////////////////////////////
//		This class specifies an assembly instruction object
class CFuncAsmInstObject : public CFuncObject
{
public:
	CFuncAsmInstObject(CFuncDescFile &ParentFuncFile, CFuncDesc &ParentFunc, CStringArray &argv);
	virtual ~CFuncAsmInstObject();

	virtual CString ExportToDiff();
	virtual void ExportToDiff(CStringArray &anArray);

	virtual CString CreateOutputLine(DWORD nOutputOptions = 0);

	virtual void GetSymbols(CStringArray &aSymbolArray);	// Returns specially encoded symbol array (see implementation comments)

	CString	GetOpCodeBytes();
	int		GetOpCodeByteCount();

	CString	GetOperandBytes();
	int		GetOperandByteCount();

protected:
	CByteArray m_OpCodeBytes;			// Array of OpCode Bytes for this instruction
	CByteArray m_OperandBytes;			// Array of Operand Bytes for ths instruction
	CString	m_strDstOperand;			// Encoded Destination Operand
	CString	m_strSrcOperand;			// Encoded Source Operand
	CString	m_strOpCodeText;			// Textual OpCode mnemonic
	CString	m_strOperandText;			// Textual Operands
};

//////////////////////////////////////////////////////////////////////
// CFuncDataByteObject Class
//////////////////////////////////////////////////////////////////////
//		This class specifies a function embedded data byte object
class CFuncDataByteObject : public CFuncObject
{
public:
	CFuncDataByteObject(CFuncDescFile &ParentFuncFile, CFuncDesc &ParentFunc, CStringArray &argv);
	virtual ~CFuncDataByteObject();

	virtual CString ExportToDiff();
	virtual void ExportToDiff(CStringArray &anArray);

	virtual CString CreateOutputLine(DWORD nOutputOptions = 0);
};

//////////////////////////////////////////////////////////////////////
// CFuncDesc Class
//////////////////////////////////////////////////////////////////////
//		This specifies exactly one function as an array of CFuncObject
//		objects:
class CFuncDesc : public CTypedPtrArray<CObArray, CFuncObject *>
{
public:
	CFuncDesc(DWORD nAddress, LPCTSTR pszNames);
	virtual ~CFuncDesc();

	void FreeAll();

	virtual BOOL AddName(DWORD nAddress, LPCTSTR pszLabel);
	virtual CString GetMainName();
	virtual DWORD GetMainAddress();

	virtual BOOL AddLabel(DWORD nAddress, LPCTSTR pszLabel);
	virtual BOOL AddrHasLabel(DWORD nAddress);
	virtual CString GetPrimaryLabel(DWORD nAddress);
	virtual CStringArray *GetLabelList(DWORD nAddress);

	virtual DWORD GetFuncSize();		// Returns size of function in byte counts (used for address calculation)

	virtual CString ExportToDiff();
	virtual void ExportToDiff(CStringArray &anArray);

	int Add(CFuncObject *anObj);


	// Warning: This object supports ADDing only!  Removing an item does NOT remove labels
	//			that have been added nor decrement the function size!  To modify a function
	//			description, create a new function object with only the new elements desired!!
	//			This has been done to improve speed efficiency!

protected:
	DWORD m_dwMainAddress;		// Main Address is the address defined at instantiation time.  Additional relocated declarations might be defined in the name list
	DWORD m_dwFunctionSize;		// Count of bytes in function -- used to calculate addressing inside the function

	// Note: The File's Label Table only contains those labels defined
	//			internally in this function and may not directly
	//			correspond to that of the file level.  These might
	//			contain 'L' labels.
	CMap<DWORD, DWORD, CStringArray*, CStringArray*> m_FuncNameTable;	// Table of names for this function.  First entry is typical default
	CMap<DWORD, DWORD, CStringArray*, CStringArray*> m_LabelTable;	// Table of labels in this function.  First entry is typical default
};

//////////////////////////////////////////////////////////////////////
// CFuncDescFile Class
//////////////////////////////////////////////////////////////////////
//		This specifies all of the information from a single Func
//		Description File.  It contains a CFuncDescArray of all of
//		the functions in the file, but also contains the file mapping
//		and label information:
class CFuncDescFile : public CObject
{
public:
	CFuncDescFile();
	virtual ~CFuncDescFile();

	void FreeAll();

	virtual BOOL ReadFuncDescFile(CFile& inFile, CFile *msgFile = NULL, CFile *errFile = NULL, int nStartLineCount = 0);			// Read already open control file 'infile', outputs messages to 'msgFile' and errors to 'errFile', nStartLineCount = initial line counter value

	virtual BOOL AddLabel(DWORD nAddress, LPCTSTR pszLabel);
	virtual BOOL AddrHasLabel(DWORD nAddress);
	virtual CString GetPrimaryLabel(DWORD nAddress);
	virtual CStringArray *GetLabelList(DWORD nAddress);

	virtual int GetFuncCount();
	virtual CFuncDesc *GetFunc(int nIndex);

	CString GetFuncPathName();
	CString GetFuncFileName();

	virtual BOOL IsROMMemAddr(DWORD nAddress);
	virtual BOOL IsRAMMemAddr(DWORD nAddress);
	virtual BOOL IsIOMemAddr(DWORD nAddress);

	virtual SetProgressCallback(LPFUNCANALPROGRESSCALLBACK pfnCallback, LPARAM lParam)
			{ m_pfnProgressCallback = pfnCallback;  m_lParamProgressCallback = lParam; }

protected:
	CString		m_strFilePathName;
	CString		m_strFileName;

	TMemRange	m_ROMMemMap;		// Mapping of ROM memory
	TMemRange	m_RAMMemMap;		// Mapping of RAM memory
	TMemRange	m_IOMemMap;			// Mapping of memory mapped I/O

	LPFUNCANALPROGRESSCALLBACK m_pfnProgressCallback;
	LPARAM		m_lParamProgressCallback;

	// Note: The File's Label Table only contains those labels defined
	//			at the file level by "!" entries.  NOT those down at the
	//			individual function level.  Specifically it doesn't
	//			include 'L' names.
	CMap<DWORD, DWORD, CStringArray*, CStringArray*> m_LabelTable;	// Table of labels.  First entry is typical default
	CTypedPtrArray<CObArray, CFuncDesc*> m_Functions;	// Array of functions in the file

private:
	CMap<CString, LPCTSTR, WORD, WORD>	ParseCmdsOP1;	// Used internally to store control file commands
};

//////////////////////////////////////////////////////////////////////
// CFuncDescFileArray Class
//////////////////////////////////////////////////////////////////////
//		This specifies a cluster of Func Definition Files as an array
//		of CFuncDescFile Objects.
class CFuncDescFileArray : public CTypedPtrArray<CObArray, CFuncDescFile *>
{
public:
	CFuncDescFileArray();
	virtual ~CFuncDescFileArray();

	void FreeAll();

	virtual int GetFuncCount();

	virtual double CompareFunctions(FUNC_COMPARE_METHOD nMethod,
									int nFile1Ndx, int nFile1FuncNdx,
									int nFile2Ndx, int nFile2FuncNdx,
									BOOL bBuildEditScript);
	virtual CString DiffFunctions(FUNC_COMPARE_METHOD nMethod,
									int nFile1Ndx, int nFile1FuncNdx,
									int nFile2Ndx, int nFile2FuncNdx,
									DWORD nOutputOptions,
									double &nMatchPercent,
									CSymbolMap *pSymbolMap = NULL);

	virtual SetProgressCallback(LPFUNCANALPROGRESSCALLBACK pfnCallback, LPARAM lParam)
			{ m_pfnProgressCallback = pfnCallback;  m_lParamProgressCallback = lParam; }

protected:
	LPFUNCANALPROGRESSCALLBACK m_pfnProgressCallback;
	LPARAM		m_lParamProgressCallback;
};


//////////////////////////////////////////////////////////////////////
// CSymbolMap Class
//////////////////////////////////////////////////////////////////////
//		This class stores mapping between two sets of functions by
//		hashing and storing address accesses and finding highest
//		hit matches.
class CSymbolMap : public CObject
{
public:
	CSymbolMap();
	virtual ~CSymbolMap();

	void FreeAll();
	BOOL IsEmpty();

	void AddObjectMapping(CFuncObject &aLeftObject, CFuncObject &aRightObject);

	void GetLeftSideCodeSymbolList(CStringArray &anArray);		// Returns sorted list of left-side code symbols
	void GetRightSideCodeSymbolList(CStringArray &anArray);		// Returns sorted list of right-side code symbols
	void GetLeftSideDataSymbolList(CStringArray &anArray);		// Returns sorted list of left-side data symbols
	void GetRightSideDataSymbolList(CStringArray &anArray);		// Returns sorted list of right-side data symbols

	DWORD GetLeftSideCodeHitList(LPCTSTR aSymbol, CStringArray &aSymbolArray, CDWordArray &aHitCountArray);		// Returns sorted array of target hit symbols and corresponding counts, function returns total hit count
	DWORD GetRightSideCodeHitList(LPCTSTR aSymbol, CStringArray &aSymbolArray, CDWordArray &aHitCountArray);	// Returns sorted array of target hit symbols and corresponding counts, function returns total hit count
	DWORD GetLeftSideDataHitList(LPCTSTR aSymbol, CStringArray &aSymbolArray, CDWordArray &aHitCountArray);		// Returns sorted array of target hit symbols and corresponding counts, function returns total hit count
	DWORD GetRightSideDataHitList(LPCTSTR aSymbol, CStringArray &aSymbolArray, CDWordArray &aHitCountArray);	// Returns sorted array of target hit symbols and corresponding counts, function returns total hit count

private:
	void GetSymbolList(CMap<CString, LPCTSTR, CStringArray*, CStringArray*> &mapSymbolArrays,
						CStringArray &anArray);
	DWORD GetHitList(CMap<CString, LPCTSTR, CStringArray*, CStringArray*> &mapSymbolArrays,
						LPCTSTR aSymbol, CStringArray &aSymbolArray, CDWordArray &aHitCountArray);

//	void SortStringArray(CStringArray &aStringArray);
//	void SortStringDWordArray(CStringArray &aStringArray, CDWordArray &aDWordArray);

protected:
	void AddLeftSideCodeSymbol(LPCTSTR aLeftSymbol, LPCTSTR aRightSymbol);
	void AddRightSideCodeSymbol(LPCTSTR aRightSymbol, LPCTSTR aLeftSymbol);
	void AddLeftSideDataSymbol(LPCTSTR aLeftSymbol, LPCTSTR aRightSymbol);
	void AddRightSideDataSymbol(LPCTSTR aRightSymbol, LPCTSTR aLeftSymbol);

	CMap<CString, LPCTSTR, CStringArray*, CStringArray*> m_LeftSideCodeSymbols;
	CMap<CString, LPCTSTR, CStringArray*, CStringArray*> m_RightSideCodeSymbols;
	CMap<CString, LPCTSTR, CStringArray*, CStringArray*> m_LeftSideDataSymbols;
	CMap<CString, LPCTSTR, CStringArray*, CStringArray*> m_RightSideDataSymbols;
};




#endif // !defined(AFX_FUNCDESC_H__E6CF1B55_6B17_4909_A17F_B572ED0ED0E6__INCLUDED_)

