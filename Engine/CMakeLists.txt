# Collect all source files in the Engine directory and subdirectories
file(GLOB_RECURSE ENGINE_SOURCES
    "*.cpp"
    "*.hpp"
)

# Create the UmapyoiEngine library
add_library(UmapyoiEngine STATIC ${ENGINE_SOURCES})

# Link GLFW to the engine
target_link_libraries(UmapyoiEngine PUBLIC glfw)

# Windows OpenGL linking
target_link_libraries(UmapyoiEngine PUBLIC opengl32)

# Make the Engine headers available to Game
target_include_directories(UmapyoiEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})