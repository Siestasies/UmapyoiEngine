# Collect all source files in the Engine directory and subdirectories
file(GLOB_RECURSE ENGINE_SOURCES
    "*.cpp"
    "*.hpp"
)

# Force CMake to reconfigure when source files are added/removed
set_property(DIRECTORY PROPERTY CMAKE_CONFIGURE_DEPENDS "*.cpp" "*.hpp")

# Create the UmapyoiEngine library
add_library(UmapyoiEngine STATIC ${ENGINE_SOURCES})

# Link libraries
target_link_libraries(UmapyoiEngine 
    PUBLIC 
        glfw
        OpenGL::GL  # Cross-platform OpenGL linking
)

# Include GLM headers
target_include_directories(UmapyoiEngine 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${glm_SOURCE_DIR}  # Add GLM include directory
)

# Set C++ standard for this target
target_compile_features(UmapyoiEngine PUBLIC cxx_std_20)