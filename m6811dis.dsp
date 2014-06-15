# Microsoft Developer Studio Project File - Name="M6811DIS" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=M6811DIS - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "M6811DIS.MAK".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "M6811DIS.MAK" CFG="M6811DIS - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "M6811DIS - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "M6811DIS - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "M6811DIS - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "M6811DIS - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
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

# Name "M6811DIS - Win32 Release"
# Name "M6811DIS - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ChildFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\CtlDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\CtlView.cpp
# End Source File
# Begin Source File

SOURCE=.\DfcLib.cpp
# End Source File
# Begin Source File

SOURCE=.\font.cpp
# End Source File
# Begin Source File

SOURCE=.\hexedit.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Programs\Common\Source\lstctlex.cpp
# End Source File
# Begin Source File

SOURCE=.\M6811DIS.cpp
# End Source File
# Begin Source File

SOURCE=.\hlp\M6811DIS.hpj
USERDEP__M6811="$(ProjDir)\hlp\AfxCore.rtf"	"$(ProjDir)\hlp\AfxPrint.rtf"	

!IF  "$(CFG)" == "M6811DIS - Win32 Release"

# Begin Custom Build - Making help file...
OutDir=.\Release
ProjDir=.
TargetName=M6811DIS
InputPath=.\hlp\M6811DIS.hpj

"$(OutDir)\$(TargetName).hlp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	call "$(ProjDir)\makehelp.bat"

# End Custom Build

!ELSEIF  "$(CFG)" == "M6811DIS - Win32 Debug"

# Begin Custom Build - Making help file...
OutDir=.\Debug
ProjDir=.
TargetName=M6811DIS
InputPath=.\hlp\M6811DIS.hpj

"$(OutDir)\$(TargetName).hlp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	call "$(ProjDir)\makehelp.bat"

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\M6811DIS.rc
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\NFPages.cpp
# End Source File
# Begin Source File

SOURCE=.\NFWizard.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Programs\Common\Source\ReadVer.cpp
# End Source File
# Begin Source File

SOURCE=.\Splash.cpp
# End Source File
# Begin Source File

SOURCE=.\SplashDg.cpp
# End Source File
# Begin Source File

SOURCE=.\SSDFCPg.cpp
# End Source File
# Begin Source File

SOURCE=.\SSSheet.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\SysSet.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ChildFrm.h
# End Source File
# Begin Source File

SOURCE=.\CtlDoc.h
# End Source File
# Begin Source File

SOURCE=.\CtlView.h
# End Source File
# Begin Source File

SOURCE=.\DfcLib.h
# End Source File
# Begin Source File

SOURCE=.\font.h
# End Source File
# Begin Source File

SOURCE=.\hexedit.h
# End Source File
# Begin Source File

SOURCE=.\M6811DIS.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\NFPages.h
# End Source File
# Begin Source File

SOURCE=.\NFWizard.h
# End Source File
# Begin Source File

SOURCE=..\..\Programs\Common\Include\ReadVer.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\Splash.h
# End Source File
# Begin Source File

SOURCE=.\SplashDg.h
# End Source File
# Begin Source File

SOURCE=.\SSDFCPg.h
# End Source File
# Begin Source File

SOURCE=.\SSSheet.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\SysSet.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\6811chip.bmp
# End Source File
# Begin Source File

SOURCE=.\res\CtlDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\Kathy.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Logiprob.bmp
# End Source File
# Begin Source File

SOURCE=.\res\M6811DIS.ico
# End Source File
# Begin Source File

SOURCE=.\res\M6811DIS.rc2
# End Source File
# Begin Source File

SOURCE=.\Splsh16.bmp
# End Source File
# Begin Source File

SOURCE=.\res\SrcDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\wizard1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Wizard2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Wizard3.bmp
# End Source File
# End Group
# Begin Group "Help Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\hlp\AfxCore.rtf
# End Source File
# Begin Source File

SOURCE=.\hlp\AfxPrint.rtf
# End Source File
# Begin Source File

SOURCE=.\hlp\AppExit.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\Bullet.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\CurArw2.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\CurArw4.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\CurHelp.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\EditCopy.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\EditCut.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\EditPast.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\EditUndo.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\FileNew.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\FileOpen.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\FilePrnt.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\FileSave.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\HlpSBar.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\HlpTBar.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\M6811DIS.cnt
# End Source File
# Begin Source File

SOURCE=.\MakeHelp.bat
# End Source File
# Begin Source File

SOURCE=.\hlp\RecFirst.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\RecLast.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\RecNext.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\RecPrev.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\Scmax.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\ScMenu.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\Scmin.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\M6811DIS.reg
# End Source File
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# Begin Source File

SOURCE=..\..\DevStudio\Vc\lib\Version.lib
# End Source File
# End Target
# End Project
# Section M6811DIS : {00000000-0035-0000-3500-00007B373241}
# 	1:10:IDB_SPLASH:102
# 	2:21:SplashScreenInsertKey:4.0
# End Section
# Section M6811DIS : {9475FB84-6DC5-11D1-858F-00600828300C}
# 	2:17:CLASS: CSplashDlg:CSplashDlg
# 	2:21:CLASS: CSplashCommand:CSplashCommand
# 	2:10:ENUM: enum:enum
# 	2:12:SplashDg.cpp:SplashDg.cpp
# 	2:19:Application Include:m6811dis.h
# 	2:10:SplashDg.h:SplashDg.h
# End Section
# Section M6811DIS : {062E8E83-83C7-11D1-858F-00600828300C}
# 	2:21:DefaultSinkHeaderFile:hexedit.h
# 	2:16:DefaultSinkClass:CHexEdit
# End Section
# Section M6811DIS : {57687361-646E-0000-C000-00002B010000}
# 	1:17:CG_IDS_DISK_SPACE:104
# 	1:19:CG_IDS_PHYSICAL_MEM:103
# 	1:25:CG_IDS_DISK_SPACE_UNAVAIL:105
# 	2:14:PhysicalMemory:CG_IDS_PHYSICAL_MEM
# 	2:9:DiskSpace:CG_IDS_DISK_SPACE
# 	2:16:SpaceUnavailable:CG_IDS_DISK_SPACE_UNAVAIL
# 	2:7:NewFunc:1
# 	2:10:SysInfoKey:1234
# End Section
# Section M6811DIS : {062E8E81-83C7-11D1-858F-00600828300C}
# 	2:5:Class:CHexEdit
# 	2:10:HeaderFile:hexedit.h
# 	2:8:ImplFile:hexedit.cpp
# End Section
# Section M6811DIS : {BEF6E003-A874-101A-8BBA-00AA00300CAB}
# 	2:5:Class:COleFont
# 	2:10:HeaderFile:font.h
# 	2:8:ImplFile:font.cpp
# End Section
