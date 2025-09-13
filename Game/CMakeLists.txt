# Collect all source files in the Game directory
file(GLOB GAME_SOURCES
    "*.cpp"
    "*.hpp"
)

# Create the executable
add_executable(UmapyoiGame ${GAME_SOURCES})

# Link against the Engine library
target_link_libraries(UmapyoiGame 
    PRIVATE 
        UmapyoiEngine
)

# Set C++ standard
target_compile_features(UmapyoiGame PUBLIC cxx_std_20)

# Set as startup project in Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT UmapyoiGame)