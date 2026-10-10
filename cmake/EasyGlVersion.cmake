# =====================================================================================
# easy-gl release identity -> the generated public header easygl/Version.hpp.
#
# The single source of truth is project(easy-gl VERSION ...) plus EASYGL_VERSION_PRERELEASE in
# the root CMakeLists.txt; this file only renders that decision into a header. Nothing else in
# the tree may hard-code the number -- docs/releasing.md lists the hand-maintained copies
# (CHANGELOG.md, README.md) that a release bump must update by hand.
#
# CMAKE_CURRENT_BINARY_DIR, not CMAKE_BINARY_DIR: when easy-gl is consumed via
# add_subdirectory (as CNA does), CMAKE_BINARY_DIR is the *consumer's* build root, and the
# generated header belongs in easy-gl's own.
# =====================================================================================

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/Version.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/easygl/Version.hpp"
    @ONLY)
