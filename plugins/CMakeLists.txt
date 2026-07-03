# Plugins (ADR-0017): zur Laufzeit ladbare CMake-MODULE-Targets.
#
# HARD RULE (ADR-0017, Entscheidung 2): Plugins linken NIE bcad_hexagon —
# zwei Kern-Kopien mit doppelter typeinfo im Prozess brächen die
# Fehler-Barriere. Sie sehen nur das Include-INTERFACE-Target
# bcad_plugin_api (Plugin-API + header-only Kern-Header model/ports);
# die Kern-Symbole liefert zur Ladezeit die exportierte Symboltabelle
# des ladenden Executables (ENABLE_EXPORTS, Symbol-Naht slice-026b).
#
# arch-check Regel P2 begrenzt die Includes dieses Baums; Regel P1
# verbietet hier jedes eigene dynamische Laden.

set(BCAD_PLUGIN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/plugins)

function(bcad_add_plugin name)
    add_library(${name} MODULE ${ARGN})
    target_link_libraries(${name} PRIVATE bcad_plugin_api)
    set_target_properties(${name} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${BCAD_PLUGIN_OUTPUT_DIR}
        PREFIX "")
endfunction()

# Beispiel-Plugin (zugleich AK-Fixture Happy Path).
bcad_add_plugin(bcad_example_plugin example/example_plugin.cpp)

# Test-Fixtures der Plugin-Host-AK-Tests.
bcad_add_plugin(bcad_throwing_plugin testing/throwing_plugin.cpp)
bcad_add_plugin(bcad_unload_throwing_plugin testing/unload_throwing_plugin.cpp)
bcad_add_plugin(bcad_abi_mismatch_plugin testing/abi_mismatch_plugin.cpp)
bcad_add_plugin(bcad_no_symbols_plugin testing/no_symbols_plugin.cpp)
