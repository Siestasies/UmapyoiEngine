@echo off
echo Building UmapyoiEngine...

if exist build rmdir /s /q build
mkdir build
cd build

echo Configuring with CMake...
cmake .. -G "Visual Studio 17 2022" -A x64

echo Building UmapyoiEngine...
cmake --build . --config Debug

echo Build complete! Running UmapyoiGame...
cd Game\Debug
UmapyoiGame.exe