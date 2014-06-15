// SymbolMapTreeView.cpp : implementation file
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: SymbolMapTreeView.cpp,v $
// Revision 1.1  2003/09/13 05:45:50  dewhisna
// Initial Revision
//
//

#include "stdafx.h"
#include "FuncView.h"
#include "SymbolMapTreeView.h"

#include "FuncViewPrjDoc.h"
#include "../funcdesc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSymbolMapTreeView

IMPLEMENT_DYNCREATE(CSymbolMapTreeView, CTreeView)

CSymbolMapTreeView::CSymbolMapTreeView()
{
}

CSymbolMapTreeView::~CSymbolMapTreeView()
{
}


BEGIN_MESSAGE_MAP(CSymbolMapTreeView, CTreeView)
	//{{AFX_MSG_MAP(CSymbolMapTreeView)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSymbolMapTreeView diagnostics

#ifdef _DEBUG
void CSymbolMapTreeView::AssertValid() const
{
	CTreeView::AssertValid();
}

void CSymbolMapTreeView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}

CFuncViewPrjDoc* CSymbolMapTreeView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFuncViewPrjDoc)));
	return (CFuncViewPrjDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSymbolMapTreeView message handlers

void CSymbolMapTreeView::OnInitialUpdate() 
{
	CTreeCtrl &theTree = GetTreeCtrl();

	theTree.ModifyStyle(0, TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT |
							TVS_SHOWSELALWAYS, SWP_NOACTIVATE);


	// Call default which will call OnUpdate which will call FillSymbolMapList:
	CTreeView::OnInitialUpdate();
}

void CSymbolMapTreeView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	if (pSender == this) return;

	switch (lHint) {
		case 0:				// Normal Update
			FillSymbolMapList();
			break;

		case SYMBOL_MAP_ALL:
			FillSymbolMapList();
			break;

		case SYMBOL_MAP_LEFT_CODE:
		case SYMBOL_MAP_RIGHT_CODE:
		case SYMBOL_MAP_LEFT_DATA:
		case SYMBOL_MAP_RIGHT_DATA:
			break;
	}


	if ((lHint >= FILESET_TYPE_BASE) && (lHint <= FILESET_TYPE_LAST)) {
		// FILESET_TYPE_BASE+nFileSet = Document Fileset Function update (pHint = &CFuncViewFuncRef)
	}

	if ((lHint >= SYMBOL_MAP_TYPE_BASE) && (lHint <= SYMBOL_MAP_TYPE_LAST)) {
		// SYMBOL_MAP_TYPE_BASE+nSymbolMapType = Symbol Map update (pHint = &LPCTSTR)
	}
}

void CSymbolMapTreeView::FillSymbolMapList()
{
	CTreeCtrl &theTree = GetTreeCtrl();
	theTree.DeleteAllItems();

	CFuncViewPrjDoc *pDoc = GetDocument();
	if (pDoc == NULL) return;

	HTREEITEM hLeftCode = theTree.InsertItem("Left-Side Code Symbols");
	HTREEITEM hRightCode = theTree.InsertItem("Right-Side Code Symbols");
	HTREEITEM hLeftData = theTree.InsertItem("Left-Side Data Symbols");
	HTREEITEM hRightData = theTree.InsertItem("Right-Side Data Symbols");

	if ((hLeftCode == NULL) ||
		(hRightCode == NULL) ||
		(hLeftData == NULL) ||
		(hRightData == NULL)) return;

	theTree.SetItemData(hLeftCode, 1);
	theTree.SetItemData(hRightCode, 2);
	theTree.SetItemData(hLeftData, 3);
	theTree.SetItemData(hRightData, 4);

	CStringArray anArr;
	CSymbolMap &theSymbols = pDoc->GetSymbolMap();

	anArr.RemoveAll();
	theSymbols.GetLeftSideCodeSymbolList(anArr);
	FillSymbolMapSubList(hLeftCode, anArr, SYMBOL_MAP_LEFT_CODE);

	anArr.RemoveAll();
	theSymbols.GetRightSideCodeSymbolList(anArr);
	FillSymbolMapSubList(hRightCode, anArr, SYMBOL_MAP_RIGHT_CODE);

	anArr.RemoveAll();
	theSymbols.GetLeftSideDataSymbolList(anArr);
	FillSymbolMapSubList(hLeftData, anArr, SYMBOL_MAP_LEFT_DATA);

	anArr.RemoveAll();
	theSymbols.GetRightSideDataSymbolList(anArr);
	FillSymbolMapSubList(hRightData, anArr, SYMBOL_MAP_RIGHT_DATA);
}

void CSymbolMapTreeView::FillSymbolMapSubList(HTREEITEM hParent, CStringArray &aSymbolList, DWORD nItemData)
{
	int i;
	HTREEITEM hSymbol;
	CTreeCtrl &theTree = GetTreeCtrl();

	for (i=0; i<aSymbolList.GetSize(); i++) {
		hSymbol = theTree.InsertItem(aSymbolList.GetAt(i), hParent);
		if (hSymbol == NULL) continue;
		theTree.SetItemData(hSymbol, nItemData);
	}

	theTree.Expand(hParent, TVE_EXPAND);
}

void CSymbolMapTreeView::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	*pResult = 0;

	CFuncViewPrjDoc *pDoc = GetDocument();
	if (pDoc == NULL) return;

	CTreeCtrl &theTree = GetTreeCtrl();
	HTREEITEM hSel = theTree.GetSelectedItem();
	DWORD nSelType = 0;
	CString strSelSymbol;

	if (hSel) {
		nSelType = theTree.GetItemData(hSel);
		strSelSymbol = theTree.GetItemText(hSel);
	}

	if (nSelType >= SYMBOL_MAP_TYPE_BASE) {
		pDoc->UpdateAllViews(this, (LPARAM)nSelType, (!strSelSymbol.IsEmpty() ? (CObject*)LPCTSTR(strSelSymbol) : NULL));
	} else {
		if (nSelType == 0) {
			pDoc->UpdateAllViews(this, (LPARAM)SYMBOL_MAP_ALL, NULL);
		} else {
			pDoc->UpdateAllViews(this, (LPARAM)(SYMBOL_MAP_TYPE_BASE + nSelType), NULL);
		}
	}
}

