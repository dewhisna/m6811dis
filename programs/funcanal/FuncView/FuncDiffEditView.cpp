// FuncDiffEditView.cpp : implementation file
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: FuncDiffEditView.cpp,v $
// Revision 1.1  2003/09/13 05:45:39  dewhisna
// Initial Revision
//
//

#include "stdafx.h"
#include "FuncView.h"

#include "FuncViewPrjDoc.h"
#include "FuncDiffEditView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFuncDiffEditView

IMPLEMENT_DYNCREATE(CFuncDiffEditView, CEditView)

CFuncDiffEditView::CFuncDiffEditView()
{
	m_fntEditor.CreatePointFont(100, "Courier New");
	m_strMatchPercentage = "";
}

CFuncDiffEditView::~CFuncDiffEditView()
{
}


BEGIN_MESSAGE_MAP(CFuncDiffEditView, CEditView)
	//{{AFX_MSG_MAP(CFuncDiffEditView)
	ON_COMMAND(ID_SYMBOLS_ADD, OnSymbolsAdd)
	ON_UPDATE_COMMAND_UI(ID_SYMBOLS_ADD, OnUpdateSymbolsAdd)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFuncDiffEditView diagnostics

#ifdef _DEBUG
void CFuncDiffEditView::AssertValid() const
{
	CEditView::AssertValid();
}

void CFuncDiffEditView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}

CFuncViewPrjDoc* CFuncDiffEditView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFuncViewPrjDoc)));
	return (CFuncViewPrjDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFuncDiffEditView message handlers

void CFuncDiffEditView::OnInitialUpdate() 
{
	CEdit &theEdit = GetEditCtrl();

	theEdit.SetFont(&m_fntEditor);
	theEdit.SetReadOnly(TRUE);

	CEditView::OnInitialUpdate();	// This calls OnUpdate
}

void CFuncDiffEditView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	if (pSender == this) return;

//	CEdit &theEdit = GetEditCtrl();
//	theEdit.SetWindowText("");

	switch (lHint) {
		case 0:						// Normal Update
			m_LeftFunc.Reset();
			m_RightFunc.Reset();
			break;

		case FILESET_TYPE_BASE:		// Left function changed
			m_LeftFunc = *((CFuncViewFuncRef *)pHint);
			break;

		case FILESET_TYPE_BASE+1:	// Right function changed
			m_RightFunc = *((CFuncViewFuncRef *)pHint);
			break;
	}

	if ((lHint == 0) || ((lHint >= FILESET_TYPE_BASE) && (lHint <= FILESET_TYPE_LAST))) {
		DoDiff();
	}
}

void CFuncDiffEditView::DoDiff()
{
	CEdit &theEdit = GetEditCtrl();

	if ((m_LeftFunc.m_pFile == NULL) ||
		(m_RightFunc.m_pFile == NULL) ||
		(m_LeftFunc.m_nIndex < 0) ||
		(m_RightFunc.m_nIndex < 0)) {
		m_strMatchPercentage = "";
		theEdit.SetWindowText("");
		return;
	}

//	double nCompResult = ::CompareFunctions(FCM_DYNPROG_GREEDY,
//								m_pLeftFunc->m_pFile, m_pLeftFunc->m_nIndex,
//								m_pRightFunc->m_pFile, m_pRightFunc->m_nIndex, TRUE);

	CWaitCursor wait;
	double nCompResult;

	CString strTemp = ::DiffFunctions(FCM_DYNPROG_GREEDY,
								m_LeftFunc.m_pFile, m_LeftFunc.m_nIndex,
								m_RightFunc.m_pFile, m_RightFunc.m_nIndex, OO_NONE, nCompResult);

	m_strMatchPercentage.Format("Match: %f%%", nCompResult*100);

	// Convert LF's to CRLF's so editor will display them correctly:
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

void CFuncDiffEditView::OnSymbolsAdd() 
{
	if ((m_LeftFunc.m_pFile == NULL) ||
		(m_RightFunc.m_pFile == NULL) ||
		(m_LeftFunc.m_nIndex < 0) ||
		(m_RightFunc.m_nIndex < 0)) {
		return;
	}

	CFuncViewPrjDoc *pDoc = GetDocument();
	if (pDoc == NULL) return;

	CWaitCursor wait;
	double nCompResult;
	CSymbolMap &theSymbolMap = pDoc->GetSymbolMap();
	CString strTemp;

	CFuncDesc *pLeftFunc = m_LeftFunc.m_pFile->GetFunc(m_LeftFunc.m_nIndex);
	CFuncDesc *pRightFunc = m_RightFunc.m_pFile->GetFunc(m_RightFunc.m_nIndex);

	if ((pLeftFunc != NULL) && (pRightFunc != NULL)) {
		if (pDoc->LookupLeftFuncAssociation(pLeftFunc->GetMainName(), strTemp)) return;
		if (pDoc->LookupRightFuncAssociation(pRightFunc->GetMainName(), strTemp)) return;

		if (!pDoc->GetCompDiffText().IsEmpty()) pDoc->GetCompDiffText() += "\n";
		pDoc->GetCompDiffText() += ::DiffFunctions(FCM_DYNPROG_GREEDY,
						m_LeftFunc.m_pFile, m_LeftFunc.m_nIndex,
						m_RightFunc.m_pFile, m_RightFunc.m_nIndex, OO_ADD_ADDRESS, nCompResult, &theSymbolMap);

		pDoc->AddFunctionAssociation(pLeftFunc->GetMainName(), pRightFunc->GetMainName());
	}

	pDoc->UpdateAllViews(this, SYMBOL_MAP_ALL, NULL);
	pDoc->UpdateAllViews(this, COMP_DIFF_TYPE_BASE, NULL);	// Update CompDiffEditViews
}

void CFuncDiffEditView::OnUpdateSymbolsAdd(CCmdUI* pCmdUI) 
{
	if ((m_LeftFunc.m_pFile == NULL) ||
		(m_RightFunc.m_pFile == NULL) ||
		(m_LeftFunc.m_nIndex < 0) ||
		(m_RightFunc.m_nIndex < 0)) {
		pCmdUI->Enable(FALSE);
		return;
	}

	pCmdUI->Enable(TRUE);
}

