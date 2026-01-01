@echo off
setlocal enabledelayedexpansion

set "ROOT=%~dp0.."
set "COMPILER_SRC=%ROOT%\compiler\core\*.c %ROOT%\compiler\main.c"
set "COMPILER_EXE=%ROOT%\tools\wce.exe"
set "SHOWCASE_DIR=%ROOT%\examples\showcase"

echo [1/3] Building WebCee Compiler...
if not exist "%ROOT%\tools" mkdir "%ROOT%\tools"
gcc -std=c11 -Wall -Wextra -Wno-unused-function %COMPILER_SRC% -o "%COMPILER_EXE%"
if %ERRORLEVEL% NEQ 0 (
    echo Compiler build failed!
    exit /b 1
)
echo Compiler built at %COMPILER_EXE%

echo [2/3] Compiling Showcase UI...
"%COMPILER_EXE%" "%SHOWCASE_DIR%\ui.wce" "%SHOWCASE_DIR%\ui_gen.c"
if %ERRORLEVEL% NEQ 0 (
    echo UI compilation failed!
    exit /b 1
)

echo [3/3] Building Showcase Application...
gcc -std=c11 "%SHOWCASE_DIR%\main.c" "%SHOWCASE_DIR%\ui_gen.c" "%ROOT%\src\webcee.c" -I "%ROOT%\include" -o "%SHOWCASE_DIR%\showcase.exe" -lws2_32
if %ERRORLEVEL% NEQ 0 (
    echo Showcase build failed!
    exit /b 1
)

echo.
echo Build Complete!
echo.
echo [Showcase]
echo   Run: %SHOWCASE_DIR%\showcase.exe
echo.
echo [New Project]
echo   Run: create_project.bat MyNewApp
echo.
