@echo off
:: Create build directory if it doesn't exist
if not exist build mkdir build

:: Navigate and build
cd build && cmake --build . && cd ..

:: Define paths (relative to the script location)
set "SOURCE=build\helios.exe"
set "DEST_DIR=examples\helios"

:: Ensure the destination directory exists
if not exist "%DEST_DIR%" mkdir "%DEST_DIR%"

:: Move the file
if exist "%SOURCE%" (
    move /y "%SOURCE%" "%DEST_DIR%\"
) else (
    echo [ERROR] Executable file %SOURCE% not found. Did the build fail?
)

pause