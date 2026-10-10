# dming —— 课堂随机点名器（Go 版）

`Dming.sh` 的 Go 重写版，编译成单个可执行文件，并可打成 Windows 安装包。

## 和旧脚本的区别

| | Dming.sh | dming（Go） |
|---|---|---|
| 班级号 | 脚本里硬编码 `CLASS="14"` | **读 `class.txt`** |
| 依赖 | bash + sed + tr | 无（单个可执行文件） |
| 平台 | Linux / Termux | Windows / macOS / Linux / Termux |
| 打开浏览器 | 只调用 `xdg-open` | 按平台自动选择合适命令 |
| 名单解析 | 逐行 sed 去空白 | 逐行 `TrimSpace`，兼容 CRLF 和 UTF-8 BOM |
| 分发 | 拷贝脚本 | `build.sh` / `build.bat` 交叉编译，Inno Setup 出安装包 |

## 数据文件

跟可执行文件放在**同一个目录**：

- `p.txt` —— 每行一个学生姓名，空行忽略。
- `class.txt` —— 班级名称，取**第一个非空行**（当前是 `14`）。

`p.txt` 不存在时会直接报错退出，提示缺哪个文件。

## 编译

要求 Go 版本见 `go.mod`。

**Linux / macOS / Termux：**

```bash
./build.sh                  # 全部平台，产物在 dist/
./build.sh darwin/arm64     # 只编一个平台
```

**Windows：**

```bat
build.bat                   :: 全部平台
build.bat windows amd64     :: 只编一个
```

覆盖的平台：`windows/amd64`、`windows/arm64`、`darwin/amd64`、`darwin/arm64`、
`linux/amd64`、`linux/arm64`、`android/arm64`（Termux）。

## Windows 安装包（Inno Setup）

**在 Windows 上双击 `installer\build-installer.bat` 就行**，它会依次做三件事：
检查 Go 和 Inno Setup → 编译 `dist\dming-windows-amd64.exe` → 调用
`ISCC.exe installer\dming.iss` 打包。产物是 `dist\dming-3.4-setup.exe`。

前提只有两个：**装了 Go**（版本见 `go.mod`）和 **[Inno Setup 6](https://jrsoftware.org/isdl.php)**。
脚本会自己去找 `ISCC.exe`（`Program Files (x86)` / `Program Files` / `LocalAppData`），
找不到就明确告诉你缺什么，不会半路莫名失败。

不想用批处理也可以：先用 `build.bat windows amd64` 编译，再用 Inno Setup 打开
`installer\dming.iss` 按 F9。

> ⚠️ 从 zip 里**直接双击**可能提示「Windows 已保护你的电脑」或被静默拦下 —— 那是
> Windows 给下载文件加的 Zone.Identifier 标记。右键 zip → 属性 → 勾「解除锁定」，
> 解压后再运行；或对 `build-installer.bat` 同样操作。

安装包的行为：

- 装到 `%LocalAppData%\Programs\dming`，**不需要管理员权限** —— 所以 `p.txt` /
  `class.txt` 可以直接编辑，不用提权。
- 开始菜单快捷方式 + 可选的桌面快捷方式，另有一个「数据目录」快捷方式直达名单文件。
- 安装向导里可以勾选生成示例名单 `p.txt`（张三、李四、王五、赵六）。
- **不会打包你现在的 `p.txt`** —— 学生名单是隐私数据，安装包里只有程序、图标和
  `class.txt`，名单由使用者在数据目录里自己放。
- 卸载和覆盖升级都**不会删除** `p.txt` / `class.txt`。
- 安装完可直接勾选「打开数据目录」去替换名单。

几个坑，已经处理好了：

- `installer\dming.iss` 必须保持 **UTF-8 带 BOM** 编码，否则 Inno Setup 读不出中文。
- `build.bat` 和 `installer\build-installer.bat` 故意写成**纯 ASCII**：cmd.exe 按 OEM
  代码页解码批处理文件，UTF-8 的中文会变成乱码。
- 批处理里 `:build` 子例程内部用 `setlocal`/`endlocal` 包住 `GOOS`/`GOARCH`/`EXT`，
  否则这些变量会泄漏给后面的目标，编出错的平台。
- 编译失败时 `:fail` 分支**中间不能有任何命令**：`echo` 成功会把 `ERRORLEVEL`
  重置成 0，把失败吞掉，然后 ISCC 会因为找不到 exe 报一个莫名其妙的错。
- Go 编译出来的 exe 本身不带图标资源。安装包会把 `assets\dming.ico` 一起装进去，
  快捷方式和卸载项都引用它。想让 exe 文件本身也带图标，可以额外做一步：

  ```bat
  go install github.com/akavel/rsrc@latest
  rsrc -ico assets\dming.ico -o rsrc_windows_amd64.syso
  build.bat windows amd64
  ```

  （`rsrc_*.syso` 会被 Go 自动打包进 exe，不需要改任何代码。）
- 想做 **Windows on ARM** 安装包：把 `dming.iss` 顶部的 `SourceExe` 换成
  `..\dist\dming-windows-arm64.exe`，`ArchitecturesAllowed` 改成 `arm64`
  （需要 Inno Setup 6.3 以上）。
- 想要**中文向导界面**：把 `ChineseSimplified.isl` 放进 Inno Setup 安装目录的
  `Languages\` 子目录，再取消 `dming.iss` 里 `[Languages]` 那行的注释。

## 使用

```
dming                生成页面并用默认浏览器打开，3 秒后删除临时文件
dming -keep          保留 DMING*.html
dming -no-open       只生成文件，打印路径
dming -delay 8s      延长清理前的等待时间
dming -quiet         只输出临时文件路径
```

## 各平台如何打开浏览器

| 平台 | 尝试的命令（按顺序） |
|---|---|
| Windows | `rundll32 url.dll,FileProtocolHandler` → `cmd /c start` |
| macOS | `open` |
| Linux | `xdg-open` → `termux-open` → `gio open` → `sensible-browser` → `x-www-browser` → `wslview` |

全部失败时会打印临时文件路径，手动打开即可。

## 文件说明

| 路径 | 说明 |
|---|---|
| `main.go` | 命令行、读取 `p.txt` / `class.txt`、临时文件与清理 |
| `page.go` | 点名页面 HTML 模板与渲染 |
| `browser.go` | 跨平台打开浏览器 |
| `build.sh` | Linux / macOS / Termux 交叉编译脚本 |
| `build.bat` | Windows 交叉编译脚本 |
| `installer/dming.iss` | Inno Setup 打包脚本 |
| `installer/build-installer.bat` | 一键编译 + 打包 |
| `assets/dming.ico` | 多尺寸图标（16~256px），由 `icon-source.png` 生成 |
| `assets/dming.png` | 256px 预览图 |
| `assets/icon-source.png` | 图标源图：`Dming.webp` 的第 15 帧（620×620） |
| `assets/make-icon.py` | 图标生成脚本（纯标准库，改图标时重跑） |
| `class.txt` | 班级名称 |
| `p.txt` | 学生名单（自己维护，不进安装包） |

## 图标

`assets/dming.ico` 取自 `Dming.webp`（620×620、24 帧的动图，内容是 😎）。
动图里帧 4 / 9 / 15 / 19 是覆盖整幅画布的关键帧，可以独立使用，这里选了最居中的
**第 15 帧**，抽出来存成 `assets/icon-source.png`。

重新生成图标：

```bash
python3 assets/make-icon.py
```

它会把 `icon-source.png` 按面积平均缩放到 16 / 24 / 32 / 48 / 64 / 128 / 256，
写成 ICO（全部用 BMP 条目，兼容性最好），同时输出 256px 的预览 `dming.png`。
想换图标就直接用新的正方形 PNG 覆盖 `icon-source.png` 再跑一遍。

想换动图里的**另一帧**：需要先用 libwebp 之类的解码器把那一帧解出来存成 PNG
（libwebp 的简单解码 API 不支持动图，得先按 RIFF 结构把 `ANMF` 里的
`ALPH` / `VP8 ` 子块取出来包成一个静态 WebP），然后覆盖 `icon-source.png`。

