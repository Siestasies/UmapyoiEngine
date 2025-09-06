# Collect all source files in the Game directory and subdirectories
file(GLOB_RECURSE GAME_SOURCES
    "*.cpp"
    "*.hpp"
)

# Create the Game executable
add_executable(UmapyoiGame ${GAME_SOURCES})

# Link against the UmapyoiEngine library
target_link_libraries(UmapyoiGame PRIVATE UmapyoiEngine)