Dewtronics M6811 Code-Seeking Disassembler Version 1.20

This version is a DOS-32 version and must run under either Microsoft Win95,
Win98, or NT or under Linux (or similar UNIX) using Wine.  I have personally
tested it under Wine Alpha release 990613 and it actually runs better under
Wine than it does under Windows (surprise, surprise)...

To run the program, you must install the base program file M6811DIS.EXE.
Selecting this component will also install the required shared DLL's (MFC42,
MSVCRT, MSVCIRT, and DEWCMN1) in the Windows\System directory.  You will
also need at least one Data File Converter (DFC) installed.  DFC's are
dynamic libraries that provide the read/write functionality for data files.
This allows additional file types to be added without having to recompile or
modify the main program in any way.  A Software Toolkit will soon be available
to allow end users to create their own DFC's.  In this release DFC's for Intel
Hex and Binary files are included.  It is also recommended that you install
the instruction manual in at least one of the provided formats.  The extra
support files are optional files that may be useful when later reassembling
files.  And the example files contain sample control files to illustrate how
to use the disassembler, and are therefore optional for the installation.

Below is a list of files that are installed and where they are installed:

File          Location     Shared  Required  Description
------------  -----------  ------  --------  ----------------------------------------
M6811DIS.EXE  <TargetDir>    no     yes      Main Program File
MFC42.DLL     <WinSys>       yes    yes      Shared DLL for Application and DFC's
MSVCRT.DLL    <WinSys>       yes    yes      Shared DLL for Application and DFC's
MSVCIRT.DLL   <WinSys>       yes    yes      Shared DLL for Application and DFC's
DEWCMN1.DLL   <WinSys>       yes    yes      Shared DLL for Application and DFC's
INTELHEX.DFC  <WinSys>       yes    yes(*)   Intel Hex Data File Converter
BINARY.DFC    <WinSys>       yes    yes(*)   Binary Data File Converter
M6811DIS.PDF  <TargetDir>    no     no       Instruction Manual in Adobe PDF Format
M6811DIS.PS   <TargetDir>    no     no       Instruction Manual in PostScript Format
M6811DIS.TXT  <TargetDir>    no     no       Instruction Manual in Plain Text Format
M6811DIS.DOC  <TargetDir>    no     no       Instruction Manual in MS Word 97 Format
PORTSE9.ASM   <TargetDir>    no     no       Supplemental File for HC11E9 Reassembly
PORTSE9.H     <TargetDir>    no     no       Supplemental File for HC11E9 Reassembly
PORTSF1.ASM   <TargetDir>    no     no       Supplemental File for HC11F1 Reassembly
PORTSF1.H     <TargetDir>    no     no       Supplemental File for HC11F1 Reassembly
AV94BNBH.CTL  <TargetDir>    no     no       Example Control File
README.TXT    <TargetDir>    no     no       This File

(*) At least one DFC is required in order for the disassembler to read source
       data files.  Since the default DFC is binary, it is recommended that
       you at least install the BINARY.DFC.

Where:
   <TargetDir> is the directory you select to install the program under
   <WinSys> is the Windows System directory

Refer to the instruction manual for program changes and enhancements from the
previous 1.0 Version.

Windows Installation:
---------------------
For Windows, the package is available in two forms: 1) A self-extracting
InstallShield PFTW package called M6811DIS_v1p20.exe, and 2) A PKZIP/WinZip archive
called M6811DIS_v1p20.zip.  If you are installing from the PFTW package, just run
the M6811DIS_v1p20.exe.  If you are installing from the zip, unzip the files in the
M6811DIS_v1p20.zip archive into a temporary directory and run the SETUP.EXE program.
After installation, simply delete the files you unzipped from the temporary
directory.

Two setup type configurations have been defined: Typical and Custom.  Typical
installs all of the above files and Custom allows you total control to pick and
choose as needed.  The required shared files are automatically selected for
installation when the main program and/or DFC's are selected for installation.

Linux Installation:
-------------------
The above files are available in the M6811DIS_v1p20_RAW.tar.gz file so that you
can install and run without needing Windows itself.  For Linux, just extract the
files you want/need and start it with Wine.  This is just a archive of all of the
above files but without the fancy setup program.  While you can also use this tarball
to install the disassembler on Windows as well, it is recommended that you use the
setup program in the M6811DIS_v1p20.zip file or use the self-extracting
M6811DIS_v1p20.exe instead, as they check file versions and dependencies and
properly install the DLLs and other shared files.

The latest version of Wine can be found at:  www.winehq.com

Refer to the Wine documentation for information on setting up drive mappings,
directory locations, and how to launch windows applications.

Uninstalling from Windows:
--------------------------
To uninstall the program on Windows, select "Add/Remove Programs" under the
Control Panel, find "M6811DIS Ver 1.20" and select Add/Remove.

Uninstalling from Linux:
------------------------
On Linux, just delete the files that you extracted.


Email any problems/questions to: "Donald Whisnant" <dewtronics@tech-center.com>

Enjoy!

