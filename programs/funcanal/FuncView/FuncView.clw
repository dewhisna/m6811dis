; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CChildFrame6
LastTemplate=CEditView
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "FuncView.h"
LastPage=0

ClassCount=20
Class1=CFuncViewApp
Class2=CFuncViewPrjDoc
Class3=CFuncViewPrjView
Class4=CMainFrame
Class5=CAboutDlg
Class6=CChildFrameBase
Class7=CChildFrame
Class8=CChildFrame2
Class9=CFuncListView
Class10=CFuncDiffEditView
Class11=CChildFrame3

ResourceCount=4
Resource1=IDR_MAINFRAME
Resource2=IDR_FUNCVPTYPE
Class12=CSymbolMapTreeView
Class13=CSymbolMapListView
Class14=CChildFrame4
Class15=COutputView
Class16=CChildFrame5
Class17=CCompMatrixView
Resource3=IDD_ABOUTBOX
Class18=CProgressDlg
Class19=CChildFrame6
Class20=CCompDiffEditView
Resource4=CG_IDD_PROGRESS

[CLS:CFuncViewApp]
Type=0
HeaderFile=FuncView.h
ImplementationFile=FuncView.cpp
Filter=N

[CLS:CFuncViewPrjDoc]
Type=0
HeaderFile=FuncViewPrjDoc.h
ImplementationFile=FuncViewPrjDoc.cpp
Filter=N
BaseClass=CDocument
VirtualFilter=DC
LastObject=ID_VIEW_COMP_DIFF

[CLS:CFuncViewPrjView]
Type=0
HeaderFile=FuncViewPrjVw.h
ImplementationFile=FuncViewPrjVw.cpp
Filter=C
LastObject=ID_EDIT_CLEAR
BaseClass=CTreeView
VirtualFilter=VWC


[CLS:CMainFrame]
Type=0
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=T
LastObject=ID_WINDOW_NEW
BaseClass=CMDIFrameWnd
VirtualFilter=fWC


[CLS:CChildFrameBase]
Type=0
HeaderFile=ChildFrmBase.h
ImplementationFile=ChildFrmBase.cpp
BaseClass=CMDIChildWnd
Filter=M
LastObject=CChildFrameBase

[CLS:CChildFrame]
Type=0
HeaderFile=ChildFrm.h
ImplementationFile=ChildFrm.cpp
BaseClass=CChildFrameBase
Filter=M
LastObject=CChildFrame

[CLS:CAboutDlg]
Type=0
HeaderFile=FuncView.cpp
ImplementationFile=FuncView.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_PRINT_SETUP
Command4=ID_FILE_MRU_FILE1
Command5=ID_APP_EXIT
Command6=ID_VIEW_TOOLBAR
Command7=ID_VIEW_STATUS_BAR
Command8=ID_APP_ABOUT
CommandCount=8

[TB:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_CUT
Command5=ID_EDIT_COPY
Command6=ID_EDIT_PASTE
Command7=ID_FILE_PRINT
Command8=ID_APP_ABOUT
CommandCount=8

[MNU:IDR_FUNCVPTYPE]
Type=1
Class=CFuncViewPrjView
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_CLOSE
Command4=ID_FILE_SAVE
Command5=ID_FILE_SAVE_AS
Command6=ID_FILE_PRINT
Command7=ID_FILE_PRINT_PREVIEW
Command8=ID_FILE_PRINT_SETUP
Command9=ID_FILE_MRU_FILE1
Command10=ID_APP_EXIT
Command11=ID_EDIT_UNDO
Command12=ID_EDIT_CUT
Command13=ID_EDIT_COPY
Command14=ID_EDIT_PASTE
Command15=ID_EDIT_CLEAR
Command16=ID_VIEW_TOOLBAR
Command17=ID_VIEW_STATUS_BAR
Command18=ID_VIEW_OUTPUT_WINDOW
Command19=ID_VIEW_FUNC_DIFF
Command20=ID_VIEW_SYMBOL_MAP
Command21=ID_VIEW_COMP_MATRIX
Command22=ID_VIEW_COMP_DIFF
Command23=ID_SYMBOLS_ADD
Command24=ID_SYMBOLS_RESET
Command25=ID_SYMBOLS_EXPORT
Command26=ID_VIEW_SYMBOL_MAP
Command27=ID_MATRIX_REBUILD
Command28=ID_MATRIX_REBUILD_FORCE
Command29=ID_VIEW_COMP_MATRIX
Command30=ID_WINDOW_NEW
Command31=ID_WINDOW_CASCADE
Command32=ID_WINDOW_TILE_HORZ
Command33=ID_WINDOW_ARRANGE
Command34=ID_APP_ABOUT
CommandCount=34

[ACL:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_EDIT_COPY
Command2=ID_FILE_NEW
Command3=ID_FILE_OPEN
Command4=ID_FILE_PRINT
Command5=ID_FILE_SAVE
Command6=ID_EDIT_PASTE
Command7=ID_SYMBOLS_ADD
Command8=ID_EDIT_UNDO
Command9=ID_EDIT_CLEAR
Command10=ID_EDIT_CUT
Command11=ID_NEXT_PANE
Command12=ID_PREV_PANE
Command13=ID_EDIT_COPY
Command14=ID_EDIT_PASTE
Command15=ID_EDIT_CUT
Command16=ID_EDIT_UNDO
CommandCount=16

[CLS:CChildFrame2]
Type=0
HeaderFile=ChildFrm2.h
ImplementationFile=ChildFrm2.cpp
BaseClass=CChildFrameBase
Filter=M
LastObject=ID_SYMBOLS_ADD
VirtualFilter=mfWC

[CLS:CFuncListView]
Type=0
HeaderFile=FuncListView.h
ImplementationFile=FuncListView.cpp
BaseClass=CListView
Filter=C
LastObject=CFuncListView
VirtualFilter=VWC

[CLS:CFuncDiffEditView]
Type=0
HeaderFile=FuncDiffEditView.h
ImplementationFile=FuncDiffEditView.cpp
BaseClass=CEditView
Filter=C
LastObject=CFuncDiffEditView
VirtualFilter=VWC

[CLS:CChildFrame3]
Type=0
HeaderFile=ChildFrm3.h
ImplementationFile=ChildFrm3.cpp
BaseClass=CChildFrameBase
Filter=M
LastObject=CChildFrame3
VirtualFilter=fWC

[CLS:CSymbolMapTreeView]
Type=0
HeaderFile=SymbolMapTreeView.h
ImplementationFile=SymbolMapTreeView.cpp
BaseClass=CTreeView
Filter=C
LastObject=CSymbolMapTreeView
VirtualFilter=VWC

[CLS:CSymbolMapListView]
Type=0
HeaderFile=SymbolMapListView.h
ImplementationFile=SymbolMapListView.cpp
BaseClass=CListView
Filter=C
LastObject=CSymbolMapListView
VirtualFilter=VWC

[CLS:CChildFrame4]
Type=0
HeaderFile=ChildFrm4.h
ImplementationFile=ChildFrm4.cpp
BaseClass=CChildFrameBase
Filter=M
LastObject=CChildFrame4
VirtualFilter=fWC

[CLS:COutputView]
Type=0
HeaderFile=OutputView.h
ImplementationFile=OutputView.cpp
BaseClass=CEditView
Filter=C
LastObject=COutputView
VirtualFilter=VWC

[CLS:CChildFrame5]
Type=0
HeaderFile=ChildFrm5.h
ImplementationFile=ChildFrm5.cpp
BaseClass=CChildFrameBase
Filter=M
LastObject=CChildFrame5
VirtualFilter=fWC

[CLS:CCompMatrixView]
Type=0
HeaderFile=CompMatrixView.h
ImplementationFile=CompMatrixView.cpp
BaseClass=CView
Filter=C
LastObject=ID_SYMBOLS_ADD
VirtualFilter=VWC

[DLG:CG_IDD_PROGRESS]
Type=1
Class=CProgressDlg
ControlCount=4
Control1=IDCANCEL,button,1342242817
Control2=CG_IDC_PROGDLG_PROGRESS,msctls_progress32,1350565889
Control3=CG_IDC_PROGDLG_PERCENT,static,1342308352
Control4=CG_IDC_PROGDLG_STATUS,static,1342308352

[CLS:CProgressDlg]
Type=0
HeaderFile=ProgDlg.h
ImplementationFile=ProgDlg.cpp
BaseClass=CDialog

[CLS:CChildFrame6]
Type=0
HeaderFile=ChildFrm6.h
ImplementationFile=ChildFrm6.cpp
BaseClass=CChildFrameBase
Filter=M
LastObject=CChildFrame6
VirtualFilter=fWC

[CLS:CCompDiffEditView]
Type=0
HeaderFile=CompDiffEditView.h
ImplementationFile=CompDiffEditView.cpp
BaseClass=CEditView
Filter=C
LastObject=CCompDiffEditView
VirtualFilter=VWC

