# Adapter-Schicht. Hier — und NUR hier — werden externe Bibliotheken
# eingebunden (ADR-0001/0002/0003). bcad_adapters hängt am Kern, nie
# umgekehrt.

find_package(Qt6 REQUIRED COMPONENTS Core)   # REQ-TEC-002, ADR-Folge GUI
find_package(OpenCASCADE REQUIRED)           # REQ-TEC-003, ADR-0002
find_package(SQLite3 REQUIRED)               # REQ-TEC-007, ADR-0003

add_library(bcad_adapters
    ui/qt_probe.cpp
    geometry/occ_probe.cpp
    persistence/sqlite_probe.cpp
)

target_include_directories(bcad_adapters
    PUBLIC  ${PROJECT_SOURCE_DIR}/src
    PRIVATE ${OpenCASCADE_INCLUDE_DIR}
)
target_link_libraries(bcad_adapters
    PRIVATE
        bcad_hexagon          # Adapter -> Kern (erlaubte Richtung)
        Qt6::Core
        SQLite::SQLite3
        TKernel               # OpenCascade Foundation (Standard_Version)
)
target_compile_features(bcad_adapters PUBLIC cxx_std_20)
