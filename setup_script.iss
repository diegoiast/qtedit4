[Setup]
AppName=qtedit4
AppVersion=0.1
DefaultDirName={pf}\qtedit4
DefaultGroupName=qtedit4
UninstallDisplayIcon={app}\bin\qtedit4.exe
OutputDir=dist
OutputBaseFilename=qtedit4-win64
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64
ShowComponentSizes=yes

[Files]
Source: "dist\windows-msvc\usr\bin\qtedit4.exe"; DestDir: "{app}"; 
Source: "dist\windows-msvc\usr\bin\*"; DestDir: "{app}";
;Source: "dist\windows-msvc\usr\share\icons\*"; DestDir: "{app}"; Flags: recursesubdirs ignoreversion noregerror allowunsafefiles
Source: "dist\windows-msvc\usr\share\icons\breeze\actions\16\*.svg"; DestDir: "{app}\icons\actions\16\"; Flags:  ignoreversion
Source: "dist\windows-msvc\usr\share\icons\breeze\actions\32\*.svg"; DestDir: "{app}\icons\actions\32\"; Flags:  ignoreversion
Source: "dist\windows-msvc\usr\share\icons\breeze\devices\16\*.svg"; DestDir: "{app}\icons\devices\16\"; Flags:  ignoreversion
Source: "dist\windows-msvc\usr\share\icons\breeze\devices\32\*.svg"; DestDir: "{app}\icons\devices\32\"; Flags:  ignoreversion

[Icons]
Name: "{group}\qtedit4"; Filename: "{app}\qtedit4.exe"

;[Tasks]
;Name: desktopicon; Description: "Create a desktop icon"; GroupDescription: "Additional icons:"
