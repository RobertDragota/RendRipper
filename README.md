# RendRipper

RendRipper is a C++20 application for viewing, slicing and previewing 3D models.
It integrates a custom renderer with OpenGL, ImGui based user interface and
utility tools to generate and slice models.

## Features

- Load `.obj` or `.stl` files and manipulate them with translation, rotation and
  scaling gizmos.
- Generate a mesh from a single image through the bundled
  [TripoSR](3DModelGenerator/TripoSR) pipeline.
- Slice models using `CuraEngine` with the provided BambuLab printer profiles and
  visualize the resulting G-code layer by layer.
- Preview G-code lines in real time using OpenGL.

The UI manager exposes helpers for these tasks, including model generation,
slicing and interaction with the viewport.
G-code files are parsed into colored line segments so each layer can be drawn
individually.

## Building

This project uses CMake. Clone the repository with its submodules:

```bash
git clone --recurse-submodules <repo-url>
cd RendRipper
mkdir build && cd build
cmake ..
cmake --build .
```

The build expects external packages such as **assimp**, **nlohmann_json**,
**curl** and an OpenGL development environment. On Windows the paths to
`CuraEngine.exe` and the Python interpreter for TripoSR are defined in
`CMakeLists.txt` via compile definitions.

## Running

After building, launch the `RendRipper` executable. The main window hosts a
viewport for model rendering and menus for loading models, generating meshes from
images and starting the slicing process.

When slicing completes, the generated G-code can be previewed by selecting the
layer to display. The renderer runs until the window is closed by the user.

## Directory Overview

- `src/` – C++ source files for the renderer, UI and utilities.
- `resources/` – shaders, printer definitions and example G-code.
- `3DModelGenerator/TripoSR/` – Python code for image to 3D model conversion.
- `Slicer/` – folder intended for a `CuraEngine` build.
- `external/` – third-party dependencies pulled as git submodules.

## License

This repository currently does not ship a top-level license file. Consult
subdirectory READMEs for the respective licenses of included third-party code.
