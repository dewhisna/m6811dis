// OutputView.h : header file
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: OutputView.h,v $
// Revision 1.1  2003/09/13 05:45:47  dewhisna
// Initial Revision
//
//

#if !defined(AFX_OUTPUTVIEW_H__F016EE3C_8F7F_4B35_A9C2_E54F1F80B97B__INCLUDED_)
#define AFX_OUTPUTVIEW_H__F016EE3C_8F7F_4B35_A9C2_E54F1F80B97B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CFuncViewPrjDoc;

/////////////////////////////////////////////////////////////////////////////
// COutputViewFile

class COutputView;

class COutputViewFile : public CFile
{
public:
// Constructors
	COutputViewFile(COutputView &rOutputView);
	virtual ~COutputViewFile();

#ifdef _DEBUG
public:
	virtual void Dump(CDumpContext& dc) const;
	virtual void AssertValid() const;
#endif

// Implementation:
protected:
	COutputView &m_rOutputView;
	BOOL m_bEraseOnNextWrite;

public:
	virtual DWORD GetPosition() const;
	BOOL GetStatus(CFileStatus& rStatus) const;
	virtual LONG Seek(LONG lOff, UINT nFrom);
	virtual void SetLength(DWORD dwNewLen);
	virtual UINT Read(void* lpBuf, UINT nCount);
	virtual void Write(const void* lpBuf, UINT nCount);
	virtual void Abort();
	virtual void Flush();
	virtual void Close();

	// Unsupported APIs
	virtual CFile* Duplicate() const;
	virtual void LockRange(DWORD dwPos, DWORD dwCount);
	virtual void UnlockRange(DWORD dwPos, DWORD dwCount);
};


/////////////////////////////////////////////////////////////////////////////
// COutputView view

class COutputView : public CEditView
{
protected:
	COutputView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(COutputView)

// Attributes
public:
	CFuncViewPrjDoc* GetDocument();
	inline COutputViewFile *GetOutputFile() const { return m_pOutputViewFile; }

protected:
	COutputViewFile *m_pOutputViewFile;
	CFont m_fntOutput;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COutputView)
	public:
	virtual void OnInitialUpdate();
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~COutputView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(COutputView)
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in OutputView.cpp
inline CFuncViewPrjDoc* COutputView::GetDocument()
   { return (CFuncViewPrjDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OUTPUTVIEW_H__F016EE3C_8F7F_4B35_A9C2_E54F1F80B97B__INCLUDED_)
