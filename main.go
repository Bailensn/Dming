// Command dming 是一个课堂随机点名器。
//
// 它从「程序所在目录」读取两个纯文本文件：
//
//	p.txt      每行一个学生姓名（空行自动忽略）
//	class.txt  班级名称，取第一个非空行
//
// 读取完毕后生成一个临时 HTML 页面（DMING*.html），用系统默认浏览器打开，
// 页面上提供「开始点名 / 停止点名」和「重置名单」两个按钮。程序在浏览器
// 打开后等待一段时间即删除临时文件，可用 -keep 保留。
//
// 支持 Linux（含 Termux/Android）、macOS、Windows。
package main

import (
	"flag"
	"fmt"
	"os"
	"os/signal"
	"path/filepath"
	"strings"
	"syscall"
	"time"
)

// version 显示在点名页面左上角，可按需修改。
const version = "3.4"

func main() {
	normalizeArgs()
	flag.Usage = usage
	keep := flag.Bool("keep", false, "保留生成的临时 HTML 文件，不自动删除")
	delay := flag.Duration("delay", 3*time.Second, "浏览器打开后等待多久再删除临时文件")
	noOpen := flag.Bool("no-open", false, "只生成 HTML 文件，不自动打开浏览器")
	quiet := flag.Bool("quiet", false, "安静模式，只输出临时文件路径")
	flag.Parse()

	if err := run(options{keep: *keep, delay: *delay, noOpen: *noOpen, quiet: *quiet}); err != nil {
		fmt.Fprintln(os.Stderr, "错误："+err.Error())
		os.Exit(1)
	}
}

func usage() {
	fmt.Fprintf(flag.CommandLine.Output(), `%s %s —— 课堂随机点名器

用法：
  %s [选项]

数据文件（放在程序所在目录）：
  p.txt       每行一个学生姓名
  class.txt   班级名称，取第一个非空行

选项：
`, filepath.Base(os.Args[0]), version, filepath.Base(os.Args[0]))
	flag.PrintDefaults()
}

// normalizeArgs 修正一类 Android 环境下的 argv。
//
// Android 10+ 的 SELinux 策略禁止 untrusted_app 直接 execve 应用私有目录下的文件，
// Termux 用 termux-exec（LD_PRELOAD 拦截 libc execve）改走 /system/bin/linker64 启动，
// 而 bionic 链接器会把「链接器自身的 argv」交给被启动的程序，于是 Go 程序看到的
// os.Args[1] 变成了它自己的可执行文件路径。这会让 flag 解析在第一个参数处就停下，
// 所以这里先把这个多余项剔除；其他平台不受影响。
func normalizeArgs() {
	if len(os.Args) < 2 {
		return
	}
	exe, err := os.Executable()
	if err != nil {
		return
	}
	self := []string{exe}
	if resolved, err := filepath.EvalSymlinks(exe); err == nil {
		self = append(self, resolved)
	}
	for _, s := range self {
		if os.Args[1] == s {
			os.Args = append(os.Args[:1], os.Args[2:]...)
			return
		}
	}
}

type options struct {
	keep   bool
	delay  time.Duration
	noOpen bool
	quiet  bool
}

func (o options) logf(format string, a ...any) {
	if !o.quiet {
		fmt.Printf(format+"\n", a...)
	}
}

func run(opt options) error {
	dir, err := appDir()
	if err != nil {
		return err
	}

	students, err := readStudents(filepath.Join(dir, "p.txt"))
	if err != nil {
		return err
	}

	className, err := readClass(filepath.Join(dir, "class.txt"))
	if err != nil {
		return err
	}

	page, err := renderPage(className, students)
	if err != nil {
		return err
	}

	opt.logf("已从 %s 读取 %d 个姓名", filepath.Join(dir, "p.txt"), len(students))
	opt.logf("班级：%s", className)

	opt.logf("正在生成临时页面...")
	pagePath, err := writeTempPage(page)
	if err != nil {
		return err
	}
	opt.logf("临时文件: %s", pagePath)

	if opt.noOpen {
		fmt.Println(pagePath)
		return nil
	}

	if err := openBrowser(pagePath); err != nil {
		opt.logf("未能自动打开浏览器（%v），请手动打开上面的文件", err)
	} else {
		opt.logf("已调用系统默认浏览器打开")
	}

	if opt.keep {
		return nil
	}

	// 浏览器需要一点时间去读取文件，之后再清理。
	waitForCleanup(opt.delay)

	if err := os.Remove(pagePath); err != nil {
		return fmt.Errorf("临时文件删除失败: %w", err)
	}
	opt.logf("临时文件已删除")
	return nil
}

// appDir 返回数据文件所在目录。
//
// 优先使用可执行文件所在目录，这样双击 / 快捷方式启动也能找到 p.txt；
// 但 `go run` 会把可执行文件放进临时目录，此时退回当前工作目录。
func appDir() (string, error) {
	var candidates []string
	if exe, err := os.Executable(); err == nil {
		if resolved, err := filepath.EvalSymlinks(exe); err == nil {
			exe = resolved
		}
		candidates = append(candidates, filepath.Dir(exe))
	}
	if wd, err := os.Getwd(); err == nil {
		candidates = append(candidates, wd)
	}
	if len(candidates) == 0 {
		return "", fmt.Errorf("无法确定程序所在目录")
	}
	for _, dir := range candidates {
		if fileExists(filepath.Join(dir, "p.txt")) || fileExists(filepath.Join(dir, "class.txt")) {
			return dir, nil
		}
	}
	return candidates[0], nil
}

func fileExists(path string) bool {
	info, err := os.Stat(path)
	return err == nil && !info.IsDir()
}

// readStudents 读取名单文件，每行一个姓名，忽略空行，
// 兼容 Windows 换行（\r\n）并去掉每行首尾空白。
func readStudents(path string) ([]string, error) {
	lines, err := readLines(path)
	if err != nil {
		return nil, fmt.Errorf("未找到名单文件 %s: %w", path, err)
	}
	var students []string
	for _, line := range lines {
		if name := strings.TrimSpace(line); name != "" {
			students = append(students, name)
		}
	}
	if len(students) == 0 {
		return nil, fmt.Errorf("名单文件 %s 为空", path)
	}
	return students, nil
}

// readClass 读取班级文件，返回第一个非空行。
func readClass(path string) (string, error) {
	lines, err := readLines(path)
	if err != nil {
		return "", fmt.Errorf("未找到班级文件 %s（请写入班级名称，例如 14）: %w", path, err)
	}
	for _, line := range lines {
		if class := strings.TrimSpace(line); class != "" {
			return class, nil
		}
	}
	return "", fmt.Errorf("班级文件 %s 为空（请写入班级名称，例如 14）", path)
}

func readLines(path string) ([]string, error) {
	data, err := os.ReadFile(path)
	if err != nil {
		return nil, err
	}
	// 去掉 UTF-8 BOM，避免第一个姓名/班级名带上不可见字符。
	text := strings.TrimPrefix(string(data), "\ufeff")
	return strings.Split(strings.ReplaceAll(text, "\r\n", "\n"), "\n"), nil
}

// writeTempPage 把页面写到当前目录下的 DMING*.html。
func writeTempPage(page []byte) (string, error) {
	dir, err := os.Getwd()
	if err != nil {
		dir = ""
	}
	f, err := os.CreateTemp(dir, "DMING*.html")
	if err != nil {
		// 当前目录不可写时退回系统临时目录。
		f, err = os.CreateTemp("", "DMING*.html")
		if err != nil {
			return "", fmt.Errorf("无法创建临时文件: %w", err)
		}
	}
	path := f.Name()
	if _, err := f.Write(page); err != nil {
		f.Close()
		os.Remove(path)
		return "", fmt.Errorf("写入临时文件失败: %w", err)
	}
	if err := f.Close(); err != nil {
		os.Remove(path)
		return "", fmt.Errorf("写入临时文件失败: %w", err)
	}
	if abs, err := filepath.Abs(path); err == nil {
		path = abs
	}
	return path, nil
}

// waitForCleanup 等待指定时长；用户按 Ctrl+C 时立即返回。
func waitForCleanup(d time.Duration) {
	if d <= 0 {
		return
	}
	sig := make(chan os.Signal, 1)
	signal.Notify(sig, os.Interrupt, syscall.SIGTERM)
	defer signal.Stop(sig)
	select {
	case <-time.After(d):
	case <-sig:
	}
}
