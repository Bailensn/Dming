# 用 GitHub Actions 云端打包

本机没有 cmd、PATH 坏了、不想装 Go —— 都没关系。
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
git commit -m "dming: Go 版点名器"
git branch -M main
git remote add origin https://github.com/<你的用户名>/dming.git
git push -u origin main
```

推送时会要求登录。GitHub 早就不支持密码了，用 **Personal Access Token**：
Settings → Developer settings → Personal access tokens → Tokens (classic) →
Generate new token，勾上 `repo`，把它当密码粘贴。

> 手机上推也行：用 Termux 的 git、或者 GitHub 官方 App 上传文件。

## 每次打包

推送完成后：

1. 打开仓库页面 → 顶部 **Actions** 标签
2. 左侧选 **build**
3. 想手动触发就点 **Run workflow** → **Run workflow**（推送时本来就会自动跑）
4. 等 1～3 分钟，等两个 job 都变绿：
   - **Windows exe**
   - **linux-amd64**、**darwin-arm64** 等（矩阵里的 5 个平台）
5. 点进跑完的那次运行，页面底部 **Artifacts** 区域下载：
   - `dming-windows` —— 里面有绿色版文件夹和两个架构的 exe
   - `dming-darwin-arm64` 等其他平台

下载得到的是 zip，解压即用。

> 手机上逛 GitHub 网页同样能下：Artifacts 区域点一下就能下 zip。
> 但注意**必须先登录**，Artifacts 对未登录用户不可见。

## 工作流做了什么

`.github/workflows/build.yml`：

| job | 跑在 | 做的事 |
|---|---|---|
| `windows` | `windows-latest` | 装 Go → `rsrc` 生成图标资源 → 编译 amd64/arm64 → 验证图标 → 组绿色版 → 上传 |
| `cross` | `ubuntu-latest` | 交叉编译 linux / darwin / android 共 5 个目标 |

### exe 图标是怎么嵌进去的

Go 链接器本身不读 `.ico`，但 `go build` 会**自动编译并链接**当前目录下形如
`rsrc_windows_amd64.syso` 的资源目标文件 —— 这是 Go 官方的机制，
意味着**源码一行都不用改**：

```bat
go install github.com/akavel/rsrc@latest
rsrc -ico assets\dming.ico -o rsrc_windows_amd64.syso -arch amd64
rsrc -ico assets\dming.ico -o rsrc_windows_arm64.syso -arch arm64
```

文件名后缀必须和目标平台匹配（`windows_amd64` / `windows_arm64`），
不匹配的会被 Go 忽略。反过来说两个文件放在一起也不会互相干扰，
各自只在自己对应的那次编译里生效。

工作流里还加了一步 PowerShell 校验，用 `System.Drawing` 把 exe 的图标读出来，
确认真的嵌进去了 —— 不然容易「以为嵌了其实没嵌」。

### 几个刻意的选择

- `go-version-file: go.mod` + `check-latest: true` —— 按 `go.mod` 的最低版本要求装
  最新的稳定版。**不要**在 workflow 里写死 `go-version: '1.27.1'` 这种补丁号，
  云端缓存里未必有，`setup-go` 会直接失败。
- `*.syso` 已加进 `.gitignore` —— 它是构建产物，构建时现生成。

### 为什么不做安装包

一开始用 Inno Setup 打了安装包，但坑太多：

- **32 位 Win10 和老版本 Win10 会被挡住**，表现是弹一句
  「该软件暂不支持在您的电脑上打开，请联系软件分发者」——
  这句是 **Windows 自己**弹的，在进入安装向导之前就拒绝了，用户完全看不到原因。
- 安装包对这个程序没有任何实质好处：它是**纯 Go 静态二进制**，
  不写注册表、不需要 DLL、不需要管理员权限，绿色版双击就能用。

所以现在只出 exe + 绿色版。如果哪天真需要安装包，
`git log` 里能找到被删除的 `installer/dming.iss`。

## 隐私

**`p.txt` 是学生真实名单，绝对不要提交。** 已经处理好了：

- `.gitignore` 里排除了 `p.txt`，本地 `git add -A` 不会带上它。
- 仓库里放的是 `p.sample.txt`（张三、李四、王五、赵六）当示例。
- 绿色版里的 `p.txt` 就是这份示例名单，不含任何真实姓名。

另外 `dist/`、`*.exe`、`*.syso` 也在 `.gitignore` 里，编译产物不会污染仓库。

> 提醒：`.gitignore` 只能挡住「还没提交过」的文件。如果 `p.txt` 曾经被提交过，
> 它仍然留在历史里，得用 `git rm --cached p.txt` 再改写历史。本仓库从第一次提交起
> 就排除了，没有这个问题。

## 常见问题

**下载的 exe 双击没反应 / 提示「Windows 已保护你的电脑」？**

那是 Windows 给**下载文件**加的 Zone.Identifier 标记，不是程序的问题。
右键 zip → 属性 → 勾「解除锁定」，解压后再运行。

**exe 图标没显示？**

Windows 图标缓存。换个文件夹、或者刷新一下资源管理器。
如果要在同一目录反复重建同名 exe，Windows 会一直显示旧图标。

**Actions 页面没有 build 这个条目？**
确认 `.github/workflows/build.yml` 真的推上去了（GitHub 只认仓库根目录下的
`.github/workflows/`）。文件路径大小写敏感。

**job 报 `setup-go` 找不到某个 Go 版本？**
`go.mod` 里的 `go` 指令写的是具体补丁号（比如 `1.27.1`）而该版本还没发布。
改成两位版本号（`go 1.21`）即可，本仓库已经是这样。

**想发 Release？**
在本地或云端跑通后，给提交打个 `v3.4` 之类的标签推上去，
工作流会触发；再在 GitHub 上创建 Release 并把 Artifacts 里的内容传上去即可。
