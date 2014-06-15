// CompMatrixView.cpp : implementation file
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: CompMatrixView.cpp,v $
// Revision 1.1  2003/09/13 05:45:38  dewhisna
// Initial Revision
//
//

#include "stdafx.h"
#include "FuncView.h"

#include "CompMatrixView.h"
#include "FuncViewPrjDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCompMatrixView

IMPLEMENT_DYNCREATE(CCompMatrixView, CView)

CCompMatrixView::CCompMatrixView()
{
	m_pGridCtrl = NULL;
}

CCompMatrixView::~CCompMatrixView()
{
	if (m_pGridCtrl)
		delete m_pGridCtrl;
}


BEGIN_MESSAGE_MAP(CCompMatrixView, CView)
	//{{AFX_MSG_MAP(CCompMatrixView)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_SYMBOLS_ADD, OnSymbolsAdd)
	ON_UPDATE_COMMAND_UI(ID_SYMBOLS_ADD, OnUpdateSymbolsAdd)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CCompMatrixView diagnostics

#ifdef _DEBUG
void CCompMatrixView::AssertValid() const
{
	CView::AssertValid();
}

void CCompMatrixView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CFuncViewPrjDoc* CCompMatrixView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFuncViewPrjDoc)));
	return (CFuncViewPrjDoc*)m_pDocument;
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CCompMatrixView printing

BOOL CCompMatrixView::OnPreparePrinting(CPrintInfo* pInfo) 
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CCompMatrixView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo) 
{
	if (m_pGridCtrl) m_pGridCtrl->OnBeginPrinting(pDC, pInfo);
}

void CCompMatrixView::OnPrint(CDC* pDC, CPrintInfo* pInfo) 
{
	if (m_pGridCtrl) m_pGridCtrl->OnPrint(pDC, pInfo);
}

void CCompMatrixView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo) 
{
	if (m_pGridCtrl) m_pGridCtrl->OnEndPrinting(pDC, pInfo);
}


/////////////////////////////////////////////////////////////////////////////
// CGridCtrl required handlers

void CCompMatrixView::OnDraw(CDC* pDC) 
{
	// Do nothing here -- CGridCtrl will draw itself for us
}

void CCompMatrixView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);

	if ((m_pGridCtrl) && (::IsWindow(m_pGridCtrl->m_hWnd))) {
		CRect rect;
		GetClientRect(rect);
		m_pGridCtrl->MoveWindow(rect);
	}
}

BOOL CCompMatrixView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
    if ((m_pGridCtrl) && (::IsWindow(m_pGridCtrl->m_hWnd)))
        if (m_pGridCtrl->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
            return TRUE;

	return CView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

BOOL CCompMatrixView::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
//	return CView::OnEraseBkgnd(pDC);
}


/////////////////////////////////////////////////////////////////////////////
// CCompMatrixView message handlers

void CCompMatrixView::OnInitialUpdate() 
{
	if (m_pGridCtrl == NULL)
	{
		// Create the Gridctrl object
		m_pGridCtrl = new CGridCtrl;
		if (!m_pGridCtrl) return;

		// Create the Gridctrl window
		CRect rect;
		GetClientRect(rect);
		m_pGridCtrl->Create(rect, this, 100);

		// Set settings:
		m_pGridCtrl->SetEditable(FALSE);
		m_pGridCtrl->EnableDragAndDrop(FALSE);
	}

	// The following will call OnUpdate which will call FillMatrixHeaders
	//		and FillMatrixData
	CView::OnInitialUpdate();
}

void CCompMatrixView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	if (pSender == this) return;

	switch (lHint) {
		case 0:						// Normal Update
		case MATRIX_TYPE_BASE:		// Matrix data changed
			FillMatrixHeaders();
			FillMatrixData();
			break;
	}
}

void CCompMatrixView::FillMatrixHeaders()
{
	if (m_pGridCtrl == NULL) return;

	CWaitCursor waitCursor;

	m_pGridCtrl->DeleteAllItems();

	CFuncViewPrjDoc *pDoc = GetDocument();
	if (pDoc == NULL) return;

	int nRows = pDoc->GetFuncCount(CFuncViewPrjDoc::FVFS_LEFT_FILES, -1);
	int nCols = pDoc->GetFuncCount(CFuncViewPrjDoc::FVFS_RIGHT_FILES, -1);

	try {
		m_pGridCtrl->SetRowCount(nRows+1);
		m_pGridCtrl->SetColumnCount(nCols+1);
		m_pGridCtrl->SetFixedRowCount(1);
		m_pGridCtrl->SetFixedColumnCount(1);
	}
	catch (CMemoryException& e)
	{
		e.ReportError();
		e.Delete();
		return;
	}

	// Set Column headers (right-hand function):
	CFuncDesc *pFunc;
	int nFileIndex;
	int nFuncIndex;
	int i;

	nFileIndex = 0;
	nFuncIndex = 0;
	for (i=0; i<nCols; i++) {
		pFunc = pDoc->GetNextFunction(CFuncViewPrjDoc::FVFS_RIGHT_FILES, nFileIndex, nFuncIndex);
		if (pFunc == NULL) continue;

		GV_ITEM anItem;
		anItem.mask = GVIF_TEXT|GVIF_FORMAT;
		anItem.row = 0;
		anItem.col = i+1;
		anItem.nFormat = DT_LEFT|DT_WORDBREAK|DT_NOPREFIX;
		anItem.strText = pFunc->GetMainName();
		m_pGridCtrl->SetItem(&anItem);
	}

	// Set Row headers (left-hand function):
	nFileIndex = 0;
	nFuncIndex = 0;
	for (i=0; i<nRows; i++) {
		pFunc = pDoc->GetNextFunction(CFuncViewPrjDoc::FVFS_LEFT_FILES, nFileIndex, nFuncIndex);
		if (pFunc == NULL) continue;

		GV_ITEM anItem;
		anItem.mask = GVIF_TEXT|GVIF_FORMAT;
		anItem.row = i+1;
		anItem.col = 0;
		anItem.nFormat = DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS|DT_NOPREFIX;
		anItem.strText = pFunc->GetMainName();
		m_pGridCtrl->SetItem(&anItem);
	}

	m_pGridCtrl->AutoSize(GVS_BOTH);
}

void CCompMatrixView::FillMatrixData(int nSingleRow, int nSingleCol)
{
	if (m_pGridCtrl == NULL) return;

	CWaitCursor waitCursor;

	CFuncViewPrjDoc *pDoc = GetDocument();
	if (pDoc == NULL) return;

	double **ppnMatrix = pDoc->GetMatrix();
	if ((!pDoc->HaveMatrix()) || (ppnMatrix == NULL)) return;

	int nRows = pDoc->GetMatrixRowCount();
	int nCols = pDoc->GetMatrixColumnCount();

	// Set Data:
	int i,j,k;
	double nMaxCompResultX;
	double nMaxCompResultY;
	CFuncDesc *pLeftFunc;
	int nLeftFileIndex;
	int nLeftFuncIndex;
	CString strLeftName;
	CFuncDesc *pRightFunc;
	int nRightFileIndex;
	int nRightFuncIndex;
	CString strRightName;
	CString strTemp;
	BOOL bLeft;

	if (nSingleRow != -1) {
		pDoc->GetFileFuncIndexFromLinearIndex(CFuncViewPrjDoc::FVFS_LEFT_FILES, nSingleRow,
													nLeftFileIndex, nLeftFuncIndex);
	} else {
		nLeftFileIndex = 0;
		nLeftFuncIndex = 0;
	}
	for (i=((nSingleRow != -1) ? nSingleRow : 0); i<((nSingleRow != -1) ? (nSingleRow+1) : nRows); i++) {
		nMaxCompResultX = 0;

		pLeftFunc = pDoc->GetNextFunction(CFuncViewPrjDoc::FVFS_LEFT_FILES, nLeftFileIndex, nLeftFuncIndex);
		if (pLeftFunc) {
			strLeftName = pLeftFunc->GetMainName();
		} else {
			strLeftName = "";
		}

		bLeft = pDoc->LookupLeftFuncAssociation(strLeftName, strTemp);

		if (nSingleCol != -1) {
			pDoc->GetFileFuncIndexFromLinearIndex(CFuncViewPrjDoc::FVFS_RIGHT_FILES, nSingleCol,
														nRightFileIndex, nRightFuncIndex);
		} else {
			nRightFileIndex = 0;
			nRightFuncIndex = 0;
		}
		for (j=0; j<nCols; j++) nMaxCompResultX = __max(nMaxCompResultX, ppnMatrix[i][j]);
		for (j=((nSingleCol != -1) ? nSingleCol : 0); j<((nSingleCol != -1) ? (nSingleCol+1) : nCols); j++) {
			pRightFunc = pDoc->GetNextFunction(CFuncViewPrjDoc::FVFS_RIGHT_FILES, nRightFileIndex, nRightFuncIndex);
			if (pRightFunc) {
				strRightName = pRightFunc->GetMainName();
			} else {
				strRightName = "";
			}

			GV_ITEM anItem;
			anItem.mask = ((ppnMatrix[i][j] != 0) ? GVIF_TEXT|GVIF_FORMAT|GVIF_BKCLR :
														GVIF_FORMAT|GVIF_BKCLR);
			anItem.row = i+1;
			anItem.col = j+1;
			anItem.nFormat = DT_RIGHT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS|DT_NOPREFIX;
			anItem.crBkClr = CLR_DEFAULT;
			if (ppnMatrix[i][j] != 0) anItem.strText.Format("%f%%", ppnMatrix[i][j]*100);
			if ((ppnMatrix[i][j] != 0) && (ppnMatrix[i][j] == nMaxCompResultX)) {
				nMaxCompResultY = 0;
				for (k=0; k<nRows; k++) nMaxCompResultY = __max(nMaxCompResultY, ppnMatrix[k][j]);
				if (nMaxCompResultX == nMaxCompResultY) {
					anItem.crBkClr = RGB(0, 192, 192);
				}
			}
			if (anItem.crBkClr == CLR_DEFAULT) {
				if (bLeft || pDoc->LookupRightFuncAssociation(strRightName, strTemp))
					anItem.crBkClr = RGB(255, 255, 0);
			}
			m_pGridCtrl->SetItem(&anItem);
		}
	}

	m_pGridCtrl->AutoSize(GVS_BOTH);
}

void CCompMatrixView::OnSymbolsAdd() 
{
	if (m_pGridCtrl == NULL) return;

	CWaitCursor waitCursor;

	CFuncViewPrjDoc *pDoc = GetDocument();
	if (pDoc == NULL) return;

	CCellID aCell(-1, -1);
	BOOL bDone = FALSE;
	CFuncDesc *pLeftFunc;
	CFuncDesc *pRightFunc;
	int nLeftFileIndex;
	int nLeftFuncIndex;
	CString strLeftName;
	int nRightFileIndex;
	int nRightFuncIndex;
	CString strRightName;
	double nCompResult;
	CSymbolMap &theSymbolMap = pDoc->GetSymbolMap();
	CString strTemp;
	int i;

	int nRows = pDoc->GetMatrixRowCount();
	int nCols = pDoc->GetMatrixColumnCount();

	while (1) {
		aCell = m_pGridCtrl->GetNextItem(aCell, GVNI_ALL | GVNI_SELECTED);
		if ((aCell.row == -1) || (aCell.col == -1)) break;
		pLeftFunc = pDoc->GetFileFuncIndexFromLinearIndex(CFuncViewPrjDoc::FVFS_LEFT_FILES, aCell.row-1, nLeftFileIndex, nLeftFuncIndex);
		pRightFunc = pDoc->GetFileFuncIndexFromLinearIndex(CFuncViewPrjDoc::FVFS_RIGHT_FILES, aCell.col-1, nRightFileIndex, nRightFuncIndex);
		if ((pLeftFunc == NULL) || (pRightFunc == NULL)) continue;

		strLeftName = pLeftFunc->GetMainName();
		strRightName = pRightFunc->GetMainName();

		if (pDoc->LookupLeftFuncAssociation(strLeftName, strTemp)) continue;
		if (pDoc->LookupRightFuncAssociation(strRightName, strTemp)) continue;

		if (!pDoc->GetCompDiffText().IsEmpty()) pDoc->GetCompDiffText() += "\n";
		pDoc->GetCompDiffText() += ::DiffFunctions(FCM_DYNPROG_GREEDY,
					pDoc->GetFuncDescFile(CFuncViewPrjDoc::FVFS_LEFT_FILES, nLeftFileIndex), nLeftFuncIndex,
					pDoc->GetFuncDescFile(CFuncViewPrjDoc::FVFS_RIGHT_FILES, nRightFileIndex), nRightFuncIndex,
					OO_ADD_ADDRESS, nCompResult, &theSymbolMap);

		pDoc->AddFunctionAssociation(strLeftName, strRightName);

		nLeftFileIndex = 0;
		nLeftFuncIndex = 0;
		for (i=0; i<nRows; i++) {
			pLeftFunc = pDoc->GetNextFunction(CFuncViewPrjDoc::FVFS_LEFT_FILES, nLeftFileIndex, nLeftFuncIndex);
			if (pLeftFunc == NULL) continue;
			if (pLeftFunc->GetMainName().Compare(strLeftName) == 0) FillMatrixData(i, -1);
		}

		nRightFileIndex = 0;
		nRightFuncIndex = 0;
		for (i=0; i<nCols; i++) {
			pRightFunc = pDoc->GetNextFunction(CFuncViewPrjDoc::FVFS_RIGHT_FILES, nRightFileIndex, nRightFuncIndex);
			if (pRightFunc == NULL) continue;
			if (pRightFunc->GetMainName().Compare(strRightName) == 0) FillMatrixData(-1, i);
		}
	}

	pDoc->UpdateAllViews(this, SYMBOL_MAP_ALL, NULL);
	pDoc->UpdateAllViews(this, COMP_DIFF_TYPE_BASE, NULL);	// Update CompDiffEditViews
}

void CCompMatrixView::OnUpdateSymbolsAdd(CCmdUI* pCmdUI) 
{
	if (m_pGridCtrl == NULL) {
		pCmdUI->Enable(FALSE);
		return;
	}

	pCmdUI->Enable(m_pGridCtrl->GetSelectedCount() != 0);
}

