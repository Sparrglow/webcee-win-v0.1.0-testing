@echo off
:: 获取脚本所在目录（%~dp0 自动包含结尾的 \）
set "SRC_DIR=%~dp0"
:: 获取目标路径的绝对路径（%~f1 自动解析相对路径）
set "TARGET_DIR=%~f1"

if "%~1"=="" (
    echo Usage: create_project.bat [Path]
    exit /b 1
)

echo [WebCee] Creating project at: "%TARGET_DIR%"

:: 1. 检查源文件是否存在
if not exist "%SRC_DIR%tools\wce.exe" (
    echo Error: Compiler not found at "%SRC_DIR%tools\wce.exe"
    echo Please run the build task first.
    exit /b 1
)

:: 2. 创建目录结构
if not exist "%TARGET_DIR%" mkdir "%TARGET_DIR%"
if not exist "%TARGET_DIR%\tools" mkdir "%TARGET_DIR%\tools"
if not exist "%TARGET_DIR%\lib" mkdir "%TARGET_DIR%\lib"

:: 3. 复制核心文件
copy /Y "%SRC_DIR%tools\wce.exe" "%TARGET_DIR%\tools\" >nul
copy /Y "%SRC_DIR%src\webcee.c" "%TARGET_DIR%\lib\" >nul
copy /Y "%SRC_DIR%include\*.h" "%TARGET_DIR%\lib\" >nul

:: 4. 生成示例代码 ui.wce
(
echo wce_container^(^) {
echo     wce_text^("Hello WebCee!"^);
echo }
) > "%TARGET_DIR%\ui.wce"

:: 5. 生成入口代码 main.c
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
echo     return 0;
echo }
) > "%TARGET_DIR%\main.c"

:: 6. 生成构建脚本 build.bat
(
echo @echo off
echo tools\wce.exe ui.wce ui_gen.c
echo gcc main.c ui_gen.c lib\webcee.c -I lib -o app.exe -lws2_32
echo if %%ERRORLEVEL%% EQU 0 echo Build success! Run app.exe
) > "%TARGET_DIR%\build.bat"

echo Project created successfully.
