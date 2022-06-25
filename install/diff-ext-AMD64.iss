;Copyright (c) 2003-2010, Sergey Zorin. All rights reserved.
;
;This software is distributable under the BSD license. See the terms of the
;BSD license in the LICENSE file provided with this software.

[Setup]
Compression=lzma/normal
SolidCompression=yes
AppName=diff-ext
AppId=A0482097-C69D-4DEC-8AB6-D3A259ACC151
AppVerName=diff-ext 1.9.2
AppPublisher=Z
DefaultDirName={pf}\Z\diff-ext
DefaultGroupName=Z\diff-ext
DisableProgramGroupPage=false
OutputDir=.
OutputBaseFilename=diff-ext
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64

; uncomment the following line if you want your installation to run on NT 3.51 too.
; MinVersion=4,3.51

PrivilegesRequired=none
ShowLanguageDialog=yes

[Languages]
Name: en; MessagesFile: compiler:Default.isl; LicenseFile: ..\LICENSE.rtf

[Components]
Name: main; Description: diff-ext; Types: full compact custom; Flags: fixed

Name: icons; Description: Context menu icons; Types: full custom; Languages: en

Name: icons\diff_ext_icons; Description: diff-ext; Types: full custom; Flags: exclusive
Name: icons\meta_diff_icons; Description: meta-diff; Types: custom; Flags: exclusive

Name: translations; Description: Translations; Types: custom; Languages: en


[Files]
; Platform independent files
Source: ..\diff-ext\icons\diff-ext\clear.ico; DestDir: {app}\icons; Flags: ; Components: icons\diff_ext_icons
Source: ..\diff-ext\icons\diff-ext\diff3.ico; DestDir: {app}\icons; Flags: ; Components: icons\diff_ext_icons
Source: ..\diff-ext\icons\diff-ext\diff3_with.ico; DestDir: {app}\icons; Flags: ; Components: icons\diff_ext_icons
Source: ..\diff-ext\icons\diff-ext\diff.ico; DestDir: {app}\icons; Flags: ; Components: icons\diff_ext_icons
Source: ..\diff-ext\icons\diff-ext\diff_later.ico; DestDir: {app}\icons; Flags: ; Components: icons\diff_ext_icons
Source: ..\diff-ext\icons\diff-ext\diff_with.ico; DestDir: {app}\icons; Flags: ; Components: icons\diff_ext_icons
Source: ..\diff-ext\icons\meta-diff\clear.ico; DestDir: {app}\icons; Flags: ; Components: icons\meta_diff_icons
Source: ..\diff-ext\icons\meta-diff\diff3.ico; DestDir: {app}\icons; Flags: ; Components: icons\meta_diff_icons
Source: ..\diff-ext\icons\meta-diff\diff3_with.ico; DestDir: {app}\icons; Flags: ; Components: icons\meta_diff_icons
Source: ..\diff-ext\icons\meta-diff\diff.ico; DestDir: {app}\icons; Flags: ; Components: icons\meta_diff_icons
Source: ..\diff-ext\icons\meta-diff\diff_later.ico; DestDir: {app}\icons; Flags: ; Components: icons\meta_diff_icons
Source: ..\diff-ext\icons\meta-diff\diff_with.ico; DestDir: {app}\icons; Flags: ; Components: icons\meta_diff_icons

; 64-bit AMD files
Source: ..\build\diff-ext.dll; DestDir: {app}; Flags: regserver restartreplace uninsrestartdelete
Source: ..\build\diff-ext-setup.exe; DestDir: {app}; Flags: 


;restartreplace uninsrestartdelete ignoreversion
;regserver restartreplace uninsrestartdelete uninsremovereadonly
Source: ..\LICENSE; DestDir: {app}; Flags: uninsremovereadonly; Attribs: readonly

[Icons]
Name: {group}\setup; Filename: {app}\diff-ext-setup.exe; WorkingDir: {app}
Name: {group}\Uninstall diff-ext; Filename: {uninstallexe}

[Run]
Filename: {app}\diff-ext-setup.exe; Flags: postinstall

[Registry]
Root: HKCU; Subkey: Software\Z; Flags: uninsdeletekeyifempty
Root: HKCU; Subkey: Software\Z\diff-ext; Flags: uninsdeletekey
Root: HKCU; Subkey: Software\Z\diff-ext\history; Flags: uninsdeletekey
Root: HKCU; Subkey: Software\Z\diff-ext; ValueType: string; ValueName: diff; ValueData: "<path to diff> ""%1"" ""%2"""; Flags: uninsdeletekey createvalueifdoesntexist
Root: HKCU; Subkey: Software\Z\diff-ext; ValueType: string; ValueName: diff3; ValueData: "<path to diff3> ""%1"" ""%2"" ""%3"""; Flags: uninsdeletekey createvalueifdoesntexist
Root: HKCU; Subkey: Software\Z\diff-ext; ValueType: dword; ValueName: language; ValueData: 1033; Flags: uninsdeletekey; Languages: en
Root: HKCU; Subkey: Software\Z\diff-ext; ValueType: dword; ValueName: 3way_compare_supported; ValueData: 0; Flags: uninsdeletekey createvalueifdoesntexist
Root: HKCU; Subkey: Software\Z\diff-ext; ValueType: dword; ValueName: persistent_selection; ValueData: 1; Flags: uninsdeletekey createvalueifdoesntexist

[Code]
procedure CurStepChanged(CurStep: TSetupStep);
var
  dword_data: Cardinal;
  string_data: String;
  history: TArrayOfString;
  i: Integer;
begin
  if CurStep = ssPostInstall then begin
    if RegKeyExists(HKLM, 'Software\Z\diff-ext') then begin
      if RegQueryDWordValue(HKLM, 'Software\Z\diff-ext', 'language', dword_data) then begin
        RegWriteDWordValue(HKCU, 'Software\Z\diff-ext', 'language', dword_data);
      end;

      if RegQueryDWordValue(HKLM, 'Software\Z\diff-ext', '3way_compare_supported', dword_data) then begin
        RegWriteDWordValue(HKCU, 'Software\Z\diff-ext', '3way_compare_supported', dword_data);
      end;

      if RegQueryStringValue(HKLM, 'Software\Z\diff-ext', 'diff', string_data) then begin
        RegWriteStringValue(HKCU, 'Software\Z\diff-ext', 'diff', string_data);
      end;

      if RegQueryStringValue(HKLM, 'Software\Z\diff-ext', 'diff3', string_data) then begin
        RegWriteStringValue(HKCU, 'Software\Z\diff-ext', 'diff3', string_data);
      end;

      if RegGetValueNames(HKLM, 'Software\Z\diff-ext\history', history) then begin
        for i := 0 to GetArrayLength(history)-1 do begin
          RegQueryStringValue(HKLM, 'Software\Z\diff-ext\history', history[i], string_data);
          RegWriteStringValue(HKCU, 'Software\Z\diff-ext\history', history[i], string_data);
        end;
      end;

      RegDeleteKeyIncludingSubkeys(HKLM, 'Software\Z\diff-ext');
      RegDeleteKeyIfEmpty(HKLM, 'Software\Z');
    end;
  end;
end;

procedure CurUninstallStepChanged(CurStep: TUninstallStep);
begin
  if CurStep = usPostUninstall then begin
    if RegKeyExists(HKCR, '\Directory\shellex\ContextMenuHandlers\diff-ext') then begin
      RegDeleteKeyIncludingSubkeys(HKCR, '\Directory\shellex\ContextMenuHandlers\diff-ext');
    end;
    if RegKeyExists(HKCR, '\Folder\shellex\ContextMenuHandlers\diff-ext') then begin
      RegDeleteKeyIncludingSubkeys(HKCR, '\Folder\shellex\ContextMenuHandlers\diff-ext');
    end;
  end;
end;


