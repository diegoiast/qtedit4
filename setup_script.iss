
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
     'Installed', Val, True) and (Val = 1) then
    Result := True
  else if RegQueryDWordValue(HKLM, 'SOFTWARE\WOW6432Node\Microsoft\VisualStudio\14.0\VC\Runtimes\x64',
     'Installed', Val, True) and (Val = 1) then
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
      '1) Click "Open download page" to get vc_redist.x64.exe from Microsoft.' + #13#10 +
      '2) Install it, then return here and click "Re-check".';

    BtnOpen := TNewButton.Create(VCPage);
    BtnOpen.Parent := VCPage.Surface;
    BtnOpen.Caption := 'Open download page';
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
