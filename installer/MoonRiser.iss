; Inno Setup script - builds MoonRiser-Setup.exe  (made by Ernomoon)

#define AppName "Moon Riser"
#define AppVer  "0.3.0"

[Setup]
AppId={{B6B0B8D0-7C1E-4A53-9C0B-5A5D6E0A1F11}
AppName={#AppName}
AppVersion={#AppVer}
AppPublisher=Ernomoon
DefaultDirName={autopf}\MoonRiser
DisableDirPage=yes
DisableProgramGroupPage=yes
PrivilegesRequired=admin
ArchitecturesInstallIn64BitMode=x64
OutputDir=Output
OutputBaseFilename=MoonRiser-Setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
UninstallDisplayName={#AppName}

[InstallDelete]
; remove the old test version (it was called "Riser Maker")
Type: filesandordirs; Name: "{commoncf64}\VST3\Riser Maker.vst3"

[Files]
; A VST3 on Windows is a folder. It goes where FL Studio looks for VST3 plugins by default.
Source: "..\build\MoonRiser_artefacts\Release\VST3\Moon Riser.vst3\*"; \
  DestDir: "{commoncf64}\VST3\Moon Riser.vst3"; Flags: recursesubdirs createallsubdirs ignoreversion

[Messages]
FinishedLabel=Moon Riser is installed. In FL Studio open Options -> Manage plugins -> Find installed plugins, then look for "Moon Riser" (Fx).
