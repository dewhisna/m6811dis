@echo off
REM -- First make map file from Microsoft Visual C++ generated resource.h
echo // MAKEHELP.BAT generated Help Map file.  Used by M6811DIS.HPJ. >"hlp\M6811DIS.hm"
echo. >>"hlp\M6811DIS.hm"
echo // Commands (ID_* and IDM_*) >>"hlp\M6811DIS.hm"
makehm ID_,HID_,0x10000 IDM_,HIDM_,0x10000 resource.h >>"hlp\M6811DIS.hm"
echo. >>"hlp\M6811DIS.hm"
echo // Prompts (IDP_*) >>"hlp\M6811DIS.hm"
makehm IDP_,HIDP_,0x30000 resource.h >>"hlp\M6811DIS.hm"
echo. >>"hlp\M6811DIS.hm"
echo // Resources (IDR_*) >>"hlp\M6811DIS.hm"
makehm IDR_,HIDR_,0x20000 resource.h >>"hlp\M6811DIS.hm"
echo. >>"hlp\M6811DIS.hm"
echo // Dialogs (IDD_*) >>"hlp\M6811DIS.hm"
makehm IDD_,HIDD_,0x20000 resource.h >>"hlp\M6811DIS.hm"
echo. >>"hlp\M6811DIS.hm"
echo // Frame Controls (IDW_*) >>"hlp\M6811DIS.hm"
makehm IDW_,HIDW_,0x50000 resource.h >>"hlp\M6811DIS.hm"
REM -- Make help for Project M6811DIS


echo Building Win32 Help files
start /wait hcw /C /E /M "hlp\M6811DIS.hpj"
if errorlevel 1 goto :Error
if not exist "hlp\M6811DIS.hlp" goto :Error
if not exist "hlp\M6811DIS.cnt" goto :Error
echo.
if exist Debug\nul copy "hlp\M6811DIS.hlp" Debug
if exist Debug\nul copy "hlp\M6811DIS.cnt" Debug
if exist Release\nul copy "hlp\M6811DIS.hlp" Release
if exist Release\nul copy "hlp\M6811DIS.cnt" Release
echo.
goto :done

:Error
echo hlp\M6811DIS.hpj(1) : error: Problem encountered creating help file

:done
echo.
