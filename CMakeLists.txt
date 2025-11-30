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
    add_compile_options(/wd4005)
    add_compile_options(/wd26439)

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

# Build Engine first, then Game and EditorApp
add_subdirectory(Engine)
add_subdirectory(Game)
add_subdirectory(EditorApp)

# ===== INSTALLATION RULES =====

# Install game executable
install(TARGETS UmapyoiGame
    RUNTIME DESTINATION bin
    COMPONENT Application
)

# Install runtime folders
install(DIRECTORY ${CMAKE_SOURCE_DIR}/Assets 
    DESTINATION .
    COMPONENT Application
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/Configs 
    DESTINATION .
    COMPONENT Application
    PATTERN "imgui.ini" EXCLUDE
)

# Don't install Logs - should be created in AppData at runtime
# install(DIRECTORY ${CMAKE_SOURCE_DIR}/Logs DESTINATION .)

# Install runtime DLLs
install(FILES
    ${CMAKE_SOURCE_DIR}/Engine/fmod/lib/x64/fmod.dll
    ${CMAKE_SOURCE_DIR}/Engine/freetype/win64/freetype.dll
    DESTINATION bin
    COMPONENT Application
)

# ===== VISUAL C++ REDISTRIBUTABLE =====
if(EXISTS "${CMAKE_SOURCE_DIR}/Redist/VC_redist.x64.exe")
    install(FILES ${CMAKE_SOURCE_DIR}/Redist/VC_redist.x64.exe
        DESTINATION bin
        COMPONENT Application
    )
    
    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS
        "ExecWait '\\\"$INSTDIR\\\\bin\\\\VC_redist.x64.exe\\\" /install /quiet /norestart'"
    )
    
    message(STATUS "VC++ Redistributable will be included in installer")
else()
    message(WARNING "VC_redist.x64.exe not found in Redist/ folder")
endif()

# ===== CPACK CONFIGURATION =====
set(CPACK_GENERATOR "NSIS;ZIP")
set(CPACK_PACKAGE_NAME "SenkiGame")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_VENDOR "HarikitteIkou")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Senki by Harikitte Ikou")

# NSIS specific settings
set(CPACK_NSIS_DISPLAY_NAME "Senki Game")
set(CPACK_NSIS_PACKAGE_NAME "SenkiGame")
set(CPACK_NSIS_CONTACT "HarikitteIkou")
set(CPACK_NSIS_MODIFY_PATH ON)
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)

# Set install location to Program Files\DigiPen\Senki (per rubric)
set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "DigiPen\\\\Senki")

# Start Menu shortcut
set(CPACK_NSIS_CREATE_ICONS_EXTRA
    "CreateShortCut '$SMPROGRAMS\\\\SenkiGame\\\\Senki Game.lnk' '$INSTDIR\\\\bin\\\\UmapyoiGame.exe' '' '$INSTDIR\\\\Assets\\\\icon.ico'"
)
set(CPACK_NSIS_DELETE_ICONS_EXTRA
    "Delete '$SMPROGRAMS\\\\SenkiGame\\\\Senki Game.lnk'"
)

# Desktop shortcut - using MUI_FINISHPAGE
set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
    ${CPACK_NSIS_EXTRA_INSTALL_COMMANDS}
    CreateShortCut '$DESKTOP\\\\Senki.lnk' '$INSTDIR\\\\bin\\\\UmapyoiGame.exe' '' '$INSTDIR\\\\Assets\\\\icon.ico'
")

set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
    Delete '$DESKTOP\\\\Senki.lnk'
    Delete '$SMPROGRAMS\\\\SenkiGame\\\\Senki Game.lnk'
")

# Enable the "Create Desktop Shortcut" checkbox on finish page
set(CPACK_NSIS_MODIFY_PATH ON)
set(CPACK_NSIS_MUI_FINISHPAGE_RUN "UmapyoiGame.exe")

set(CPACK_COMPONENTS_ALL Application)

# Tie NSIS “Create Desktop Icon” checkbox to our exe
set(CPACK_PACKAGE_EXECUTABLES "UmapyoiGame" "Senki")

include(CPack)