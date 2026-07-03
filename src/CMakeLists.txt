add_subdirectory(hexagon)
add_subdirectory(plugin_api)
add_subdirectory(adapters)

# Composition Root — verdrahtet Kern + Adapter (ADR-0001).
add_executable(b-cad main.cpp)
target_link_libraries(b-cad PRIVATE bcad_hexagon bcad_adapters)
# Symbol-Naht (ADR-0017 / slice-026b, Entscheidung 1): das Executable
# exportiert seine Symbole für zur Laufzeit geladene Plugins
# (weak-vtable/typeinfo-Unifikation über die Modul-Grenze; der Kern
# bleibt statisch, statisches Kern-Dazulinken in Plugins ist verboten).
set_target_properties(b-cad PROPERTIES ENABLE_EXPORTS ON)
