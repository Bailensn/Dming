@echo off

setlocal
cd /d "%~dp0"

set OUT=dist
if not exist "%OUT%" mkdir "%OUT%"

set CC=
where gcc >nul 2>&1
if not errorlevel 1 set CC=gcc
if not defined CC (
	where clang >nul 2>&1
	if not errorlevel 1 set CC=clang
)
if not defined CC goto :nocc

set WFLAGS=-std=gnu99 -O2 -Wall -Wextra -Wl,--dynamicbase -Wl,--nxcompat

echo.
echo === Compiling with %CC% ===
%CC% %WFLAGS% -o "%OUT%\dming.exe" dming.c -lshell32 -luser32 -lws2_32
if errorlevel 1 goto :fail

set RC=
where windres >nul 2>&1
if not errorlevel 1 set RC=windres

if defined RC (
	echo.
	echo === Compiling resources ^(icon + version info^) ===
	%RC% dming.rc -O coff -o dming_res.o
	if exist dming_res.o (
		%CC% %WFLAGS% -o "%OUT%\dming.exe" dming.c dming_res.o -lshell32 -luser32 -lws2_32
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


:nocc
echo.
echo No C compiler found on PATH.
echo Install MinGW-w64 or LLVM/Clang, open a new terminal, then retry.
exit /b 1

:fail
echo.
echo BUILD FAILED.
exit /b 1
