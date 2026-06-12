; Inno Setup script for AZADI: Rise of the Dawn (Windows installer)
; Invoked by tools/ci/package_windows.ps1 with /D defines:
;   AppVersion  - semantic version, e.g. 1.2.0
;   StageDir    - the UAT staged Win64 build folder (contains Azadi.exe)
;   OutputDir   - where the installer .exe should be written

#ifndef AppVersion
  #define AppVersion "0.0.0"
#endif
#ifndef StageDir
  #define StageDir "..\..\Build\Windows"
#endif
#ifndef OutputDir
  #define OutputDir "..\..\dist"
#endif

#define AppName "AZADI: Rise of the Dawn"
#define AppPublisher "Azadi Community"
#define AppExeName "Azadi.exe"
#define AppURL "https://github.com/imarash/azadi-ue5"

[Setup]
AppId={{7A2AD1A2-4A7D-D4F1-B02E-36A2C40D85F1}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppURL}
AppSupportURL={#AppURL}/issues
DefaultDirName={autopf}\Azadi
DefaultGroupName=AZADI
DisableProgramGroupPage=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
OutputDir={#OutputDir}
OutputBaseFilename=AzadiSetup-{#AppVersion}
UninstallDisplayIcon={app}\{#AppExeName}
LicenseFile=..\..\LICENSE

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#StageDir}\*"; DestDir: "{app}"; Flags: recursesubdirs createallsubdirs ignoreversion

[Icons]
Name: "{group}\AZADI"; Filename: "{app}\{#AppExeName}"
Name: "{group}\{cm:UninstallProgram,AZADI}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\AZADI"; Filename: "{app}\{#AppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#AppExeName}"; Description: "{cm:LaunchProgram,AZADI}"; Flags: nowait postinstall skipifsilent
