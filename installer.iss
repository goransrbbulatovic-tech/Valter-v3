; ============================================================
;  Inno Setup — Valter
;  Program za evidenciju filmova i kljuceva za bioskop
; ============================================================

#define AppName      "Valter - Program za evidenciju filmova i kljuceva za bioskop"
#define AppVersion   "1.0.0"
#define AppPublisher "KC Bar"
#define AppExeName   "Valter.exe"
#define AppId        "{A1B2C3D4-E5F6-7890-ABCD-EF1234567890}"

[Setup]
AppId={{#AppId}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher={#AppPublisher}
DefaultDirName={autopf}\KCBar\Valter
DefaultGroupName={#AppName}
AllowNoIcons=yes
OutputDir=.\installer_output
OutputBaseFilename=Valter_Setup_v{#AppVersion}
SetupIconFile=icon.ico
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=lowest
DisableProgramGroupPage=yes
DisableDirPage=no
UninstallDisplayName={#AppName}
VersionInfoVersion={#AppVersion}
VersionInfoCompany={#AppPublisher}
VersionInfoDescription=Valter - Program za evidenciju filmova i kljuceva za bioskop

; Automatski instaliraj Visual C++ Runtime ako nedostaje
[Files]
; ── Glavni executable ────────────────────────────────────────────────────────
Source: "release\{#AppExeName}"; DestDir: "{app}"; Flags: ignoreversion

; ── Ikona ────────────────────────────────────────────────────────────────────
Source: "icon.ico"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist

; ── Svi Qt5 DLL-ovi iz release foldera (wildcard — hvata SVE dll-ove) ────────
Source: "release\*.dll"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist

; ── Qt platformski plugin (obavezan — bez njega ne startuje) ─────────────────
Source: "release\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion recursesubdirs skipifsourcedoesntexist

; ── Qt PrintSupport plugin (za PDF export) ───────────────────────────────────
Source: "release\printsupport\*"; DestDir: "{app}\printsupport"; Flags: ignoreversion recursesubdirs skipifsourcedoesntexist

; ── Qt ImageFormats plugin ───────────────────────────────────────────────────
Source: "release\imageformats\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs skipifsourcedoesntexist

; ── Qt Styles plugin (moderan izgled na Windows 10/11) ──────────────────────
Source: "release\styles\*"; DestDir: "{app}\styles"; Flags: ignoreversion recursesubdirs skipifsourcedoesntexist

; ── Svi ostali plugini koje windeployqt doda ─────────────────────────────────
Source: "release\bearer\*";         DestDir: "{app}\bearer";         Flags: ignoreversion recursesubdirs skipifsourcedoesntexist
Source: "release\iconengines\*";    DestDir: "{app}\iconengines";    Flags: ignoreversion recursesubdirs skipifsourcedoesntexist

; ── Visual C++ Runtime (ako nije instaliran na sistemu) ──────────────────────
Source: "release\vcruntime140.dll";   DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
Source: "release\vcruntime140_1.dll"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
Source: "release\msvcp140.dll";       DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
Source: "release\msvcp140_1.dll";     DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
Source: "release\msvcp140_2.dll";     DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
Source: "release\concrt140.dll";      DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist

[Icons]
Name: "{group}\{#AppName}";       Filename: "{app}\{#AppExeName}"; IconFilename: "{app}\{#AppExeName}"
Name: "{group}\Deinstalacija";    Filename: "{uninstallexe}"
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\{#AppExeName}"; Tasks: desktopicon; IconFilename: "{app}\{#AppExeName}"

[Tasks]
Name: "desktopicon"; Description: "Kreirati ikonu na radnoj povrsini"; GroupDescription: "Dodatne opcije:"; Flags: unchecked

[Run]
Filename: "{app}\{#AppExeName}"; Description: "Pokrenuti Valter"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: filesandordirs; Name: "{app}"
