; basic script template for NSIS installers
;
; Written by Philip Chu
; Copyright (c) 2004-2005 Technicat, LLC
;
; This software is provided 'as-is', without any express or implied warranty.
; In no event will the authors be held liable for any damages arising from the use of this software.
 
; Permission is granted to anyone to use this software for any purpose,
; including commercial applications, and to alter it ; and redistribute
; it freely, subject to the following restrictions:
 
;    1. The origin of this software must not be misrepresented; you must not claim that
;       you wrote the original software. If you use this software in a product, an
;       acknowledgment in the product documentation would be appreciated but is not required.
 
;    2. Altered source versions must be plainly marked as such, and must not be
;       misrepresented as being the original software.
 
;    3. This notice may not be removed or altered from any source distribution.
 
; This version modified for use by Gambit; changes copyright (C) 2006 by The Gambit Project

 
 
;!define setup "setup.exe"
 
; change this to wherever the files to be packaged reside
!define srcdir "E:\Gambit"
 
!define company "Gambit"
 
!define prodname "Gambit"
!define exec "gambit.exe"
!define setup "gambit-0.2006.01.20.exe"
!define filetype ".gbt"
 
; optional stuff
 
; text file to open in notepad after installation
!define notefile "README"
 
; license text file
!define licensefile "COPYING"
 
; icons must be Microsoft .ICO files
; !define icon "icon.ico"
 
; installer background screen
; !define screenimage background.bmp
 
; file containing list of file-installation commands
; !define files "files.nsi"
 
; file containing list of file-uninstall commands
; !define unfiles "unfiles.nsi"
 
; registry stuff
 
!define regkey "Software\${company}\${prodname}"
!define uninstkey "Software\Microsoft\Windows\CurrentVersion\Uninstall\${prodname}"
 
!define startmenu "$SMPROGRAMS\${prodname}"
!define uninstaller "uninstall.exe"
 
;--------------------------------
 
XPStyle on
ShowInstDetails hide
ShowUninstDetails hide
 
Name "${prodname}"
Caption "${prodname}"
 
!ifdef icon
Icon "${icon}"
!endif
 
OutFile "${setup}"
 
SetDateSave on
SetDatablockOptimize on
CRCCheck on
SilentInstall normal
 
InstallDir "$PROGRAMFILES\${prodname}"
InstallDirRegKey HKLM "${regkey}" ""
 
!ifdef licensefile
LicenseText "License"
LicenseData "${srcdir}\${licensefile}"
!endif
 
; pages
; we keep it simple - leave out selectable installation types
 
!ifdef licensefile
Page license
!endif
 
; Page components
Page directory
Page instfiles
 
UninstPage uninstConfirm
UninstPage instfiles
 
;--------------------------------
 
AutoCloseWindow false
ShowInstDetails show
 
 
!ifdef screenimage
 
; set up background image
; uses BgImage plugin
 
Function .onGUIInit
	; extract background BMP into temp plugin directory
	InitPluginsDir
	File /oname=$PLUGINSDIR\1.bmp "${screenimage}"
 
	BgImage::SetBg /NOUNLOAD /FILLSCREEN $PLUGINSDIR\1.bmp
	BgImage::Redraw /NOUNLOAD
FunctionEnd
 
Function .onGUIEnd
	; Destroy must not have /NOUNLOAD so NSIS will be able to unload and delete BgImage before it exits
	BgImage::Destroy
FunctionEnd
 
!endif
 
; beginning (invisible) section
Section
 
  WriteRegStr HKLM "${regkey}" "Install_Dir" "$INSTDIR"
  ; write uninstall strings
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Product" \
                     "DisplayName" "Gambit"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ImageMaker" \
                   "UninstallString" "$INSTDIR\uninstall.exe"

  WriteRegStr HKLM "Software\Classes\Applications\gambit.exe\shell\FriendlyCache" "" "Gambit"
  WriteRegStr HKLM "Software\Classes\Applications\gambit.exe\shell\open\command" "" '$INSTDIR\bin\gambit.exe "%1"'
  
  
  WriteRegStr HKCR ".gbt" "" "Gambit.Workbook"
  WriteRegStr HKCR "Gambit.Workbook\shell\open\command" "" '"$INSTDIR\bin\gambit.exe "%1"'
  WriteRegStr HKCR "Gambit.Workbook\DefaultIcon\" "" "$INSTDIR\gambit.ico"

  WriteRegStr HKCR ".efg" "" "Gambit.ExtensiveGame"
  WriteRegStr HKCR "Gambit.ExtensiveGame\shell\open\command" "" '"$INSTDIR\bin\gambit.exe "%1"'
  WriteRegStr HKCR "Gambit.ExtensiveGame\DefaultIcon\" "" "$INSTDIR\gambit.ico"

  WriteRegStr HKCR ".nfg" "" "Gambit.StrategicGame"
  WriteRegStr HKCR "Gambit.StrategicGame\shell\open\command" "" '"$INSTDIR\bin\gambit.exe "%1"'
  WriteRegStr HKCR "Gambit.StrategicGame\DefaultIcon\" "" "$INSTDIR\gambit.ico"

  SetOutPath $INSTDIR
 
 
; package all files, recursively, preserving attributes
; assume files are in the correct places
File /a /r /x "*.nsi" "${srcdir}\*.*"
 
!ifdef licensefile
File /a "${srcdir}\${licensefile}"
!endif
 
!ifdef notefile
File /a "${srcdir}\${notefile}"
!endif
 
!ifdef icon
File /a "${srcdir}\${icon}"
!endif
 
; any application-specific files
!ifdef files
!include "${files}"
!endif
 
  WriteUninstaller "${uninstaller}"
  
SectionEnd
 
; create shortcuts
Section
  
  CreateDirectory "${startmenu}"
  SetOutPath $INSTDIR ; for working directory
!ifdef icon
  CreateShortCut "${startmenu}\${prodname}.lnk" "$INSTDIR\${exec}" "" "$INSTDIR\${icon}"
!else
  CreateShortCut "${startmenu}\${prodname}.lnk" "$INSTDIR\bin\gambit.exe"
!endif
 
!ifdef notefile
  CreateShortCut "${startmenu}\Release Notes.lnk "$INSTDIR\${notefile}"
!endif
 
  CreateShortCut "${startmenu}\Gambit manual.lnk "$INSTDIR\doc\gambit\index.html"
 
  CreateShortCut "${startmenu}\Uninstall Gambit.lnk "$INSTDIR\${uninstaller}"
  
!ifdef website
WriteINIStr "${startmenu}\web site.url" "InternetShortcut" "URL" ${website}
 ; CreateShortCut "${startmenu}\Web Site.lnk "${website}" "URL"
!endif
 
!ifdef notefile
ExecShell "open" "$INSTDIR\${notefile}"
!endif
 
SectionEnd
 
; Uninstaller
; All section names prefixed by "Un" will be in the uninstaller
 
UninstallText "This will uninstall ${prodname}."
 
!ifdef icon
UninstallIcon "${icon}"
!endif
 
Section "Uninstall"
 
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Gambit"  
  
  Delete "${startmenu}\*.*"
  RmDir "${startmenu}"
 
!ifdef licensefile
Delete "$INSTDIR\${licensefile}"
!endif
 
!ifdef notefile
Delete "$INSTDIR\${notefile}"
!endif
 
!ifdef icon
Delete "$INSTDIR\${icon}"
!endif
 
  Delete "$INSTDIR\bin\*.*"
  RmDir "$INSTDIR\bin"
  Delete "$INSTDIR\doc\gambit\*.*"
  RmDir "$INSTDIR\doc\gambit"
  Delete "$INSTDIR\doc\icons\*.*"
  RmDir "$INSTDIR\doc\icons"
  Delete "$INSTDIR\doc\images\*.*"
  RmDir "$INSTDIR\doc\images"
  Delete $INSTDIR\doc\screens\*.*"
  RmDir "$INSTDIR\doc\screens"
  RmDir "$INSTDIR\doc"
  Delete "$INSTDIR\games\*.*"
  RmDir "$INSTDIR\games"
  Delete "$INSTDIR\*.*"
  RmDir "$INSTDIR"
 
!ifdef unfiles
!include "${unfiles}"
!endif
 
SectionEnd