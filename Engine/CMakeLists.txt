# Create GLAD library from local files
set(GLAD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/glad")
add_library(glad STATIC
    ${GLAD_DIR}/src/glad.c
)
target_include_directories(glad PUBLIC ${GLAD_DIR}/include)

# Collect all source files in the Engine directory (excluding GLAD subdirectory)
file(GLOB ENGINE_SOURCES
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
        glad
        OpenGL::GL
)

# Include directories
target_include_directories(UmapyoiEngine 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${glm_SOURCE_DIR}
        ${stb_SOURCE_DIR}
)

# Set C++ standard for this target
target_compile_features(UmapyoiEngine PUBLIC cxx_std_20)