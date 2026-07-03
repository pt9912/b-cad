if(NOT BUILD_TESTING)
    return()
endif()

find_package(GTest REQUIRED)   # REQ-TEC-005

# Kern-Tests — dependency-frei (nur bcad_hexagon, kein Qt/OCC/SQLite);
# Port-Doubles statt echter Adapter (ADR-0001 §Testbarkeit).
add_executable(bcad_tests
    hexagon/test_bootstrap_info.cpp
    hexagon/test_evaluate.cpp
    hexagon/test_evaluate_lists.cpp
    hexagon/test_evaluate_volume.cpp
    hexagon/test_greeting_service.cpp
    hexagon/test_material.cpp
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
    adapters/test_ifc_spf_reader.cpp   # slice-019b Commit i: SPF-Codec (isoliert)
    adapters/test_ifc_import.cpp       # slice-019b Commit ii: Mapping-AK + Integration
    adapters/test_ifc_export.cpp       # slice-019c: Export-Roundtrip-AK + E-IO-001
    adapters/test_step_stl_export.cpp  # slice-020b: STEP (B-Rep) + STL (binär) Export-AK + E-IO-001 + Integration
    adapters/test_dxf_codec.cpp        # slice-021b Commit i: DXF-Codec-Roundtrip (isoliert)
    adapters/test_dxf_import.cpp       # slice-021b Commit ii: Import-AK + Integration
    adapters/test_dxf_export.cpp       # slice-021b Commit ii: Export-AK + Roundtrip + E-IO-001
    adapters/test_pdf_export.cpp       # slice-025b: PDF-Export-AK + voll-Decode-Orakel + Maßstabs-Sonde + E-IO-001 + export-only
    adapters/test_png_export.cpp       # slice-025c: PNG-Export-AK + voll-Decode-Orakel (Inflate/Adler/CRC/Filter) + degenerierte BBox + E-IO-001 + export-only
    adapters/test_plugin_host.cpp      # slice-026b: Plugin-Host-AK (reale .so durch echten Host: Happy/werfend/ABI-Mismatch/defekt/symbollos) + Kontext-Invalidierung
)
target_link_libraries(bcad_adapter_tests
    PRIVATE bcad_adapters bcad_hexagon GTest::gtest_main)
# slice-026b (ADR-0017): die Plugin-AK-Tests laden REALE MODULE-Targets
# durch den echten Host — Symbol-Naht wie im Produkt (ENABLE_EXPORTS,
# Entscheidung 1); der Plugin-Ablageort kommt als Compile-Definition.
set_target_properties(bcad_adapter_tests PROPERTIES ENABLE_EXPORTS ON)
target_compile_definitions(bcad_adapter_tests
    PRIVATE BCAD_TEST_PLUGIN_DIR="${CMAKE_BINARY_DIR}/plugins")
add_dependencies(bcad_adapter_tests
    bcad_example_plugin bcad_throwing_plugin
    bcad_abi_mismatch_plugin bcad_no_symbols_plugin)

include(GoogleTest)
gtest_discover_tests(bcad_tests)
gtest_discover_tests(bcad_adapter_tests)
