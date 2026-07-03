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
    # slice-029 (a-check-Vorbereitung): ui verzeichnislich nach Richtung
    # getrennt — view/ = driven (ModelChangedPort-Implementierungen +
    # Rendering), command/ = driving (MeshSource-Naht ueber ViewModelPort).
    ui/view/viewer_scene.cpp
    ui/view/viewer_widget.cpp
    ui/command/view_model_mesh_source.cpp
    geometry/occ_probe.cpp
    geometry/occ_solids.cpp          # geteilter OCC-Solid-Builder (slice-020b)
    geometry/occ_geometry_adapter.cpp
    # slice-020b: STEP/STL-Export (ADR-0014) — geometrie-residenter
    # ModelExporterPort. STL nutzt nur den GeometryKernelPort + Kern-Geometrie
    # (kein direktes OCC); STEP nutzt OCC-DataExchange (B-Rep der Solid-Bauteile).
    geometry/stl_export_adapter.cpp
    geometry/step_export_adapter.cpp
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
    # slice-021b Commit i: DXF-Codec (ADR-0015 Option D) — hand-gerollter ASCII-
    # DXF-Subset (R12) Reader+Writer, io-resident, format-agnostisch. KEINE neue
    # Dependency (reines C++/STL); arch-check Regel A/B isolieren ihn.
    io/dxf_reader.cpp
    io/dxf_writer.cpp
    # slice-021b Commit ii: DXF-Import/-Export-Adapter (ADR-0015 Option D) —
    # ModelImporterPort/ModelExporterPort hinter dem DXF-Codec, io-resident,
    # atomar (Temp+Rename). KEINE neue Dependency.
    io/dxf_import_adapter.cpp
    io/dxf_export_adapter.cpp
    # slice-025b: PDF-Export (ADR-0016 Option D) — hand-gerollter Vektor-PDF-
    # Writer + Domänen→Plan-Mapping (maßstäblicher 2D-Grundriss), export-only,
    # io-resident. Geteilte Helfer plan_geometry (2D-Projektion) + io_atomic_write
    # (atomarer binär-treuer Writer, von 025c/PNG wiederverwendbar). KEINE neue
    # Dependency (reines C++/STL); arch-check Regel A/B isolieren sie.
    io/io_atomic_write.cpp
    io/plan_geometry.cpp
    io/pdf_writer.cpp
    io/pdf_export_adapter.cpp
    # slice-025c: PNG-Export (ADR-0016 Option D) — hand-gerollter Raster-PNG-
    # Encoder (Bitmap + Chunks + CRC-32/Adler-32/stored-DEFLATE) + Domänen→Raster-
    # Mapping (Fit-to-Canvas, Geschoss-Farben; Reuse plan_geometry/io_atomic_write),
    # export-only. KEINE neue Dependency (kein zlib/libpng); arch-check Regel A/B.
    io/png_writer.cpp
    io/png_export_adapter.cpp
    # slice-026b: Plugin-Host (ADR-0017) — Driving Adapter, lädt Shared-
    # Library-Plugins über den System-Lader (dlfcn-Monopol, arch-check
    # Regel P1) und vermittelt den Plugin-Kontext (Port-Subset v1).
    # KEINE neue Dependency (der dynamische Lader ist Teil der glibc).
    plugin/plugin_host.cpp
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
        # (TKBO: BRepAlgoAPI_Cut, LH-FA-DOR-004/WIN-005, slice-013b/ADR-0011)
        # + STEP-DataExchange (TKDESTEP/TKDE/TKXSBase: STEPControl_Writer,
        # slice-020b/ADR-0014 — libocct-data-exchange-dev, keine neue Dependency)
        # + Mesh→Shape-Vernähung der Dächer (TKShHealing: BRepBuilderAPI_Sewing,
        # slice-024a/ADR-0014 — dieselbe OCC-Distribution, keine neue Dependency).
        TKernel TKMath TKG2d TKG3d TKGeomBase TKBRep TKGeomAlgo TKTopAlgo TKPrim TKMesh TKBO
        TKDESTEP TKDE TKXSBase TKShHealing
        # slice-026b (ADR-0017): dynamischer Lader für den Plugin-Host —
        # auf glibc >= 2.34 leer/in libc, portabel via CMAKE_DL_LIBS.
        ${CMAKE_DL_LIBS}
)
target_compile_features(bcad_adapters PUBLIC cxx_std_20)
