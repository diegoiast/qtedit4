[Setup]
AppName=qtedit4
AppVersion=0.1
DefaultDirName={pf}\qtedit4
DefaultGroupName=qtedit4
UninstallDisplayIcon={app}\qtedit4.ico
OutputDir=dist
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
Name: "{group}\qtedit4"; Filename: "{app}\qtedit4.exe"
Name: "{userdesktop}\qtedit4"; Filename: "{app}\qtedit4.exe"; IconFilename: "{app}\qtedit4.ico"

;[Tasks]
;Name: desktopicon; Description: "Create a desktop icon"; GroupDescription: "Additional icons:"
