@echo off
setlocal

set SDL3_DIR=C:/SDL3/SDL3-3.4.4/cmake

:: -- Configure ---------------------------------------------------------------
cmake -B build -S . -DSDL3_DIR="%SDL3_DIR%"
if errorlevel 1 (
    echo [ERROR] CMake configuration failed.
    pause
    exit /b 1
)

:: -- Build -------------------------------------------------------------------
cmake --build build
if errorlevel 1 (
    echo [ERROR] Build failed.
    pause
    exit /b 1
)

echo [OK] Build complete.
pause