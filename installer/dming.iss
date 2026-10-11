; ============================================================================
;  dming 点名器 —— Inno Setup 6 打包脚本
;
;  编译方式（二选一）：
;    1. 双击 installer\build-installer.bat（会先编译 exe 再打包）
;    2. 先用 build.bat 编译，再用 Inno Setup 打开本文件按 F9，
;       或命令行： "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" installer\dming.iss
;
;  ！！本文件必须保持 UTF-8 (带 BOM) 编码，否则 Inno Setup 读不出中文。
;
;  默认装到 %LocalAppData%\Programs\dming（用户目录，不需要管理员权限），
;  这样 p.txt / class.txt 可以直接编辑，不需要提权。
; ============================================================================

#define AppName      "dming 点名器"
#define AppShortName "dming"
#define AppVersion   "3.4"
#define AppPublisher "白异山"
#define AppExeName   "dming.exe"
#define AppIconName  "dming.ico"

; 要打包 Windows on ARM 版本时，把下面改成 dming-windows-arm64.exe，
; 并把 ArchitecturesAllowed 改成 arm64（Inno 6.3 以上才有 arm64）。
#define SourceExe    "..\dist\dming-windows-amd64.exe"

[Setup]
; AppId 是卸载/升级时的唯一标识，一旦发布就不要再改。
AppId={{30838C6C-B5FB-4BBA-84B3-F6009744C469}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher={#AppPublisher}
VersionInfoVersion=3.4.0.0
DefaultDirName={localappdata}\Programs\{#AppShortName}
DefaultGroupName={#AppName}
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
OutputDir=..\dist
OutputBaseFilename={#AppShortName}-{#AppVersion}-setup
SetupIconFile=..\assets\dming.ico
; Go 编译出来的 exe 本身不带图标资源，所以卸载项指向随包安装的 .ico。
UninstallDisplayIcon={app}\{#AppIconName}
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
; 刻意「不」写 ArchitecturesAllowed。
;
; 原因：这个程序是纯 Go 静态二进制，装在 %LocalAppData% 用户目录，
; 不碰 Program Files 的 32/64 位重定向，所以限制架构没有任何好处，
; 只会把 32 位 Win10 和部分 ARM 设备挡在门外 —— 表现就是
; 「该软件暂不支持在您的电脑上打开」，而且那句话是 Windows 自己弹的，
; 根本进不到 Inno 的向导界面，用户完全看不到原因。
;
; 同理不写 ArchitecturesInstallIn64BitMode：不装 Program Files 就不需要它。
; 如果需要区分 32/64 位安装包，用命令行覆盖即可：
;   ISCC.exe /DARCH=x64compatible installer\dming.iss
#ifdef ARCH
ArchitecturesAllowed={#ARCH}
ArchitecturesInstallIn64BitMode={#ARCH}
#endif

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
; 想要中文向导界面：下载 ChineseSimplified.isl
; （https://jrsoftware.org/files/istrans/ 或 https://github.com/jrsoftware/issrc）
; 放进 Inno Setup 安装目录的 Languages\ 子目录，再取消下面这行的注释。
;Name: "chinese"; MessagesFile: "compiler:Languages\ChineseSimplified.isl"

[Tasks]
Name: "desktopicon"; Description: "创建桌面快捷方式"; GroupDescription: "附加快捷方式："
Name: "samplelist";  Description: "创建示例名单 p.txt（张三、李四、王五、赵六）"; GroupDescription: "数据文件："; Flags: checkedonce

[Files]
Source: "{#SourceExe}";  DestDir: "{app}"; DestName: "{#AppExeName}"; Flags: ignoreversion
; 图标单独放一份，供快捷方式和卸载项引用。
Source: "..\assets\dming.ico"; DestDir: "{app}"; DestName: "{#AppIconName}"; Flags: ignoreversion
; class.txt 只在不存在时写入，升级和卸载都不会覆盖用户改过的班级号。
; 注意：故意不打包 p.txt —— 学生名单是隐私数据，由使用者在数据目录里自己放。
Source: "..\class.txt";   DestDir: "{app}"; Flags: onlyifdoesntexist uninsneveruninstall

[Icons]
Name: "{group}\{#AppName}";                  Filename: "{app}\{#AppExeName}"; WorkingDir: "{app}"; IconFilename: "{app}\{#AppIconName}"
Name: "{group}\数据目录（p.txt / class.txt）"; Filename: "{app}"; IconFilename: "{app}\{#AppIconName}"
Name: "{group}\卸载 {#AppName}";             Filename: "{uninstallexe}"
Name: "{autodesktop}\{#AppName}";            Filename: "{app}\{#AppExeName}"; WorkingDir: "{app}"; IconFilename: "{app}\{#AppIconName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#AppExeName}"; Description: "立即运行 {#AppName}"; Flags: nowait postinstall skipifsilent
Filename: "{app}"; Description: "打开数据目录，编辑 p.txt / class.txt"; Flags: postinstall shellexec skipifsilent unchecked

[Code]
{ 按需生成示例名单。用 UTF-8 写入（程序会自行忽略 BOM）。 }
procedure CurStepChanged(CurStep: TSetupStep);
var
  ListPath: String;
  Names: TArrayOfString;
begin
  if CurStep <> ssPostInstall then
    Exit;
  if not WizardIsTaskSelected('samplelist') then
    Exit;

  ListPath := ExpandConstant('{app}\p.txt');
  if FileExists(ListPath) then
    Exit;

  SetArrayLength(Names, 4);
  Names[0] := '张三';
  Names[1] := '李四';
  Names[2] := '王五';
  Names[3] := '赵六';
  SaveStringsToUTF8File(ListPath, Names, False);
end;
