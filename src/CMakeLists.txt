set(SOURCE_FILES 
    main.cpp 
    editor.cpp 
    texture_wic.cpp 
    asset_loader.cpp 
    camera.cpp 
    shaders.cpp 
    gui/gui.cpp
)

set(HEADER_FILES 
    actions.h
    asset_loader.h
    camera.h
    editor.h
    model.h
    pch.h
    texture_wic.h
    utility.h
    shaders.h
    gui/gui.h
)

set(LIB_FILES 
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
    ${imgui_filedialog_SOURCE_DIR}/ImGuiFileDialog.cpp
)

set(SHADER_FILES shaders/textured.hlsl)
set_source_files_properties(${SHADER_FILES} PROPERTIES LANGUAGE HLSL)

set(ALL_PROJECT_FILES ${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES} ${LIB_FILES})

source_group("Sources" FILES ${SOURCE_FILES})
source_group("Headers" FILES ${HEADER_FILES})
source_group("Shaders" FILES ${SHADER_FILES})
source_group("Libs" FILES ${LIB_FILES})

# You need to add all files on the executable for them to be visible on 
# the source_group in Visual Studio
# https://stackoverflow.com/a/31653555/14815076
add_executable(${PROJECT_NAME} WIN32 ${ALL_PROJECT_FILES})

# Compiler Flags & Definitions
target_compile_options(${PROJECT_NAME} PRIVATE /W4)
target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE)
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")

# Include Directories
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${assimp_SOURCE_DIR}/include
    ${imgui_SOURCE_DIR} 
    ${imgui_SOURCE_DIR}/backends
    ${imgui_filedialog_SOURCE_DIR}
)

# Linking
target_link_libraries(${PROJECT_NAME} PRIVATE 
    d3d11 
    dxgi 
    user32 
    assimp::assimp 
    ImGuiFileDialog
)

# Copy shaders to build folder
set(SHADER_DEST_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders")

if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(SHADER_DEST_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADER_DEST_DIR}"
)

foreach(SHADER_PATH ${SHADER_FILES})
    get_filename_component(SHADER_NAME ${SHADER_PATH} NAME)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_PATH}"
            "${SHADER_DEST_DIR}/${SHADER_NAME}"
        COMMENT "Copying ${SHADER_NAME} to build directory"
    )
endforeach()