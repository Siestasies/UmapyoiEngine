cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
project(Uma_Engine VERSION 1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Remove default warning level and set W4 for MSVC
if(MSVC)
    # Remove any existing /W flags
    string(REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    string(REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    string(REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
    # Add W4
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")

    add_compile_options(/wd4996)  # Suppress 'deprecated' warnings (e.g., use of unsafe functions)
    add_compile_options(/wd4201)  # Suppress warning for unnamed struct/union
    add_compile_options(/wd26495)  # Suppress warning for unnamed struct/union

endif()

# Create Logs directory if it doesn't exist
file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/Logs")

# Find OpenGL
find_package(OpenGL REQUIRED)

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

# Download GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG master
)
set(GLM_TEST_ENABLE OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glm)

# We'll include GLAD manually in the Engine directory
# No FetchContent needed for GLAD

# Download stb (stb_image is header-only)
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
)
FetchContent_MakeAvailable(stb)

# ===== BUILD LUA 5.4.6 FROM SOURCE =====
message(STATUS "Building Lua 5.4.6 from source...")

FetchContent_Declare(
    lua
    URL https://www.lua.org/ftp/lua-5.4.6.tar.gz
    URL_HASH SHA256=7d5ea1b9cb6aa0b59ca3dde1c6adcb57ef83a1ba8e5432c0ecd06bf439b3ad88
)

FetchContent_MakeAvailable(lua)

# Collect Lua source files
file(GLOB LUA_SOURCES "${lua_SOURCE_DIR}/src/*.c")
list(REMOVE_ITEM LUA_SOURCES 
    "${lua_SOURCE_DIR}/src/lua.c"    # Remove interpreter
    "${lua_SOURCE_DIR}/src/luac.c"   # Remove compiler
)

# Create Lua static library
add_library(liblua STATIC ${LUA_SOURCES})
target_include_directories(liblua PUBLIC "${lua_SOURCE_DIR}/src")

if(WIN32)
    target_compile_definitions(liblua PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

message(STATUS "Lua library created: liblua")
message(STATUS "Lua include dir: ${lua_SOURCE_DIR}/src")

# ===== ADD SOL2 (HEADER-ONLY) =====
message(STATUS "Adding Sol2 header library...")

FetchContent_Declare(
    sol2
    GIT_REPOSITORY https://github.com/ThePhD/sol2.git
    GIT_TAG v3.3.1
)

FetchContent_MakeAvailable(sol2)

message(STATUS "Sol2 headers available at: ${sol2_SOURCE_DIR}/include")

# Build Engine first, then Game
add_subdirectory(Engine)
add_subdirectory(Game)