// CompDiffEditView.cpp : implementation file
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: CompDiffEditView.cpp,v $
// Revision 1.1  2003/09/13 05:45:37  dewhisna
// Initial Revision
//
//

#include "stdafx.h"
#include "FuncView.h"

#include "FuncViewPrjDoc.h"
#include "CompDiffEditView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCompDiffEditView

IMPLEMENT_DYNCREATE(CCompDiffEditView, CEditView)

CCompDiffEditView::CCompDiffEditView()
{
	m_fntEditor.CreatePointFont(100, "Courier New");
}

CCompDiffEditView::~CCompDiffEditView()
{
}


BEGIN_MESSAGE_MAP(CCompDiffEditView, CEditView)
	//{{AFX_MSG_MAP(CCompDiffEditView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CCompDiffEditView diagnostics

#ifdef _DEBUG
void CCompDiffEditView::AssertValid() const
{
	CEditView::AssertValid();
}

void CCompDiffEditView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}

CFuncViewPrjDoc* CCompDiffEditView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFuncViewPrjDoc)));
	return (CFuncViewPrjDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCompDiffEditView message handlers

void CCompDiffEditView::OnInitialUpdate() 
{
	CEdit &theEdit = GetEditCtrl();

	theEdit.SetFont(&m_fntEditor);
	theEdit.SetReadOnly(TRUE);

	CEditView::OnInitialUpdate();	// This calls OnUpdate
}

void CCompDiffEditView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	if (pSender == this) return;

	CEdit &theEdit = GetEditCtrl();

	CFuncViewPrjDoc *pDoc = GetDocument();
	if (pDoc == NULL) {
		theEdit.SetWindowText("");
		return;
	}

	if ((lHint == 0) ||
		((lHint >= COMP_DIFF_TYPE_BASE) && (lHint <= COMP_DIFF_TYPE_LAST))) {

		// Convert LF's to CRLF's so editor will display them correctly:
		CString strTemp = pDoc->GetCompDiffText();
		CString strTemp2 = "";
		int pos;

		while (!strTemp.IsEmpty()) {
			pos = strTemp.Find('\n');
			if (pos != -1) {
				strTemp2 += strTemp.Left(pos);
				strTemp2 += "\r\n";
				strTemp = strTemp.Mid(pos+1);
			} else {
				strTemp2 += strTemp;
				strTemp = "";
			}
		}

		theEdit.SetWindowText(strTemp2);
	}
}

