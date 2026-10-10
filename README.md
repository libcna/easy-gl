# easy-gl

`easy-gl` is a toolkit-independent C++20 wrapper over OpenGL and OpenGLES.

It provides a small, modern API for common rendering tasks while keeping platform integration in the host application.

## Why easy-gl

- No windowing toolkit lock-in (`SDL`, `GLFW`, `Qt`, etc. stay in host code).
- No context creation in the library (host owns window + GL context lifecycle).
- Public API under `include/easygl` without exposing toolkit types.
- OpenGL/OpenGLES differences are handled through capability checks.
- Current focus is a small working vertical slice (Hello Triangle first).

## Project status

This repository currently provides the core needed for initialization, capability detection, and basic draw flow used by:

- `hello-triangle-sdl` example
- `easy-gl-smoke-tests`
- `easy-gl-resource-smoke-tests`

The API is intentionally compact and evolving.

## Version

Current release: **0.1.1**, built and tested against meta-gl
[`v0.4.1`](https://github.com/libcna/meta-gl/releases/tag/v0.4.1) (pre-1.0 — a minor release may
change the public API; see [`CHANGELOG.md`](CHANGELOG.md) for what the release contains and
[`docs/releasing.md`](docs/releasing.md) for how versions are managed). meta-gl is a sibling
checkout (`../meta-gl`), so check out that tag next to easy-gl to reproduce the release.
Compiled code reads the easy-gl version it was built against from `easygl::version_string` and
the `EASYGL_VERSION_*` macros in `<easygl/Version.hpp>`, a header the build generates from
`CMakeLists.txt`.

## Architecture boundaries

`easy-gl` owns OpenGL/OpenGLES interaction.

The host application owns:

- window creation
- GL context creation and activation
- event processing
- swap/present
- providing a `GetProcAddress` callback

`easy-gl` does **not** create windows or contexts.

## Requirements

- C++20 compiler
- CMake `3.23+`
- A working OpenGL/OpenGLES runtime on target machine
- Host-side context/toolkit integration (example uses `SDL3`)

## Build

```bash
cmake -S . -B build \
  -DEASYGL_BUILD_TESTS=ON \
  -DEASYGL_BUILD_EXAMPLES=ON

cmake --build build --target easy-gl hello-triangle-sdl easy-gl-smoke-tests easy-gl-resource-smoke-tests
```

### CMake options

- `EASYGL_BUILD_SHARED` (default `OFF`) — build shared instead of static library.
- `EASYGL_BUILD_TESTS` (default `ON`) — build test executables.
- `EASYGL_BUILD_EXAMPLES` (default `ON`) — build integration placeholder examples.
- `EASYGL_ENABLE_WARNINGS` (default `ON`) — enable warning profile.
- `EASYGL_USE_SYSTEM_OPENGL` (default `ON`) — link system OpenGL where applicable.

## Run tests

```bash
ctest --test-dir build --output-on-failure
```

## Quick start integration

After your host app creates and activates a GL context, initialize `easy-gl` with a loader callback:

```cpp
#include <easygl/easygl.hpp>

easygl::Device device;
device.initialize(my_get_proc_address); // easygl::GLGetProcAddressFn
```

Toolkit-specific examples are available in:

- `examples/integration-placeholder/README.md`

### Minimal render flow (concept)

```cpp
easygl::Device device;
device.initialize(my_get_proc_address);

easygl::Shader vs(easygl::ShaderStage::Vertex);
vs.compile_from_source(vertex_source);

easygl::Shader fs(easygl::ShaderStage::Fragment);
fs.compile_from_source(fragment_source);

easygl::Program program;
program.attach(vs);
program.attach(fs);
program.link();

easygl::VertexArray vao;
vao.create();

easygl::Buffer vbo;
vbo.create();
vbo.bind(easygl::BufferTarget::Array);
vbo.set_data(vertices, vertices_size);

vao.bind();
vao.set_attribute_pointer(0, 3, easygl::DataType::Float, false, stride, offset);
vao.enable_attribute(0);

device.set_clear_color(0.2f, 0.3f, 0.3f, 1.0f);
device.clear(easygl::ClearFlags::Color);

program.use();
vao.bind();
device.draw_arrays(easygl::PrimitiveType::Triangles, 0, 3);
```

For a complete running sample, see:

- `examples/integration-placeholder/hello-triangle-sdl/HelloTriangle.cpp`

## Public API overview

Main entry points in `include/easygl`:

- `easygl::Device` — initialization, state, draw commands.
- `easygl::Capabilities` / `easygl::ContextInfo` — runtime capability/context inspection.
- `easygl::Shader`, `easygl::Program` — shader compile/link/use.
- `easygl::Buffer`, `easygl::VertexArray`, `easygl::Texture` — GPU resources.
- `easygl::Feature` — explicit feature queries and gating.

## OpenGL vs OpenGLES notes

- Some features are OpenGL-only (for example immediate mode / polygon mode style flags).
- Query support at runtime via `device.supports(...)` or `device.require(...)`.
- Treat unsupported features as unavailable on OpenGLES targets.

## Include

Use the umbrella header:

```cpp
#include <easygl/easygl.hpp>
```
