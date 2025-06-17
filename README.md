# Walnut Ray Tracing Engine

A real-time ray tracing renderer and scene editor built using the Walnut framework and GLM. This project demonstrates physically-based rendering (PBR) with interactive material and scene manipulation via an ImGui interface.

---

## Features

- **Physically-Based Ray Tracing**: Supports Lambertian (diffuse) and emissive materials with a foundation for metallic/roughness workflow.
- **Interactive Scene Editing**: Add, move, and modify spheres and their material properties in real time using the ImGui UI.
- **Progressive Rendering**: Accumulation buffer refines image quality over time; reset and accumulation toggles included.
- **Multithreaded Rendering**: Leverages C++ parallel execution for fast image generation.
- **Camera Controls**: Perspective camera with live viewport resizing.

---

## Getting Started

### Requirements
- C++17 or later
- [Visual Studio 2022](https://visualstudio.com) or [CLion](https://www.jetbrains.com/clion/) (not strictly required, however included setup scripts only support visual studio)
- [Vulkan SDK](https://vulkan.lunarg.com/sdk/home#windows) (preferably a recent version)

### Building

1. Clone this repository and Walnut as a submodule or dependency.
2. Ensure GLM is available in your include path.
3. Use CMake to configure and build the project:
    ```
    mkdir build
    cd build
    cmake ..
    make
    ```
4. Run the resulting executable.

---

## Usage

- **Viewport**: Displays the rendered scene. Resize the window to adjust resolution.
- **Settings Panel**:
    - View last render time
    - Toggle accumulation (progressive rendering)
    - Manual render and reset buttons
- **Scene Panel**:
    - Edit sphere position, radius, and material assignment
    - Modify material albedo, roughness, metallic, emission color, and emission power in real time
- **File Menu**: Exit application

---

## Code Structure

| File              | Purpose                                            |
|-------------------|---------------------------------------------------|
| `WalnutApp.cpp`   | Main application, UI, and scene setup             |
| `Renderer.h/cpp`  | Ray tracing logic, accumulation, multithreading   |
| `Scene.h`         | Material, sphere, and scene data structures       |
| `Camera.h`        | Camera and ray direction generation               |
| `Utils`           | Random sampling, color conversion, math helpers   |

---

## Material System

- **Albedo**: Base color of the material
- **Roughness**: Surface microfacet roughness (0 = smooth, 1 = rough)
- **Metallic**: Controls blend between diffuse and specular reflection
- **Emission**: Color and power for self-illuminating surfaces

---

## Ray Tracing Pipeline

1. **Primary Ray Generation**: Camera rays per pixel
2. **Intersection**: Analytic sphere intersection
3. **Shading**:
    - Lambertian diffuse with (currently) uniform hemisphere sampling
    - Emissive surfaces contribute light
    - Color accumulation and progressive refinement
4. **Output**: RGBA image with tone mapping and clamping

---

## Limitations & Future Work

- Performance drops with many objects
- Lambertian BRDF uses uniform hemisphere sampling (not cosine-weighted)
- No texture mapping or advanced BRDFs
- No post-processing
- Only spheres supported as primitives

---

## Getting Started
Once you've cloned, run `scripts/Setup.bat` to generate Visual Studio 2022 solution/project files. Once you've opened the solution, you can run the WalnutApp project to see a basic example (code in `WalnutApp.cpp`). I recommend modifying that WalnutApp project to create your own application, as everything should be setup and ready to go.

### 3rd party libaries
- [Dear ImGui](https://github.com/ocornut/imgui)
- [GLFW](https://github.com/glfw/glfw)
- [stb_image](https://github.com/nothings/stb)
- [GLM](https://github.com/g-truc/glm) (included for convenience)

### Additional
- Walnut uses the [Roboto](https://fonts.google.com/specimen/Roboto) font ([Apache License, Version 2.0](https://www.apache.org/licenses/LICENSE-2.0))