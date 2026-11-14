# CLAUDE.md — easy-gl

## Project role

easy-gl is the higher-level OOP/RAII wrapper built on top of meta-gl.

It should provide convenient C++ resource classes and ownership/lifetime management while delegating low-level OpenGL calls to meta-gl whenever possible.

Layering:

```text
OpenGL / OpenGL ES
        ↓
meta-gl
  - procedural type-safe wrapper
  - enum class wrappers for GL domains
  - std::span for raw data views
  - lightweight typed handles
  - no RAII ownership
        ↓
easy-gl
  - OOP API
  - RAII resource classes
  - move-only ownership
  - convenience methods
  - higher-level rendering helpers
```

## Main goals

- Provide safe and convenient C++ classes over OpenGL resources.
- Own and release OpenGL resources automatically.
- Hide repetitive low-level binding/deletion details where reasonable.
- Use meta-gl as the low-level API instead of calling raw OpenGL directly when a meta-gl wrapper exists.
- Keep the public API simple, readable, and practical.

## C++ standard

Target C++20 as the portable baseline.

Allowed and encouraged:

- RAII
- move-only resource classes
- `enum class`
- `std::span`
- `std::string_view`
- simple templates
- simple concepts where they improve type safety
- `constexpr if` for typed dispatch when needed

Use carefully:

- `std::ranges`, only for simple utility code
- `consteval`, only for small compile-time validation helpers

Do not introduce:

- C++ modules
- C++26-only features
- heavy template metaprogramming
- complex framework architecture
- unnecessary inheritance hierarchies

## Ownership model

OpenGL resource classes in easy-gl should use RAII.

Typical classes:

```cpp
class Texture;
class Buffer;
class Shader;
class Program;
class VertexArray;
class Framebuffer;
class Renderbuffer;
```

Resource classes should usually be:

- non-copyable
- movable
- automatically destroyed in the destructor
- explicit about ownership

Example rule:

```cpp
Texture(const Texture&) = delete;
Texture& operator=(const Texture&) = delete;
Texture(Texture&&) noexcept = default;
Texture& operator=(Texture&&) noexcept = default;
```

Do not silently copy OpenGL ownership.

## Relationship to meta-gl

easy-gl depends on meta-gl.

Prefer calling meta-gl functions rather than raw OpenGL functions.

Good:

```cpp
meta::bindTexture(meta::TextureTarget::Texture2D, textureId_);
```

Avoid direct raw OpenGL calls if meta-gl already has the wrapper:

```cpp
glBindTexture(GL_TEXTURE_2D, id);
```

Direct raw OpenGL calls are acceptable only when meta-gl does not yet expose the needed operation. In that case, consider whether the missing operation belongs in meta-gl first.

## API style

Keep easy-gl easy to use.

Good:

```cpp
Texture texture = Texture::create2D(width, height, format);
texture.setData(pixels);
texture.bind(target);                    // binds on texture unit 0
texture.active_bind(unit, target);       // binds on a specific texture unit
```

Good:

```cpp
Buffer vertexBuffer = Buffer::createVertexBuffer(vertices);
Program program = Program::fromSources(vertexShaderSource, fragmentShaderSource);
```

Avoid exposing low-level OpenGL details unnecessarily in the high-level API.

## Error handling

Use practical error handling appropriate for a high-level wrapper.

Allowed:

- exceptions, if the project already uses them
- explicit result types, if the project already uses them
- assertions for programmer errors
- logs for OpenGL/debug errors

Do not introduce a new global error-handling philosophy without checking existing project style first.

Do not force `std::expected` everywhere unless the project has already standardized on it.

## Class design rules

### Keep RAII classes focused

A resource class should manage one OpenGL resource type.

Avoid giant classes that manage many unrelated resources.

### Prefer composition over inheritance

Avoid complex class hierarchies unless there is a clear existing project pattern.

### Make binding explicit where it matters

Do not hide too much global OpenGL state mutation.

Convenience is good, but state changes should remain understandable.

### Use typed meta-gl enums

Use meta-gl enum classes and handle types in easy-gl APIs where appropriate.

This keeps the high-level API aligned with the low-level type-safe wrapper.

## Refactoring rules for Claude

When modifying this project:

1. Read this file first.
2. Check meta-gl API before adding direct raw OpenGL calls.
3. Keep changes small and reviewable.
4. Prefer one resource type at a time.
5. Preserve existing naming/style unless there is a clear reason to change it.
6. Do not redesign the whole project in one patch.
7. Do not introduce modules or experimental C++26 features.
8. Build after changes when possible.
9. Show the diff before continuing with more refactoring.

## Preferred first steps

Good initial refactoring steps:

1. Update documentation to clarify that meta-gl is the low-level wrapper and easy-gl is the OOP/RAII layer.
2. Ensure easy-gl resource classes call meta-gl where wrappers already exist.
3. Convert one raw OpenGL resource class to move-only RAII if it is not already.
4. Replace raw enum parameters in easy-gl public APIs with meta-gl enum classes.
5. Add convenience methods only after the lower-level meta-gl wrapper is stable.

Avoid large all-at-once rewrites.
