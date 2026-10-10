package main

import (
	"fmt"
	"os/exec"
	"path/filepath"
	"runtime"
)

// openBrowser 用系统默认浏览器 / 默认程序打开一个本地文件。
//
// 各平台使用的命令：
//
//	windows  rundll32 url.dll,FileProtocolHandler <file>，失败再退回 cmd /c start
//	darwin   open <file>
//	其他     xdg-open → termux-open → gio open → sensible-browser → x-www-browser → wslview
//
// 只要有一条命令能起来就算成功；全部失败时返回最后一个错误。
func openBrowser(path string) error {
	abs, err := filepath.Abs(path)
	if err != nil {
		abs = path
	}

	var lastErr error
	for _, argv := range openerCommands(abs) {
		cmd := exec.Command(argv[0], argv[1:]...)
		if err := cmd.Start(); err != nil {
			lastErr = err
			continue
		}
		// 浏览器是长驻进程，不阻塞等待，只在后台回收。
		go func() { _ = cmd.Wait() }()
		return nil
	}
	if lastErr == nil {
		lastErr = fmt.Errorf("当前系统没有可用的打开命令")
	}
	return lastErr
}

// openerCommands 返回按优先级排列的候选命令。
func openerCommands(path string) [][]string {
	switch runtime.GOOS {
	case "windows":
		return [][]string{
			{"rundll32", "url.dll,FileProtocolHandler", path},
			{"cmd", "/c", "start", "", path},
		}
	case "darwin":
		return [][]string{
			{"open", path},
		}
	default: // linux / android / *bsd
		return [][]string{
			{"xdg-open", path},
			{"termux-open", path}, // Android + Termux:API
			{"gio", "open", path},
			{"sensible-browser", path},
			{"x-www-browser", path},
			{"wslview", path}, // WSL
		}
	}
}
