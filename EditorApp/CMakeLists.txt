# Build ImGui as a static library (editor-only dependency)
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

# Collect EditorApp source files
file(GLOB EDITOR_APP_ROOT_SOURCES
     "*.cpp"
 )

 file(GLOB_RECURSE EDITOR_APP_SUBDIRECTORY_SOURCES
     "Editor/*.cpp"
     "Editor/Cmds/*.cpp"
     "Editor/Core/*.cpp"
     "Editor/Systems/*.cpp"
     "Scripts/*.cpp"
 )

 set(EDITOR_APP_SOURCES
     ${EDITOR_APP_ROOT_SOURCES}
     ${EDITOR_APP_SUBDIRECTORY_SOURCES}
 )

 file(GLOB EDITOR_APP_ROOT_HEADERS
     "*.h"
     "*.hpp"
 )

 file(GLOB_RECURSE EDITOR_APP_SUBDIRECTORY_HEADERS
     "Editor/*.h"
     "Editor/*.hpp"
     "Editor/Cmds/*.h"
     "Editor/Cmds/*.hpp"
     "Editor/Core/*.h"
     "Editor/Core/*.hpp"
     "Editor/Systems/*.h"
     "Editor/Systems/*.hpp"
     "Scripts/*.h"
     "Scripts/*.hpp"
 )

 set(EDITOR_APP_HEADERS
     ${EDITOR_APP_ROOT_HEADERS}
     ${EDITOR_APP_SUBDIRECTORY_HEADERS}
 )

# Create the UmapyoiEditor executable
add_executable(UmapyoiEditor ${EDITOR_APP_SOURCES} ${EDITOR_APP_HEADERS})

add_compile_definitions(PROJECT_ROOT="${CMAKE_SOURCE_DIR}")


target_compile_definitions(UmapyoiEditor PRIVATE UMAPYOI_EDITOR)

# Copy PlayFab DLLs next to the executable after build
foreach(DLL
    PlayFabServices.Win32.dll
    PlayFabCore.Win32.dll
    libHttpClient.Win32.dll)
    add_custom_command(TARGET UmapyoiEditor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PLAYFAB_BIN_DIR}/${DLL}"
            $<TARGET_FILE_DIR:UmapyoiEditor>
        COMMENT "Copying ${DLL} to editor output"
    )
endforeach()

# Set working directory for Visual Studio debugging
set_target_properties(UmapyoiEditor PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:UmapyoiEditor>"
)

# Link against the Engine library and ImGui
target_link_libraries(UmapyoiEditor
    PRIVATE
        Uma_Engine
        imgui
)

# Include directories for editor code
target_include_directories(UmapyoiEditor
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/Editor
        ${CMAKE_CURRENT_SOURCE_DIR}/Scripts
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
)

# Set C++ standard
target_compile_features(UmapyoiEditor PUBLIC cxx_std_20)

# Define where assets live
set(ASSET_DIR ${CMAKE_SOURCE_DIR}/Assets)

# Copy Assets folder after build (to the actual exe location)
add_custom_command(TARGET UmapyoiEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${ASSET_DIR}
        $<TARGET_FILE_DIR:UmapyoiEditor>/Assets
    COMMENT "Copying Assets folder next to executable"
)

# Define where logs live
set(LOGS_DIR ${CMAKE_SOURCE_DIR}/Logs)

# Copy Logs folder after build (to the actual exe location)
add_custom_command(TARGET UmapyoiEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${LOGS_DIR}
        $<TARGET_FILE_DIR:UmapyoiEditor>/Logs
    COMMENT "Copying Logs folder next to executable"
)

# Define where config live
set(CONFIG_DIR ${CMAKE_SOURCE_DIR}/Configs)

# Copy configs folder after build (to the actual exe location)
add_custom_command(TARGET UmapyoiEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CONFIG_DIR}
        $<TARGET_FILE_DIR:UmapyoiEditor>/Configs
    COMMENT "Copying Configs folder next to executable"
)

# FMOD runtime DLL (Windows only)
set(FMOD_DLL ${CMAKE_SOURCE_DIR}/Engine/fmod/lib/x64/fmod.dll)

# Copy FMOD runtime DLL after build (to the actual exe location)
add_custom_command(TARGET UmapyoiEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${FMOD_DLL}
        $<TARGET_FILE_DIR:UmapyoiEditor>
    COMMENT "Copying fmod.dll next to executable"
)

# FreeType runtime DLL (Windows only)
set(FREETYPE_DLL ${CMAKE_SOURCE_DIR}/Engine/freetype/win64/freetype.dll)

# Copy FreeType runtime DLL after build (to the actual exe location)
add_custom_command(TARGET UmapyoiEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${FREETYPE_DLL}
        $<TARGET_FILE_DIR:UmapyoiEditor>
    COMMENT "Copying freetype.dll next to executable"
)

if(MSVC)
    set_target_properties(UmapyoiEditor PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup")
endif()