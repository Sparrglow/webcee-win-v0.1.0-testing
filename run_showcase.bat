@echo off
if not exist "examples\showcase\showcase.exe" (
    echo Showcase executable not found. Building it now...
    call tools\build_project.bat
)

echo.
echo Starting WebCee Showcase...
echo.
examples\showcase\showcase.exe
