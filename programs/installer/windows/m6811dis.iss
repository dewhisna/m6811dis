;****************************************************************************
;**
;** Copyright (C) 2014 Donna Whisnant, a.k.a. Dewtronics.
;** Contact: http://www.dewtronics.com/
;**
;****************************************************************************

[Setup]
AppId=m6811dis
AppName=m6811dis
AppVersion=2.00
AppVerName=m6811dis 2.00
OutputBaseFilename=m6811dis-2.00
AppCopyright=Copyright (C) 2014 Donna Whisnant, a.k.a. Dewtronics.
AppPublisher=Dewtronics
AppPublisherURL=http://www.dewtronics.com/
AppContact=Donna Whisnant
AppSupportURL=https://sourceforge.net/projects/m6811dis/
AppSupportPhone=
AppComments=Motorola 6811 Code-Seeking Disassembler
DefaultDirName={sd}\m6811dis
DefaultGroupName=m6811dis
ShowLanguageDialog=auto
Compression=lzma

[Files]
; app
Source: "..\..\dist\windows\m6811dis\m6811dis\m6811dis.exe"; DestDir: "{app}\m6811dis"; Flags: ignoreversion;
Source: "..\..\dist\windows\m6811dis\m6811dis\libgcc_s_sjlj-1.dll"; DestDir: "{app}\m6811dis"; Flags: ignoreversion;
Source: "..\..\dist\windows\m6811dis\m6811dis\libstdc++-6.dll"; DestDir: "{app}\m6811dis"; Flags: ignoreversion;
Source: "..\..\dist\windows\m6811dis\m6811dis\libwinpthread-1.dll"; DestDir: "{app}\m6811dis"; Flags: ignoreversion;

; doc
Source: "..\..\dist\windows\m6811dis\doc\m6811dis.pdf"; DestDir: "{app}\doc"; Flags: ignoreversion;

; examples:
Source: "..\..\dist\windows\m6811dis\examples\av94bnbh.ctl"; DestDir: "{app}\examples"; Flags: ignoreversion;
Source: "..\..\dist\windows\m6811dis\examples\av94bnbh.log"; DestDir: "{app}\examples"; Flags: ignoreversion;
Source: "..\..\dist\windows\m6811dis\examples\base.ctl"; DestDir: "{app}\examples"; Flags: ignoreversion;
Source: "..\..\dist\windows\m6811dis\examples\base.log"; DestDir: "{app}\examples"; Flags: ignoreversion;

; support:
Source: "..\..\dist\windows\m6811dis\support\portse9.asm"; DestDir: "{app}\support"; Flags: ignoreversion;
Source: "..\..\dist\windows\m6811dis\support\portse9.h"; DestDir: "{app}\support"; Flags: ignoreversion;
Source: "..\..\dist\windows\m6811dis\support\portsf1.asm"; DestDir: "{app}\support"; Flags: ignoreversion;
Source: "..\..\dist\windows\m6811dis\support\portsf1.h"; DestDir: "{app}\support"; Flags: ignoreversion;

[Icons]
Name: "{group}\M6811DIS User Manual"; Filename: "{app}\doc\m6811dis.pdf";
Name: "{group}\{cm:UninstallProgram,{#SetupSetting("AppName")}}"; Filename: "{uninstallexe}";
Name: "{group}\Examples"; Filename: "{app}\examples";

