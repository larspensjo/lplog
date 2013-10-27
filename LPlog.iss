; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{BBC896CB-86E5-4F63-9AA8-967AE0F9ECEB}
AppName=LPLog
AppVersion=3.0b1
AppPublisher=LPLog
AppPublisherURL=https://github.com/larspensjo/lplog
AppSupportURL=https://github.com/larspensjo/lplog
AppUpdatesURL=TBD
DefaultDirName={pf}\LPlog
DefaultGroupName=LPlog
AllowNoIcons=yes
OutputBaseFilename=setup
Compression=lzma
SolidCompression=yes
;InfoBeforeFile=../Disclaimer.rtf
InfoAfterFile=readme.md
WizardImageFile=lplog.bmp

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "distro\*"; DestDir: "{app}"; Flags: recursesubdirs

[Icons]
Name: "{group}\LPlog"; Filename: "{app}\lplog.exe"; IconFilename: "{app}\lplog.exe"
Name: "{group}\{cm:UninstallProgram,LPLog}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\LPlog"; Filename: "{app}\lplog.exe"; Tasks: desktopicon; IconFilename: "{app}\icon.bmp"

[Run]
Filename: "{app}\lplog.exe"; Description: "{cm:LaunchProgram,LPlog}"; Flags: nowait postinstall skipifsilent
