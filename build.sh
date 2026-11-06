#!/usr/bin/env bash

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

RES=""
if [ -n "$RC" ] && command -v "$RC" >/dev/null 2>&1; then
	echo "=== 编译资源（图标 + 版本信息）==="
	"$RC" dming.rc -O coff -o dming_res.o
	RES="dming_res.o"
elif [ -n "$RC" ]; then
	echo "警告：找不到 $RC，产物将不含图标和版本信息" >&2
fi

echo "=== 编译 $TARGET ==="
$CC $CFLAGS -o "$BIN" dming.c $RES

echo
echo "完成: $BIN"
ls -la "$BIN"
