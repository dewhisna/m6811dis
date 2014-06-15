# Microsoft Developer Studio Project File - Name="FuncView" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=FuncView - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "FuncView.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "FuncView.mak" CFG="FuncView - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "FuncView - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "FuncView - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "FuncView - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "FuncView - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "FuncView - Win32 Release"
# Name "FuncView - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ChildFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\ChildFrm2.cpp
# End Source File
# Begin Source File

SOURCE=.\ChildFrm3.cpp
# End Source File
# Begin Source File

SOURCE=.\ChildFrm4.cpp
# End Source File
# Begin Source File

SOURCE=.\ChildFrm5.cpp
# End Source File
# Begin Source File

SOURCE=.\ChildFrm6.cpp
# End Source File
# Begin Source File

SOURCE=.\ChildFrmBase.cpp
# End Source File
# Begin Source File

SOURCE=.\CompDiffEditView.cpp
# End Source File
# Begin Source File

SOURCE=.\CompMatrixView.cpp
# End Source File
# Begin Source File

SOURCE=..\funccomp.cpp
# End Source File
# Begin Source File

SOURCE=..\funcdesc.cpp
# End Source File
# Begin Source File

SOURCE=.\FuncDiffEditView.cpp
# End Source File
# Begin Source File

SOURCE=.\FuncListView.cpp
# End Source File
# Begin Source File

SOURCE=.\FuncView.cpp
# End Source File
# Begin Source File

SOURCE=.\FuncView.rc
# End Source File
# Begin Source File

SOURCE=.\FuncViewPrjDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\FuncViewPrjVw.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\OutputView.cpp
# End Source File
# Begin Source File

SOURCE=.\ProgDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\SymbolMapListView.cpp
# End Source File
# Begin Source File

SOURCE=.\SymbolMapTreeView.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ChildFrm.h
# End Source File
# Begin Source File

SOURCE=.\ChildFrm2.h
# End Source File
# Begin Source File

SOURCE=.\ChildFrm3.h
# End Source File
# Begin Source File

SOURCE=.\ChildFrm4.h
# End Source File
# Begin Source File

SOURCE=.\ChildFrm5.h
# End Source File
# Begin Source File

SOURCE=.\ChildFrm6.h
# End Source File
# Begin Source File

SOURCE=.\ChildFrmBase.h
# End Source File
# Begin Source File

SOURCE=.\CompDiffEditView.h
# End Source File
# Begin Source File

SOURCE=.\CompMatrixView.h
# End Source File
# Begin Source File

SOURCE=..\funccomp.h
# End Source File
# Begin Source File

SOURCE=..\funcdesc.h
# End Source File
# Begin Source File

SOURCE=.\FuncDiffEditView.h
# End Source File
# Begin Source File

SOURCE=.\FuncListView.h
# End Source File
# Begin Source File

SOURCE=.\FuncView.h
# End Source File
# Begin Source File

SOURCE=.\FuncViewPrjDoc.h
# End Source File
# Begin Source File

SOURCE=.\FuncViewPrjVw.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\OutputView.h
# End Source File
# Begin Source File

SOURCE=.\ProgDlg.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\SymbolMapListView.h
# End Source File
# Begin Source File

SOURCE=.\SymbolMapTreeView.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\CompMatrixType.ico
# End Source File
# Begin Source File

SOURCE=.\res\FuncDiffType.ico
# End Source File
# Begin Source File

SOURCE=.\res\FuncView.ico
# End Source File
# Begin Source File

SOURCE=.\res\FuncView.rc2
# End Source File
# Begin Source File

SOURCE=.\res\FuncViewPrjDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\OutputType.ico
# End Source File
# Begin Source File

SOURCE=.\res\SymbolMapType.ico
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\FuncView.reg
# End Source File
# End Target
# End Project
