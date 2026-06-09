# Adapter-Schicht. Hier — und NUR hier — werden externe Bibliotheken
# eingebunden (ADR-0001/0002/0003). bcad_adapters hängt am Kern, nie
# umgekehrt.

find_package(Qt6 REQUIRED COMPONENTS Core)   # REQ-TEC-002, ADR-Folge GUI
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
    geometry/occ_probe.cpp
    geometry/occ_geometry_adapter.cpp
    persistence/sqlite_probe.cpp
    persistence/sqlite_project_repository.cpp
)

target_include_directories(bcad_adapters
    PUBLIC  ${PROJECT_SOURCE_DIR}/src
    PRIVATE ${OpenCASCADE_INCLUDE_DIR}
            ${CMAKE_CURRENT_BINARY_DIR}/generated
)
target_link_libraries(bcad_adapters
    PRIVATE
        bcad_hexagon          # Adapter -> Kern (erlaubte Richtung)
        Qt6::Core
        SQLite::SQLite3
        # OCC-Modeling-Toolkits (gezielt; nicht ${OpenCASCADE_LIBRARIES},
        # das auch nicht-installierte DRAW-/Viewer-Toolkits listet):
        # Footprint-Fläche + Prisma + Volumen-Properties.
        TKernel TKMath TKG2d TKG3d TKGeomBase TKBRep TKGeomAlgo TKTopAlgo TKPrim
)
target_compile_features(bcad_adapters PUBLIC cxx_std_20)
