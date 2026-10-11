# dming —— 课堂随机点名器（Go 版）

`Dming.sh` 的 Go 重写版：编译成**单个可执行文件**，没有运行时依赖，支持
Windows / macOS / Linux / Termux（Android）。

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
- `class.txt` —— 班级名称，取**第一个非空行**（仓库里带的是 `14`）。

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

### exe 图标（Windows）

Go 链接器本身不读 `.ico`，但 `go build` 会**自动编译并链接**当前目录下形如
`rsrc_windows_amd64.syso` 的资源文件 —— 这是 Go 官方的机制，
所以**源码一行都不用改**：

```bat
go install github.com/akavel/rsrc@latest
rsrc -ico assets\dming.ico -o rsrc_windows_amd64.syso -arch amd64
rsrc -ico assets\dming.ico -o rsrc_windows_arm64.syso -arch arm64
build.bat windows amd64
```

后缀必须和目标平台一致，否则 Go 会忽略它；两个 `.syso` 放在一起也不会互相干扰，
各自只在自己对应的那次编译里生效。`*.syso` 是构建产物，已在 `.gitignore` 里。

GitHub Actions 里已经自动做了这一步，并且会额外用 PowerShell 把图标从 exe 里读出来
验证一遍 —— 不然容易「以为嵌了其实没嵌」。

`build.bat` 的两个注意点（已经处理好了）：

- 批处理故意写成**纯 ASCII**：cmd.exe 按 OEM 代码页解码批处理文件，UTF-8 中文会乱码。
- `:build` 子例程用 `setlocal`/`endlocal` 包住 `GOOS`/`GOARCH`/`EXT`，否则这些变量会
  泄漏给后续目标，编出错的平台；`:fail` 分支中间不能有任何命令，因为 `echo` 成功会把
  `ERRORLEVEL` 重置成 0，把失败吞掉。

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

## 用 GitHub Actions 云端打包

本机没有 cmd、PATH 坏了、不想装 Go —— 都没关系。云端 Windows runner 是
**完整 Windows 环境**，`cmd.exe` 和 PATH 全部正常，Build 完在网页上下载产物就行，
手机上也能操作。

### 首次：把项目推上去

1. 建仓库：https://github.com/new
   - Repository name 填 `dming`
   - 建议选 **Private**，见下面「隐私」一节
   - **不要**勾 Add a README / .gitignore / license（本仓库已经有了）
2. 推代码：

   ```bash
   git init
   git add -A
   git commit -m "dming: Go 版点名器"
   git branch -M main
   git remote add origin https://github.com/<你的用户名>/dming.git
   git push -u origin main
   ```

   推送会要求登录。GitHub 早就不支持密码了，用 **Personal Access Token**：
   Settings → Developer settings → Personal access tokens → Tokens (classic) →
   Generate new token，勾上 `repo`，把它当密码粘贴。

   > 手机上推也行：用 Termux 的 git，或者 GitHub 官方 App 上传文件。

### 每次打包

推送代码后：

1. 仓库页面 → 顶部 **Actions** 标签
2. 左侧选 **build**
3. 想手动触发就点 **Run workflow**（推送时本来就会自动跑）
4. 等 1～3 分钟，等两个 job 都变绿
5. 点进跑完的那次运行，页面底部 **Artifacts** 区域下载：
   - `dming-windows` —— 绿色版文件夹 + 两个架构的 exe
   - `dming-darwin-arm64` 等其他平台

下载得到的是 zip，解压即用。

> 手机上逛 GitHub 网页同样能下：Artifacts 区域点一下就能下 zip。
> 但**必须先登录**，Artifacts 对未登录用户不可见。

### 发 Release

平时 push 到 main **只构建不发版**，产物在 Artifacts 里。想正式发一个版本时打 tag，
**tag 名就是提交的 7 位短 SHA，不带任何前缀**：

```bash
# 看当前提交的短 SHA，例如 56ad711 改为直接分发 exe...
git log --oneline -1

# 用这个短 SHA 打 tag 并推送
git tag 56ad711
git push origin 56ad711
```

省事写法，直接用当前提交：

```bash
git tag "$(git rev-parse --short=7 HEAD)" && git push origin --tags
```

推上去之后 `release` job 会把前面所有平台的产物汇总成一个 Release，挂在那个 tag 下，
可以按 tag、按提交直接下载。

**tag 名会被校验。** workflow 用的是 `tags: ['[0-9a-f]*']`，只认十六进制：

| tag 名 | 结果 |
|---|---|
| `56ad711` | ✅ 正常发版 |
| `56ad71` | ✅ 能容忍（git 前缀匹配会补全成 7 位） |
| `v56ad711` | ❌ 不触发 workflow（`v` 不是十六进制） |
| `deadbee` | ❌ 触发但 job 报错：本仓库找不到该提交 |
| `main` / `HEAD` | ❌ 不触发 |

`deadbee` 这种「长得像 SHA 但仓库里没有」的情况，靠 release job 里的
`git rev-parse --verify --quiet "${TAG}^{commit}"` 兜住 —— 直接报错，
而不是发一个指向不存在提交的 Release。

### 工作流做了什么

| job | 跑在 | 做的事 |
|---|---|---|
| `windows` | `windows-latest` | 装 Go → `rsrc` 生成图标资源 → 编译 amd64/arm64 → 验证图标 → 组绿色版 → 打 zip → 上传 |
| `cross` | `ubuntu-latest` | 交叉编译 linux / darwin / android 共 5 个目标 |
| `release` | `ubuntu-latest` | **只在 tag 上跑**，汇总所有产物建 Release |

`release` 依赖前两个（`needs: [windows, cross]`），所以只有全部构建成功才会发版，
不会出现「Release 里少了某个平台」的情况。

权限是最小化的：顶层 `permissions: contents: read`，只有 `release` job 单独拿到
`contents: write` —— 这样跑第三方 action 的构建 job 没有写权限。

几个刻意的选择：

- `go-version-file: go.mod` + `check-latest: true` —— 按 `go.mod` 的最低版本要求装最新
  稳定版。**不要**写死 `go-version: '1.27.1'` 这种补丁号，云端缓存里未必有，
  `setup-go` 会直接失败。
- `dist/`、`*.exe`、`*.syso` 都在 `.gitignore` 里，构建产物不会污染仓库。

#### Windows 上 inline 脚本的编码是个坑

GitHub Actions 会把 `run:` 块写成**无 BOM 的 UTF-8** 文件。而：

| shell | 怎么读脚本 | 中文会怎样 |
|---|---|---|
| `cmd` | OEM 代码页（英文 runner 上是 cp437） | **乱码**，还会写坏重定向的文件 |
| `powershell`（5.1） | 无 BOM 时按系统 ANSI 代码页 | **乱码**，直接语法报错 |
| `pwsh`（PowerShell 7） | 默认 UTF-8 | 正常 ✓ |
| `bash`（Ubuntu runner） | 默认 UTF-8 | 正常 ✓ |

踩过的具体报错长这样，`exe 里没有图标资源` 被读成了 `exe é‡Œæ²¡æœ‰å›¾æ ‡èµ„æº`：

```
Unexpected token 'æº"' in expression or statement.
The string is missing the terminator: ".
```

所以约定是：

- **`cmd` / `powershell` 的 `run:` 块一律纯 ASCII**（步骤名 `name:` 是元数据，不受影响，
  中文照写）。
- **需要中文就换 `pwsh`**。绿色版的中文 `使用说明.txt` 就是 `pwsh` + `Set-Content -Encoding utf8`
  写出来的。
- zip 里带中文文件名有额外的编码风险，所以打完包会用
  `System.IO.Compression.ZipFile` 把条目标题**读回来核对**，名字坏了就让 job 直接失败，
  而不是把乱码文件名的包发出去。

#### stderr 会让 powershell 步骤直接失败

runner 对 `powershell` / `pwsh` 步骤会预设 `$ErrorActionPreference = 'Stop'`。
在这种设置下，**原生命令只要往 stderr 写一个字**，PowerShell 就会把它包成
`NativeCommandError` 并抛成终止性错误，步骤立刻挂：

```
+ & $exe -h 2>&1 | Out-String | Write-Host
+ ~~~~~~~~~~~~~~
    + CategoryInfo : NotSpecified: (...) []. RemoteException
    + FullyQualifiedErrorId : NativeCommandError
```

`dming.exe -h` 正好就是这种：Go 的 `flag` 包把用法说明写到 **stderr**（这是惯例，
不是 bug）。所以冒烟测试改成跑 `dming.exe -no-open`（纯 stdout），
顺带把测试数据也准备了，验证程序真能读名单、真能出页面：

```powershell
Set-Content -Path (Join-Path $tmp 'p.txt') -Value "Alice`nBob`nCarol" -Encoding utf8
& .\dming.exe -no-open
# 然后断言 DMING*.html 存在，且里面含班级号和三个姓名
```

这比原来只跑个 `-h` 有用得多 —— 它真的在 Windows 上端到端跑了一遍。

### 为什么不做安装包

一开始用 Inno Setup 打了安装包，但坑太多：

- **32 位 Win10 和老版本 Win10 会被挡住**，表现是弹一句
  「该软件暂不支持在您的电脑上打开，请联系软件分发者」—— 这句是 **Windows 自己**
  弹的，在进入安装向导之前就拒绝了，用户完全看不到原因。
- 安装包对这个程序没有实质好处：它是**纯 Go 静态二进制**，不写注册表、
  不需要 DLL、不需要管理员权限，绿色版双击就能用。

所以现在只出 exe + 绿色版。绿色版长这样：

```
dming-portable\
├── dming.exe          ← 双击这个
├── p.txt              ← 换成你的名单（每行一个姓名）
├── class.txt          ← 班级名称
├── dming.ico
└── 使用说明.txt
```

`p.txt` 和 `class.txt` 必须和 `dming.exe` 在**同一个文件夹**里。

> 如果哪天真需要安装包，`git log` 里能找到被删掉的 `installer/dming.iss`。

## 隐私

**`p.txt` 是学生真实名单，绝对不要提交。** 已经处理好了：

- `.gitignore` 排除了 `p.txt`，本地 `git add -A` 不会带上它。
- 仓库里放的是 `p.sample.txt`（张三、李四、王五、赵六）当示例。
- 绿色版里的 `p.txt` 就是这份示例名单，不含任何真实姓名。

> 提醒：`.gitignore` 只能挡住「还没提交过」的文件。如果 `p.txt` 曾经被提交过，
> 它仍然留在历史里，得用 `git rm --cached p.txt` 再改写历史。本仓库从第一次提交起
> 就排除了，没有这个问题。

## 图标

`assets/dming.ico` 由 `assets/icon-source.png` 生成 —— 一张 620×620、带 alpha 的
正方形 PNG（最初是从一张动图里抽出来的第 15 帧，那张动图已不再随仓库分发）。

重新生成：

```bash
python3 assets/make-icon.py
```

它会把 `icon-source.png` 按**面积平均**缩放到 16 / 24 / 32 / 48 / 64 / 128 / 256，
写成 ICO。纯标准库，不需要 Pillow。ICO 里全部用 BMP 条目而不是 PNG 压缩条目 ——
后者体积小得多，但少数图标解析器不认，这里选了稳妥。

**想换图标**：用新的正方形 PNG 覆盖 `assets/icon-source.png`，再跑一遍即可。

## 常见问题

**下载的 exe 双击没反应 / 提示「Windows 已保护你的电脑」？**

那是 Windows 给**下载文件**加的 Zone.Identifier 标记，不是程序的问题。
右键 zip → 属性 → 勾「解除锁定」，解压后再运行。

**exe 图标没显示？**

Windows 图标缓存。换个文件夹，或刷新一下资源管理器。在同一目录反复重建同名 exe 时，
Windows 会一直显示旧图标。

**macOS / Linux 上跑不起来？**

去掉隔离属性并加执行权限：

```bash
chmod +x dming-darwin-arm64
xattr -d com.apple.quarantine dming-darwin-arm64
```

**Actions 页面没有 build 这个条目？**

确认 `.github/workflows/build.yml` 真的推上去了（GitHub 只认仓库根目录下的
`.github/workflows/`）。文件路径大小写敏感。

**job 报 `setup-go` 找不到某个 Go 版本？**

`go.mod` 里的 `go` 指令写的是具体补丁号（比如 `1.27.1`）而该版本还没发布。
改成两位版本号（`go 1.21`）即可，本仓库已经是这样。

## 文件说明

| 路径 | 说明 |
|---|---|
| `main.go` | 命令行、读取 `p.txt` / `class.txt`、临时文件与清理 |
| `page.go` | 点名页面 HTML 模板与渲染 |
| `browser.go` | 跨平台打开浏览器 |
| `build.sh` | Linux / macOS / Termux 交叉编译脚本 |
| `build.bat` | Windows 交叉编译脚本 |
| `.github/workflows/build.yml` | GitHub Actions 云端构建（嵌图标 + 出绿色版 + 发 Release） |
| `assets/dming.ico` | 多尺寸图标（16~256px），由 `icon-source.png` 生成 |
| `assets/icon-source.png` | 图标源图（620×620，带 alpha） |
| `assets/make-icon.py` | 图标生成脚本（纯标准库） |
| `class.txt` | 班级名称 |
| `p.txt` | 学生名单（自己维护，**不进版本库**） |
| `p.sample.txt` | 示例名单（张三、李四、王五、赵六） |
| `LICENSE` | Apache-2.0 |
