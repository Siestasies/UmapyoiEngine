# Create GLAD library from local files
set(GLAD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/glad")
add_library(glad STATIC
    ${GLAD_DIR}/src/glad.c)
target_include_directories(glad PUBLIC ${GLAD_DIR}/include)

# Add ImGui (INSERT HERE - before ENGINE_SOURCES)
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/imgui")
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
set(IMGUI_HEADERS
    ${IMGUI_DIR}/imgui.h
    ${IMGUI_DIR}/imconfig.h
    ${IMGUI_DIR}/imgui_internal.h
    ${IMGUI_DIR}/imstb_rectpack.h
    ${IMGUI_DIR}/imstb_textedit.h
    ${IMGUI_DIR}/imstb_truetype.h
    ${IMGUI_DIR}/backends/imgui_impl_glfw.h
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.h
)
add_library(imgui STATIC ${IMGUI_SOURCES} ${IMGUI_HEADERS})
target_include_directories(imgui PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)
target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

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
    "MemoryManager/.cpp"
)

# Remove any ImGui files that might have been picked up by GLOB_RECURSE
list(FILTER ENGINE_SOURCES EXCLUDE REGEX "imgui/.*")

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
    "MemoryManager/.h"
    "MemoryManager/*.hpp"
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
    "MemoryManager/.cpp" "MemoryManager/.h" "MemoryManager/*.hpp"
)

# Create the Uma_Engine library
add_library(Uma_Engine STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS})

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
	imgui
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
        ${IMGUI_DIR}              # <-- Add this
        ${IMGUI_DIR}/backends     # <-- Add this
)

# Set C++ standard
target_compile_features(Uma_Engine PUBLIC cxx_std_20)

# Enable verbose output for debugging
set_target_properties(Uma_Engine PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)