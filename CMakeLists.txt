cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
project(UmapyoiEngine VERSION 1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------------
# Find OpenGL
# ------------------------
find_package(OpenGL REQUIRED)

# ------------------------
# Download GLFW
# ------------------------
include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
FetchContent_MakeAvailable(glfw)

# ------------------------
# Download GLM
# ------------------------
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG master
)
set(GLM_TEST_ENABLE OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glm)

# ------------------------
# FMOD paths and architecture detection
# ------------------------
set(FMOD_API_DIR "${CMAKE_SOURCE_DIR}/Extern/fmod/api/core")
set(FMOD_INCLUDE_DIR "${FMOD_API_DIR}/inc")
set(FMOD_LIB_DIR "${FMOD_API_DIR}/lib/x64")

# Detect architecture
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(FMOD_ARCH_DIR "x64")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(FMOD_ARCH_DIR "x86")
else()
    message(FATAL_ERROR "Unsupported pointer size: ${CMAKE_SIZEOF_VOID_P}")
endif()

set(FMOD_LIB_DIR "${FMOD_API_DIR}/lib/${FMOD_ARCH_DIR}")

# ------------------------
# Add FMOD wrapper
# ------------------------
# Must set FMOD_INCLUDE_DIR and FMOD_LIB_DIR *before* add_subdirectory
add_subdirectory(Extern/fmod)



# ------------------------
# Build Engine first, then Game
# ------------------------
add_subdirectory(Engine)
add_subdirectory(Game)
