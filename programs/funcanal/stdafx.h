// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//
// $Log: stdafx.h,v $
// Revision 1.1  2002/07/01 05:50:02  dewhisna
// Initial Revision
//
//

#if !defined(AFX_STDAFX_H__8A6A4117_76AF_42B3_B639_E6120063A4C6__INCLUDED_)
#define AFX_STDAFX_H__8A6A4117_76AF_42B3_B639_E6120063A4C6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afx.h>
//#include <afxwin.h>         // MFC core and standard components
//#include <afxext.h>         // MFC extensions
//#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
//#ifndef _AFX_NO_AFXCMN_SUPPORT
//#include <afxcmn.h>			// MFC support for Windows Common Controls
//#endif // _AFX_NO_AFXCMN_SUPPORT
//
//#include <iostream>

#include <math.h>
#include <limits.h>
#include <float.h>
#include <afxtempl.h>		// Template Classes
#include <afxcoll.h>		// Collection Classes

#ifdef LINUX
#define cprintf printf
#include <unistd.h>
#else
#include <io.h>
#include <conio.h>
#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__8A6A4117_76AF_42B3_B639_E6120063A4C6__INCLUDED_)
