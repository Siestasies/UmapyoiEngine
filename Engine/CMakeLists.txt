# Engine/CMakeLists.txt
cmake_minimum_required(VERSION 3.15)

# ------------------------
# Collect all engine source files
# ------------------------
file(GLOB_RECURSE ENGINE_SOURCES
    "*.cpp"
    "*.hpp"
)

# Force reconfigure when source files change
set_property(DIRECTORY PROPERTY CMAKE_CONFIGURE_DEPENDS "*.cpp" "*.hpp")

# ------------------------
# Create Engine library
# ------------------------
add_library(UmapyoiEngine STATIC ${ENGINE_SOURCES})

# ------------------------
# Include directories
# ------------------------
target_include_directories(UmapyoiEngine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}   # Engine headers
        ${glm_SOURCE_DIR}             # GLM headers
)

# ------------------------
# Link libraries
# ------------------------
target_link_libraries(UmapyoiEngine
    PUBLIC
        glfw
        OpenGL::GL
        FMOD::Core
        FMOD::Studio
)

# ------------------------
# Set C++ standard
# ------------------------
target_compile_features(UmapyoiEngine PUBLIC cxx_std_20)
