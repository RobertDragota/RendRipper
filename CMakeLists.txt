cmake_minimum_required(VERSION 3.30)
project(RendRipper)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_TOOLCHAIN_FILE "D:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake"
#		CACHE STRING "")

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


# MeshLib paths (SDK structure)
set(MESHLIB_ROOT ${CMAKE_SOURCE_DIR}/external/meshlib)
set(MESHLIB_INCLUDE_DIR ${MESHLIB_ROOT}/include)
set(MESHLIB_LIB_DIR ${MESHLIB_ROOT}/lib/Release)
set(MESHLIB_APP_DIR ${MESHLIB_ROOT}/app)

find_package(CURL        REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

find_package(assimp REQUIRED)
find_package(OpenGL REQUIRED)

#find_package(CGAL REQUIRED COMPONENTS Core)
#find_package(Boost REQUIRED COMPONENTS system)

# Include directories
include_directories(
		${Vulkan_INCLUDE_DIRS}
		${IM_GUI_INCLUDE}
		${IM_GUI_BACKEND_INCLUDE}
		${GLFW_INCLUDE_DIR}
		${GLM_INCLUDE_DIRS}
		${STB_INCLUDE_DIR}
		${IM_GUIZMO_INCLUDE}
		${MESHLIB_INCLUDE_DIR}
)

# Link directories for GLFW, GLM, lib3mf and MeshLib libraries
link_directories(
		${GLFW_LIB_DIR}
		${GLM_LIB_DIR}
		${MESHLIB_LIB_DIR}
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



# Create MeshLib imported targets
# Note: Check the actual library names in your install/lib directory
# Common MeshLib library names:
add_library(MRMesh SHARED IMPORTED)
set_target_properties(MRMesh PROPERTIES
		IMPORTED_LOCATION "${MESHLIB_LIB_DIR}/MRMesh.dll"
		IMPORTED_IMPLIB "${MESHLIB_LIB_DIR}/MRMesh.lib"
		INTERFACE_INCLUDE_DIRECTORIES "${MESHLIB_INCLUDE_DIR}"
)

# Add other MeshLib libraries as needed
add_library(MRIOFormats SHARED IMPORTED)
set_target_properties(MRIOFormats PROPERTIES
		IMPORTED_LOCATION "${MESHLIB_LIB_DIR}/MRIOFormats.dll"
		IMPORTED_IMPLIB "${MESHLIB_LIB_DIR}/MRIOFormats.lib"
		INTERFACE_INCLUDE_DIRECTORIES "${MESHLIB_INCLUDE_DIR}"
)

add_library(MRCommonPlugins SHARED IMPORTED)
set_target_properties(MRCommonPlugins PROPERTIES
		IMPORTED_LOCATION "${MESHLIB_LIB_DIR}/MRCommonPlugins.dll"
		IMPORTED_IMPLIB "${MESHLIB_LIB_DIR}/MRCommonPlugins.lib"
		INTERFACE_INCLUDE_DIRECTORIES "${MESHLIB_INCLUDE_DIR}"
)

# Create executable
add_executable(RendRipper
		${SRC_FILES}
		${IMGUI_FILES}
)

target_sources(RendRipper PRIVATE src/main.cpp)

# ─── Import the already-built CuraEngine lib ───
# We import the static archive and expose all three include dirs:
add_library(_CuraEngine STATIC IMPORTED)
set_target_properties(_CuraEngine PROPERTIES
		IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/Slicer/CuraEngine/build/Release/_CuraEngine.lib"
)
target_include_directories(_CuraEngine INTERFACE
		"$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Slicer/CuraEngine/src>"          # core headers
		"$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Slicer/CuraEngine/include>"      # public interface
		"$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Slicer/CuraEngine/build/Release>"# generated Cura.pb.h
)

# ────────────────────────────────────────────────

target_compile_definitions(RendRipper PRIVATE
		PYTHON_EXECUTABLE=\"${CMAKE_CURRENT_SOURCE_DIR}/3DModelGenerator/TripoSR/.venv/Scripts/python.exe\"
		GENERATE_MODEL_SCRIPT=\"${CMAKE_CURRENT_SOURCE_DIR}/3DModelGenerator/TripoSR/run.py\"
		OUTPUT_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/resources/generated_models\"
		MESHLIB_AVAILABLE  # Enable MeshLib features
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
		CURL::libcurl
		nlohmann_json::nlohmann_json
		_CuraEngine
		MRMesh # Add MeshLib core
		MRIOFormats # Add MeshLib IO
		MRCommonPlugins # Add MeshLib plugins
		#CGAL::CGAL
)

# Copy lib3mf DLL to output directory
if (WIN32)

	# Copy MeshLib DLLs to output directory
	add_custom_command(TARGET RendRipper POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${MESHLIB_LIB_DIR}/MRMesh.dll"
			$<TARGET_FILE_DIR:RendRipper>
	)

	add_custom_command(TARGET RendRipper POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${MESHLIB_LIB_DIR}/MRIOFormats.dll"
			$<TARGET_FILE_DIR:RendRipper>
	)

	add_custom_command(TARGET RendRipper POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${MESHLIB_LIB_DIR}/MRCommonPlugins.dll"
			$<TARGET_FILE_DIR:RendRipper>
	)

	# Copy all MeshLib DLLs (safer approach)
	file(GLOB MESHLIB_DLLS "${MESHLIB_LIB_DIR}/*.dll")
	foreach(DLL_FILE ${MESHLIB_DLLS})
		add_custom_command(TARGET RendRipper POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy_if_different
				${DLL_FILE}
				$<TARGET_FILE_DIR:RendRipper>
		)
	endforeach()
endif()