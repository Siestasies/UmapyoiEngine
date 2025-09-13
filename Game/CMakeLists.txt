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

# Copy Assets folder to the build directory where the executable is
# This works for both Debug and Release builds
add_custom_command(TARGET UmapyoiGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/Assets"
        "$<TARGET_FILE_DIR:UmapyoiGame>/Assets"
    COMMENT "Copying Assets folder to build directory"
)