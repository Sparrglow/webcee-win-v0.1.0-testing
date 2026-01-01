@echo off
setlocal
set "SRC_ROOT=%~dp0"
set "PROJ_NAME=%~1"

if "%PROJ_NAME%"=="" (
    echo Usage: create_project.bat <ProjectName>
    exit /b 1
)

if exist "%PROJ_NAME%" (
    echo Error: Directory %PROJ_NAME% already exists.
    exit /b 1
)

:: 1. Check and Build Compiler if missing
if not exist "%SRC_ROOT%tools\wce.exe" (
    echo [WebCee] Compiler tool not found. Building it now...
    if not exist "%SRC_ROOT%tools" mkdir "%SRC_ROOT%tools"
    gcc "%SRC_ROOT%compiler\core\*.c" "%SRC_ROOT%compiler\main.c" -o "%SRC_ROOT%tools\wce.exe" -std=c11 -Wall -Wno-unused-function
    if errorlevel 1 (
        echo [WebCee] Failed to build compiler. Cannot create project.
        exit /b 1
    )
)

echo [WebCee] Creating project %PROJ_NAME%...
mkdir "%PROJ_NAME%"
mkdir "%PROJ_NAME%\tools"
mkdir "%PROJ_NAME%\lib"

:: 2. Copy Compiler
copy "%SRC_ROOT%tools\wce.exe" "%PROJ_NAME%\tools\" >nul

:: 3. Copy Library Files
copy "%SRC_ROOT%src\webcee.c" "%PROJ_NAME%\lib\" >nul
copy "%SRC_ROOT%include\*.h" "%PROJ_NAME%\lib\" >nul

:: 4. Create ui.wce
(
echo wce_container^(^) {
echo     wce_card^(^) {
echo         wce_text^("Hello from %PROJ_NAME%!"^);
echo         wce_row^(^) {
echo             wce_button^("Click Me"^);
echo         }
echo     }
echo }
) > "%PROJ_NAME%\ui.wce"

:: 5. Create main.c
(
echo #include "lib/webcee.h"
echo #include ^<stdio.h^>
echo.
echo extern void wce_ui_main^(void^);
echo.
echo int main^(^) {
echo     if ^(wce_init^(8080^) != 0^) return 1;
echo     wce_ui_main^(^);
echo     if ^(wce_start^(^) != 0^) return 1;
echo     printf^("Server running at http://localhost:8080\n"^);
echo     while^(1^) { wce_sleep^(1000^); }
echo     return 0;
echo }
) > "%PROJ_NAME%\main.c"

:: 6. Create build.bat
(
echo @echo off
echo.
echo echo [1/2] Compiling UI...
echo tools\wce.exe ui.wce ui_gen.c
echo if %%ERRORLEVEL%% NEQ 0 exit /b 1
echo.
echo echo [2/2] Compiling App...
echo gcc main.c ui_gen.c lib\webcee.c -I lib -o app.exe -lws2_32
echo if %%ERRORLEVEL%% NEQ 0 exit /b 1
echo.
echo echo Build success! Run app.exe to start.
) > "%PROJ_NAME%\build.bat"

echo.
echo Project created successfully!
echo.
echo Structure:
echo   %PROJ_NAME%\
echo   +-- tools\      (Compiler)
echo   +-- lib\        (Runtime Library)
echo   +-- ui.wce      (UI Definition)
echo   +-- main.c      (Entry Point)
echo   +-- build.bat   (Build Script)
echo.
echo To start:
echo   cd %PROJ_NAME%
echo   build.bat
