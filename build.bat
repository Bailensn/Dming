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

rem The version string lives in TWO places: APP_VERSION in src\dming.c and
rem FILEVERSION / FileVersion in src\dming.rc. Change one, change the other,
rem or the exe properties will show a stale version.
echo === Regenerating src\page.h ===
set PY=
where python >nul 2>&1
if not errorlevel 1 set PY=python
if not defined PY (
	where py >nul 2>&1
	if not errorlevel 1 set PY=py
)
if not defined PY goto :nopython
%PY% src\mkpage.py
if errorlevel 1 goto :fail

set CC=
where gcc >nul 2>&1
if not errorlevel 1 set CC=gcc
if not defined CC (
	where clang >nul 2>&1
	if not errorlevel 1 set CC=clang
)
if not defined CC goto :nocc

rem Deliberately NO -s here. Stripping saves only ~3 KB, but it pushes the PE
rem closer to the "no symbols + tiny import table + no resources" profile that
rem packed malware has. Better to look like an ordinary mingw build.
rem --dynamicbase / --nxcompat ask for ASLR + DEP explicitly; modern legitimate
rem binaries always have them.
set WFLAGS=-std=gnu99 -O2 -Wall -Wextra -Wl,--dynamicbase -Wl,--nxcompat

echo.
echo === Compiling with %CC% ===
%CC% %WFLAGS% -o "%OUT%\dming.exe" src\dming.c
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
		%CC% %WFLAGS% -o "%OUT%\dming.exe" src\dming.c src\dming_res.o -lshell32 -luser32
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
