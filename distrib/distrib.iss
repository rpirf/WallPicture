#define ProgramName			"WallPicture"
#define ProgramVersion "1.0"
#define InstallName     ProgramName + "_" + ProgramVersion + "_setup_x64"
#define ExeName       		"WallPicture.exe"

#define ReleaseDir    		SourcePath + "\..\x64\Release"
#define PackagesDir			SourcePath + "\packages"
#define PicturesDir			SourcePath + "\..\pictures"
#define QtDir				GetEnv("QTDIR")

[Setup]
AppId={{9C18EF2A-4826-4E2B-A20F-F7B34675972D}
AppName={#ProgramName}
AppVersion={#ProgramVersion}
VersionInfoVersion={#ProgramVersion}

ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

DefaultDirName={commonpf}\{#ProgramName} {#ProgramVersion}
DefaultGroupName={#ProgramName} {#ProgramVersion}

OutputDir={#SourcePath}
OutputBaseFilename={#InstallName}

Compression=lzma
SolidCompression=yes

SetupIconFile="{#SourcePath}\icons\picture.ico"

Uninstallable=true
UninstallDisplayName="{#ProgramName} {#ProgramVersion}"
UninstallDisplayIcon="{app}\icons\picture.ico"

[Languages]
Name: "Russian"; MessagesFile: "compiler:Languages\Russian.isl"

[Files]
Source: "{#ReleaseDir}\{#ExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#PicturesDir}\*"; DestDir: "{app}\pictures"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#QtDir}\bin\Qt5Multimedia.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt5Core.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt5Gui.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt5Network.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt5Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtDir}\plugins\platforms\qwindows.dll"; DestDir: "{app}\platforms"; Flags: ignoreversion
Source: "{#QtDir}\plugins\imageformats\qjpeg.dll"; DestDir: "{app}\imageformats"; Flags: ignoreversion
Source: "{#QtDir}\plugins\mediaservice\dsengine.dll"; DestDir: "{app}\mediaservice"; Flags: ignoreversion
Source: "{#SourcePath}\icons\picture.ico"; DestDir: "{app}\icons"; Flags: ignoreversion
Source: "{#SourcePath}\icons\uninstall.ico"; DestDir: "{app}\icons"; Flags: ignoreversion

; Visual C++ Redistributable Package 2022
Source: "{#PackagesDir}\msvs2022_vc_redist.x64.exe"; DestDir: "{tmp}"; Flags: deleteafterinstall nocompression;

[Tasks]
Name: "desctopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Icons]
Name: "{group}\{#ProgramName} {#ProgramVersion}"; Filename: "{app}\{#ExeName}"; IconFilename: "{app}\icons\picture.ico";
Name: "{commondesktop}\{#ProgramName} {#ProgramVersion}"; Filename: "{app}\{#ExeName}"; IconFilename: "{app}\icons\picture.ico"; Tasks: desctopicon
Name: "{group}\Удалить {#ProgramName} {#ProgramVersion}"; Filename: "{uninstallexe}"; IconFilename: "{app}\icons\uninstall.ico"

[Code]
#include "check_vcredist_installed.pas"

[Run]
; Visual C++ 2022 Redistributable Package install
Filename: {tmp}\msvs2022_vc_redist.x64.exe; Parameters: "/passive /norestart /Q:a /c:""msiexec /qb /i vcredist.msi"" "; Check: VC2022RedistNeedsInstall; StatusMsg: Installing 2022 RunTime...
