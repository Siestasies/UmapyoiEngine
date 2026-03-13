@echo off
echo Building Uma_Engine...

if exist build rmdir /s /q build
mkdir build
cd build

echo.
echo Select Visual Studio version:
echo 1. Visual Studio 2026
echo 2. Visual Studio 2022
echo.
choice /c 12 /n /m "Enter your choice (1-2): "

if %errorlevel%==1 (
    set "VS_GENERATOR=Visual Studio 18 2026"
) else (
    set "VS_GENERATOR=Visual Studio 17 2022"
)

echo Configuring with CMake using %VS_GENERATOR%...
cmake .. -G "%VS_GENERATOR%" -A x64
if %errorlevel% neq 0 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

echo.
echo Select target to build:
echo 1. Game (UmapyoiGame)
echo 2. Editor (UmapyoiEditor)
echo 3. Both
echo.
choice /c 123 /n /m "Enter your choice (1-3): "

set TARGET_CHOICE=%errorlevel%

echo.
echo Select build configuration:
echo 1. Debug
echo 2. Release
echo 3. Both
echo.
choice /c 123 /n /m "Enter your choice (1-3): "

set CONFIG_CHOICE=%errorlevel%

if %TARGET_CHOICE%==1 (
    if %CONFIG_CHOICE%==1 goto build_game_debug
    if %CONFIG_CHOICE%==2 goto build_game_release
    if %CONFIG_CHOICE%==3 goto build_game_both
)

if %TARGET_CHOICE%==2 (
    if %CONFIG_CHOICE%==1 goto build_editor_debug
    if %CONFIG_CHOICE%==2 goto build_editor_release
    if %CONFIG_CHOICE%==3 goto build_editor_both
)

if %TARGET_CHOICE%==3 (
    if %CONFIG_CHOICE%==1 goto build_all_debug
    if %CONFIG_CHOICE%==2 goto build_all_release
    if %CONFIG_CHOICE%==3 goto build_all_both
)

REM === GAME BUILDS ===
:build_game_debug
echo Building Game (Debug)...
cmake --build . --target UmapyoiGame --config Debug
if %errorlevel% neq 0 (
    echo Game Debug build failed!
    pause
    exit /b 1
)
echo Game Debug build complete! Running UmapyoiGame...
cd Game\Debug
UmapyoiGame.exe
cd ..\..
goto end

:build_game_release
echo Building Game (Release)...
cmake --build . --target UmapyoiGame --config Release
if %errorlevel% neq 0 (
    echo Game Release build failed!
    pause
    exit /b 1
)
echo Game Release build complete! Running UmapyoiGame...
cd Game\Release
UmapyoiGame.exe
cd ..\..
goto end

:build_game_both
echo Building Game (Debug)...
cmake --build . --target UmapyoiGame --config Debug
if %errorlevel% neq 0 (
    echo Game Debug build failed!
    pause
    exit /b 1
)
echo.
echo Building Game (Release)...
cmake --build . --target UmapyoiGame --config Release
if %errorlevel% neq 0 (
    echo Game Release build failed!
    pause
    exit /b 1
)
echo Both builds complete! Running Release build...
cd Game\Release
UmapyoiGame.exe
cd ..\..
goto end

REM === EDITOR BUILDS ===
:build_editor_debug
echo Building Editor (Debug)...
cmake --build . --target UmapyoiEditor --config Debug
if %errorlevel% neq 0 (
    echo Editor Debug build failed!
    pause
    exit /b 1
)
echo Editor Debug build complete! Running UmapyoiEditor...
cd EditorApp\Debug
UmapyoiEditor.exe
cd ..\..
goto end

:build_editor_release
echo Building Editor (Release)...
cmake --build . --target UmapyoiEditor --config Release
if %errorlevel% neq 0 (
    echo Editor Release build failed!
    pause
    exit /b 1
)
echo Editor Release build complete! Running UmapyoiEditor...
cd EditorApp\Release
UmapyoiEditor.exe
cd ..\..
goto end

:build_editor_both
echo Building Editor (Debug)...
cmake --build . --target UmapyoiEditor --config Debug
if %errorlevel% neq 0 (
    echo Editor Debug build failed!
    pause
    exit /b 1
)
echo.
echo Building Editor (Release)...
cmake --build . --target UmapyoiEditor --config Release
if %errorlevel% neq 0 (
    echo Editor Release build failed!
    pause
    exit /b 1
)
echo Both builds complete! Running Release build...
cd EditorApp\Release
UmapyoiEditor.exe
cd ..\..
goto end

REM === BUILD ALL (GAME + EDITOR) ===
:build_all_debug
echo Building All Targets (Debug)...
cmake --build . --config Debug
if %errorlevel% neq 0 (
    echo Debug build failed!
    pause
    exit /b 1
)
echo All Debug builds complete!
echo Which executable to run?
echo 1. Game
echo 2. Editor
choice /c 12 /n /m "Enter your choice (1-2): "
if %errorlevel%==1 (
    cd Game\Debug
    UmapyoiGame.exe
    cd ..\..
) else (
    cd EditorApp\Debug
    UmapyoiEditor.exe
    cd ..\..
)
goto end

:build_all_release
echo Building All Targets (Release)...
cmake --build . --config Release
if %errorlevel% neq 0 (
    echo Release build failed!
    pause
    exit /b 1
)
echo All Release builds complete!
echo Which executable to run?
echo 1. Game
echo 2. Editor
choice /c 12 /n /m "Enter your choice (1-2): "
if %errorlevel%==1 (
    cd Game\Release
    UmapyoiGame.exe
    cd ..\..
) else (
    cd EditorApp\Release
    UmapyoiEditor.exe
    cd ..\..
)
goto end

:build_all_both
echo Building All Targets (Debug)...
cmake --build . --config Debug
if %errorlevel% neq 0 (
    echo Debug build failed!
    pause
    exit /b 1
)
echo.
echo Building All Targets (Release)...
cmake --build . --config Release
if %errorlevel% neq 0 (
    echo Release build failed!
    pause
    exit /b 1
)
echo All builds complete!
echo Which executable to run?
echo 1. Game (Release)
echo 2. Editor (Release)
echo 3. Game (Debug)
echo 4. Editor (Debug)
choice /c 1234 /n /m "Enter your choice (1-4): "
if %errorlevel%==1 (
    cd Game\Release
    UmapyoiGame.exe
    cd ..\..
)
if %errorlevel%==2 (
    cd EditorApp\Release
    UmapyoiEditor.exe
    cd ..\..
)
if %errorlevel%==3 (
    cd Game\Debug
    UmapyoiGame.exe
    cd ..\..
)
if %errorlevel%==4 (
    cd EditorApp\Debug
    UmapyoiEditor.exe
    cd ..\..
)
goto end

:end
pause