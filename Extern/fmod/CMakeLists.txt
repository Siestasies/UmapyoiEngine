cmake_minimum_required(VERSION 3.15)
project(FMOD)

# Paths relative to this folder
set(FMOD_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/api/core/lib")
set(FMOD_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/api/core/inc")

# Select libraries for Windows
if(WIN32)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(FMOD_CORE_LIB "${FMOD_LIB_DIR}/fmodL_vc.lib")
        set(FMOD_STUDIO_LIB "${FMOD_LIB_DIR}/fmodstudioL_vc.lib")
    else()
        set(FMOD_CORE_LIB "${FMOD_LIB_DIR}/fmod_vc.lib")
        set(FMOD_STUDIO_LIB "${FMOD_LIB_DIR}/fmodstudio_vc.lib")
    endif()
endif()

# Core library
add_library(FMOD::Core UNKNOWN IMPORTED)
set_target_properties(FMOD::Core PROPERTIES
    IMPORTED_LOCATION "${FMOD_CORE_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${FMOD_INCLUDE_DIR}"
)

# Studio library
add_library(FMOD::Studio UNKNOWN IMPORTED)
set_target_properties(FMOD::Studio PROPERTIES
    IMPORTED_LOCATION "${FMOD_STUDIO_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${FMOD_INCLUDE_DIR}"
)
