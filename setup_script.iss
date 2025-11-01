#define VersionString "0.0.15"
#define AppId "1f7e9ebf-ed92-4d88-8eac-89e3fe53282c"
#define VC_Redist_URL "https://aka.ms/vs/17/release/vc_redist.x64.exe"

[Setup]
AppName=qtedit4
AppVersion={#VersionString}
AppId={#AppId}
DefaultDirName={pf}\qtedit4
DefaultGroupName=qtedit4
UninstallDisplayIcon={app}\qtedit4.ico
OutputDir=dist
;OutputBaseFilename=qtedit4-qt6.8.1-v{#VersionString}-x86_64
OutputBaseFilename=qtedit4-win64
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64
ShowComponentSizes=yes
SetupIconFile=qtedit4.ico

[Files]
Source: "dist\windows-msvc\usr\bin\qtedit4.exe"; DestDir: "{app}";
Source: "dist\windows-msvc\usr\bin\*.dll"; DestDir: "{app}";
Source: "dist\windows-msvc\usr\bin\generic\*.dll"; DestDir: "{app}\generic";
Source: "dist\windows-msvc\usr\bin\iconengines\*.dll"; DestDir: "{app}\iconengines";
Source: "dist\windows-msvc\usr\bin\imageformats\*.dll"; DestDir: "{app}\imageformats";
Source: "dist\windows-msvc\usr\bin\networkinformation\*.dll"; DestDir: "{app}\networkinformation";
Source: "dist\windows-msvc\usr\bin\platforms\*.dll"; DestDir: "{app}\platforms";
Source: "dist\windows-msvc\usr\bin\styles\*.dll"; DestDir: "{app}\styles";
Source: "dist\windows-msvc\usr\bin\tls\*.dll"; DestDir: "{app}\tls";

;Source: "dist\windows-msvc\usr\share\icons\*"; DestDir: "{app}"; Flags: recursesubdirs ignoreversion noregerror allowunsafefiles
Source: "dist\windows-msvc\usr\share\icons\breeze\index.theme"; DestDir: "{app}\icons\breeze"; Flags:  ignoreversion
Source: "dist\windows-msvc\usr\share\icons\breeze\actions\16\*.svg"; DestDir: "{app}\icons\breeze\actions\16\"; Flags: ignoreversion
Source: "dist\windows-msvc\usr\share\icons\breeze\actions\22\*.svg"; DestDir: "{app}\icons\breeze\actions\22\"; Flags: ignoreversion
Source: "dist\windows-msvc\usr\share\icons\breeze\actions\32\*.svg"; DestDir: "{app}\icons\breeze\actions\32\"; Flags: ignoreversion
Source: "dist\windows-msvc\usr\share\icons\breeze\devices\16\*.svg"; DestDir: "{app}\icons\breeze\devices\16\"; Flags: ignoreversion
Source: "dist\windows-msvc\usr\share\icons\breeze\devices\22\*.svg"; DestDir: "{app}\icons\breeze\devices\22\"; Flags: ignoreversion
Source: "dist\windows-msvc\usr\qtedit4.ico"; DestDir: "{app}\"; Flags: ignoreversion

[Registry]
; Associate qtedit4 with .txt files
Root: HKCR; Subkey: ".txt"; ValueType: string; ValueName: ""; ValueData: "qtedit4.File"; Flags: createvalueifdoesntexist

; Define the file type description for qtedit4
Root: HKCR; Subkey: "qtedit4.File"; ValueType: string; ValueName: ""; ValueData: "Text Document (qtedit4)"

; Set the default icon for .txt files associated with qtedit4
Root: HKCR; Subkey: "qtedit4.File\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\qtedit4.exe,0"

; Add "Edit with qtedit4" to the right-click context menu
Root: HKCR; Subkey: "qtedit4.File\shell\edit"; ValueType: string; ValueName: ""; ValueData: "Edit with qtedit4"
Root: HKCR; Subkey: "qtedit4.File\shell\edit\command"; ValueType: string; ValueName: ""; ValueData: """{app}\qtedit4.exe"" ""%1"""

; Optional: Register for "Open" action if you want to make double-click open with qtedit4
Root: HKCR; Subkey: "qtedit4.File\shell\open"; ValueType: string; ValueName: ""; ValueData: "Open with qtedit4"
Root: HKCR; Subkey: "qtedit4.File\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\qtedit4.exe"" ""%1"""

[UninstallDelete]
Type: filesandordirs; Name: "{app}\*";

[Icons]
Name: "{group}\qtedit4 v{#VersionString}"; Filename: "{app}\qtedit4.exe"; Comment: "qtedit4 editor - version {#VersionString}"; Flags: uninsneveruninstall


[Code]

var
  VCPage: TWizardPage;
  BtnOpen: TNewButton;
  BtnRecheck: TNewButton;
  MsgLbl: TNewStaticText;

function IsVC2015_2022x64Installed: Boolean;
var
  Val: Cardinal;
begin
  Result := False;
  if RegQueryDWordValue(HKLM, 'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64',
     'Installed', Val) and (Val = 1) then
    Result := True
  else if RegQueryDWordValue(HKLM, 'SOFTWARE\WOW6432Node\Microsoft\VisualStudio\14.0\VC\Runtimes\x64',
     'Installed', Val) and (Val = 1) then
    Result := True;
end;

procedure OpenVCRedistPage;
var
  ErrorCode: Integer;
begin
  ShellExec('', ExpandConstant('{#VC_Redist_URL}'), '', '', SW_SHOWNORMAL, ewNoWait, ErrorCode);
end;

procedure BtnOpenClick(Sender: TObject);
begin
  OpenVCRedistPage;
end;

procedure BtnRecheckClick(Sender: TObject);
begin
  if IsVC2015_2022x64Installed then
    MsgBox('Great! The Microsoft Visual C++ runtime is installed. Click Next to continue.', mbInformation, MB_OK)
  else
    MsgBox('Still not detected. Please finish installing the runtime, then click Re-check.', mbError, MB_OK);
end;

procedure InitializeWizard;
begin
  if not IsVC2015_2022x64Installed then
  begin
    VCPage := CreateCustomPage(wpWelcome,
      'Dependency Required',
      'Microsoft Visual C++ 2015–2022 x64 Runtime not found');

    MsgLbl := TNewStaticText.Create(VCPage);
    MsgLbl.Parent := VCPage.Surface;
    MsgLbl.Width := VCPage.SurfaceWidth;
    MsgLbl.Top := ScaleY(8);
    MsgLbl.WordWrap := True;
    MsgLbl.Caption :=
      'qtedit4 needs the Microsoft Visual C++ 2015–2022 x64 runtime.' + #13#10#13#10 +
      '1) Click "Download" to get vc_redist.x64.exe from Microsoft.' + #13#10 +
      '2) Install it, then return here and click "Re-check".';

    BtnOpen := TNewButton.Create(VCPage);
    BtnOpen.Parent := VCPage.Surface;
    BtnOpen.Caption := 'Download';
    BtnOpen.Left := 0;
    BtnOpen.Top := MsgLbl.Top + MsgLbl.Height + ScaleY(12);
    BtnOpen.OnClick := @BtnOpenClick;

    BtnRecheck := TNewButton.Create(VCPage);
    BtnRecheck.Parent := VCPage.Surface;
    BtnRecheck.Caption := 'Re-check';
    BtnRecheck.Left := BtnOpen.Left + BtnOpen.Width + ScaleX(8);
    BtnRecheck.Top := BtnOpen.Top;
    BtnRecheck.OnClick := @BtnRecheckClick;
  end;
end;

function NextButtonClick(CurPageID: Integer): Boolean;
begin
  Result := True;
  if Assigned(VCPage) and (CurPageID = VCPage.ID) then
  begin
    if not IsVC2015_2022x64Installed then
    begin
      MsgBox('The Microsoft Visual C++ runtime is required. Please install it before continuing.',
        mbError, MB_OK);
      Result := False;
    end;
  end;
end;

procedure RemoveOldShortcuts;
var
  OldShortcutPath: string;
  FindRec: TFindRec;
  CurrentShortcutName: string;
begin
  OldShortcutPath := ExpandConstant('{group}\*');
  CurrentShortcutName := 'qtedit4 v' + '{#VersionString}' + '.lnk';
  if FindFirst(OldShortcutPath, FindRec) then
  begin
    try
      repeat
        if (FindRec.Attributes and FILE_ATTRIBUTE_DIRECTORY = 0) and
           (ExtractFileExt(FindRec.Name) = '.lnk') and
           (CompareText(FindRec.Name, CurrentShortcutName) <> 0) then
        begin
          DeleteFile(ExpandConstant('{group}\' + FindRec.Name));
          Log('Deleted old shortcut: ' + FindRec.Name);
        end;
      until not FindNext(FindRec);
    finally
      FindClose(FindRec);
    end;
  end;
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssInstall then
  begin
    RemoveOldShortcuts;
  end;
end;
