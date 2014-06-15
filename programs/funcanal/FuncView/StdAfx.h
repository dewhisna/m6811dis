// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//
// $Log: StdAfx.h,v $
// Revision 1.1  2003/09/13 05:45:49  dewhisna
// Initial Revision
//
//

#if !defined(AFX_STDAFX_H__EBE47A17_CB62_474D_9F1B_6D1CE2070A3E__INCLUDED_)
#define AFX_STDAFX_H__EBE47A17_CB62_474D_9F1B_6D1CE2070A3E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxcview.h>
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <math.h>
#include <limits.h>
#include <float.h>
#include <afxtempl.h>		// Template Classes
#include <afxcoll.h>		// MFC Collection Classes

#ifdef LINUX
#define cprintf printf
#include <unistd.h>
#else
#include <io.h>
#include <conio.h>
#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__EBE47A17_CB62_474D_9F1B_6D1CE2070A3E__INCLUDED_)
