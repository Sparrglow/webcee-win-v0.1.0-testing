@echo off
setlocal

:: 1. 获取路径信息
:: %~dp0 是脚本所在目录，以 \ 结尾
set "SRC_ROOT=%~dp0"
:: %~f1 将第一个参数转换为绝对路径
set "TARGET_DIR=%~f1"

if "%~1"=="" (
    echo Usage: create_project.bat [Path]
    exit /b 1
)

echo [WebCee] Target Project Path: "%TARGET_DIR%"

:: 2. 检查并自动构建编译器 (如果缺失)
set "COMPILER_EXE=%SRC_ROOT%tools\wce.exe"

if not exist "%COMPILER_EXE%" (
    echo [WebCee] Compiler binary not found. Building from source...
    
    :: 确保 tools 目录存在
    if not exist "%SRC_ROOT%tools" mkdir "%SRC_ROOT%tools"
    
    :: 调用 GCC 编译
    gcc "%SRC_ROOT%compiler\core\*.c" "%SRC_ROOT%compiler\main.c" -o "%COMPILER_EXE%" -std=c11 -Wall -Wno-unused-function
    
    if errorlevel 1 (
        echo [Error] Failed to build compiler. Please check if GCC is installed and source files exist.
        exit /b 1
    )
    echo [WebCee] Compiler built successfully.
)

:: 3. 创建目标项目目录结构
if not exist "%TARGET_DIR%" mkdir "%TARGET_DIR%"
if not exist "%TARGET_DIR%\tools" mkdir "%TARGET_DIR%\tools"
if not exist "%TARGET_DIR%\lib" mkdir "%TARGET_DIR%\lib"

:: 4. 复制必要文件
echo [WebCee] Copying files...
copy /Y "%COMPILER_EXE%" "%TARGET_DIR%\tools\" >nul
copy /Y "%SRC_ROOT%src\webcee.c" "%TARGET_DIR%\lib\" >nul
copy /Y "%SRC_ROOT%include\*.h" "%TARGET_DIR%\lib\" >nul

:: 5. 生成 ui.wce 模板
(
echo wce_container^(^) {
echo     wce_text^("Hello WebCee!"^);
echo     wce_button^("Click Me"^);
echo }
) > "%TARGET_DIR%\ui.wce"

:: 6. 生成 main.c 模板
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
) > "%TARGET_DIR%\main.c"

:: 7. 生成 build.bat 脚本
(
echo @echo off
echo echo [1/2] Compiling UI...
echo tools\wce.exe ui.wce ui_gen.c
echo if %%ERRORLEVEL%% NEQ 0 exit /b 1
echo.
echo echo [2/2] Compiling App...
echo gcc main.c ui_gen.c lib\webcee.c -I lib -o app.exe -lws2_32
echo if %%ERRORLEVEL%% NEQ 0 exit /b 1
echo.
echo echo Build success! Run app.exe to start.
) > "%TARGET_DIR%\build.bat"

echo [WebCee] Project created successfully!
echo.
echo To start:
echo   cd /d "%TARGET_DIR%"
echo   build.bat
