#!/usr/bin/env bash
# 本地编译 dming（C 版）。
#
# 默认编当前平台：
#   ./build.sh
#
# 指定交叉编译器（需要你自己装好对应工具链）：
#   CC=x86_64-w64-mingw32-gcc EXT=.exe RC=x86_64-w64-mingw32-windres ./build.sh windows-amd64
#
# 多平台交叉编译在 GitHub Actions 里做（见 .github/workflows/build.yml），
# 因为在本地凑齐 i686/x86_64/aarch64 三套 mingw 加各架构 Linux 工具链太麻烦。

set -euo pipefail
cd "$(dirname "$0")"

OUT="dist"
CC="${CC:-cc}"
EXT="${EXT:-}"
RC="${RC:-}"
TARGET="${1:-$(uname -s | tr 'A-Z' 'a-z')-$(uname -m)}"
BIN="$OUT/dming-$TARGET$EXT"

CFLAGS="${CFLAGS:--std=gnu99 -O2 -Wall -Wextra}"

mkdir -p "$OUT"

echo "=== 生成 page.h ==="
python3 src/mkpage.py

# Windows 的话顺手把图标和版本信息也编进去
RES=""
if [ -n "$RC" ] && command -v "$RC" >/dev/null 2>&1; then
	echo "=== 编译资源（图标 + 版本信息）==="
	( cd src && "$RC" dming.rc -O coff -o dming_res.o )
	RES="src/dming_res.o"
elif [ -n "$RC" ]; then
	echo "警告：找不到 $RC，产物将不含图标和版本信息" >&2
fi

echo "=== 编译 $TARGET ==="
# shellcheck disable=SC2086
$CC $CFLAGS -o "$BIN" src/dming.c $RES

echo
echo "完成: $BIN"
ls -la "$BIN"
