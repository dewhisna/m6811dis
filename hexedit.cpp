// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


#include "stdafx.h"
#include "hexedit.h"

// Dispatch interfaces referenced by this interface
#include "font.h"

/////////////////////////////////////////////////////////////////////////////
// CHexEdit

IMPLEMENT_DYNCREATE(CHexEdit, CWnd)

/////////////////////////////////////////////////////////////////////////////
// CHexEdit properties

COleFont CHexEdit::GetFont()
{
	LPDISPATCH pDispatch;
	GetProperty(DISPID_FONT, VT_DISPATCH, (void*)&pDispatch);
	return COleFont(pDispatch);
}

void CHexEdit::SetFont(LPDISPATCH propVal)
{
	SetProperty(DISPID_FONT, VT_DISPATCH, propVal);
}

OLE_COLOR CHexEdit::GetForeColor()
{
	OLE_COLOR result;
	GetProperty(DISPID_FORECOLOR, VT_I4, (void*)&result);
	return result;
}

void CHexEdit::SetForeColor(OLE_COLOR propVal)
{
	SetProperty(DISPID_FORECOLOR, VT_I4, propVal);
}

OLE_COLOR CHexEdit::GetBackColor()
{
	OLE_COLOR result;
	GetProperty(DISPID_BACKCOLOR, VT_I4, (void*)&result);
	return result;
}

void CHexEdit::SetBackColor(OLE_COLOR propVal)
{
	SetProperty(DISPID_BACKCOLOR, VT_I4, propVal);
}

BOOL CHexEdit::GetEnabled()
{
	BOOL result;
	GetProperty(DISPID_ENABLED, VT_BOOL, (void*)&result);
	return result;
}

void CHexEdit::SetEnabled(BOOL propVal)
{
	SetProperty(DISPID_ENABLED, VT_BOOL, propVal);
}

short CHexEdit::GetByteGrouping()
{
	short result;
	GetProperty(0x1, VT_I2, (void*)&result);
	return result;
}

void CHexEdit::SetByteGrouping(short propVal)
{
	SetProperty(0x1, VT_I2, propVal);
}

BOOL CHexEdit::GetShowASCII()
{
	BOOL result;
	GetProperty(0x2, VT_BOOL, (void*)&result);
	return result;
}

void CHexEdit::SetShowASCII(BOOL propVal)
{
	SetProperty(0x2, VT_BOOL, propVal);
}

BOOL CHexEdit::GetEditASCII()
{
	BOOL result;
	GetProperty(0x3, VT_BOOL, (void*)&result);
	return result;
}

void CHexEdit::SetEditASCII(BOOL propVal)
{
	SetProperty(0x3, VT_BOOL, propVal);
}

BOOL CHexEdit::GetReadOnly()
{
	BOOL result;
	GetProperty(0x4, VT_BOOL, (void*)&result);
	return result;
}

void CHexEdit::SetReadOnly(BOOL propVal)
{
	SetProperty(0x4, VT_BOOL, propVal);
}

long CHexEdit::GetMemory()
{
	long result;
	GetProperty(0x5, VT_I4, (void*)&result);
	return result;
}

void CHexEdit::SetMemory(long propVal)
{
	SetProperty(0x5, VT_I4, propVal);
}

short CHexEdit::GetMemModMask()
{
	short result;
	GetProperty(0x6, VT_I2, (void*)&result);
	return result;
}

void CHexEdit::SetMemModMask(short propVal)
{
	SetProperty(0x6, VT_I2, propVal);
}

long CHexEdit::GetMemoryRange()
{
	long result;
	GetProperty(0x7, VT_I4, (void*)&result);
	return result;
}

void CHexEdit::SetMemoryRange(long propVal)
{
	SetProperty(0x7, VT_I4, propVal);
}

short CHexEdit::GetDisplayWidth()
{
	short result;
	GetProperty(0x8, VT_I2, (void*)&result);
	return result;
}

void CHexEdit::SetDisplayWidth(short propVal)
{
	SetProperty(0x8, VT_I2, propVal);
}

unsigned long CHexEdit::GetModifiedTextColor()
{
	unsigned long result;
	GetProperty(0x9, VT_I4, (void*)&result);
	return result;
}

void CHexEdit::SetModifiedTextColor(unsigned long propVal)
{
	SetProperty(0x9, VT_I4, propVal);
}

unsigned long CHexEdit::GetModifiedTextBackColor()
{
	unsigned long result;
	GetProperty(0xa, VT_I4, (void*)&result);
	return result;
}

void CHexEdit::SetModifiedTextBackColor(unsigned long propVal)
{
	SetProperty(0xa, VT_I4, propVal);
}

unsigned long CHexEdit::GetHighlightedTextColor()
{
	unsigned long result;
	GetProperty(0xb, VT_I4, (void*)&result);
	return result;
}

void CHexEdit::SetHighlightedTextColor(unsigned long propVal)
{
	SetProperty(0xb, VT_I4, propVal);
}

unsigned long CHexEdit::GetHighlightedTextBackColor()
{
	unsigned long result;
	GetProperty(0xc, VT_I4, (void*)&result);
	return result;
}

void CHexEdit::SetHighlightedTextBackColor(unsigned long propVal)
{
	SetProperty(0xc, VT_I4, propVal);
}

unsigned long CHexEdit::GetHighlightedModTextColor()
{
	unsigned long result;
	GetProperty(0xd, VT_I4, (void*)&result);
	return result;
}

void CHexEdit::SetHighlightedModTextColor(unsigned long propVal)
{
	SetProperty(0xd, VT_I4, propVal);
}

unsigned long CHexEdit::GetHighlightedModTextBackColor()
{
	unsigned long result;
	GetProperty(0xe, VT_I4, (void*)&result);
	return result;
}

void CHexEdit::SetHighlightedModTextBackColor(unsigned long propVal)
{
	SetProperty(0xe, VT_I4, propVal);
}

unsigned long CHexEdit::GetDisabledTextColor()
{
	unsigned long result;
	GetProperty(0xf, VT_I4, (void*)&result);
	return result;
}

void CHexEdit::SetDisabledTextColor(unsigned long propVal)
{
	SetProperty(0xf, VT_I4, propVal);
}

unsigned long CHexEdit::GetDisabledTextBackColor()
{
	unsigned long result;
	GetProperty(0x10, VT_I4, (void*)&result);
	return result;
}

void CHexEdit::SetDisabledTextBackColor(unsigned long propVal)
{
	SetProperty(0x10, VT_I4, propVal);
}

/////////////////////////////////////////////////////////////////////////////
// CHexEdit operations

BOOL CHexEdit::DoProperties()
{
	BOOL result;
	InvokeHelper(0x11, DISPATCH_METHOD, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CHexEdit::AboutBox()
{
	InvokeHelper(0xfffffdd8, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}