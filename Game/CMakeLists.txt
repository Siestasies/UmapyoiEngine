# Collect all source files in the Game directory and subdirectories
file(GLOB_RECURSE GAME_SOURCES
    "*.cpp"
    "*.hpp"
)

# Force CMake to reconfigure when source files are added/removed
set_property(DIRECTORY PROPERTY CMAKE_CONFIGURE_DEPENDS "*.cpp" "*.hpp")

# Create the Game executable
add_executable(UmapyoiGame ${GAME_SOURCES})

# Link against the UmapyoiEngine library
# Since UmapyoiEngine is linked as PUBLIC, this automatically includes GLFW, GLM, and OpenGL
target_link_libraries(UmapyoiGame 
    PRIVATE 
        UmapyoiEngine
)