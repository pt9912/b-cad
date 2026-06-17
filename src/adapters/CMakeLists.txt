# Adapter-Schicht. Hier — und NUR hier — werden externe Bibliotheken
# eingebunden (ADR-0001/0002/0003). bcad_adapters hängt am Kern, nie
# umgekehrt.

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets)  # REQ-TEC-002, ADR-0009
find_package(OpenCASCADE REQUIRED)           # REQ-TEC-003, ADR-0002
find_package(SQLite3 REQUIRED)               # REQ-TEC-007, ADR-0003

# slice-008a: SQLite-DDL aus spec/data-model.yaml (committet, generiert via
# d-migrate — ADR-0006) in den Adapter einbetten (kein Laufzeit-Dateipfad).
# Drift gegen data-model.yaml prüft `make schema-check`.
# schema.sql ist Configure-Input (file READ unten): bei Änderung neu
# konfigurieren, sonst behielte der eingebettete DDL bei Inkrementell-Builds
# den alten Inhalt und maskierte lokal eine Schema-Drift.
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/persistence/schema.sql)
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/persistence/schema.sql BCAD_SCHEMA_SQL)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/persistence/schema_sql.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/adapters/persistence/schema_sql.h
    @ONLY
)

add_library(bcad_adapters
    ui/qt_probe.cpp
    ui/viewer_scene.cpp
    ui/viewer_widget.cpp
    geometry/occ_probe.cpp
    geometry/occ_geometry_adapter.cpp
    # slice-020b: STEP/STL-Export (ADR-0014) — geometrie-residenter
    # ModelExporterPort. STL nutzt nur den GeometryKernelPort + Kern-Geometrie
    # (kein direktes OCC); STEP-Adapter (OCC-DataExchange) folgt im selben Verz.
    geometry/stl_export_adapter.cpp
    persistence/sqlite_probe.cpp
    persistence/sqlite_project_repository.cpp
    # slice-019b: IFC-Import (ADR-0013 Option D) — hand-gerollter IFC-SPF-
    # Subset-Codec + Domänen-Mapping. KEINE neue Dependency (kein vcpkg/Conan,
    # kein OCC-Zwang); reines C++/STL, arch-check Regel A/B isolieren es.
    io/ifc_spf_reader.cpp
    io/ifc_import_adapter.cpp
    # slice-019c: IFC-Export (ADR-0013 Option D) — SPF-Writer (Spiegel des
    # Readers) + Domänen→IFC4-Mapping, atomar (Temp+Rename). KEINE neue Dependency.
    io/ifc_spf_writer.cpp
    io/ifc_export_adapter.cpp
)

target_include_directories(bcad_adapters
    PUBLIC  ${PROJECT_SOURCE_DIR}/src
    PRIVATE ${OpenCASCADE_INCLUDE_DIR}
            ${CMAKE_CURRENT_BINARY_DIR}/generated
)
target_link_libraries(bcad_adapters
    PUBLIC
        # ADR-0009: der Viewer-Header (ui/viewer_widget.h) exponiert
        # Qt-Widget-Typen — Konsumenten (Composition Root, Adapter-Tests)
        # brauchen die Qt-Include-Pfade.
        Qt6::Widgets
        Qt6::OpenGL
        Qt6::OpenGLWidgets
    PRIVATE
        bcad_hexagon          # Adapter -> Kern (erlaubte Richtung)
        Qt6::Core
        SQLite::SQLite3
        # OCC-Modeling-Toolkits (gezielt; nicht ${OpenCASCADE_LIBRARIES},
        # das auch nicht-installierte DRAW-/Viewer-Toolkits listet):
        # Footprint-Fläche + Prisma + Volumen-Properties + Tessellation
        # (TKMesh, ADR-0009 (b)) + boolesche Subtraktion der Wandöffnungen
        # (TKBO: BRepAlgoAPI_Cut, LH-FA-DOR-004/WIN-005, slice-013b/ADR-0011).
        TKernel TKMath TKG2d TKG3d TKGeomBase TKBRep TKGeomAlgo TKTopAlgo TKPrim TKMesh TKBO
)
target_compile_features(bcad_adapters PUBLIC cxx_std_20)
