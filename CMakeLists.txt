cmake_minimum_required(VERSION 3.20)
project(Shellshock LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
set(FETCHCONTENT_BASE_DIR ${PROJECT_SOURCE_DIR}/libs CACHE PATH "Library storage." FORCE)

# >>>>> Assimp
FetchContent_Declare(assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG master
)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(assimp)

# >>>>> ImGui
FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG docking
)
FetchContent_MakeAvailable(imgui)

# >>>>> ImGuiFileDialog
FetchContent_Declare(imgui_filedialog
    GIT_REPOSITORY https://github.com/aiekick/ImGuiFileDialog.git
    GIT_TAG master
)
FetchContent_MakeAvailable(imgui_filedialog)

target_include_directories(ImGuiFileDialog PUBLIC ${imgui_SOURCE_DIR})

# >>>>> ImGuizmo
# Note: ImGuizmo does not have its own CMakeLists.txt
FetchContent_Declare(imguizmo
    GIT_REPOSITORY https://github.com/CedricGuillemet/ImGuizmo
    GIT_TAG master
)
FetchContent_GetProperties(imguizmo)

if(NOT imguizmo_POPULATED)
    FetchContent_Populate(imguizmo)
    add_library(ImGuizmo STATIC 
        ${imguizmo_SOURCE_DIR}/ImGuizmo.cpp 
        ${imguizmo_SOURCE_DIR}/ImGuizmo.h
        ${imguizmo_SOURCE_DIR}/ImSequencer.cpp 
        ${imguizmo_SOURCE_DIR}/ImCurveEdit.cpp
    )

    target_include_directories(ImGuizmo PRIVATE ${imgui_SOURCE_DIR})
    target_include_directories(ImGuizmo PUBLIC ${imguizmo_SOURCE_DIR})
    target_link_libraries(ImGuizmo PUBLIC imgui)
endif()

# >>>>> DirectXTK
FetchContent_Declare(
    directxtk
    GIT_REPOSITORY https://github.com/microsoft/DirectXTK.git
    GIT_TAG        main 
)
FetchContent_MakeAvailable(directxtk)

# >>>>> Sudirectories
add_subdirectory(src)

target_link_libraries(Shellshock PRIVATE ImGuizmo)