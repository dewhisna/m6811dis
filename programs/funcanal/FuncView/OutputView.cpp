// OutputView.cpp : implementation file
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: OutputView.cpp,v $
// Revision 1.1  2003/09/13 05:45:47  dewhisna
// Initial Revision
//
//

#include "stdafx.h"
#include "FuncView.h"

#include "FuncViewPrjDoc.h"
#include "OutputView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COutputViewFile

COutputViewFile::COutputViewFile(COutputView &rOutputView)
		: m_rOutputView(rOutputView), m_bEraseOnNextWrite(TRUE)
{
}

COutputViewFile::~COutputViewFile()
{
}

#ifdef _DEBUG
void COutputViewFile::AssertValid() const
{
	CFile::AssertValid();
}

void COutputViewFile::Dump(CDumpContext& dc) const
{
	CFile::Dump(dc);
}
#endif

DWORD COutputViewFile::GetPosition() const
{
	return 0;		// Always a zero-length file
}

BOOL COutputViewFile::GetStatus(CFileStatus& rStatus) const
{
	rStatus.m_ctime = 0;
	rStatus.m_mtime = 0;
	rStatus.m_atime = 0;
	rStatus.m_size = 0;
	rStatus.m_attribute = normal;
	rStatus.m_szFullName[0] = '\0';
	return TRUE;
}

LONG COutputViewFile::Seek(LONG lOff, UINT nFrom)
{
	return 0;		// All offsets are legal, but we are zero length
}

void COutputViewFile::SetLength(DWORD dwNewLen)
{
	// Do nothing
}

UINT COutputViewFile::Read(void* lpBuf, UINT nCount)
{
	return 0;		// This is an output only file
}

void COutputViewFile::Write(const void* lpBuf, UINT nCount)
{
	CEdit &theEdit = m_rOutputView.GetEditCtrl();

	if (m_bEraseOnNextWrite) {
		m_bEraseOnNextWrite = FALSE;

		theEdit.SetWindowText("");
	}

	CString strTemp;
	LPTSTR pBuffer = strTemp.GetBuffer(nCount + 1);
	memcpy(pBuffer, lpBuf, nCount);
	strTemp.ReleaseBuffer(nCount);

	UINT nLen = m_rOutputView.GetBufferLength();

	theEdit.SetSel(nLen, nLen, TRUE);
	theEdit.ReplaceSel(strTemp);

	nLen = m_rOutputView.GetBufferLength();
	theEdit.SetSel(nLen, nLen, FALSE);
}

void COutputViewFile::Abort()
{
	m_bEraseOnNextWrite = TRUE;
}

void COutputViewFile::Flush()
{
	// Do nothing
}

void COutputViewFile::Close()
{
	m_bEraseOnNextWrite = TRUE;
}

CFile* COutputViewFile::Duplicate() const
{
	AfxThrowNotSupportedException();
	return NULL;
}

void COutputViewFile::LockRange(DWORD dwPos, DWORD dwCount)
{
	AfxThrowNotSupportedException();
}

void COutputViewFile::UnlockRange(DWORD dwPos, DWORD dwCount)
{
	AfxThrowNotSupportedException();
}


/////////////////////////////////////////////////////////////////////////////
// COutputView

IMPLEMENT_DYNCREATE(COutputView, CEditView)

COutputView::COutputView()
{
	m_pOutputViewFile = NULL;
	m_fntOutput.CreatePointFont(100, "Courier New");
}

COutputView::~COutputView()
{
	if (m_pOutputViewFile) {
		delete m_pOutputViewFile;
		m_pOutputViewFile = NULL;
	}
}


BEGIN_MESSAGE_MAP(COutputView, CEditView)
	//{{AFX_MSG_MAP(COutputView)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// COutputView diagnostics

#ifdef _DEBUG
void COutputView::AssertValid() const
{
	CEditView::AssertValid();
}

void COutputView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}

CFuncViewPrjDoc* COutputView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFuncViewPrjDoc)));
	return (CFuncViewPrjDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// COutputView message handlers

void COutputView::OnInitialUpdate() 
{
	CEdit &theEdit = GetEditCtrl();

	m_pOutputViewFile = new COutputViewFile(*this);

	theEdit.SetFont(&m_fntOutput);
	theEdit.SetReadOnly();
	theEdit.SetWindowText(GetDocument()->GetOutputText());

	UINT nLen = GetBufferLength();
	theEdit.SetSel(nLen-1, nLen, FALSE);
	theEdit.SetSel(nLen, nLen, FALSE);

	CEditView::OnInitialUpdate();
}

void COutputView::OnDestroy() 
{
	CEdit &theEdit = GetEditCtrl();

	theEdit.GetWindowText(GetDocument()->GetOutputText());

	CEditView::OnDestroy();
}

