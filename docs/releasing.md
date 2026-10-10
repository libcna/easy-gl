# Releasing easy-gl

*Current as of 0.1.1 (2026-10-10).*

easy-gl follows [Semantic Versioning 2.0.0](https://semver.org/spec/v2.0.0.html). A release is
an annotated git tag plus a `CHANGELOG.md` entry — there is no separate release branch, no
release workflow, and nothing is published to a package registry.

## Where the version lives

The version is decided in exactly **one** place:

```cmake
# CMakeLists.txt (repository root)
project(easy-gl VERSION 0.1.1 ...)
set(EASYGL_VERSION_PRERELEASE "")   # e.g. "beta.1"; empty on a final release
```

`project(VERSION …)` accepts numeric components only, so the pre-release identifier sits beside
it and the two are joined into `EASYGL_VERSION_STRING` (`0.1.1`, or `0.2.0-beta.1`).
`EASYGL_VERSION_PRERELEASE` is deliberately a normal variable and not a cache entry: a cached
copy would keep an existing build directory reporting the previous release after a bump.

Everything else derives from that:

| Consumer | How it gets the version |
|---|---|
| C++ code | `#include <easygl/Version.hpp>` (also pulled in by `easygl.hpp`) → `easygl::version_string`, `easygl::version_major`, `EASYGL_VERSION_MAJOR`, … |
| The library | `VERSION` / `SOVERSION` target properties |
| The configure banner | `-- easy-gl: version <x.y.z>` |

`easygl/Version.hpp` is **generated** by `cmake/EasyGlVersion.cmake` from
`cmake/templates/Version.hpp.in` into `<build>/include/easygl/Version.hpp` and published through
the `generated_headers` file set, so `add_subdirectory` consumers such as CNA see it through the
`easy-gl::easy-gl` target. Never edit the generated file, never add a hand-written copy under
`include/easygl/` (it would shadow the generated one), and never hard-code the version anywhere
else. `easy-gl-version-tests` checks the header against the version CMake configured without
naming a concrete release, so a bump never edits it.

Two copies are maintained by hand and must be updated as part of a bump:

- `CHANGELOG.md` — the release entry and its link definitions at the bottom.
- `README.md` — the *Version* section.

## Numbers that are *not* the easy-gl version

- **meta-gl's version** (`<metagl/Version.hpp>`, `metagl::GetVersionString()`) belongs to the
  low-level layer underneath and moves independently.
- **The OpenGL / OpenGL ES version** of a live context (`easygl::ContextInfo`) is a property of
  the driver, reported at runtime.
- **`"version": 6`** in `CMakePresets.json` is the presets schema version.

## Pre-1.0 policy

While the major version is 0, a minor bump may change the public API. Pre-release identifiers
are `alpha.N` → `beta.N` → `rc.N`, ordered as SemVer orders them.

## The meta-gl dependency

easy-gl consumes meta-gl as a **sibling checkout** with `add_subdirectory(../meta-gl)`, so a
build takes whatever revision that checkout happens to be on and the easy-gl tag alone does not
select it. Every release entry therefore names the meta-gl tag (or, failing one, the commit) it
was built and tested against. Release meta-gl first when easy-gl needs something new from it,
so the entry can name a tag rather than a raw hash.

CNA consumes easy-gl the same way, as a sibling checkout of `../easy-gl`.

## Cutting a release

1. **Choose the version.** Edit `project(easy-gl VERSION …)` and/or `EASYGL_VERSION_PRERELEASE`
   in the root `CMakeLists.txt`.
2. **Write the changelog entry.** Move what is under `## [Unreleased]` into a new
   `## [x.y.z] — YYYY-MM-DD` section in `CHANGELOG.md` and add the link definitions at the bottom
   of the file.
3. **Record the meta-gl revision and the build environment** in that entry:

   ```bash
   git -C ../meta-gl describe --tags --exact-match HEAD   # must name a tag, ideally
   git -C ../meta-gl log -1 --format='%H (%D, %ad)' --date=short
   git -C ../meta-gl status --short                        # library sources must be clean
   printf '%s\n' "$(lsb_release -ds)" "$(g++ --version | head -1)" "$(cmake --version | head -1)"
   ```
4. **Update `README.md`** — the *Version* section.
5. **Build and test** in the existing `build/` directory (the build rules in the parent
   `CLAUDE.md` — shared ccache, no new build directories). Use a **Debug** build: the smoke
   tests check with `assert()`, which `NDEBUG` turns into no-ops in Release.

   ```bash
   export CCACHE_DIR=/rv/cnaccache CCACHE_BASEDIR=/rv
   cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug \
       -DEASYGL_BUILD_TESTS=ON -DEASYGL_BUILD_EXAMPLES=ON \
       -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCMAKE_C_COMPILER_LAUNCHER=ccache
   cmake --build build --parallel
   ctest --test-dir build --output-on-failure
   ```

   The configure output prints `-- easy-gl: version <x.y.z>` and, further down,
   `-- meta-gl: version <x.y.z>` — check both. The build must be warning-free and every test must pass.
6. **Commit** the version-bearing files by explicit name (`CMakeLists.txt`, `CHANGELOG.md`,
   `README.md`), never `git add -A`.
7. **Tag** with a `v` prefix and an annotated tag:

   ```bash
   git tag -a v0.1.1 -m "easy-gl 0.1.1"
   ```

   The tag string carries the `v`; `EASYGL_VERSION_STRING` never does.
8. **Push only when the project owner asks**, and push the tag explicitly:

   ```bash
   git push origin develop
   git push origin v0.1.1
   ```

   Never move or recreate a tag once it is pushed — downstream entries record what it pointed
   at. If a pushed release needs a fix, release the next patch version instead.
9. **Open the next cycle** by adding an empty `## [Unreleased]` section back to `CHANGELOG.md`
   if step 2 consumed it.
