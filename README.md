# dming —— 课堂随机点名器（纯 C 版）

单文件可执行程序，零第三方依赖。读 `p.txt` 里的名单随机点名，
生成一个网页用默认浏览器打开。

支持 Windows / macOS / Linux / Android(Termux)。

## 为什么用 C 而不是 Go

**因为杀软误报。** 之前这程序是 Go 写的，但 Go 链接出的 PE 有几个特点
（没有常规 CRT、节表不寻常、导入表极简），国内杀软（尤其 360）的启发式引擎
很容易直接判毒，双击就秒。用系统自带 C 工具链编出的常规 PE 误报率低得多。

顺带的好处：

- 体积小：Windows 版约 50 KB（Go 版 2 MB 起），macOS / Linux 更小
- 启动快，无运行时
- 交叉编译链路清晰，CI 里不用装一整套 Go 工具链

## 数据文件

**跟可执行文件放在同一个目录**（不是当前工作目录）：

- `p.txt` —— 每行一个学生姓名，空行忽略，自动去掉首尾空白
- `class.txt` —— 班级名称，取**第一个非空行**

两个文件都兼容 CRLF 和 UTF-8 BOM。缺哪个会明确报出完整路径；
在 Windows 上还会弹窗提示（双击运行时看不见控制台）。

所以你可以从任何目录启动它，比如建快捷方式 —— 它照样去程序所在目录找名单。

> Termux 用户注意：Android 上 `/proc/self/exe` 永远指向 `linker64`
> 而不是程序本身（termux-exec 为了绕开 SELinux 对私有目录的执行限制，
> 统一交给 `/system/bin/linker64` 启动）。所以这里优先用 `/proc/self/exe`，
> 但会排掉 linker，并回退到 `argv[0]` 解析（含 PATH 搜索）。
> 这条逻辑在 Go 版里其实是坏的，只是当时没暴露出来。

## 命令行

```
dming                生成页面并用默认浏览器打开，3 秒后删除临时文件
dming -keep          保留生成的 DMING*.html
dming -no-open       只生成文件，打印路径
dming -delay 8s      延长清理前的等待时间（也支持 800ms，裸数字按秒算）
dming -quiet         安静模式，只输出临时文件路径
dming -h             帮助
dming -v             版本
```

## 编译

### 本地

```bash
./build.sh                                  # 编当前平台，产物在 dist/
```

Windows 上：

```bat
build.bat
```

交叉编译（工具链自备）：

```bash
CC=x86_64-w64-mingw32-gcc EXT=.exe RC=x86_64-w64-mingw32-windres \
  ./build.sh windows-amd64
```

### 页面模板

页面 HTML 在 `src/page.html`，通过 `src/mkpage.py` 转成 C 字符串字面量
`src/page.h`。改页面就编辑 `page.html`，然后：

```bash
python3 src/mkpage.py
```

`page.h` 是生成物，不要直接改。

之所以能生成得这么干净，是因为 `page.html` 里**没有一个双引号** ——
HTML 属性统一用单引号，JS 字符串也用单引号。这样每行只要加一对引号和结尾的
`\n`，不需要任何转义（`"` 和 `\` 都是转义重灾区，少碰为妙）。

### Windows 资源（图标 + 版本信息）

`src/dming.rc` 定义 exe 的图标和版本信息。没有资源信息的裸 exe 在杀软眼里
更可疑，所以这不是装饰。

```bat
cd src
windres dming.rc -O coff -o dming_res.o
cd ..
gcc -std=gnu99 -O2 -s -o dist\dming.exe src\dming.c src\dming_res.o -luser32
```

`build.bat` 会自动做这一步；找不到 `windres` 就跳过并给警告（不致命）。

CI 里编完会用 `tools/check-pe.py` 断言三个 exe 都真的有 `.rsrc` 段 ——
光看「文件属性里有没有图标」测不出来：文件没有图标资源时 Windows 返回的是
通用程序图标而不是失败，等于白测。

## 覆盖的平台

| 64 位 | 32 位 |
|---|---|
| `windows/amd64`、`windows/arm64` | `windows/386` |
| `darwin/amd64`、`darwin/arm64` | —— |
| `linux/amd64`、`linux/arm64` | `linux/386`、`linux/arm` |
| `android/arm64`（Termux） | —— |

几点说明：

- **`windows/arm` 不存在** —— Windows 只有 `386` / `amd64` / `arm64`，
  32 位就是 `386`。
- **`darwin` 没有 32 位** —— macOS 从 10.15 起不支持 32 位程序。
- **不编 `android/arm`** —— 32 位 Android 设备基本见不到了。注意 Termux 必须用
  `android` 而不是 `linux` 目标：Android 内核只认 PIE，`linux/arm64` 的产物会被
  拒绝加载。
- Linux 各架构一律**静态链接**，免得依赖构建机的 glibc 版本。
- Android 上**不能**静态链接（需要 bionic 的动态装载器）。

## GitHub Actions

推送代码后自动构建全部平台，**全绿就自动发 Release**，不需要手动打 tag。

### 每次打包

1. 仓库页面 → **Actions** → 左侧 **build**
2. 四个构建 job 跑完：**Windows**、**linux-\***、**android-arm64**、**macOS**
3. 全绿后 **release** job 自动汇总成 Release
4. 去仓库首页右侧 **Releases** 下载

也可以从当次运行的 **Artifacts** 拿（构建完就有，不用等 Release）。

### 发 Release

tag 名由工作流自己算，就是本次提交的 **7 位短 SHA**（如 `3d462a5`）。

```bash
git push origin main     # 就这一条，剩下的全自动
```

Release 分两种：

| 情况 | 结果 |
|---|---|
| 全部平台构建成功 | **正式 Release** |
| 有平台失败 / 被取消 | 照样发，但标记成 **Pre-Release**，并在说明顶部列出哪些平台没出来 |

Pre-Release 在 GitHub 上带 `Pre-release` 标记，也**不会**被算作 "Latest release"，
所以半成品不会被当成稳定版推给用户，但产物还是能下载来试。

判断逻辑在 release job 里：读 `needs.<job>.result`，只要有一个不是 `success`
就置 `prerelease=true`。所以那个 job 的 `if` 必须写 `!cancelled()` 而不是默认的
`success()` —— 否则任一构建失败时整个 job 会被跳过，什么都发不出来。

（顺带一提：`if` 得写成 `${{ !cancelled() && ... }}`。直接裸写 `!cancelled()`
的话，开头的 `!` 会被 YAML 当成标签标记，脚本根本解析不了。）

极端情况下所有平台都失败、一个产物都没有 —— 这时候直接报错退出，不建空 Release。

> 副作用：**每推送一次 main 就产生一个 Release**。嫌吵的话把 release job 的
> `if` 收窄成只在 `workflow_dispatch` 时才发。

手动推 tag 依然有效，此时 Release 挂在你推的那个 tag 上。tag 名会被校验，
只认十六进制（`tags: ['[0-9a-f]*']`）。

用 `GITHUB_TOKEN` 创建的 tag 不会再次触发 workflow（GitHub 的防递归规则），
所以不存在自己触发自己。

### 工作流里几个刻意的选择

- **Windows 目标在 ubuntu runner 上用 mingw 交叉编译**，不在 windows runner 上编。
  除了 runner 更快，更重要的是：打包（绿色版目录 + 含中文文件名的 zip）可以直接
  用 bash 和 `zip`，彻底避开 PowerShell / cmd 的编码问题。之前那版在 windows
  runner 上被 `$ErrorActionPreference` 和代码页坑过好几次。
- Windows 的 i686/x86_64 用 apt 的 `gcc-mingw-w64`（Debian 只有这两个 target），
  aarch64 用 [llvm-mingw](https://github.com/mstorsjo/llvm-mingw)，版本写死以便复现。
- **linux-amd64 那一步会真的把程序跑一遍**：造一个含空行、前后空格、CRLF、
  中文名的 `p.txt`，`cd /` 到别的目录执行，再断言生成的 HTML 里班级号、
  版本号和姓名数组都对。这是端到端测试，不是只看编译过没过。

## 隐私

**`p.txt` 是学生真实名单，绝对不要提交。**

- `.gitignore` 排除了 `p.txt`
- 仓库里放的是 `p.sample.txt`（张三、李四、王五、赵六）当示例
- 绿色版里的 `p.txt` 就是这份示例名单，不含任何真实数据

## 文件说明

| 路径 | 说明 |
|---|---|
| `src/dming.c` | 主程序（全部逻辑） |
| `src/page.html` | 点名页面模板，可读的源文件 |
| `src/page.h` | 由 `mkpage.py` 从 `page.html` 生成，不要直接改 |
| `src/mkpage.py` | 模板生成器 |
| `src/dming.rc` | Windows 资源：图标 + 版本信息 |
| `tools/check-pe.py` | 校验 exe 里有 `.rsrc` 段（CI 用） |
| `build.sh` / `build.bat` | 本地构建脚本 |
| `.github/workflows/build.yml` | 云端构建 + 自动发 Release |
| `assets/dming.ico` | 多尺寸图标（16~256px） |
| `assets/icon-source.png` | 图标源图（620×620，带 alpha） |
| `assets/make-icon.py` | 图标生成脚本（纯标准库） |
| `class.txt` | 班级名称 |
| `p.txt` | 学生名单（自己维护，**不进版本库**） |
| `p.sample.txt` | 示例名单 |
| `LICENSE` | Apache-2.0 |

## 常见问题

**Windows 上双击没反应 / 提示「Windows 已保护你的电脑」？**

那是 Windows 给**下载文件**加的 Zone.Identifier 标记，不是程序的问题。
右键 zip → 属性 → 勾「解除锁定」，解压后再运行。

**还是被杀软报毒？**

C 版已经大幅降低误报率，但**只要没做代码签名，任何从网上下载的 exe 都可能
被误报** —— 这是签名问题，不是语言问题。彻底解决只有两条路：买代码签名证书，
或者把样本提交给厂商做误报申诉（360 有在线申诉入口）。有证书的话在 CI 里用
`signtool` 签一下就能彻底摆脱。

**提示「未找到名单文件 ...\p.txt」？**

它只认**程序所在目录**，不是当前目录。确认 `p.txt` 和 `dming.exe` 在同一个
文件夹里（错误信息里有完整路径，照着找就行）。

**exe 图标没显示？**

Windows 图标缓存。换个文件夹，或刷新资源管理器。同一目录反复重建同名 exe 时
Windows 会一直显示旧图标。

**macOS / Linux 上跑不起来？**

```bash
chmod +x dming-darwin-arm64
xattr -d com.apple.quarantine dming-darwin-arm64
```

**Actions 里没有 build 这个条目？**

确认 `.github/workflows/build.yml` 推上去了（GitHub 只认仓库根目录下的
`.github/workflows/`）。路径大小写敏感。

## 图标

`assets/dming.ico` 由 `assets/icon-source.png` 生成 —— 一张 620×620、带 alpha 的
正方形 PNG（最初是从一张动图里抽出来的第 15 帧）。

```bash
python3 assets/make-icon.py
```

按**面积平均**缩放到 16 / 24 / 32 / 48 / 64 / 128 / 256，写成 ICO。
纯标准库，不需要 Pillow。ICO 里全部用 BMP 条目而不是 PNG 压缩条目 ——
后者体积小得多，但少数图标解析器不认，这里选了稳妥。

**换图标**：用新的正方形 PNG 覆盖 `assets/icon-source.png` 再跑一遍即可。
