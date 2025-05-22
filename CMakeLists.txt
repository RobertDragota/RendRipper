cmake_minimum_required(VERSION 3.30)
project(RendRipper)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Paths to source and external dependencies
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

# Include directories
include_directories(
        ${Vulkan_INCLUDE_DIRS}
        ${IM_GUI_INCLUDE}
        ${IM_GUI_BACKEND_INCLUDE}
        ${GLFW_INCLUDE_DIR}
        ${GLM_INCLUDE_DIRS}
        ${STB_INCLUDE_DIR}
        ${IM_GUIZMO_INCLUDE}
)

# Link directories for GLFW, GLM and OpenCV libraries
link_directories(
        ${GLFW_LIB_DIR}
        ${GLM_LIB_DIR}
)

# ImGuizmo integration
add_library(ImGuizmo
        ${IM_GUIZMO_INCLUDE}/ImGuizmo.cpp
)
target_include_directories(ImGuizmo PUBLIC ${IM_GUIZMO_INCLUDE})

# Gather project sources
file(GLOB_RECURSE SRC_FILES
        "${SRC}/*.cpp"
        "${SRC}/*.h"
)
file(GLOB_RECURSE IMGUI_FILES
        "${IM_GUI_INCLUDE}/imgui.cpp"
        "${IM_GUI_INCLUDE}/imgui_demo.cpp"
        "${IM_GUI_INCLUDE}/imgui_draw.cpp"
        "${IM_GUI_INCLUDE}/imgui_tables.cpp"
        "${IM_GUI_INCLUDE}/imgui_widgets.cpp"
        "${IM_GUI_BACKEND_INCLUDE}/imgui_impl_opengl3.cpp"
        "${IM_GUI_BACKEND_INCLUDE}/imgui_impl_glfw.cpp"
)
file(COPY resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# External libraries as submodules
add_subdirectory(external/glfw)
add_subdirectory(external/glm)
add_subdirectory(external/glad)
add_subdirectory(external/ImGuiFileDialog)

# Create executable
add_executable(RendRipper
        ${SRC_FILES}
        ${IMGUI_FILES}
)

# Link libraries to the executable
target_link_libraries(RendRipper PUBLIC
        glfw
        glad
        glm
        assimp::assimp
        OpenGL::GL
        ImGuizmo
        ImGuiFileDialog
)


if (WIN32)
    # wherever you installed OpenCV
    set(OpenCV_BIN_DIR "D:/opencv/build/x64/vc16/bin")
    add_custom_command(TARGET RendRipper POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${OpenCV_BIN_DIR}/opencv_world4110.dll"
            $<TARGET_FILE_DIR:RendRipper>
    )
endif()