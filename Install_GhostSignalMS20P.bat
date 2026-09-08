@echo off
REM ============================================================================
REM  Ghost Signal MS20P - One-Click Installer (Windows)
REM  Double-click this file to install the VST3 plugin and Standalone app.
REM ============================================================================
setlocal enabledelayedexpansion
set "APP_NAME=GhostSignalMS20P"
set "VST3_FOLDER=GhostSignalMS20P.vst3"

echo.
echo  ============================================================
echo   Ghost Signal MS20P - Installer
echo  ============================================================
echo.

REM ---------------------------------------------------------------
REM 1. Locate the VST3 bundle (packaged layout or source build)
REM ---------------------------------------------------------------
set "SRC_ROOT=%~dp0"
set "VST3_SRC="

for %%C in (
    "%SRC_ROOT%VST3\%VST3_FOLDER%"
    "%SRC_ROOT%build\GhostSignalMS20P_artefacts\Release\VST3\%VST3_FOLDER%"
    "%SRC_ROOT%build\GhostSignalMS20P_artefacts\VST3\%VST3_FOLDER%"
) do (
    if not defined VST3_SRC if exist "%%~C" set "VST3_SRC=%%~C"
)

if not defined VST3_SRC (
    echo  [ERROR] Could not find %VST3_FOLDER%.
    echo          Build the project first:
    echo            cmake -S . -B build
    echo            cmake --build build --config Release
    echo.
    pause
    exit /b 1
)

REM ---------------------------------------------------------------
REM 2. Locate the Standalone app
REM ---------------------------------------------------------------
set "EXE_SRC="
for %%C in (
    "%SRC_ROOT%App\%APP_NAME%.exe"
    "%SRC_ROOT%build\GhostSignalMS20P_artefacts\Release\Standalone\%APP_NAME%.exe"
    "%SRC_ROOT%build\GhostSignalMS20P_artefacts\Standalone\%APP_NAME%.exe"
) do (
    if not defined EXE_SRC if exist "%%~C" set "EXE_SRC=%%~C"
)

REM ---------------------------------------------------------------
REM 3. Install the VST3 plugin (user-level, no admin required)
REM ---------------------------------------------------------------
set "VST3_DEST=%LOCALAPPDATA%\Programs\Common\VST3"
if not exist "%VST3_DEST%" mkdir "%VST3_DEST%"
if exist "%VST3_DEST%\%VST3_FOLDER%" rmdir /s /q "%VST3_DEST%\%VST3_FOLDER%"
xcopy /s /e /q /y "%VST3_SRC%" "%VST3_DEST%\%VST3_FOLDER%\" >nul
if errorlevel 1 (
    echo  [ERROR] Failed to install the VST3 plugin to:
    echo          %VST3_DEST%
    pause
    exit /b 1
)
echo  [OK] VST3 plugin installed to:
echo       %VST3_DEST%\%VST3_FOLDER%

REM If running with admin rights, also copy to the system VST3 folder.
net session >nul 2>&1
if "!errorlevel!"=="0" (
    set "SYS_VST3=C:\Program Files\Common Files\VST3"
    if not exist "!SYS_VST3!" mkdir "!SYS_VST3!"
    if exist "!SYS_VST3!\%VST3_FOLDER%" rmdir /s /q "!SYS_VST3!\%VST3_FOLDER%"
    xcopy /s /e /q /y "%VST3_SRC%" "!SYS_VST3!\%VST3_FOLDER%\" >nul
    echo  [OK] VST3 plugin also installed to the system folder:
    echo       !SYS_VST3!\%VST3_FOLDER%
)

REM ---------------------------------------------------------------
REM 4. Install the Standalone app
REM ---------------------------------------------------------------
if defined EXE_SRC (
    set "APP_DEST=%LOCALAPPDATA%\Programs\%APP_NAME%"
    if not exist "!APP_DEST!" mkdir "!APP_DEST!"
    copy /y "%EXE_SRC%" "!APP_DEST!\%APP_NAME%.exe" >nul
    if errorlevel 1 (
        echo  [WARN] Could not copy the Standalone app.
    ) else (
        echo  [OK] Standalone app installed to:
        echo       !APP_DEST!\%APP_NAME%.exe
    )
) else (
    echo  [SKIP] Standalone app not found; VST3 plugin installed only.
)

echo.
echo  ============================================================
echo   Install complete!
echo   - Restart your DAW / plug-in manager and rescan VST3.
echo   - Launch the Standalone app from:
echo       %APP_DEST%\%APP_NAME%.exe
echo  ============================================================
echo.
pause
exit /b 0