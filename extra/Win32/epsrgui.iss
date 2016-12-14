; Script generated by the Inno Script Studio Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "EPSRgui"
#define MyAppVersion "0.1"
#define MyAppPublisher "Sam Callear"
#define MyAppURL "https://www.projectaten.com/"
#define MyAppExeName "EPSRgui.exe"

; Locations of bin directories of Qt, GnuWin(32), and MinGW(32)
#define QtDir "C:\Qt\5.7.0\5.7\mingw53_32"
#define GnuWinDir "C:\GnuWin32"
#define MinGWDir "C:\Qt\5.7.0\5.7\mingw53_32"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{1A390F02-C24F-49EC-8D80-0444EFB8CB80}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\EPSRgui
DefaultGroupName={#MyAppName}
LicenseFile=..\..\COPYING
OutputDir=..\..\
OutputBaseFilename=EPSRgui-0.1
SetupIconFile=EPSRgui.ico
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "..\..\release\EPSRgui.exe"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "EPSRgui.ico"; DestDir: "{app}\bin"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files
; Source: "{#GnuWinDir}\bin\freetype6.dll"; DestDir: "{app}\bin"
; Source: "{#GnuWinDir}\bin\readline5.dll"; DestDir: "{app}\bin"
; Source: "{#GnuWinDir}\bin\history5.dll"; DestDir: "{app}\bin"
; Source: "{#GnuWinDir}\bin\zlib1.dll"; DestDir: "{app}\bin"
; Source: "{#GnuWinDir}\bin\libftgl.dll"; DestDir: "{app}\bin"
Source: "{#MinGWDir}\bin\libgcc_s_dw2-1.dll"; DestDir: "{app}\bin"
Source: "{#MinGWDir}\bin\libstdc++-6.dll"; DestDir: "{app}\bin"
Source: "{#MinGWDir}\bin\libwinpthread-1.dll"; DestDir: "{app}\bin"
Source: "{#QtDir}\bin\Qt5Gui.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt5Core.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt5PrintSupport.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt5Svg.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt5Widgets.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
; Source: "{#QtDir}\bin\libEGL.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
; Source: "{#QtDir}\bin\libGLESv2.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#QtDir}\plugins\iconengines\qsvgicon.dll"; DestDir: "{app}\bin\iconengines"; Flags: ignoreversion
Source: "{#QtDir}\plugins\platforms\qwindows.dll"; DestDir: "{app}\bin\platforms"; Flags: ignoreversion
Source: "{#QtDir}\plugins\imageformats\*.dll"; DestDir: "{app}\bin\imageformats"; Flags: ignoreversion
; Source: "C:\Windows\System32\D3DCompiler_43.dll"; DestDir: "{app}\bin"; Flags: ignoreversion

[Icons]
Name: "{group}\{#MyAppName}"; IconFilename: "{app}\bin\EPSRgui.ico"; Filename: "{app}\bin\{#MyAppExeName}"; WorkingDir: "{app}"
Name: "{commondesktop}\{#MyAppName}"; IconFilename: "{app}\bin\EPSRgui.ico"; Filename: "{app}\bin\{#MyAppExeName}"; WorkingDir: "{app}"; Tasks: desktopicon

[Run]
Filename: "{app}\bin\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent