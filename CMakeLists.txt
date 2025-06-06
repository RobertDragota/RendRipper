cmake_minimum_required(VERSION 3.30)
project(RendRipper)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ────────────────────────────────────────────────────────────────────────────────
# 1) Enforce C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ────────────────────────────────────────────────────────────────────────────────
# 2) Paths to source and external dependencies
set(SRC                     ${CMAKE_SOURCE_DIR}/src)
set(IM_GUI_INCLUDE          ${CMAKE_SOURCE_DIR}/external/imgui)
set(IM_GUI_BACKEND_INCLUDE  ${CMAKE_SOURCE_DIR}/external/imgui/backends)
set(GLFW_INCLUDE_DIR        ${CMAKE_SOURCE_DIR}/external/glfw/include)
set(GLFW_LIB_DIR            ${CMAKE_SOURCE_DIR}/external/glfw/lib)
set(GLM_INCLUDE_DIRS        ${CMAKE_SOURCE_DIR}/external/glm)
set(GLM_LIB_DIR             ${CMAKE_SOURCE_DIR}/external/glm/lib)
set(STB_INCLUDE_DIR         ${CMAKE_SOURCE_DIR}/external/stb_lib)
set(IM_GUIZMO_INCLUDE       ${CMAKE_SOURCE_DIR}/external/imguizmo)

# 3) MeshLib paths (assume MeshLibDist was extracted into external/meshlib)
set(MESHLIB_ROOT            ${CMAKE_SOURCE_DIR}/external/meshlib)
set(MESHLIB_INCLUDE_DIR     ${MESHLIB_ROOT}/include)
set(MESHLIB_LIB_ROOT        ${MESHLIB_ROOT}/lib)
set(MESHLIB_DLL_ROOT        ${MESHLIB_ROOT}/app)


# ────────────────────────────────────────────────────────────────────────────────
# 5) Find other packages
find_package(CURL           REQUIRED)
find_package(nlohmann_json  CONFIG   REQUIRED)
find_package(assimp         REQUIRED)
find_package(OpenGL         REQUIRED)

# ────────────────────────────────────────────────────────────────────────────────
# 6) Compiler flags for MSVC (all configs) and a Debug‐only iterator‐ABI define
if(MSVC)
	# For all MSVC builds, add /bigobj and /utf-8
	add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/bigobj>")
	add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

	# In Debug only, force _ITERATOR_DEBUG_LEVEL=0 (to match how MeshLib was compiled)
	add_compile_definitions(
			"$<$<CONFIG:Debug>:_ITERATOR_DEBUG_LEVEL=0>"
	)
endif()

# ────────────────────────────────────────────────────────────────────────────────
# 7) Global include directories (so all subprojects can see ImGui, GLFW, GLM, stb, etc.)
include_directories(
		${IM_GUI_INCLUDE}
		${IM_GUI_BACKEND_INCLUDE}
		${GLFW_INCLUDE_DIR}
		${GLM_INCLUDE_DIRS}
		${STB_INCLUDE_DIR}
		${IM_GUIZMO_INCLUDE}
		${MESHLIB_INCLUDE_DIR}      # MeshLib headers
		${GLAD_INCLUDE}     # GLAD headers (desktop only)
)



# Because we generated GLAD with core‐profile 4.6, there is no ES “x” or BlendingBarrier—
# only desktop‐GL function pointers.  That eliminates all the “glad_glAlphaFuncx” errors.

# ────────────────────────────────────────────────────────────────────────────────
# 9) Pull in external libraries as submodules
add_subdirectory(external/glfw)
add_subdirectory(external/glm)
add_subdirectory(external/glad)
# (We no longer do add_subdirectory(external/glad) because we replaced it above.)
add_subdirectory(external/ImGuiFileDialog)


# ────────────────────────────────────────────────────────────────────────────────
# 10) Fix ImGuiFileDialog include path (so it can find “imgui.h”)
target_include_directories(ImGuiFileDialog PUBLIC
		${IM_GUI_INCLUDE}
)

# ────────────────────────────────────────────────────────────────────────────────
# 11) Create an INTERFACE target “MeshLibAll” that links every .lib under
#     ${MESHLIB_LIB_ROOT}/<CONFIG>/*.lib
add_library(MeshLibAll INTERFACE)
set_property(TARGET MeshLibAll PROPERTY
		INTERFACE_LINK_LIBRARIES
		"$<$<CONFIG:Debug>:${MESHLIB_LIB_ROOT}/Debug/*.lib>"
		"$<$<CONFIG:RelWithDebInfo>:${MESHLIB_LIB_ROOT}/RelWithDebInfo/*.lib>"
		"$<$<CONFIG:Release>:${MESHLIB_LIB_ROOT}/Release/*.lib>"
)

# ────────────────────────────────────────────────────────────────────────────────
# 12) Gather project sources
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

# Copy “resources” folder into the build directory
file(COPY "${CMAKE_SOURCE_DIR}/resources" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})


# ────────────────────────────────────────────────────────────────────────────────
# 13) ImGuizmo integration
add_library(ImGuizmo
		${IM_GUIZMO_INCLUDE}/ImGuizmo.cpp
)
# ImGuizmo must see “imgui.h” as well, so do:
target_include_directories(ImGuizmo PUBLIC
		${IM_GUIZMO_INCLUDE}
		${IM_GUI_INCLUDE}     # ← so ImGuizmo.cpp can do #include <imgui.h>
)

# ────────────────────────────────────────────────────────────────────────────────
# 14) Declare the executable
add_executable(RendRipper
		${SRC_FILES}
		${IMGUI_FILES}
		src/MeshRepairer.cpp
		src/MeshRepairer.h
		src/GCodeModel.cpp
		src/GCodeModel.h
)

# ────────────────────────────────────────────────────────────────────────────────
# 15) _CuraEngine (imported) example, if you still need it
add_library(_CuraEngine STATIC IMPORTED)
set_target_properties(_CuraEngine PROPERTIES
		IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/Slicer/CuraEngine/build/Release/_CuraEngine.lib"
)
target_include_directories(_CuraEngine INTERFACE
		"$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Slicer/CuraEngine/src>"
		"$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Slicer/CuraEngine/include>"
		"$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Slicer/CuraEngine/build/Release>"
)

# ────────────────────────────────────────────────────────────────────────────────
# 16) Optional meshlib availability define (if your code tests MESHLIB_AVAILABLE)
target_compile_definitions(RendRipper PRIVATE
                PYTHON_EXECUTABLE=\"${CMAKE_CURRENT_SOURCE_DIR}/3DModelGenerator/TripoSR/.venv/Scripts/python.exe\"
                GENERATE_MODEL_SCRIPT=\"${CMAKE_CURRENT_SOURCE_DIR}/3DModelGenerator/TripoSR/run.py\"
                OUTPUT_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/resources/generated_models\"
                CURA_ENGINE_EXE=\"${CMAKE_CURRENT_SOURCE_DIR}/Slicer/CuraEngine/build/Release/CuraEngine.exe\"
                FDMPRINTER_DEF=\"${CMAKE_CURRENT_SOURCE_DIR}/Slicer/CuraEngine/build/Release/fdmprinter.def.json\"
                BAMBULAB_BASE_DEF=\"${CMAKE_CURRENT_SOURCE_DIR}/Slicer/CuraEngine/build/Release/bambulab_base.def.json\"
                BAMBULAB_A1MINI_DEF=\"${CMAKE_CURRENT_SOURCE_DIR}/Slicer/CuraEngine/build/Release/bambulab_a1mini.def.json\"
                DRAGON_SETTINGS_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/Slicer/CuraEngine/build/Release/dragon_settings.json\"
                MODEL_SETTINGS_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/Slicer/CuraEngine/build/Release/model_settings.json\"
                MESHLIB_AVAILABLE
)

# ────────────────────────────────────────────────────────────────────────────────
# 17) Include directories for RendRipper
#     (We already added all the includes globally above, but if you want to be explicit:)
target_include_directories(RendRipper PRIVATE
		${IM_GUI_INCLUDE}
		${IM_GUI_BACKEND_INCLUDE}
		${GLFW_INCLUDE_DIR}
		${GLM_INCLUDE_DIRS}
		${STB_INCLUDE_DIR}
		${IM_GUIZMO_INCLUDE}
		${MESHLIB_INCLUDE_DIR}
		${GLAD_INCLUDE}
)

# ────────────────────────────────────────────────────────────────────────────────
# 18) Link libraries to RendRipper
target_link_libraries(RendRipper PUBLIC
		glfw
		glad       # ← our desktop‐only GLAD static library
		glm
		assimp::assimp
		OpenGL::GL
		ImGuizmo
		ImGuiFileDialog
		CURL::libcurl
		nlohmann_json::nlohmann_json
		_CuraEngine

		MeshLibAll         # ← pulls in all .lib files from the correct lib/<CFG> folder
)

# ────────────────────────────────────────────────────────────────────────────────
# 20) Optional alias so you can write “MeshLib::all” if you like
add_library(MeshLib::all ALIAS MeshLibAll)
