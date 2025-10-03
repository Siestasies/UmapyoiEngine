@echo off
echo Cleaning Uma Engine build files...
echo.

REM Remove build directories
if exist build (
    echo Removing build/
    rmdir /s /q build
)

if exist Build (
    echo Removing Build/
    rmdir /s /q Build
)

if exist out (
    echo Removing out/
    rmdir /s /q out
)

if exist bin (
    echo Removing bin/
    rmdir /s /q bin
)

if exist x64 (
    echo Removing x64/
    rmdir /s /q x64
)

REM Remove Visual Studio files
if exist .vs (
    echo Removing .vs/
    rmdir /s /q .vs
)

echo Removing Visual Studio solution and project files...
del /q *.sln 2>nul
del /q *.vcxproj 2>nul
del /q *.vcxproj.filters 2>nul
del /q *.vcxproj.user 2>nul

REM Remove CMake cache files
echo Removing CMake cache files...
del /q CMakeCache.txt 2>nul
del /q cmake_install.cmake 2>nul
del /q Makefile 2>nul

if exist CMakeFiles (
    echo Removing CMakeFiles/
    rmdir /s /q CMakeFiles
)

REM Remove any stray build artifacts
echo Removing build artifacts...
del /q *.dir 2>nul
del /q *.vcxproj.* 2>nul

echo.
echo Clean complete!
pause