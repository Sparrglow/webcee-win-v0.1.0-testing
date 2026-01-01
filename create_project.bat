@echo off
setlocal

:: 1. 获取路径信息
set "SRC_ROOT=%~dp0"
set "TARGET_DIR=%~f1"

if "%~1"=="" (
    echo Usage: create_project.bat [Path]
    exit /b 1
)

echo [WebCee] Target Project Path: "%TARGET_DIR%"

:: 2. 检查并自动构建编译器
set "COMPILER_EXE=%SRC_ROOT%tools\wce.exe"

if not exist "%COMPILER_EXE%" (
    echo [WebCee] Compiler binary not found. Building from source...
    if not exist "%SRC_ROOT%tools" mkdir "%SRC_ROOT%tools"
    gcc "%SRC_ROOT%compiler\core\*.c" "%SRC_ROOT%compiler\main.c" -o "%COMPILER_EXE%" -std=c11 -Wall -Wno-unused-function
    if errorlevel 1 (
        echo [Error] Failed to build compiler.
        exit /b 1
    )
    echo [WebCee] Compiler built successfully.
)

:: 3. 创建目录结构
if not exist "%TARGET_DIR%" mkdir "%TARGET_DIR%"
if not exist "%TARGET_DIR%\tools" mkdir "%TARGET_DIR%\tools"
if not exist "%TARGET_DIR%\lib" mkdir "%TARGET_DIR%\lib"

:: 4. 复制文件
echo [WebCee] Copying files...
copy /Y "%COMPILER_EXE%" "%TARGET_DIR%\tools\" >nul
copy /Y "%SRC_ROOT%src\webcee.c" "%TARGET_DIR%\lib\" >nul
copy /Y "%SRC_ROOT%include\*.h" "%TARGET_DIR%\lib\" >nul

:: 5. 生成 ui.wce
(
echo wce_container^(^) {
echo     wce_card^(^) {
echo         wce_css^("text-align: center; padding: 40px;"^);
echo         wce_row^(^) {
echo             wce_text^("Welcome to WebCee"^) {
echo                 wce_css^("display: block; font-size: 32px; font-weight: bold; color: #2c3e50; margin-bottom: 15px;"^);
echo             }
echo         }
echo         wce_row^(^) {
echo             wce_text^("The lightweight C Web Framework"^) {
echo                 wce_css^("display: block; font-size: 18px; color: #7f8c8d; margin-bottom: 30px;"^);
echo             }
echo         }
echo         wce_row^(^) {
echo             wce_button^("Get Started"^) {
echo                 wce_on_click^("on_start"^);
echo                 wce_css^("background-color: #3498db; color: white; padding: 12px 24px; font-size: 16px; border-radius: 6px; border: none; cursor: pointer; transition: background 0.3s;"^);
echo             }
echo         }
echo     }
echo }
) > "%TARGET_DIR%\ui.wce"

:: 6. 生成 main.c (使用宏定义端口，增加灵活性)
(
echo #include "lib/webcee.h"
echo #include ^<stdio.h^>
echo.
echo // Define the server port here
echo #define SERVER_PORT 8080
echo.
echo extern void wce_ui_main^(void^);
echo.
echo void on_start^(^) {
echo     printf^("Welcome button clicked!\n"^);
echo }
echo.
echo int main^(^) {
echo     // Initialize WebCee server
echo     if ^(wce_init^(SERVER_PORT^) != 0^) return 1;
echo.
echo     wce_register_function^("on_start", on_start^);
echo.
echo     wce_ui_main^(^);
echo.
echo     if ^(wce_start^(^) != 0^) return 1;
echo.
echo     printf^("Server running at http://localhost:%%d\n", SERVER_PORT^);
echo     printf^("Press Enter to stop server...\n"^);
echo     getchar^(^);
echo     wce_stop^(^);
echo     return 0;
echo }
) > "%TARGET_DIR%\main.c"

:: 7. 生成 build.bat (移除硬编码的端口提示，只负责构建)
(
echo @echo off
echo setlocal
echo.
echo echo [WebCee] Building Project...
echo.
echo echo [1/2] Compiling UI Layout ^(ui.wce^)...
echo tools\wce.exe ui.wce ui_gen.c
echo if %%ERRORLEVEL%% NEQ 0 ^(
echo     echo [Error] UI Compilation failed.
echo     exit /b 1
echo ^)
echo.
echo echo [2/2] Compiling Application ^(main.c^)...
echo gcc main.c ui_gen.c lib\webcee.c -I lib -o app.exe -lws2_32
echo if %%ERRORLEVEL%% NEQ 0 ^(
echo     echo [Error] C Compilation failed.
echo     exit /b 1
echo ^)
echo.
echo echo ===================================================
echo echo  Build Successful!
echo echo ===================================================
echo echo.
echo echo  To run the server, execute:
echo echo    .\app.exe
echo echo.
echo echo ===================================================
) > "%TARGET_DIR%\build.bat"

echo [WebCee] Project created successfully!
echo.
echo To start:
echo   cd /d "%TARGET_DIR%"
echo   build.bat
