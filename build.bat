@echo off
rem ---------------------------------------------------------------------------
rem  Cross-compile dming for Windows / macOS / Linux / Android (Termux).
rem
rem  Usage:
rem    build.bat                    build every desktop target
rem    build.bat windows amd64      build one target
rem    build.bat darwin/arm64       slash form is accepted too
rem
rem  Output goes to dist\.
rem
rem  This file is ASCII-only on purpose: cmd.exe decodes batch files using the
rem  OEM code page, so UTF-8 text would show up as garbage.
rem ---------------------------------------------------------------------------

setlocal
cd /d "%~dp0"

set APP=dming
set OUT=dist
set CGO_ENABLED=0

if not exist "%OUT%" mkdir "%OUT%"

if "%~1"=="" goto :buildall
if not "%~2"=="" goto :buildpair
rem one argument: accept the "os/arch" form
for /f "tokens=1,2 delims=/" %%A in ("%~1") do call :build %%A %%B
goto :done

:buildpair
call :build %~1 %~2
goto :done

:buildall
rem 64-bit
call :build windows amd64
if errorlevel 1 goto :fail
call :build windows arm64
if errorlevel 1 goto :fail
call :build linux amd64
if errorlevel 1 goto :fail
call :build linux arm64
if errorlevel 1 goto :fail
call :build darwin amd64
if errorlevel 1 goto :fail
call :build darwin arm64
if errorlevel 1 goto :fail
call :build android arm64
if errorlevel 1 goto :fail
rem 32-bit
rem Go has no windows/arm (32-bit ARM Windows), and no 32-bit darwin at all.
call :build windows 386
if errorlevel 1 goto :fail
call :build linux 386
if errorlevel 1 goto :fail
call :build linux arm
if errorlevel 1 goto :fail
call :build android arm
if errorlevel 1 goto :fail

:done
echo.
echo Done. Output in %OUT%\
dir /b "%OUT%"
exit /b 0

:build
rem setlocal/endlocal keeps GOOS, GOARCH and EXT from leaking into later calls.
setlocal
set GOOS=%1
set GOARCH=%2
set EXT=
if "%1"=="windows" set EXT=.exe
echo   %1/%2 -^> %OUT%\%APP%-%1-%2%EXT%
go build -trimpath -ldflags "-s -w" -o "%OUT%\%APP%-%1-%2%EXT%" .
if errorlevel 1 (
	endlocal
	goto :fail
)
endlocal
goto :eof

:fail
rem Nothing may run between the failed build and this exit: any command that
rem succeeds would reset ERRORLEVEL to 0 and hide the failure from the caller.
echo.
echo BUILD FAILED.
exit /b 1
