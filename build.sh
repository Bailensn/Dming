#!/usr/bin/env bash
# 交叉编译 dming（Go 重写版点名器）。
#
# 用法：
#   ./build.sh                 # 编译全部桌面平台
#   ./build.sh darwin/arm64    # 只编译指定平台
#   ./build.sh android/arm64   # Termux / Android
#
# 产物输出到 dist/ 目录。

set -euo pipefail
cd "$(dirname "$0")"

APP="dming"
OUT="dist"
PKG="."

TARGETS=(
	# 64 位
	"linux/amd64"
	"linux/arm64"
	"darwin/amd64"
	"darwin/arm64"
	"windows/amd64"
	"windows/arm64"
	"android/arm64" # Termux
	# 32 位
	"linux/386"
	"linux/arm"
	"windows/386"
)

# 说明：
#   - 想给 windows 产物嵌图标，先跑 rsrc 生成 rsrc_windows_<arch>.syso，
#     详见 README 的「exe 图标（Windows）」。
#   - Go 不支持 windows/arm（32 位 ARM Windows），darwin 也没有 32 位。
#   - linux/arm 默认 GOARM=7；要支持更老的 ARMv6 设备就设 GOARM=6 重建。

if [ "$#" -gt 0 ]; then
	TARGETS=("$@")
fi

if ! command -v go >/dev/null 2>&1; then
	echo "错误：没有找到 go 命令，请先安装 Go（版本要求见 go.mod）" >&2
	exit 1
fi

mkdir -p "$OUT"

for target in "${TARGETS[@]}"; do
	os="${target%%/*}"
	arch="${target##*/}"
	ext=""
	[ "$os" = "windows" ] && ext=".exe"
	out="$OUT/$APP-$os-$arch$ext"

	printf '  %-16s -> %s\n' "$target" "$out"
	CGO_ENABLED=0 GOOS="$os" GOARCH="$arch" \
		go build -trimpath -ldflags "-s -w" -o "$out" "$PKG"
done

echo
echo "完成，产物在 $OUT/："
ls -1 "$OUT"
