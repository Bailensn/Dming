@echo off
rem ---------------------------------------------------------------------------
rem  Build the Windows binary and package it with Inno Setup 6.
rem
rem  Requires: Go (see go.mod) and Inno Setup 6  ->  https://jrsoftware.org/isdl.php
rem  Output:   dist\dming-<version>-setup.exe
rem
rem  This file is ASCII-only on purpose: cmd.exe decodes batch files using the
rem  OEM code page, so UTF-8 text would show up as garbage.
rem ---------------------------------------------------------------------------

setlocal
cd /d "%~dp0.."

set "ISS=installer\dming.iss"
set "SETUP=dist\dming-3.4-setup.exe"

rem Fail early with a clear message if run straight from the zip (Windows
rem marks such files with the "blocked" stream and bat/exe refuse to run).
if not exist "installer\build-installer.bat" goto :notroot

echo [1/3] Checking prerequisites ...

go version >nul 2>&1
if errorlevel 1 goto :nogo

if not exist "%ISS%" goto :noiss

set "ISCC="
if exist "%ProgramFiles(x86)%\Inno Setup 6\ISCC.exe" set "ISCC=%ProgramFiles(x86)%\Inno Setup 6\ISCC.exe"
if not defined ISCC if exist "%ProgramFiles%\Inno Setup 6\ISCC.exe" set "ISCC=%ProgramFiles%\Inno Setup 6\ISCC.exe"
if not defined ISCC if exist "%LocalAppData%\Programs\Inno Setup 6\ISCC.exe" set "ISCC=%LocalAppData%\Programs\Inno Setup 6\ISCC.exe"
if not defined ISCC goto :noiscc

echo       Go:   OK
echo       ISCC: %ISCC%
echo.

echo [2/3] Building dist\dming-windows-amd64.exe ...
call build.bat windows amd64
if errorlevel 1 goto :buildfail

echo.
echo [3/3] Packaging ...
"%ISCC%" "%ISS%"
if errorlevel 1 goto :packfail

if not exist "%SETUP%" goto :nosetup

echo.
echo Done. Installer written to:
dir /b "dist\dming-*-setup.exe"
echo.
echo The installer needs NO admin rights and installs per-user into
echo   %%LocalAppData%%\Programs\dming
exit /b 0


:notroot
echo This script must be run from the project root, i.e.
echo   installer\build-installer.bat
exit /b 1

:nogo
echo Go was not found on PATH.
echo Install it from https://go.dev/dl/ and open a new terminal, then retry.
exit /b 1

:noiss
echo Cannot find %ISS%
echo Make sure the whole project folder was extracted, then retry.
exit /b 1

:noiscc
echo Inno Setup 6 (ISCC.exe) was not found.
echo Install it from https://jrsoftware.org/isdl.php and retry,
echo or open installer\dming.iss in the Inno Setup IDE and press F9.
exit /b 1

:buildfail
echo.
echo Build failed, packaging skipped.
exit /b 1

:packfail
echo.
echo Packaging failed. See the Inno Setup output above.
exit /b 1

:nosetup
echo.
echo ISCC reported success but %SETUP% does not exist.
exit /b 1
