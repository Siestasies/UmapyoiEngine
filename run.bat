@echo off
echo Building Uma_Engine...

if exist build rmdir /s /q build
mkdir build
cd build

echo Configuring with CMake...
cmake .. -G "Visual Studio 17 2022" -A x64
if %errorlevel% neq 0 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

echo.
echo Select build configuration:
echo 1. Debug
echo 2. Release
echo 3. Both
echo.
choice /c 123 /n /m "Enter your choice (1-3): "

if %errorlevel%==1 goto build_debug
if %errorlevel%==2 goto build_release
if %errorlevel%==3 goto build_both

:build_debug
echo Building Debug...
cmake --build . --config Debug
if %errorlevel% neq 0 (
    echo Debug build failed!
    pause
    exit /b 1
)
echo Debug build complete! Running UmapyoiGame...
cd Game\Debug
UmapyoiGame.exe
cd ..\..
goto end

:build_release
echo Building Release...
cmake --build . --config Release
if %errorlevel% neq 0 (
    echo Release build failed!
    pause
    exit /b 1
)
echo Release build complete! Running UmapyoiGame...
cd Game\Release
UmapyoiGame.exe
cd ..\..
goto end

:build_both
echo Building Debug...
cmake --build . --config Debug
if %errorlevel% neq 0 (
    echo Debug build failed!
    pause
    exit /b 1
)
echo.
echo Building Release...
cmake --build . --config Release
if %errorlevel% neq 0 (
    echo Release build failed!
    pause
    exit /b 1
)
echo Both builds complete! Running Release build...
cd Game\Release
UmapyoiGame.exe
cd ..\..
goto end

:end
pause