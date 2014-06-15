// FuncViewPrjVw.cpp : implementation of the CFuncViewPrjView class
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: FuncViewPrjVw.cpp,v $
// Revision 1.1  2003/09/13 05:45:45  dewhisna
// Initial Revision
//
//

#include "stdafx.h"
#include "FuncView.h"

#include "FuncViewPrjDoc.h"
#include "FuncViewPrjVw.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFuncViewPrjView

IMPLEMENT_DYNCREATE(CFuncViewPrjView, CTreeView)

BEGIN_MESSAGE_MAP(CFuncViewPrjView, CTreeView)
	//{{AFX_MSG_MAP(CFuncViewPrjView)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR, OnUpdateEditClear)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CTreeView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CTreeView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CTreeView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFuncViewPrjView construction/destruction

CFuncViewPrjView::CFuncViewPrjView()
{
	m_hMatrixBaseNode = NULL;
}

CFuncViewPrjView::~CFuncViewPrjView()
{
}

BOOL CFuncViewPrjView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CTreeView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CFuncViewPrjView printing

BOOL CFuncViewPrjView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CFuncViewPrjView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CFuncViewPrjView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CFuncViewPrjView diagnostics

#ifdef _DEBUG
void CFuncViewPrjView::AssertValid() const
{
	CTreeView::AssertValid();
}

void CFuncViewPrjView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}

CFuncViewPrjDoc* CFuncViewPrjView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFuncViewPrjDoc)));
	return (CFuncViewPrjDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFuncViewPrjView message handlers

void CFuncViewPrjView::OnInitialUpdate()
{
	CTreeCtrl &theTree = GetTreeCtrl();

	theTree.ModifyStyle(0, TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT |
							TVS_SHOWSELALWAYS, SWP_NOACTIVATE);

	// Call default which will call OnUpdate which will call FillFileLists:
	CTreeView::OnInitialUpdate();
}

void CFuncViewPrjView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	if (pSender == this) return;

	switch (lHint) {
		case 0:						// Normal Update
			FillFileLists();
			FillMatrixLists();
			break;

		case MATRIX_TYPE_BASE:		// Matrix list only update
			FillMatrixLists();
			break;
	}

	if ((lHint >= FILESET_TYPE_BASE) && (lHint <= FILESET_TYPE_LAST)) {
		// FILESET_TYPE_BASE+nFileSet = Document Fileset Function update (pHint = &CFuncViewFuncRef)
	}


	if ((lHint >= SYMBOL_MAP_TYPE_BASE) && (lHint <= SYMBOL_MAP_TYPE_LAST)) {
		// SYMBOL_MAP_TYPE_BASE+nSymbolMapType = Symbol Map update (pHint = &LPCTSTR)
	}
}

void CFuncViewPrjView::FillFileLists()
{
	CTreeCtrl &theTree = GetTreeCtrl();
	theTree.DeleteAllItems();

	m_hMatrixBaseNode = NULL;

	CFuncViewPrjDoc *pDoc = GetDocument();
	if (pDoc == NULL) return;

	HTREEITEM hBase = theTree.InsertItem(pDoc->GetPathName() /* GetTitle() */);
	if (hBase == NULL) return;
	theTree.SetItemData(hBase, 0);			// Root document title is 0

	int i, j;
	HTREEITEM hFileSet;
	HTREEITEM hFileEntry;

	for (i=0; i<pDoc->GetFileSetCount(); i++) {
		hFileSet = theTree.InsertItem(pDoc->GetFileSetDescription(i), hBase);
		if (hFileSet == NULL) continue;
		theTree.SetItemData(hFileSet, FILESET_TYPE_BASE + (i*2));	// FileSet bases are even-numbers above FILESET_TYPE_BASE

		for (j=0; j<pDoc->GetFileCount(i); j++) {
			hFileEntry = theTree.InsertItem(pDoc->GetFilePathName(i, j), hFileSet);
			if (hFileEntry == NULL) continue;
			theTree.SetItemData(hFileEntry, FILESET_TYPE_BASE + 1 + (i*2));	// FileSet Entries are odd-numbers above FILESET_TYPE_BASE
		}

		theTree.Expand(hFileSet, TVE_EXPAND);
	}

	theTree.Expand(hBase, TVE_EXPAND);
}

void CFuncViewPrjView::FillMatrixLists()
{
	CTreeCtrl &theTree = GetTreeCtrl();

	CFuncViewPrjDoc *pDoc = GetDocument();
	if (pDoc == NULL) return;

	if (m_hMatrixBaseNode == NULL) {
		m_hMatrixBaseNode = theTree.InsertItem("Matrix Input/Output File");
		if (m_hMatrixBaseNode == NULL) return;
	}

	// Delete all of the item's children:
	if (theTree.ItemHasChildren(m_hMatrixBaseNode)) {
		HTREEITEM hNextItem;
		HTREEITEM hChildItem = theTree.GetChildItem(m_hMatrixBaseNode);

		while (hChildItem != NULL) {
			hNextItem = theTree.GetNextItem(hChildItem, TVGN_NEXT);
			theTree.DeleteItem(hChildItem);
			hChildItem = hNextItem;
		}
	}

	CString strMatrixPathName = pDoc->GetMatrixPathName();

	if (!strMatrixPathName.IsEmpty()) {
		theTree.InsertItem(strMatrixPathName, m_hMatrixBaseNode);
	}

	theTree.Expand(m_hMatrixBaseNode, TVE_EXPAND);
}

void CFuncViewPrjView::OnEditClear() 
{
	CTreeCtrl &theTree = GetTreeCtrl();
	HTREEITEM hSel = theTree.GetSelectedItem();
	if (hSel == NULL) return;
	CFuncViewPrjDoc *pDoc = GetDocument();
	if (pDoc == NULL) return;

	CString strName = theTree.GetItemText(hSel);
	DWORD nType = theTree.GetItemData(hSel);
	int ndx;

	if (nType < FILESET_TYPE_BASE) return;
	if ((nType % 2) != 1) return;

	ndx = pDoc->FindFile((nType - FILESET_TYPE_BASE - 1) / 2, strName);
	ASSERT(ndx != -1);			// Why did we not find it??
	if (ndx != -1) {
		if (pDoc->RemoveFile((nType - FILESET_TYPE_BASE - 1) / 2, ndx)) {
			theTree.DeleteItem(hSel);
			pDoc->SetModifiedFlag(TRUE);
			pDoc->UpdateAllViews(this);
		}
	}
}

void CFuncViewPrjView::OnUpdateEditClear(CCmdUI* pCmdUI) 
{
	CTreeCtrl &theTree = GetTreeCtrl();
	HTREEITEM hSel = theTree.GetSelectedItem();
	if (hSel == NULL) {
		pCmdUI->Enable(FALSE);
		return;
	}

	// Enable clearing for function description file filenames:
	DWORD nType = theTree.GetItemData(hSel);
	pCmdUI->Enable((nType >= FILESET_TYPE_BASE) && ((nType % 2) == 1));
}

