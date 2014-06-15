; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CSrcView
LastTemplate=CFormView
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "m6811dis.h"
LastPage=0

ClassCount=16
Class1=CChildFrame
Class2=CCTLFileDoc
Class3=CCTLFileView
Class4=CM6811DISApp
Class5=CAboutDlg
Class6=CMainFrame
Class7=CNewFileIntroPage
Class8=CNewFileTypePage
Class9=CNewFileSettingsPage
Class10=CNewFileWizard
Class11=CSplashWnd
Class12=CDFCSettingsPage
Class13=CSystemSettingsSheet

ResourceCount=10
Resource1=IDD_ABOUTBOX
Resource2=IDR_DSMCTLTYPE
Resource3=IDD_SS_DFC_SETTINGS
Resource4=IDR_MAINFRAME
Resource5=IDD_NEWFILE_INTRO
Resource6=IDD_NEWFILE_SETTINGS
Resource7=IDR_SRCTYPE
Resource8=IDD_M6811DIS_FORM
Class14=CSrcDoc
Class15=CSrcMdiFrame
Resource9=IDD_NEWFILE_TYPE
Class16=CSrcView
Resource10=IDD_SRCDOC_FORM

[CLS:CChildFrame]
Type=0
BaseClass=CMDIChildWnd
HeaderFile=ChildFrm.h
ImplementationFile=ChildFrm.cpp

[CLS:CCTLFileDoc]
Type=0
BaseClass=CDocument
HeaderFile=CtlDoc.h
ImplementationFile=CtlDoc.cpp

[CLS:CCTLFileView]
Type=0
BaseClass=CFormView
HeaderFile=CtlView.h
ImplementationFile=CtlView.cpp

[CLS:CM6811DISApp]
Type=0
BaseClass=CWinApp
HeaderFile=M6811DIS.h
ImplementationFile=M6811DIS.cpp
LastObject=CM6811DISApp

[CLS:CAboutDlg]
Type=0
BaseClass=CDialog
HeaderFile=M6811DIS.cpp
ImplementationFile=M6811DIS.cpp
LastObject=CAboutDlg

[CLS:CMainFrame]
Type=0
BaseClass=CMDIFrameWnd
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=T
VirtualFilter=fWC
LastObject=CMainFrame

[CLS:CNewFileIntroPage]
Type=0
BaseClass=CPropertyPage
HeaderFile=NFPages.h
ImplementationFile=NFPages.cpp
LastObject=CNewFileIntroPage
Filter=D
VirtualFilter=idWC

[CLS:CNewFileTypePage]
Type=0
BaseClass=CPropertyPage
HeaderFile=NFPages.h
ImplementationFile=NFPages.cpp
LastObject=CNewFileTypePage
Filter=D
VirtualFilter=idWC

[CLS:CNewFileSettingsPage]
Type=0
BaseClass=CPropertyPage
HeaderFile=NFPages.h
ImplementationFile=NFPages.cpp
LastObject=CNewFileSettingsPage
Filter=D
VirtualFilter=idWC

[CLS:CNewFileWizard]
Type=0
BaseClass=CPropertySheet
HeaderFile=NFWizard.h
ImplementationFile=NFWizard.cpp
LastObject=CNewFileWizard

[CLS:CSplashWnd]
Type=0
BaseClass=CWnd
HeaderFile=Splash.h
ImplementationFile=Splash.cpp
Filter=W
VirtualFilter=WC
LastObject=CSplashWnd

[CLS:CDFCSettingsPage]
Type=0
BaseClass=CPropertyPage
HeaderFile=SSDFCPg.h
ImplementationFile=SSDFCPg.cpp
LastObject=IDC_DFC_LIST

[CLS:CSystemSettingsSheet]
Type=0
BaseClass=CPropertySheet
HeaderFile=SSSheet.h
ImplementationFile=SSSheet.cpp

[DLG:IDD_M6811DIS_FORM]
Type=1
Class=CCTLFileView
ControlCount=17
Control1=IDC_STATIC,static,1342308352
Control2=IDC_SOURCE_FILE_LIST,SysListView32,1350631429
Control3=IDC_VIEW_SOURCE,button,1342242816
Control4=IDC_EDIT_SOURCE,button,1342242816
Control5=IDC_ADD_SOURCE,button,1342242816
Control6=IDC_REMOVE_SOURCE,button,1342242816
Control7=IDC_STATIC,button,1342178055
Control8=IDC_OPCODES,button,1342242819
Control9=IDC_ADDRESSES,button,1342242819
Control10=IDC_ASCII,button,1342242819
Control11=IDC_STATIC,button,1342178055
Control12=IDC_CODESEEK_ENABLED,button,1342177289
Control13=IDC_CODESEEK_DISABLED,button,1342177289
Control14=IDC_CODESEEK_SELECTIVE,button,1342177289
Control15=IDC_CODESEEK_RANGE_LIST,SysListView32,1350631429
Control16=IDC_STATIC,static,1342308352
Control17=IDC_PROJECT_TREE,SysTreeView32,1350631427

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=10
Control1=IDC_STATIC,static,1342308480
Control2=IDC_COPYRIGHT,static,1342308353
Control3=IDOK,button,1342373889
Control4=IDC_VERSION,static,1342308353
Control5=IDC_STATIC,static,1342308354
Control6=IDC_STATIC,static,1342308354
Control7=IDC_PHYSICAL_MEM,edit,1350633600
Control8=IDC_DISK_SPACE,edit,1350633600
Control9=IDC_STATIC,button,1342178055
Control10=IDC_STATIC,static,1342179854

[DLG:IDD_NEWFILE_INTRO]
Type=1
Class=CNewFileIntroPage
ControlCount=4
Control1=IDC_NFTYPE_CONTROL,button,1342308361
Control2=IDC_NFTYPE_DATA,button,1342177289
Control3=IDC_STATIC,button,1342178055
Control4=IDC_STATIC,static,1342179854

[DLG:IDD_NEWFILE_TYPE]
Type=1
Class=CNewFileTypePage
ControlCount=12
Control1=IDC_STATIC,static,1342308352
Control2=IDC_FILE_TYPE_LIST,listbox,1352728835
Control3=IDC_CF_FLAG,button,1342242819
Control4=IDC_CF_LIST,combobox,1344340226
Control5=IDC_STATIC,static,1342308352
Control6=IDC_FILENAME,edit,1350631552
Control7=IDC_STATIC,static,1342308352
Control8=IDC_LOCATION,edit,1350631552
Control9=IDC_SELECT_DIRECTORY,button,1342242816
Control10=IDC_STATIC,static,1342308352
Control11=IDC_STATIC,static,1342308352
Control12=IDC_STATIC,static,1342308352

[DLG:IDD_NEWFILE_SETTINGS]
Type=1
Class=CNewFileSettingsPage
ControlCount=5
Control1=IDC_STATIC,static,1342308352
Control2=IDC_FILENAME,edit,1350631552
Control3=IDC_STATIC,static,1342308352
Control4=IDC_LOCATION,edit,1350631552
Control5=IDC_SELECT_DIRECTORY,button,1342242816

[DLG:IDD_SS_DFC_SETTINGS]
Type=1
Class=CDFCSettingsPage
ControlCount=4
Control1=IDC_STATIC,static,1342308352
Control2=IDC_DFC_LIST,SysListView32,1350665229
Control3=IDC_LOAD,button,1342242816
Control4=IDC_UNLOAD,button,1342242816

[TB:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_CUT
Command5=ID_EDIT_COPY
Command6=ID_EDIT_PASTE
Command7=ID_FILE_PRINT
Command8=ID_APP_ABOUT
Command9=ID_CONTEXT_HELP
CommandCount=9

[MNU:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_PRINT_SETUP
Command4=ID_FILE_MRU_FILE1
Command5=ID_APP_EXIT
Command6=ID_VIEW_TOOLBAR
Command7=ID_VIEW_STATUS_BAR
Command8=ID_SETTINGS
Command9=ID_HELP_FINDER
Command10=ID_APP_ABOUT
CommandCount=10

[MNU:IDR_DSMCTLTYPE]
Type=1
Class=?
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
Command15=ID_VIEW_TOOLBAR
Command16=ID_VIEW_STATUS_BAR
Command17=ID_SETTINGS
Command18=ID_WINDOW_NEW
Command19=ID_WINDOW_CASCADE
Command20=ID_WINDOW_TILE_HORZ
Command21=ID_WINDOW_ARRANGE
Command22=ID_HELP_FINDER
Command23=ID_APP_ABOUT
CommandCount=23

[MNU:IDR_SRCTYPE]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_CLOSE
Command4=ID_FILE_SAVE
Command5=ID_FILE_SAVE_AS
Command6=ID_FILE_CONVERT
Command7=ID_FILE_PRINT
Command8=ID_FILE_PRINT_PREVIEW
Command9=ID_FILE_PRINT_SETUP
Command10=ID_FILE_MRU_FILE1
Command11=ID_APP_EXIT
Command12=ID_EDIT_UNDO
Command13=ID_EDIT_CUT
Command14=ID_EDIT_COPY
Command15=ID_EDIT_PASTE
Command16=ID_HEXEDIT_PROPERTIES
Command17=ID_VIEW_TOOLBAR
Command18=ID_VIEW_STATUS_BAR
Command19=ID_SETTINGS
Command20=ID_WINDOW_NEW
Command21=ID_WINDOW_CASCADE
Command22=ID_WINDOW_TILE_HORZ
Command23=ID_WINDOW_ARRANGE
Command24=ID_HELP_FINDER
Command25=ID_APP_ABOUT
CommandCount=25

[ACL:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_PRINT
Command5=ID_EDIT_UNDO
Command6=ID_EDIT_CUT
Command7=ID_EDIT_COPY
Command8=ID_EDIT_PASTE
Command9=ID_EDIT_UNDO
Command10=ID_EDIT_CUT
Command11=ID_EDIT_COPY
Command12=ID_EDIT_PASTE
Command13=ID_NEXT_PANE
Command14=ID_PREV_PANE
Command15=ID_CONTEXT_HELP
Command16=ID_HELP
CommandCount=16

[CLS:CSrcDoc]
Type=0
HeaderFile=DfcLib.h
ImplementationFile=DfcLib.cpp
BaseClass=CDocument
Filter=N
LastObject=CSrcDoc
VirtualFilter=DC

[CLS:CSrcMdiFrame]
Type=0
HeaderFile=DfcLib.h
ImplementationFile=DfcLib.cpp
BaseClass=CMDIChildWnd
Filter=M
LastObject=CSrcMdiFrame
VirtualFilter=mfWC

[DLG:IDD_SRCDOC_FORM]
Type=1
Class=CSrcView
ControlCount=1
Control1=IDC_HEXEDIT,{062E8E83-83C7-11D1-858F-00600828300C},1073807360

[CLS:CSrcView]
Type=0
HeaderFile=dfclib.h
ImplementationFile=dfclib.cpp
BaseClass=CFormView
Filter=D
LastObject=CSrcView
VirtualFilter=VWC

