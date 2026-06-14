if(NOT BUILD_TESTING)
    return()
endif()

find_package(GTest REQUIRED)   # REQ-TEC-005

# Kern-Tests — dependency-frei (nur bcad_hexagon, kein Qt/OCC/SQLite);
# Port-Doubles statt echter Adapter (ADR-0001 §Testbarkeit).
add_executable(bcad_tests
    hexagon/test_bootstrap_info.cpp
    hexagon/test_evaluate.cpp
    hexagon/test_greeting_service.cpp
    hexagon/test_model_change_notification.cpp
    hexagon/test_openings.cpp
    hexagon/test_roof_geometry.cpp
    hexagon/test_room_detection.cpp
    hexagon/test_slab_geometry.cpp
    hexagon/test_stair_geometry.cpp
    hexagon/test_structure_edit_service.cpp
    hexagon/test_wall_footprint.cpp
)
target_link_libraries(bcad_tests PRIVATE bcad_hexagon GTest::gtest_main)

# Adapter-Tests — linken gegen bcad_adapters und beweisen damit echte
# Linkage + Laufzeit gegen Qt6/OpenCascade/SQLite (Review-Befund
# slice-001: der Build-Gate muss Linken beweisen, nicht nur Kompilieren).
add_executable(bcad_adapter_tests
    adapters/test_probes.cpp
    adapters/test_occ_geometry_adapter.cpp
    adapters/test_sqlite_project_repository.cpp
    adapters/test_sqlite_crash_recovery.cpp
    adapters/test_viewer_scene.cpp
    adapters/test_viewer_widget.cpp
)
target_link_libraries(bcad_adapter_tests
    PRIVATE bcad_adapters bcad_hexagon GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(bcad_tests)
gtest_discover_tests(bcad_adapter_tests)
