cmake_minimum_required(VERSION 3.15)
project(FMOD)

# ------------------------
# Core library
# ------------------------
add_library(FMOD::Core UNKNOWN IMPORTED)
set_target_properties(FMOD::Core PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${FMOD_INCLUDE_DIR}"
    IMPORTED_LOCATION_DEBUG "${FMOD_LIB_DIR}/fmodL_vc.lib"
    IMPORTED_LOCATION_RELEASE "${FMOD_LIB_DIR}/fmod_vc.lib"
)

# ------------------------
# Studio library
# ------------------------
add_library(FMOD::Studio UNKNOWN IMPORTED)
set_target_properties(FMOD::Studio PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${FMOD_INCLUDE_DIR}"
    IMPORTED_LOCATION_DEBUG "${FMOD_LIB_DIR}/fmodstudioL_vc.lib"
    IMPORTED_LOCATION_RELEASE "${FMOD_LIB_DIR}/fmodstudio_vc.lib"
)
