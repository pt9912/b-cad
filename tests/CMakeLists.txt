if(NOT BUILD_TESTING)
    return()
endif()

find_package(GTest REQUIRED)   # REQ-TEC-005

add_executable(bcad_tests
    hexagon/test_bootstrap_info.cpp
)
# Tests laufen gegen den Kern (ohne Qt/OCC/SQLite) — Port-Doubles statt
# echter Adapter (ADR-0001 §Testbarkeit). Adapter-/E2E-Tests ab slice-003.
target_link_libraries(bcad_tests PRIVATE bcad_hexagon GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(bcad_tests)
