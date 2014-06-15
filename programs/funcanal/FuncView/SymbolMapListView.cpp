// SymbolMapListView.cpp : implementation file
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: SymbolMapListView.cpp,v $
// Revision 1.1  2003/09/13 05:45:49  dewhisna
// Initial Revision
//
//

#include "stdafx.h"
#include "FuncView.h"
#include "SymbolMapListView.h"

#include "FuncViewPrjDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSymbolMapListView

IMPLEMENT_DYNCREATE(CSymbolMapListView, CListView)

CSymbolMapListView::CSymbolMapListView()
{
}

CSymbolMapListView::~CSymbolMapListView()
{
}


BEGIN_MESSAGE_MAP(CSymbolMapListView, CListView)
	//{{AFX_MSG_MAP(CSymbolMapListView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSymbolMapListView diagnostics

#ifdef _DEBUG
void CSymbolMapListView::AssertValid() const
{
	CListView::AssertValid();
}

void CSymbolMapListView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

CFuncViewPrjDoc* CSymbolMapListView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFuncViewPrjDoc)));
	return (CFuncViewPrjDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSymbolMapListView message handlers

void CSymbolMapListView::OnInitialUpdate() 
{
	CListCtrl &theList = GetListCtrl();

	theList.ModifyStyle(0, LVS_NOSORTHEADER | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS, SWP_NOACTIVATE);
	theList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	theList.InsertColumn(0, "Symbol Name", LVCFMT_LEFT, 200, 0);
	theList.InsertColumn(1, "Hit Count", LVCFMT_RIGHT, 70, 1);
	theList.InsertColumn(2, "Hit Percent", LVCFMT_RIGHT, 100, 2);

	// Call the base OnInitialUpdate which will call OnUpdate which will call FillSymbolMapList:
	CListView::OnInitialUpdate();
}

void CSymbolMapListView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	if (pSender == this) return;

	switch (lHint) {
		// SYMBOL_MAP_TYPE_BASE+nSymbolMapType = Symbol Map update (pHint = &LPCTSTR)
		case 0:						// Normal update
			FillSymbolMapList();
			break;

		case SYMBOL_MAP_LEFT_CODE:
		case SYMBOL_MAP_RIGHT_CODE:
		case SYMBOL_MAP_LEFT_DATA:
		case SYMBOL_MAP_RIGHT_DATA:
			FillSymbolMapList((DWORD)lHint, (LPCTSTR)pHint);
			break;

		case SYMBOL_MAP_ALL:		// If the whole list changes, clear view
			FillSymbolMapList();
			break;
	}
}

void CSymbolMapListView::FillSymbolMapList(DWORD nSource, LPCTSTR pszSymbolName)
{
	CListCtrl &theList = GetListCtrl();
	theList.DeleteAllItems();

	CFuncViewPrjDoc *pDoc = GetDocument();
	if (pDoc == NULL) return;

	CHeaderCtrl *pHeader = theList.GetHeaderCtrl();

	CString strSymbolName = pszSymbolName;

	DWORD nTotalHitCount = 0;
	CStringArray arrSymbols;
	CDWordArray arrHitCounts;
	CSymbolMap &theSymbols = pDoc->GetSymbolMap();
	CString strHeaderText;

	switch (nSource) {
		case SYMBOL_MAP_LEFT_CODE:
			nTotalHitCount = theSymbols.GetLeftSideCodeHitList(strSymbolName, arrSymbols, arrHitCounts);
			strHeaderText = "Right-Hand Code Symbol Name";
			break;

		case SYMBOL_MAP_RIGHT_CODE:
			nTotalHitCount = theSymbols.GetRightSideCodeHitList(strSymbolName, arrSymbols, arrHitCounts);
			strHeaderText = "Left-Hand Code Symbol Name";
			break;

		case SYMBOL_MAP_LEFT_DATA:
			nTotalHitCount = theSymbols.GetLeftSideDataHitList(strSymbolName, arrSymbols, arrHitCounts);
			strHeaderText = "Right-Hand Data Symbol Name";
			break;

		case SYMBOL_MAP_RIGHT_DATA:
			nTotalHitCount = theSymbols.GetRightSideDataHitList(strSymbolName, arrSymbols, arrHitCounts);
			strHeaderText = "Left-Hand Data Symbol Name";
			break;

		default:
			strHeaderText = "Symbol Name";
			break;
	}

	if (pHeader) {
		HDITEM theHeader;

		theHeader.mask = HDI_TEXT;
		theHeader.pszText = (LPTSTR)LPCTSTR(strHeaderText);
		theHeader.cchTextMax = strHeaderText.GetLength();
		pHeader->SetItem(0, &theHeader);
	}

	int ndx;
	int i;
	CString strTemp;

	ASSERT(arrSymbols.GetSize() == arrHitCounts.GetSize());		// These should have same number of members!
	for (i=0; i<arrSymbols.GetSize(); i++) {
		ASSERT(nTotalHitCount != 0);							// Should never have members with no hits!
		ndx = theList.InsertItem(i, arrSymbols.GetAt(i));
		if (ndx == -1) continue;

		strTemp.Format("%ld", arrHitCounts.GetAt(i));
		theList.SetItemText(i, 1, strTemp);

		strTemp.Format("%f%%", ((double)arrHitCounts.GetAt(i)/nTotalHitCount)*100);
		theList.SetItemText(i, 2, strTemp);
	}
}

