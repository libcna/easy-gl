# SmokeInitTests.cpp/SmokeResourceTests.cpp mock desktop-OpenGL-shaped
# GL_VERSION strings (e.g. "3.3.0", GL_ARB_* extensions) that a real
# Emscripten/WebGL host can never actually report: under __EMSCRIPTEN__,
# meta-gl always classifies the context as WebGL 1/2 regardless of the
# reported string (see metagl's Context.cpp), which is correct for real
# browsers but makes these two desktop-oriented mocks meaningless there.
# The Emscripten-relevant coverage lives in WebGLTests.cpp and the
# GLES-shaped mocks in ContextLifecycleTests.cpp instead.
if(NOT EMSCRIPTEN)
    add_executable(easy-gl-smoke-tests
            smoke/SmokeInitTests.cpp)
    target_link_libraries(easy-gl-smoke-tests PRIVATE easy-gl::easy-gl)
    easygl_apply_warnings(easy-gl-smoke-tests)
    add_test(NAME easy-gl-smoke-tests COMMAND easy-gl-smoke-tests)

    add_executable(easy-gl-resource-smoke-tests
            smoke/SmokeResourceTests.cpp)
    target_link_libraries(easy-gl-resource-smoke-tests PRIVATE easy-gl::easy-gl)
    easygl_apply_warnings(easy-gl-resource-smoke-tests)
    add_test(NAME easy-gl-resource-smoke-tests COMMAND easy-gl-resource-smoke-tests)
endif()

add_executable(easy-gl-context-lifecycle-tests
        smoke/ContextLifecycleTests.cpp)
target_link_libraries(easy-gl-context-lifecycle-tests PRIVATE easy-gl::easy-gl)
easygl_apply_warnings(easy-gl-context-lifecycle-tests)

add_executable(easy-gl-webgl-tests
        smoke/WebGLTests.cpp)
target_link_libraries(easy-gl-webgl-tests PRIVATE easy-gl::easy-gl)
easygl_apply_warnings(easy-gl-webgl-tests)

add_test(NAME easy-gl-context-lifecycle-tests COMMAND easy-gl-context-lifecycle-tests)
add_test(NAME easy-gl-webgl-tests COMMAND easy-gl-webgl-tests)

add_executable(easy-gl-version-tests
        smoke/VersionTests.cpp)
target_link_libraries(easy-gl-version-tests PRIVATE easy-gl::easy-gl)
target_compile_definitions(easy-gl-version-tests PRIVATE
        EASYGL_TEST_EXPECTED_VERSION="${EASYGL_VERSION_STRING}")
easygl_apply_warnings(easy-gl-version-tests)
add_test(NAME easy-gl-version-tests COMMAND easy-gl-version-tests)
