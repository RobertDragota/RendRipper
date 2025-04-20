cmake_minimum_required(VERSION 3.30)
project(RendRipper)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC src/)
set(IM_GUI_INCLUDE ${CMAKE_SOURCE_DIR}/external/imgui)
set(IM_GUI_BACKEND_INCLUDE ${CMAKE_SOURCE_DIR}/external/imgui/backends)
set(GLFW_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/glfw/include)
set(GLFW_LIB_DIR ${CMAKE_SOURCE_DIR}/external/glfw/lib)
set(GLM_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/external/glm)
set(GLM_LIB_DIR ${CMAKE_SOURCE_DIR}/external/glm/lib)
set(STB_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/stb_lib)
set(IM_GUIZMO_INCLUDE ${CMAKE_SOURCE_DIR}/external/imguizmo)


find_package(assimp REQUIRED)
find_package(OpenGL REQUIRED)

# Include directories for external libraries
include_directories(${Vulkan_INCLUDE_DIRS})
include_directories(${IM_GUI_INCLUDE})
include_directories(${IM_GUI_BACKEND_INCLUDE})
include_directories(${GLFW_INCLUDE_DIR})
include_directories(${GLFW_LIB_DIR})
include_directories(${GLM_INCLUDE_DIRS})
include_directories(${STB_INCLUDE_DIR})
include_directories(${IM_GUIZMO_INCLUDE})

add_library(ImGuizmo
        ${IM_GUIZMO_INCLUDE}/ImGuizmo.cpp
        # ... any other ImGuizmo .cpp files if you want other widgets
)
target_include_directories(ImGuizmo
        PUBLIC ${IM_GUIZMO_INCLUDE}
)


file(
        GLOB_RECURSE SRC_FILES
        "${SRC}/*.cpp"
        "${SRC}/*.h"
)

file(
        GLOB_RECURSE IMGUI_FILES
        "${IM_GUI_INCLUDE}/imgui.cpp"
        "${IM_GUI_INCLUDE}/imgui_demo.cpp"
        "${IM_GUI_INCLUDE}/imgui_draw.cpp"
        "${IM_GUI_INCLUDE}/imgui_tables.cpp"
        "${IM_GUI_INCLUDE}/imgui_widgets.cpp"
        "${IM_GUI_BACKEND_INCLUDE}/imgui_impl_opengl3.cpp"
        "${IM_GUI_BACKEND_INCLUDE}/imgui_impl_glfw.cpp"
)
file(COPY resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
# Add GLFW as a subdirectory
add_subdirectory(external/glfw)
add_subdirectory(external/glm)
add_subdirectory(external/glad)


# Create the executable and include ImGui source files
add_executable(
        RendRipper
        "${SRC_FILES}"
        "${IMGUI_FILES}"

)

# Link libraries for Vulkan and GLFW
target_link_libraries(
        RendRipper PUBLIC
        glfw
        glad
        glm
        assimp::assimp
        OpenGL::GL
        ImGuizmo
)
