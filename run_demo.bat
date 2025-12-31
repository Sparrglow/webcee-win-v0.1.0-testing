@echo off
echo === WebCee v0.1 Demo Launcher ===

if not exist build mkdir build
cd build

echo [1/3] Configuring CMake...
cmake ..
if %errorlevel% neq 0 exit /b %errorlevel%

echo [2/3] Building...
cmake --build . --config Release
if %errorlevel% neq 0 exit /b %errorlevel%

echo [3/3] Running Demo...
cd Release
simple_demo.exe
