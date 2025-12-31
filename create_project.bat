@echo off
setlocal

if "%~1"=="" (
    echo Usage: create_project.bat ^<project_path^>
    exit /b 1
)

set "TARGET_DIR=%~1"
set "SOURCE_ROOT=%~dp0"

echo [WebCee] Creating project at: %TARGET_DIR%

if not exist "%TARGET_DIR%" mkdir "%TARGET_DIR%"
if not exist "%TARGET_DIR%\lib" mkdir "%TARGET_DIR%\lib"
if not exist "%TARGET_DIR%\tools" mkdir "%TARGET_DIR%\tools"
if not exist "%TARGET_DIR%\.vscode" mkdir "%TARGET_DIR%\.vscode"

echo [WebCee] Copying library files...
copy "%SOURCE_ROOT%src\webcee.c" "%TARGET_DIR%\lib\webcee.c" >nul
copy "%SOURCE_ROOT%include\webcee.h" "%TARGET_DIR%\lib\webcee.h" >nul
copy "%SOURCE_ROOT%include\webcee_build.h" "%TARGET_DIR%\lib\webcee_build.h" >nul

echo [WebCee] Copying tools...
copy "%SOURCE_ROOT%tools\wce_parser.py" "%TARGET_DIR%\tools\wce_parser.py" >nul
copy "%SOURCE_ROOT%tools\wce_parser.c" "%TARGET_DIR%\tools\wce_parser.c" >nul
copy "%SOURCE_ROOT%tools\build_project.bat" "%TARGET_DIR%\build.bat" >nul

echo [WebCee] Generating main.c...
(
echo #include ^<stdio.h^>
echo #include "lib/webcee.h"
echo.
echo int main^(void^) {
echo     if ^(wce_init^(8080^) != 0^) return 1;
echo.
echo     // New Syntax Example
echo     wce_data_set^("title", "WebCee App"^);
echo.
echo     wce_start^(^);
echo     printf^("Running at http://localhost:8080\n"^);
echo     getchar^(^);
echo     wce_stop^(^);
echo     return 0;
echo }
) > "%TARGET_DIR%\main.c"

echo [WebCee] Configuring VS Code...
(
echo {
echo     "version": "2.0.0",
echo     "tasks": [
echo         {
echo             "label": "Run WebCee App",
echo             "type": "shell",
echo             "command": ".\build.bat",
echo             "group": {
echo                 "kind": "build",
echo                 "isDefault": true
echo             },
echo             "problemMatcher": []
echo         }
echo     ]
echo }
) > "%TARGET_DIR%\.vscode\tasks.json"

echo [Success] Project created!
