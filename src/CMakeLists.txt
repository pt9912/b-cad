add_subdirectory(hexagon)
add_subdirectory(adapters)

# Composition Root — verdrahtet Kern + Adapter (ADR-0001).
add_executable(b-cad main.cpp)
target_link_libraries(b-cad PRIVATE bcad_hexagon bcad_adapters)
