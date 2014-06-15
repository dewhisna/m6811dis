// funccomp.h: interface for the Fuzzy Function Comparison Logic
//
// $Log: funccomp.h,v $
// Revision 1.2  2003/09/13 05:39:49  dewhisna
// Added output options and returning of match percentage to function diff.
//
// Revision 1.1  2003/02/09 06:59:30  dewhisna
// Initial Revision - Split comparison logic from Function File I/O
//
//

#ifndef _FUNCCOMP_H_
#define _FUNCCOMP_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CFuncDescFile;
class CSymbolMap;

enum FUNC_COMPARE_METHOD {
		FCM_DYNPROG_XDROP = 0,
		FCM_DYNPROG_GREEDY = 1
};

// Output Options:
//		These are OR'd bit fields of output-options used
//		in diff and CreateOutputLine methods
#define OO_NONE 0
#define OO_ADD_ADDRESS 1

double CompareFunctions(FUNC_COMPARE_METHOD nMethod,
						CFuncDescFile *pFile1, int nFile1FuncNdx,
						CFuncDescFile *pFile2, int nFile2FuncNdx,
						BOOL bBuildEditScript);
BOOL GetLastEditScript(CStringArray &anArray);
CString DiffFunctions(FUNC_COMPARE_METHOD nMethod,
						CFuncDescFile *pFile1, int nFile1FuncNdx,
						CFuncDescFile *pFile2, int nFile2FuncNdx,
						DWORD nOutputOptions,
						double &nMatchPercent,
						CSymbolMap *pSymbolMap = NULL);

#endif	// _FUNCCOMP_H_

