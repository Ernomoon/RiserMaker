; Inno Setup script - собирает установщик RiserMaker-Setup.exe
; Запускается автоматически на GitHub, либо вручную (см. README_RU.md)

#define AppName "Riser Maker"
#define AppVer  "0.1.0"

[Setup]
AppId={{B6B0B8D0-7C1E-4A53-9C0B-5A5D6E0A1F11}
AppName={#AppName}
AppVersion={#AppVer}
AppPublisher=RiserMaker
DefaultDirName={autopf}\RiserMaker
DisableDirPage=yes
DisableProgramGroupPage=yes
PrivilegesRequired=admin
ArchitecturesInstallIn64BitMode=x64
OutputDir=Output
OutputBaseFilename=RiserMaker-Setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
UninstallDisplayName={#AppName}

[Files]
; VST3 на Windows - это папка. Ставим её туда, где FL Studio ищет VST3 по умолчанию.
Source: "..\build\RiserMaker_artefacts\Release\VST3\Riser Maker.vst3\*"; \
  DestDir: "{commoncf64}\VST3\Riser Maker.vst3"; Flags: recursesubdirs createallsubdirs ignoreversion

[Messages]
FinishedLabel=Установка завершена. Откройте FL Studio: Options -> Manage plugins -> Find plugins. Плагин "Riser Maker" появится в списке (Fx).
