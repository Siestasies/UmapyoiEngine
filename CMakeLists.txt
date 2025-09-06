cmake_minimum_required(VERSION 3.4...3.28 FATAL_ERROR)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
project(UmapyoiEngine VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Download GLFW
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

# Build Engine first, then Game
add_subdirectory(Engine)
add_subdirectory(Game)