# Collect all source files in the Engine directory and subdirectories
file(GLOB_RECURSE ENGINE_SOURCES
    "*.cpp"
    "*.hpp"
)

# Force CMake to reconfigure when source files are added/removed
set_property(DIRECTORY PROPERTY CMAKE_CONFIGURE_DEPENDS "*.cpp" "*.hpp")

# Create the UmapyoiEngine library (static)
add_library(UmapyoiEngine STATIC ${ENGINE_SOURCES})

# fmod setup
set(FMOD_PATH "${CMAKE_SOURCE_DIR}/extern/FMOD")  # extern folder at same level as Engine

if(WIN32)
    set(FMOD_CORE_LIB "${FMOD_PATH}/lib/fmodL_vc.lib")
    set(FMOD_STUDIO_LIB "${FMOD_PATH}/lib/fmodstudioL_vc.lib")
elseif(APPLE)
    set(FMOD_CORE_LIB "${FMOD_PATH}/lib/libfmod.dylib")
    set(FMOD_STUDIO_LIB "${FMOD_PATH}/lib/libfmodstudio.dylib")
else()
    set(FMOD_CORE_LIB "${FMOD_PATH}/lib/libfmod.so")
    set(FMOD_STUDIO_LIB "${FMOD_PATH}/lib/libfmodstudio.so")
endif()

# Create imported targets
add_library(FMOD::Core STATIC IMPORTED)
set_target_properties(FMOD::Core PROPERTIES
    IMPORTED_LOCATION "${FMOD_CORE_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${FMOD_PATH}/api/core/inc"
)

add_library(FMOD::Studio STATIC IMPORTED)
set_target_properties(FMOD::Studio PROPERTIES
    IMPORTED_LOCATION "${FMOD_STUDIO_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${FMOD_PATH}/api/studio/inc"
)

# Link libraries (GLFW, OpenGL, FMOD)
target_link_libraries(UmapyoiEngine 
    PUBLIC 
        glfw
        OpenGL::GL
        FMOD::Core
        FMOD::Studio
)

# Include GLM and local headers
target_include_directories(UmapyoiEngine 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${glm_SOURCE_DIR}
)

# Set C++ standard for this target
target_compile_features(UmapyoiEngine PUBLIC cxx_std_20)
