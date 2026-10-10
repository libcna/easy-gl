/// Release identity: the generated easygl/Version.hpp must agree with the CMake project that
/// generated it. The checks are structural -- nothing here names a concrete release -- so a
/// version bump never has to edit this file (docs/releasing.md). They are explicit rather than
/// assert()-based so that they also hold in a Release (NDEBUG) build.

#include <easygl/easygl.hpp>
#include <metagl/Version.hpp>

#include <iostream>
#include <string>
#include <string_view>

#ifndef EASYGL_TEST_EXPECTED_VERSION
#error "EASYGL_TEST_EXPECTED_VERSION must be defined by tests/CMakeLists.txt"
#endif

// The macros are usable by the preprocessor, which is why they exist beside the constants.
#if !(EASYGL_VERSION_MAJOR >= 0 && EASYGL_VERSION_MINOR >= 0 && EASYGL_VERSION_PATCH >= 0)
#error "EASYGL_VERSION_* must be preprocessor-testable integer literals"
#endif

static_assert(easygl::version_major == EASYGL_VERSION_MAJOR);
static_assert(easygl::version_minor == EASYGL_VERSION_MINOR);
static_assert(easygl::version_patch == EASYGL_VERSION_PATCH);
static_assert(easygl::version_string == std::string_view{EASYGL_VERSION_STRING});
static_assert(easygl::version_prerelease == std::string_view{EASYGL_VERSION_PRERELEASE});
static_assert(easygl::version_is_prerelease == !easygl::version_prerelease.empty());

namespace
{
    int failures = 0;

    void check(bool condition, const char* what)
    {
        if (!condition)
        {
            std::cerr << "FAILED: " << what << '\n';
            ++failures;
        }
    }
}

int main()
{
    const std::string_view version = easygl::version_string;

    check(version == EASYGL_TEST_EXPECTED_VERSION,
          "version_string equals the EASYGL_VERSION_STRING CMake configured");
    check(!version.empty() && version.front() != 'v',
          "the version string carries no leading 'v' (only the git tag does)");

    const std::string numeric = std::to_string(easygl::version_major) + "." +
                                std::to_string(easygl::version_minor) + "." +
                                std::to_string(easygl::version_patch);
    if (easygl::version_is_prerelease)
    {
        check(version == numeric + "-" + std::string{easygl::version_prerelease},
              "a pre-release is spelled MAJOR.MINOR.PATCH-PRERELEASE");
    }
    else
    {
        check(version == numeric, "a final release is exactly MAJOR.MINOR.PATCH");
    }

    // easy-gl's version is its own; meta-gl underneath reports a separate one.
    check(!metagl::GetVersionString().empty(), "the meta-gl version is available beside it");

    if (failures != 0)
        return 1;

    std::cout << "easy-gl " << version << " on meta-gl " << metagl::GetVersionString()
              << ": all version checks passed\n";
    return 0;
}
