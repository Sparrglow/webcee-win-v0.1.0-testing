@echo off
setlocal

:: WebCee Unified Build Script
:: 1. Checks for .wce files
:: 2. If found, runs parser (Python or C)
:: 3. Compiles C code

set "PROJECT_ROOT=%~dp0"
set "LIB_DIR=%PROJECT_ROOT%lib"
set "TOOLS_DIR=%PROJECT_ROOT%tools"
set "BUILD_DIR=%PROJECT_ROOT%build"

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

:: --- Step 1: Check for .wce files ---
set "HAS_WCE=0"
if exist "*.wce" set "HAS_WCE=1"
if exist "ui\*.wce" set "HAS_WCE=1"

if "%HAS_WCE%"=="1" (
    echo [WebCee] UI definition (.wce) found. Running parser...
    
    :: Try Python first
    python --version >nul 2>&1
    if %ERRORLEVEL% EQU 0 (
        echo [WebCee] Using Python parser...
        python "%TOOLS_DIR%\wce_parser.py" *.wce ui\*.wce
    ) else (
        echo [WebCee] Python not found. Using C parser...
        if not exist "%TOOLS_DIR%\wce_parser.exe" (
            echo [WebCee] Compiling C parser...
            gcc "%TOOLS_DIR%\wce_parser.c" -o "%TOOLS_DIR%\wce_parser.exe"
        )
        "%TOOLS_DIR%\wce_parser.exe" *.wce ui\*.wce
    )
)

:: --- Step 2: Compile Project ---
echo [WebCee] Compiling project...

set "SRC_FILES=main.c %LIB_DIR%\webcee.c"
if exist "webcee_generated.c" set "SRC_FILES=%SRC_FILES% webcee_generated.c"

gcc -g -std=c11 %SRC_FILES% -I"%LIB_DIR%" -I. -o "%BUILD_DIR%\app.exe" -lws2_32

if %ERRORLEVEL% EQU 0 (
    echo [WebCee] Build success! Running...
    "%BUILD_DIR%\app.exe"
) else (
    echo [WebCee] Build failed.
)
