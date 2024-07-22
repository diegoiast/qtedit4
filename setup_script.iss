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
Source: "dist\windows-msvc\usr\share\icons\breeze\actions\16\*.svg"; DestDir: "{app}\icons\breeze\actions\16\"; Flags:  ignoreversion
Source: "dist\windows-msvc\usr\share\icons\breeze\actions\22\*.svg"; DestDir: "{app}\icons\breeze\actions\22\"; Flags:  ignoreversion
Source: "dist\windows-msvc\usr\share\icons\breeze\actions\32\*.svg"; DestDir: "{app}\icons\breeze\actions\32\"; Flags:  ignoreversion
Source: "dist\windows-msvc\usr\share\icons\breeze\devices\16\*.svg"; DestDir: "{app}\icons\breeze\devices\16\"; Flags:  ignoreversion
Source: "dist\windows-msvc\usr\share\icons\breeze\devices\22\*.svg"; DestDir: "{app}\icons\breeze\devices\22\"; Flags:  ignoreversion

[UninstallDelete]
Type: filesandordirs; Name: "{app}\*"; 

[Icons]
Name: "{group}\qtedit4"; Filename: "{app}\qtedit4.exe"

;[Tasks]
;Name: desktopicon; Description: "Create a desktop icon"; GroupDescription: "Additional icons:"
