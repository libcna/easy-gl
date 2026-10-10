# NEXT.md — easy-gl handoff document

## 1. Project summary

**easy-gl** is a C++20 RAII/OOP wrapper over OpenGL / OpenGL ES, built on top of **meta-gl** (a procedural type-safe OpenGL wrapper with typed handle structs, enum classes, and `std::span` helpers).

**Main goal:** Provide safe, convenient C++ resource classes that own and release OpenGL objects automatically, while delegating all low-level calls to meta-gl.

**Current phase:** Feature completion — all planned API tasks from PLAN.md have been implemented. Remaining work is configuration extensions (S1/S2), build hygiene (W1-W3), tests (U1/U2), and examples (V1-V3).

**Key architectural decisions:**
- All resource classes (`Buffer`, `Texture`, `Program`, etc.) are non-copyable, move-only RAII.
- Base class `detail::GenerationTracked` provides `handle_`, `generation_`, `is_created()`, `native_handle()`.
- meta-gl typed handles (`BufferId{handle_}`, `TextureId{handle_}`, etc.) are wrapped at every call site.
- `UniformLocation`, `AttribLocation` are typed structs — must be wrapped explicitly.
- `FramebufferAttachment::Color0` removed; use `metagl::to_framebuffer_attachment(metagl::ColorAttachment::Color0)`.

---

## 2. Current status

**Build:** Clean — all targets build without errors or warnings.

**Tests:** Four smoke test binaries exist:
- `easy-gl-smoke-tests` — init/device tests (no GL context required; native/host builds only, see below)
- `easy-gl-resource-smoke-tests` — resource object creation smoke tests (native/host builds only)
- `easy-gl-context-lifecycle-tests` — ResourceRegistry context-loss/restore cycle, plus GLES2/GLES3 tiered mock-loader coverage
- `easy-gl-webgl-tests` — WebGL 1-shaped mock-loader coverage: `ApiKind::WebGL` classification, `GL_OES_vertex_array_object` gating, and `UnsupportedFeatureException` (not a crash) from `Query`/`Sampler`/`TransformFeedback`/`Sync`/`ProgramPipeline`/`Texture::get_level_parameter*` when the underlying entry point is missing

All tests use fake function-pointer loaders and require no real GL context, so they run identically under a native host build (`cmake --preset default`) and cross-compiled with the `emscripten` preset (`cmake --preset emscripten && cmake --build --preset emscripten && ctest --preset emscripten`, executed under Node). `easy-gl-smoke-tests`/`easy-gl-resource-smoke-tests` mock a desktop-OpenGL-shaped context that a real WebGL host can never report (meta-gl always classifies `__EMSCRIPTEN__` builds as `ApiKind::WebGL`), so they are only built/registered for non-Emscripten configurations.

**Library:** `libeasy-gl.a` (static)

**Release:** `0.1.1` (2026-10-10), built against meta-gl `v0.4.1`; `0.1.0` (the same day) was
the first tagged release.
The version is decided in `CMakeLists.txt` and generated into `easygl/Version.hpp`; see
`CHANGELOG.md` and `docs/releasing.md`.

**Example:** `hello-triangle-sdl` — SDL2+OpenGL ES triangle demo (builds, requires display).

**What does NOT work yet:**
- No headless automated test execution against a *real* GL context (no EGL pbuffer setup in CI); all current tests use fake function-pointer loaders instead.
- No real-browser WebGL verification (`WEBGL_lose_context`, actual canvas) — the `emscripten` preset only proves the code cross-compiles and the mock-loader-based logic holds under Node, not that it runs correctly against a real browser GL implementation.
- `Config::log_callback` field does not exist yet (S1/S2 not done).
- No CMake install/package export (`find_package(easy-gl)` not supported — W3).
- `CXX_STANDARD` is set to 23 in CMake but CLAUDE.md targets C++20 (W1).

---

## 3. Recent changes

**New files added:**
- `include/easygl/ScopedBind.hpp` — RAII bind/unbind helper
- `include/easygl/ScopedDebugGroup.hpp` + `src/ScopedDebugGroup.cpp` — RAII GPU debug groups
- `include/easygl/UniformCache.hpp` + `src/UniformCache.cpp` — cached `glGetUniformLocation`
- `include/easygl/ResourceRegistration.hpp` + `src/ResourceRegistration.cpp` — RAII registry guard
- `CMakePresets.json` — `default`/`release` (host) and `emscripten` (WebAssembly, `Debug` build type so `assert()`-based test checks stay active under Node) configure/build/test presets
- `tests/smoke/WebGLTests.cpp` — WebGL-specific correctness coverage (see section 2)

**WebGL correctness fixes (previously tracked in the now-removed `webgl.md`):**
- `Device::initialize()` now reuses meta-gl's own `GetContextInfo()`/`GetCapabilities()` instead of duplicating `GL_VERSION`-string parsing, and surfaces `ApiKind::WebGL` plus `Capabilities::is_webgl()/is_webgl1()/is_webgl2()`.
- `Query`, `Sampler`, `TransformFeedback`, `Sync`, `ProgramPipeline::create()`, and `Texture::get_level_parameter()/get_level_parameterf()` now check `metagl::IsFunctionAvailable()` and throw `UnsupportedFeatureException` instead of crashing on a null function pointer on contexts missing the underlying entry point (e.g. WebGL 1).
- `ProgramPipeline.hpp` documents that separable shader programs have no WebGL equivalent at all (not even WebGL 2) — a permanent gap, not a version tier.
- `VertexArrayObject` support below GLES/WebGL 3.0 now depends on `GL_OES_vertex_array_object` being advertised (checked via the existing extension-based detection in `Capabilities::detect_common_features()`), rather than being silently assumed.
- Rejected as out of scope (per user decision when reviewing `webgl.md`): a real-browser/Emscripten-runtime smoke test beyond the mock-loader-based `easy-gl-webgl-tests` (see "What does NOT work yet" above).

**`Texture::bind()`/`active_bind()` semantics (behavioral fix, not a meta-gl enum change):**
- `Texture::bind(target)` activates and uses texture unit 0 (`TextureUnit::Texture0`); `Texture::active_bind(unit, target)` activates and uses the given unit. Both leave the corresponding unit active afterwards. This restores behavior that had been lost in an earlier refactoring (`bind()` previously did not call `glActiveTexture` at all).
- Documented directly on the two methods in `Texture.hpp`, and covered by a dedicated regression test (`test_texture_bind_and_active_bind_semantics` in `tests/smoke/SmokeResourceTests.cpp`) that first activates `Texture1`, then checks `bind()` switches back to `Texture0`, then checks `active_bind(Texture1, ...)` uses `Texture1`.
- The stale `texture.bind(unit)` example in `CLAUDE.md` was corrected to `bind(target)`/`active_bind(unit, target)`.

**Major additions to existing files:**
- `Device`: generic state getters (`get_boolean/float/integer/integer64`), indexed blend/color/enable, advanced draw calls, debug message API, ES 3.1+/3.2+ methods
- `Program`: `set_uniform_iv/uiv`, `set_program_uniform*` (separable), `get_uniform_fv/iv/uiv`, introspection, `load_binary`, `create_separable`, `set_parameter`
- `Texture`: copy ops, buffer textures, compressed sub-image, parameter getters, set_parameter array variants
- `Buffer`: `get_parameter`, `get_parameter64`, `get_pointer`
- `VertexArray`: separate vertex format (K1), constant attribs (L1), attribute getters (F4)
- `Framebuffer`: `attach_texture`, `blit_to`, `invalidate_sub`, `set/get_parameter`, `get_attachment_parameter`
- `Shader`: `load_binary`
- `Sampler`, `Query`, `Sync`, `Renderbuffer`: parameter getters added

**Types.hpp additions:** `ProgramParameter`, `ProgramBinaryFormat`, `ShaderBinaryFormat`, `BufferParameter`, `RenderbufferParameter`, `FramebufferAttachmentParameter`, `FramebufferDefaultParameter`, `TextureLevelParameter`, `SyncParameter`

---

## 4. Current blocker / main problem

**No blocker** — the build is clean, all planned API tasks are done.

The only remaining gap before the library is fully production-ready:
- `Config::log_callback` (S1) is missing, so `enable_debug_logging = true` has nowhere to send output.
- CMake C++ standard mismatch: `CXX_STANDARD 23` vs. documented target of C++20 (W1).

---

## 5. Known bugs and limitations

- **incomplete** — `Config::enable_debug_logging = true` silently does nothing (no callback hooked up). Fix: implement S1/S2.
- **incomplete** — `Query::result_u64()` uses 32-bit `glGetQueryObjectuiv` internally (meta-gl does not expose `glGetQueryObjectui64v`). Returns values ≤ UINT32_MAX only. Suitable for frame counts; not for high-resolution GPU timers.
- **incomplete** — No headless test runner. Smoke tests compile but require an EGL/display to actually execute GL calls.
- **incomplete** — `draw_elements_base_vertex` and similar ES 3.2+ functions compile but will fail at runtime on ES 3.0 contexts (expected; no version guard).
- **incomplete** — W3: no `find_package(easy-gl)` support; downstream users must use `add_subdirectory`.
- **needs verification** — `Program::create_separable` sets `GL_PROGRAM_SEPARABLE` before linking; this is correct for ES 3.1+ but untested at runtime.

---

## 6. Architecture notes

```
OpenGL / OpenGL ES
      ↓
meta-gl          — procedural, typed, no ownership
      ↓
easy-gl          — OOP, RAII, move-only ownership
```

**Resource ownership model:**
- Every GL object class holds `handle_` (unsigned int) from `GenerationTracked`.
- Destructor calls `destroy()` which calls the appropriate `glDelete*`.
- Move constructor/assignment zero out the source handle to prevent double-delete.
- `reset_handle_no_gl()` zeros without calling GL (used after context loss).

**meta-gl integration rules:**
- Always wrap `handle_` in the typed struct at call sites: `metagl::BufferId{handle_}`.
- `UniformLocation{location}` and `AttribLocation{index}` must be explicit.
- Never call raw `glXxx()` if meta-gl already wraps it.

**Context lifecycle:**
- `ResourceRegistry` implements `metagl::ContextListener`.
- On context loss: `OnContextLost()` calls `release_gl_handle_only()` on all registered resources.
- On context restore: `OnContextRestored()` calls `recreate_gl_resource()`.
- `ResourceRegistration` (T1) provides RAII auto-register/deregister.

**Stable boundaries (do not break):**
- Public header API — downstream projects depend on it.
- `GenerationTracked` base — all resource classes inherit it.
- meta-gl is the only low-level GL dependency; no raw GL includes in easy-gl headers.

---

## 7. Useful commands

```bash
# Configure (from repo root)
cmake -B build -DEASYGL_BUILD_TESTS=ON

# Build everything
cmake --build build

# Run smoke tests (no GL context needed for init tests)
./build/tests/easy-gl-smoke-tests

# Run hello-triangle example (requires display + SDL2 + OpenGL ES)
./build/examples/integration-placeholder/hello-triangle-sdl/hello-triangle-sdl

# Check build only (no run)
cmake --build build 2>&1 | grep -E "error:|warning:"
```

---

## 8. Next smallest tasks

1. **S1 — Add `log_callback` to `Config`**
   - Files: `include/easygl/Config.hpp`
   - Add `std::function<void(std::string_view)> log_callback;` field.
   - Verify: `cmake --build build` passes cleanly.

2. **S2 — Add `auto_error_check` to `Config` + wire it in `Device`**
   - Files: `include/easygl/Config.hpp`, `src/Device.cpp`
   - When `auto_error_check = true`, call `glGetError()` after operations in debug builds and invoke `log_callback`.
   - Verify: build passes; optionally add a test.

3. **W1 — Fix CMake C++ standard**
   - File: `CMakeLists.txt`
   - Change `CXX_STANDARD 23` to `CXX_STANDARD 20`; add `CXX_STANDARD_REQUIRED ON`.
   - Verify: `cmake --build build` passes.

4. **W2 — Verify `EASYGL_BUILD_TESTS` option works cleanly**
   - File: `CMakeLists.txt`, `tests/CMakeLists.txt`
   - Ensure `cmake -B build -DEASYGL_BUILD_TESTS=OFF` builds without the test targets.
   - Verify: `cmake --build build -DEASYGL_BUILD_TESTS=OFF` produces no test binaries.

5. **W3 — Add `cmake/EasyGlInstall.cmake` package export**
   - Files: `CMakeLists.txt`, new `cmake/EasyGlInstall.cmake`
   - Add `configure_package_config_file` + `write_basic_package_version_file`.
   - Verify: `cmake --install build --prefix /tmp/easy-gl-install` and check for `easy-glConfig.cmake`.

6. **U1 — Add one real headless test using EGL pbuffer**
   - Files: new `tests/headless/HeadlessContext.hpp`, `tests/headless/BufferTest.cpp`
   - Create EGL pbuffer context; test `Buffer::create_vertex_buffer` + `is_valid_gl_object()`.
   - Verify: `./build/tests/easy-gl-headless-tests` passes.

7. **V1 — Add textured-quad example**
   - Files: new `examples/hello-textured-quad/`
   - Demonstrate `Texture::create_2d_rgba8`, `Sampler::create_linear_clamp`, `ScopedBind`.
   - Verify: example builds and links.

8. **V3 — Add UBO example**
   - Files: new `examples/hello-ubo/`
   - Demonstrate `Buffer` with `UniformBuffer`, `Program::uniform_block_index`, `set_uniform_block_binding`.
   - Verify: example builds and links.

---

## 9. Do not do yet

- Do not redesign or refactor existing resource classes — the API is stable.
- Do not add C++23-only features (modules, `std::expected` everywhere) — target is C++20.
- Do not change the meta-gl dependency version without checking easy-gl compatibility first.
- Do not add new resource classes (e.g. `Pipeline`, `AtomicCounter`) without a clear use case.
- Do not merge `develop` into `master` until S1/S2 and W1 are done.
- Do not introduce a global error-handling philosophy — use the existing `Exception` class.
- Do not mass-rename existing public API methods — downstream breakage.

---

## 10. Resume prompt

```
Read NEXT.md first. Then inspect only the files needed for the first task listed in section 8.
Do not refactor unrelated code. Make one small, verified improvement. Run:
  cmake --build build 2>&1 | grep -E "error:|warning:"
to verify the build is clean after your change. Update NEXT.md section 3 (Recent changes)
and section 2 (Current status) to reflect what you did. Then stop and report.
```
