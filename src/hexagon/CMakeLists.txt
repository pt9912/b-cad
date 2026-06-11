# Anwendungskern (Hexagon). HARD RULE (ADR-0001, AGENTS.md §2.1):
# dieses Target hat KEINE externen Abhängigkeiten — kein Qt, kein
# OpenCascade, kein SQLite, kein bcad_adapters. Die Target-Trennung
# IST die Fitness Function: ein Adapter-Import aus dem Kern wäre hier
# ein Link-Fehler.

add_library(bcad_hexagon
    services/bootstrap_info.cpp
    services/greeting_service.cpp
    services/room_detection.cpp
    services/structure_edit_service.cpp
)

target_include_directories(bcad_hexagon
    PUBLIC  ${PROJECT_SOURCE_DIR}/src           # Konsumenten: #include "hexagon/..."
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}         # Kern sieht nur eigene Header
)
target_compile_features(bcad_hexagon PUBLIC cxx_std_20)
target_compile_definitions(bcad_hexagon PRIVATE BCAD_VERSION="${PROJECT_VERSION}")
