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
| 分发 | 拷贝脚本 | `build.sh` / `build.bat` 交叉编译，或 GitHub Actions 云端出绿色版 |

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

## Windows：直接用 exe，不做安装包

**推荐直接用免安装绿色版**，解压双击就能跑 —— 不需要管理员权限、不需要安装、
不会被系统版本或位数拦下。

```
dming-portable\
├── dming.exe          ← 双击这个
├── p.txt              ← 换成你的名单（每行一个姓名）
├── class.txt          ← 班级名称
└── 使用说明.txt
```

`p.txt` 和 `class.txt` 必须和 `dming.exe` 在**同一个文件夹**里。

### 为什么不做安装包

一开始是用 Inno Setup 打的安装包，但问题不少：

- **32 位 Win10 和老版本 Win10 会被挡住**。表现是弹一句
  「该软件暂不支持在您的电脑上打开，请联系软件分发者」—— 这句是
  **Windows 自己**弹的，在进入安装向导之前就拒绝了，用户完全看不到原因。
- 安装包对使用者没有任何实质好处：这个程序是**纯 Go 静态二进制**，
  已经装在用户目录，不写注册表、不需要 DLL，绿色版一样能用。

所以现在只出 exe + 绿色版。`installer/` 目录已经删掉了。

### exe 图标

Go 链接器本身不读 `.ico`，但 `go build` 会自动编译并链接当前目录下形如
`rsrc_windows_amd64.syso` 的资源文件 —— 这是 Go 官方的机制。所以只要先用
[`rsrc`](https://github.com/akavel/rsrc) 把图标转成 `.syso` 即可，**源码一行都不用改**：

```bat
go install github.com/akavel/rsrc@latest
rsrc -ico assets\dming.ico -o rsrc_windows_amd64.syso -arch amd64
rsrc -ico assets\dming.ico -o rsrc_windows_arm64.syso -arch arm64
build.bat windows amd64
```

后缀里的 `windows_amd64` 必须和目标平台一致，否则 Go 会忽略它。反过来说，
两个 `.syso` 放在一起也不会互相干扰 —— 各自只在自己对应的那次编译里生效。
GitHub Actions 里已经自动做了这一步。

> `*.syso` 是构建产物，已加进 `.gitignore`，不需要提交。

`build.bat` 的两个注意点（已经处理好了）：

- 批处理故意写成**纯 ASCII**：cmd.exe 按 OEM 代码页解码批处理文件，
  UTF-8 的中文会变成乱码。
- `:build` 子例程内部用 `setlocal`/`endlocal` 包住 `GOOS`/`GOARCH`/`EXT`，
  否则这些变量会泄漏给后续目标，编出错的平台；`:fail` 分支中间不能有任何命令，
  因为 `echo` 成功会把 `ERRORLEVEL` 重置成 0，把失败吞掉。

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
| `.github/workflows/build.yml` | GitHub Actions 云端构建（自动嵌图标 + 出绿色版） |
| `assets/dming.ico` | 多尺寸图标（16~256px），由 `icon-source.png` 生成 |
| `assets/dming.png` | 256px 预览图 |
| `assets/icon-source.png` | 图标源图：`Dming.webp` 的第 15 帧（620×620） |
| `assets/make-icon.py` | 图标生成脚本（纯标准库，改图标时重跑） |
| `class.txt` | 班级名称 |
| `p.txt` | 学生名单（自己维护，**不进版本库**） |
| `p.sample.txt` | 示例名单（张三、李四、王五、赵六） |
| `GITHUB_BUILD.md` | 用 GitHub Actions 云端打包的完整说明 |

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

