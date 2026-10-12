@echo off
rem ---------------------------------------------------------------------------
rem  Build dming (C version) on Windows.
rem
rem  Needs a C compiler on PATH - MinGW-w64 (gcc) or LLVM/Clang, either is fine.
rem  windres is optional: without it the exe simply has no icon/version info.
rem
rem  The full multi-platform build runs on GitHub Actions.
rem
rem  This file is ASCII-only on purpose: cmd.exe decodes batch files using the
rem  OEM code page, so UTF-8 text would show up as garbage.
rem ---------------------------------------------------------------------------

setlocal
cd /d "%~dp0"

set OUT=dist
if not exist "%OUT%" mkdir "%OUT%"

echo === Regenerating src\page.h ===
python src\mkpage.py
if errorlevel 1 goto :nopython

set CC=
where gcc >nul 2>&1
if not errorlevel 1 set CC=gcc
if not defined CC (
	where clang >nul 2>&1
	if not errorlevel 1 set CC=clang
)
if not defined CC goto :nocc

echo.
echo === Compiling with %CC% ===
%CC% -std=gnu99 -O2 -Wall -Wextra -o "%OUT%\dming.exe" src\dming.c
if errorlevel 1 goto :fail

rem Icon + version info. Optional - a resource-less exe looks more suspicious
rem to antivirus heuristics, so this is worth having, but it must not be fatal.
set RC=
where windres >nul 2>&1
if not errorlevel 1 set RC=windres

if defined RC (
	echo.
	echo === Compiling resources ^(icon + version info^) ===
	pushd src
	%RC% dming.rc -O coff -o dming_res.o
	popd
	if exist src\dming_res.o (
		%CC% -std=gnu99 -O2 -s -o "%OUT%\dming.exe" src\dming.c src\dming_res.o -luser32
		if errorlevel 1 goto :fail
	) else (
		echo Warning: windres failed - continuing without icon/version info.
	)
) else (
	echo Warning: windres not found - the exe will have no icon or version info.
)

echo.
echo Done: %OUT%\dming.exe
dir /b "%OUT%"
exit /b 0


:nopython
echo.
echo python was not found.
echo It is only used to regenerate src\page.h from src\page.html.
echo Install Python, or make sure src\page.h already exists.
exit /b 1

:nocc
echo.
echo No C compiler found on PATH.
echo Install MinGW-w64 or LLVM/Clang, open a new terminal, then retry.
exit /b 1

:fail
echo.
echo BUILD FAILED.
exit /b 1
