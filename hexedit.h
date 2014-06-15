#if !defined(AFX_HEXEDIT_H__4C6C6582_95D5_11D1_858F_00600828300C__INCLUDED_)
#define AFX_HEXEDIT_H__4C6C6582_95D5_11D1_858F_00600828300C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


// Dispatch interfaces referenced by this interface
class COleFont;

/////////////////////////////////////////////////////////////////////////////
// CHexEdit wrapper class

class CHexEdit : public CWnd
{
protected:
	DECLARE_DYNCREATE(CHexEdit)
public:
	CLSID const& GetClsid()
	{
		static CLSID const clsid
			= { 0x62e8e83, 0x83c7, 0x11d1, { 0x85, 0x8f, 0x0, 0x60, 0x8, 0x28, 0x30, 0xc } };
		return clsid;
	}
	virtual BOOL Create(LPCTSTR lpszClassName,
		LPCTSTR lpszWindowName, DWORD dwStyle,
		const RECT& rect,
		CWnd* pParentWnd, UINT nID,
		CCreateContext* pContext = NULL)
	{ return CreateControl(GetClsid(), lpszWindowName, dwStyle, rect, pParentWnd, nID); }

    BOOL Create(LPCTSTR lpszWindowName, DWORD dwStyle,
		const RECT& rect, CWnd* pParentWnd, UINT nID,
		CFile* pPersist = NULL, BOOL bStorage = FALSE,
		BSTR bstrLicKey = NULL)
	{ return CreateControl(GetClsid(), lpszWindowName, dwStyle, rect, pParentWnd, nID,
		pPersist, bStorage, bstrLicKey); }

// Attributes
public:
	COleFont GetFont();
	void SetFont(LPDISPATCH);
	OLE_COLOR GetForeColor();
	void SetForeColor(OLE_COLOR);
	OLE_COLOR GetBackColor();
	void SetBackColor(OLE_COLOR);
	BOOL GetEnabled();
	void SetEnabled(BOOL);
	short GetByteGrouping();
	void SetByteGrouping(short);
	BOOL GetShowASCII();
	void SetShowASCII(BOOL);
	BOOL GetEditASCII();
	void SetEditASCII(BOOL);
	BOOL GetReadOnly();
	void SetReadOnly(BOOL);
	long GetMemory();
	void SetMemory(long);
	short GetMemModMask();
	void SetMemModMask(short);
	long GetMemoryRange();
	void SetMemoryRange(long);
	short GetDisplayWidth();
	void SetDisplayWidth(short);
	unsigned long GetModifiedTextColor();
	void SetModifiedTextColor(unsigned long);
	unsigned long GetModifiedTextBackColor();
	void SetModifiedTextBackColor(unsigned long);
	unsigned long GetHighlightedTextColor();
	void SetHighlightedTextColor(unsigned long);
	unsigned long GetHighlightedTextBackColor();
	void SetHighlightedTextBackColor(unsigned long);
	unsigned long GetHighlightedModTextColor();
	void SetHighlightedModTextColor(unsigned long);
	unsigned long GetHighlightedModTextBackColor();
	void SetHighlightedModTextBackColor(unsigned long);
	unsigned long GetDisabledTextColor();
	void SetDisabledTextColor(unsigned long);
	unsigned long GetDisabledTextBackColor();
	void SetDisabledTextBackColor(unsigned long);

// Operations
public:
	BOOL DoProperties();
	void AboutBox();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HEXEDIT_H__4C6C6582_95D5_11D1_858F_00600828300C__INCLUDED_)
