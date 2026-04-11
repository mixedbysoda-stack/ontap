[Setup]
AppName=On Tap
AppVersion=1.0.0
AppVerName=On Tap v1.0.0
AppPublisher=Carbonated Audio
AppPublisherURL=https://carbonatedaudio.com
DefaultDirName={autopf}\Carbonated Audio\On Tap
DefaultGroupName=Carbonated Audio
OutputDir=..\..\dist\Windows\Installer
OutputBaseFilename=OnTap-v1.0.0-Windows-Installer
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64compatible
DisableProgramGroupPage=yes
PrivilegesRequired=admin

[Files]
; VST3
Source: "..\..\dist\Windows\VST3\On Tap.vst3\*"; DestDir: "{commoncf64}\VST3\On Tap.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs

; Standalone
Source: "..\..\dist\Windows\Standalone\On Tap.exe"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\On Tap"; Filename: "{app}\On Tap.exe"
Name: "{group}\Uninstall On Tap"; Filename: "{uninstallexe}"

[Run]
Filename: "{app}\On Tap.exe"; Description: "Launch On Tap"; Flags: nowait postinstall skipifsilent
