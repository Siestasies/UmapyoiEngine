# Create GLAD library from local files
set(GLAD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/glad")
add_library(glad STATIC
    ${GLAD_DIR}/src/glad.c)
target_include_directories(glad PUBLIC ${GLAD_DIR}/include)

# Collect source files more systematically (NO EDITOR CODE)
file(GLOB_RECURSE ENGINE_SOURCES
    "*.cpp"
    "Systems/*.cpp"
    "Core/*.cpp"
    "Events/*.cpp"
    "ECS/*.cpp"
    "ECS/Components/*.cpp"
    "ECS/Systems/*.cpp"
    "WIP_Scripts/*.cpp"
    "Debugging/*.cpp"
    "MemoryManager/.cpp"
)

file(GLOB_RECURSE ENGINE_HEADERS
    "*.h"
    "*.hpp"
    "Systems/*.h"
    "Systems/*.hpp"
    "Core/*.h"
    "Events/*.h"
    "ECS/*.h"
    "ECS/Components/*.h"
    "ECS/Systems/*.h"
    "WIP_Scripts/*.h"
    "WIP_Scripts/*.hpp"
    "Debugging/*.h"
    "Debugging/*.hpp"
    "MemoryManager/.h"
    "MemoryManager/*.hpp"
)

# Force CMake to reconfigure when source files change
set_property(DIRECTORY PROPERTY CMAKE_CONFIGURE_DEPENDS
    "*.cpp" "*.hpp" "*.h"
    "Systems/*.cpp" "Systems/*.hpp" "Systems/*.h"
    "Core/*.cpp" "Core/*.hpp" "Core/*.h"
    "Events/*.cpp" "Events/*.hpp" "Events/*.h"
    "ECS/*.cpp" "ECS/*.hpp" "ECS/*.h"
    "ECS/Components/*.cpp" "ECS/Components/*.hpp" "ECS/Components/*.h"
    "ECS/Systems/*.cpp" "ECS/Systems/*.hpp" "ECS/Systems/*.h"
    "WIP_Scripts/*.cpp" "WIP_Scripts/*.hpp" "WIP_Scripts/*.h"
    "Debugging/*.cpp" "Debugging/*.hpp" "Debugging/*.h"
    "MemoryManager/*.cpp" "MemoryManager/*.h" "MemoryManager/*.hpp"
)

# Create the Uma_Engine library
add_library(Uma_Engine STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS})

# Add /bigobj flag for MSVC to handle large object files (Sol2 + ECS templates)
if(MSVC)
    target_compile_options(Uma_Engine PRIVATE /bigobj)
endif()

# FMOD configuration
set(FMOD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/fmod")
add_library(fmod STATIC IMPORTED)
set_target_properties(fmod PROPERTIES
    IMPORTED_LOCATION "${FMOD_DIR}/lib/x64/fmod_vc.lib"
)

# FreeType configuration
set(FREETYPE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/freetype")
add_library(freetype STATIC IMPORTED)
set_target_properties(freetype PROPERTIES
    IMPORTED_LOCATION "${FREETYPE_DIR}/win64/freetype.lib"
)

# Link libraries - liblua MUST come first
target_link_libraries(Uma_Engine
    PUBLIC
        liblua          # Lua library (FIRST!)
        glfw
        OpenGL::GL      # Cross-platform OpenGL linking
        fmod
        glad
        freetype
)

# Include directories
target_include_directories(Uma_Engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${glm_SOURCE_DIR}  # Add GLM include directory
        ${CMAKE_CURRENT_LIST_DIR}/../libs/fmod/inc # FMOD headers
        ${CMAKE_CURRENT_SOURCE_DIR}/Core
        ${CMAKE_CURRENT_SOURCE_DIR}/ECS
        ${CMAKE_CURRENT_SOURCE_DIR}/ECS/Components
        ${CMAKE_CURRENT_SOURCE_DIR}/ECS/Systems
        ${CMAKE_CURRENT_SOURCE_DIR}/Systems
        ${CMAKE_CURRENT_SOURCE_DIR}/WIP_Scripts
        ${CMAKE_CURRENT_SOURCE_DIR}/RapidJSON
        ${stb_SOURCE_DIR}
        ${glfw_SOURCE_DIR}/include
        ${lua_SOURCE_DIR}/src          # Lua headers
        ${sol2_SOURCE_DIR}/include     # Sol2 headers
        ${FREETYPE_DIR}/include        # FreeType headers
)

# Set C++ standard
target_compile_features(Uma_Engine PUBLIC cxx_std_20)

set_target_properties(Uma_Engine PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)