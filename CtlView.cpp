// CtlView.cpp : implementation of the CCTLFileView class
//

#include "stdafx.h"
#include "M6811DIS.h"

#include "CtlDoc.h"
#include "CtlView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCTLFileView

IMPLEMENT_DYNCREATE(CCTLFileView, CFormView)

BEGIN_MESSAGE_MAP(CCTLFileView, CFormView)
	//{{AFX_MSG_MAP(CCTLFileView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CFormView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CFormView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CFormView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCTLFileView construction/destruction

CCTLFileView::CCTLFileView()
	: CFormView(CCTLFileView::IDD)
{
	//{{AFX_DATA_INIT(CCTLFileView)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// TODO: add construction code here

}

CCTLFileView::~CCTLFileView()
{
}

void CCTLFileView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCTLFileView)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BOOL CCTLFileView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CFormView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CCTLFileView printing

BOOL CCTLFileView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CCTLFileView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CCTLFileView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CCTLFileView::OnPrint(CDC* pDC, CPrintInfo*)
{
	// TODO: add code to print the controls
}

/////////////////////////////////////////////////////////////////////////////
// CCTLFileView diagnostics

#ifdef _DEBUG
void CCTLFileView::AssertValid() const
{
	CFormView::AssertValid();
}

void CCTLFileView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CCTLFileDoc* CCTLFileView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCTLFileDoc)));
	return (CCTLFileDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCTLFileView message handlers

void CCTLFileView::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();

	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();
}
