# 用 GitHub Actions 云端打包

本机没有 cmd、PATH 坏了、不想装 Go / Inno Setup —— 都没关系。
GitHub 的云端 Windows runner 是**完整 Windows 环境**，`cmd.exe` 和 PATH 全部正常，
Build 完在网页上下载产物就行；手机上也能操作。

## 一次性准备

### 1. 建一个仓库

浏览器打开 https://github.com/new

- **Repository name**：`dming`
- 选 **Private**（私有）—— 建议，见下面「隐私」一节
- **不要**勾选 Add a README / .gitignore / license（本仓库已经有了）
- 点 Create repository

### 2. 把项目推上去

仓库建好后页面会显示一段命令。在**任何一台有 git 的机器**上，进入本文件夹执行
（把 `<你的用户名>` 换成你的 GitHub 用户名）：

```bash
git init
git add -A
git commit -m "dming: Go 版点名器 + Inno Setup 安装包"
git branch -M main
git remote add origin https://github.com/<你的用户名>/dming.git
git push -u origin main
```

推送时会要求登录。GitHub 早就不支持密码了，用 **Personal Access Token**：
Settings → Developer settings → Personal access tokens → Tokens (classic) →
Generate new token，勾上 `repo`，把它当密码粘贴。

> 手机上推也行：用 Termux 的 git、或者 GitHub 官方 App 上传文件。
> 实在都不想装，可以在 GitHub 网页上按文件夹结构一个个建文件、粘贴内容。

## 每次打包

推送完成后：

1. 打开仓库页面 → 顶部 **Actions** 标签
2. 左侧选 **build**
3. 想手动触发就点 **Run workflow** → **Run workflow**（推送时本来就会自动跑）
4. 等 1～3 分钟，等两个 job 都变绿：
   - **Windows exe + 安装包**
   - **linux-amd64**、**darwin-arm64** 等（矩阵里的 5 个平台）
5. 点进跑完的那次运行，页面底部 **Artifacts** 区域下载：
   - `dming-windows` —— 里面有 `dming-windows-amd64.exe` 和 `dming-3.4-setup.exe`
   - `dming-darwin-arm64` 等其他平台

下载得到的是 zip，解压即用。

> 手机上逛 GitHub 网页同样能下：Artifacts 区域点一下就能下 zip。
> 但注意**必须先登录**，Artifacts 对未登录用户不可见。

## 工作流做了什么

`.github/workflows/build.yml`：

| job | 跑在 | 做的事 |
|---|---|---|
| `windows` | `windows-latest` | 装 Go → 编译 exe → 冒烟测试 → `choco install innosetup` → ISCC 打包 → 上传 |
| `cross` | `ubuntu-latest` | 交叉编译 linux / darwin / android 共 5 个目标 |

几个刻意的选择：

- `go-version-file: go.mod` + `check-latest: true` —— 按 `go.mod` 的最低版本要求装
  最新的稳定版。**不要**在 workflow 里写死 `go-version: '1.27.1'` 这种补丁号，
  云端缓存里未必有，`setup-go` 会直接失败。
- `choco install innosetup` —— Windows runner 自带 Chocolatey，一条命令搞定 ISCC。
- 冒烟测试那步 `continue-on-error: true` —— 只为确认产物不是坏二进制，
  它失败不该拦住打包。
- 产物路径 `dist/dming-*-setup.exe` —— 跟 `dming.iss` 里的
  `OutputDir=..\dist` 和 `OutputBaseFilename={#AppShortName}-{#AppVersion}-setup`
  对应，改版本号也不用改 workflow。

## 隐私

**`p.txt` 是学生真实名单，绝对不要提交。** 已经处理好了：

- `.gitignore` 里排除了 `p.txt`，本地 `git add -A` 不会带上它。
- 仓库里放的是 `p.sample.txt`（张三、李四、王五、赵六）当示例。
- `installer/dming.iss` 打包时也**不含**任何名单文件，安装包里只有程序、图标和
  `class.txt`。

另外 `dist/` 和 `*.exe` 也在 `.gitignore` 里，编译产物不会污染仓库。

> 提醒：`.gitignore` 只能挡住「还没提交过」的文件。如果 `p.txt` 曾经被提交过，
> 它仍然留在历史里，得用 `git rm --cached p.txt` 再改写历史。本仓库从第一次提交起
> 就排除了，没有这个问题。

## 常见问题

**装的时候提示「该软件暂不支持在您的电脑上打开，请联系软件分发者」？**

这句话是 **Windows 自己**弹的（不是 Inno 的），说明 setup 在进入向导之前就被系统拒绝了，
所以你连具体原因都看不到。原因是我最初写死了 `ArchitecturesAllowed=x64compatible`，
它要求 64 位 Windows。现在**已经去掉了架构限制**：这个程序是纯 Go 静态二进制、
装在 `%LocalAppData%` 用户目录，不碰 Program Files 的 32/64 位重定向，
限制架构没有任何好处，只会把 32 位 Win10 和部分 ARM 设备挡在门外。

重新跑一次 workflow，下载最新的 Artifacts 即可。

另外还有两个可能：

- **Win10 太老**：Inno Setup 的现代安装包需要 Windows 10 1607（2016 年 8 月）或更新。
- **ARM64 设备**：老的骁龙本可能不支持 x64 模拟。

**这两种情况直接用免安装绿色版**：Artifacts 里的 `dming-portable\` 文件夹，
解压 → 把 `p.txt` 换成你的名单 → 双击 `dming.exe`，不需要任何安装。

需要明确区分 32/64 位安装包时，命令行覆盖即可：

```bat
ISCC.exe /DARCH=x64compatible installer\dming.iss
```

**Actions 页面没有 build 这个条目？**
确认 `.github/workflows/build.yml` 真的推上去了（GitHub 只认仓库根目录下的
`.github/workflows/`）。文件路径大小写敏感。

**job 报 `setup-go` 找不到某个 Go 版本？**
`go.mod` 里的 `go` 指令写的是具体补丁号（比如 `1.27.1`）而该版本还没发布。
改成两位版本号（`go 1.21`）即可，本仓库已经是这样。

**ISCC 报「找不到源文件」？**
`installer/dming.iss` 里 `SourceExe` 指向 `..\dist\dming-windows-amd64.exe`，
必须保证前一步编译成功且输出到 `dist/`。看 workflow 里「编译 windows/amd64」那步的日志。

**想发 Release？**
在本地或云端跑通后，给提交打个 `v3.4` 之类的标签推上去，
工作流会触发；再在 GitHub 上创建 Release 并把 Artifacts 里的安装包传上去即可。
