# Collect all source files in the Engine directory and subdirectories
file(GLOB_RECURSE ENGINE_SOURCES
    "*.cpp"
    "*.hpp"
)

# Force CMake to reconfigure when source files are added/removed
set_property(DIRECTORY PROPERTY CMAKE_CONFIGURE_DEPENDS "*.cpp" "*.hpp")

# Create the Uma_Engine library
add_library(Uma_Engine STATIC ${ENGINE_SOURCES} "Core/SystemType.h"  "Core/SystemManager.h" "Systems/InputSystem.cpp" "Systems/InputSystem.h" "WIP_Scripts/Test_Ecs_System.h" "WIP_Scripts/Test_Ecs_System.cpp")

# Link libraries
target_link_libraries(Uma_Engine 
    PUBLIC 
        glfw
        OpenGL::GL  # Cross-platform OpenGL linking
)

# Include GLM headers
target_include_directories(Uma_Engine 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${glm_SOURCE_DIR}  # Add GLM include directory
)

# Set C++ standard for this target
target_compile_features(Uma_Engine PUBLIC cxx_std_20)