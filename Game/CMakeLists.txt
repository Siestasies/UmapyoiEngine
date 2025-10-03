# Collect all source files in the Game directory
file(GLOB GAME_SOURCES
    "*.cpp"
    "*.hpp"
)

# Create the executable
add_executable(UmapyoiGame ${GAME_SOURCES})

add_compile_definitions(PROJECT_ROOT="${CMAKE_SOURCE_DIR}")

# Set working directory for Visual Studio debugging
set_target_properties(UmapyoiGame PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:UmapyoiGame>"
)

# Link against the Engine library
target_link_libraries(UmapyoiGame 
    PRIVATE 
        Uma_Engine
)

# Set C++ standard
target_compile_features(UmapyoiGame PUBLIC cxx_std_20)

# Set as startup project in Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT UmapyoiGame)

# Define where assets live
set(ASSET_DIR ${CMAKE_SOURCE_DIR}/Assets)

# Copy Assets folder after build (to the actual exe location)
add_custom_command(TARGET UmapyoiGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${ASSET_DIR}
        $<TARGET_FILE_DIR:UmapyoiGame>/Assets
    COMMENT "Copying Assets folder next to executable"
)

# Define where logs live
set(LOGS_DIR ${CMAKE_SOURCE_DIR}/Logs)

# Copy Logs folder after build (to the actual exe location)
add_custom_command(TARGET UmapyoiGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${LOGS_DIR}
        $<TARGET_FILE_DIR:UmapyoiGame>/Logs
    COMMENT "Copying Logs folder next to executable"
)

# FMOD runtime DLL (Windows only)
set(FMOD_DLL ${CMAKE_SOURCE_DIR}/Engine/fmod/lib/x64/fmod.dll)

# Copy FMOD runtime DLL after build (to the actual exe location)
add_custom_command(TARGET UmapyoiGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${FMOD_DLL}
        $<TARGET_FILE_DIR:UmapyoiGame>
    COMMENT "Copying fmod.dll next to executable"
)