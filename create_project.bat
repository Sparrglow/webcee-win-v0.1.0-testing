@echo off
setlocal
if "%~1"=="" (
    echo Usage: create_project.bat ^<ProjectName^>
    exit /b 1
)

set "PROJ_NAME=%~1"
if exist "%PROJ_NAME%" (
    echo Directory %PROJ_NAME% already exists.
    exit /b 1
)

mkdir "%PROJ_NAME%"
echo Creating project %PROJ_NAME%...

:: 1. Create ui.wce
(
echo wce_container^(^) {
echo     wce_card^(^) {
echo         wce_text^("Hello from %PROJ_NAME%!"^);
echo     }
echo }
) > "%PROJ_NAME%\ui.wce"

:: 2. Create main.c
(
echo #include "webcee.h"
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

:: 3. Create build.bat
(
echo @echo off
echo set "ROOT=..\"
echo set "WCE_EXE=%%ROOT%%tools\wce.exe"
echo.
echo if not exist "%%WCE_EXE%%" ^(
echo     echo Compiler not found. Please build the library first.
echo     exit /b 1
echo ^)
echo.
echo echo [1/2] Compiling UI...
echo "%%WCE_EXE%%" ui.wce ui_gen.c
echo.
echo echo [2/2] Compiling App...
echo gcc main.c ui_gen.c "%%ROOT%%src\webcee.c" -I "%%ROOT%%include" -o app.exe -lws2_32
echo.
echo if %%ERRORLEVEL%% EQU 0 ^(
echo     echo Build success! Run app.exe to start.
echo ^)
) > "%PROJ_NAME%\build.bat"

echo Project created in %PROJ_NAME%\
echo.
echo To build and run:
echo   cd %PROJ_NAME%
echo   build.bat
