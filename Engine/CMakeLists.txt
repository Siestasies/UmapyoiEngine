# Create GLAD library from local files
set(GLAD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/glad")
add_library(glad STATIC
    ${GLAD_DIR}/src/glad.c "Core/InputEvents.h" "Core/WindowEvents.h" "Core/ECSEvents.h" "Core/PhysicsEvents.h" "Core/AudioEvents.h" "Core/SceneEvents.h" "Core/ResourceEvents.h" "Core/PlayerEvents.h")
target_include_directories(glad PUBLIC ${GLAD_DIR}/include)

# Collect source files more systematically
file(GLOB_RECURSE ENGINE_SOURCES
    "*.cpp"
    "Systems/*.cpp" 
    "Core/*.cpp"
    "ECS/*.cpp"
    "ECS/Components/*.cpp"
    "ECS/Systems/*.cpp"
    "WIP_Scripts/*.cpp"
    "Debugging/*.cpp"
)

file(GLOB_RECURSE ENGINE_HEADERS
    "*.h"
    "*.hpp"
    "Systems/*.h"
    "Systems/*.hpp"
    "Core/*.h"
    "ECS/*.h"
    "ECS/Components/*.h"
    "ECS/Systems/*.h" 
    "WIP_Scripts/*.h"
    "WIP_Scripts/*.hpp"
    "Debugging/*.h"
    "Debugging/*.hpp"
)

# Force CMake to reconfigure when source files change
set_property(DIRECTORY PROPERTY CMAKE_CONFIGURE_DEPENDS 
    "*.cpp" "*.hpp" "*.h"
    "Systems/*.cpp" "Systems/*.hpp" "Systems/*.h"
    "Core/*.cpp" "Core/*.hpp" "Core/*.h"
    "ECS/*.cpp" "ECS/*.hpp" "ECS/*.h"
    "ECS/Components/*.cpp" "ECS/Components/*.hpp" "ECS/Components/*.h"
    "ECS/Systems/*.cpp" "ECS/Systems/*.hpp" "ECS/Systems/*.h"
    "WIP_Scripts/*.cpp" "WIP_Scripts/*.hpp" "WIP_Scripts/*.h"
    "Debugging/*.cpp" "Debugging/*.hpp" "Debugging/*.h"
)

# Create the Uma_Engine library
add_library(Uma_Engine STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS} "Core/InputEvents.h" "Core/WindowEvents.h" "Core/ECSEvents.h" "Core/PhysicsEvents.h" "Core/AudioEvents.h" "Core/SceneEvents.h" "Core/ResourceEvents.h" "Core/PlayerEvents.h")

set(FMOD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/fmod")

add_library(fmod STATIC IMPORTED)
set_target_properties(fmod PROPERTIES
    IMPORTED_LOCATION "${FMOD_DIR}/lib/x64/fmod_vc.lib"
)

# Link libraries
target_link_libraries(Uma_Engine 
    PUBLIC 
        glfw
        OpenGL::GL  # Cross-platform OpenGL linking
	      fmod
        glad
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
        ${glm_SOURCE_DIR}
        ${stb_SOURCE_DIR}
        ${glfw_SOURCE_DIR}/include        # <-- add this line
)

# Set C++ standard
target_compile_features(Uma_Engine PUBLIC cxx_std_20)

# Enable verbose output for debugging
set_target_properties(Uma_Engine PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)