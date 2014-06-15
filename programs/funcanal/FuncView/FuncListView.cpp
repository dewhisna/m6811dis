// FuncListView.cpp : implementation file
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: FuncListView.cpp,v $
// Revision 1.1  2003/09/13 05:45:40  dewhisna
// Initial Revision
//
//

#include "stdafx.h"
#include "FuncView.h"

#include "FuncViewPrjDoc.h"
#include "FuncListView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFuncListView

IMPLEMENT_DYNCREATE(CFuncListView, CListView)

CFuncListView::CFuncListView()
{
	m_nFileSet = -1;
}

CFuncListView::~CFuncListView()
{
}


BEGIN_MESSAGE_MAP(CFuncListView, CListView)
	//{{AFX_MSG_MAP(CFuncListView)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnItemchanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFuncListView diagnostics

#ifdef _DEBUG
void CFuncListView::AssertValid() const
{
	CListView::AssertValid();
}

void CFuncListView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

CFuncViewPrjDoc* CFuncListView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFuncViewPrjDoc)));
	return (CFuncViewPrjDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFuncListView message handlers

void CFuncListView::OnInitialUpdate() 
{
	CListCtrl &theList = GetListCtrl();

	theList.ModifyStyle(0, LVS_NOSORTHEADER | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS, SWP_NOACTIVATE);
	theList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	theList.InsertColumn(0, "Function Name", LVCFMT_LEFT, 100, 0);
	theList.InsertColumn(1, "Absolute Addr", LVCFMT_RIGHT, 100, 1);

	// Call the base OnInitialUpdate which will call OnUpdate which will call FillFuncList:
	CListView::OnInitialUpdate();
}

void CFuncListView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	if (pSender == this) return;

	switch (lHint) {
		case 0:						// Normal update
			FillFuncList();
			break;
	}

	if ((lHint >= FILESET_TYPE_BASE) && (lHint <= FILESET_TYPE_LAST)) {
		// FILESET_TYPE_BASE+nFileSet = Document Fileset Function update (pHint = &CFuncViewFuncRef)
	}


	if ((lHint >= SYMBOL_MAP_TYPE_BASE) && (lHint <= SYMBOL_MAP_TYPE_LAST)) {
		// SYMBOL_MAP_TYPE_BASE+nSymbolMapType = Symbol Map update (pHint = &LPCTSTR)
	}
}

void CFuncListView::FillFuncList()
{
	CListCtrl &theList = GetListCtrl();
	theList.DeleteAllItems();
	m_arrFileIndexes.RemoveAll();
	m_arrFuncIndexes.RemoveAll();

	if (m_nFileSet == -1) return;

	CFuncViewPrjDoc *pDoc = GetDocument();
	if (pDoc == NULL) return;

	int nFileCount = pDoc->GetFileCount(m_nFileSet);
	int nFuncCount;
	int i,j;
	int ndx, ndx2, ndx3;
	CFuncDescFile *pFuncDescFile;
	CFuncDesc *pFunc;
	CString strTemp;

	for (i=0; i<nFileCount; i++) {
		pFuncDescFile = pDoc->GetFuncDescFile(m_nFileSet, i);
		if (pFuncDescFile == NULL) continue;

		nFuncCount = pFuncDescFile->GetFuncCount();
		for (j=0; j<nFuncCount; j++) {
			pFunc = pFuncDescFile->GetFunc(j);
			if (pFunc == NULL) continue;

			ndx = theList.InsertItem(theList.GetItemCount(), pFunc->GetMainName());
			if (ndx == -1) continue;

			strTemp.Format("0x%04lX", pFunc->GetMainAddress());
			theList.SetItemText(ndx, 1, strTemp);

			ndx2 = m_arrFileIndexes.Add(i);
			ndx3 = m_arrFuncIndexes.Add(j);

			ASSERT(ndx2 == ndx3);

			theList.SetItemData(ndx, ndx2);
		}
	}

	ASSERT(m_arrFileIndexes.GetSize() == m_arrFuncIndexes.GetSize());
}

int CFuncListView::GetFileSetIndex()
{
	return m_nFileSet;
}

void CFuncListView::SetFileSetIndex(int nFileSet)
{
	m_nFileSet = nFileSet;
}

void CFuncListView::OnItemchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	*pResult = 0;

	CFuncViewPrjDoc *pDoc = GetDocument();
	if (pDoc == NULL) return;

	CListCtrl &theList = GetListCtrl();
	int nSel = theList.GetNextItem(-1, LVNI_SELECTED);
	CFuncViewFuncRef aRef;

	if (nSel != -1) {
		aRef.m_pFile = pDoc->GetFuncDescFile(m_nFileSet, m_arrFileIndexes.GetAt(theList.GetItemData(nSel)));
		aRef.m_nIndex = m_arrFuncIndexes.GetAt(theList.GetItemData(nSel));
		pDoc->UpdateAllViews(this, FILESET_TYPE_BASE + m_nFileSet, &aRef);
	} else {
		aRef.m_pFile = NULL;
		aRef.m_nIndex = -1;
		pDoc->UpdateAllViews(this, FILESET_TYPE_BASE + m_nFileSet, &aRef);
	}
}

