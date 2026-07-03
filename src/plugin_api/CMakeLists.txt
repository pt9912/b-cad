# Plugin-API (ADR-0017): header-only Vertragsschicht zwischen Host und
# Plugins — ABI-Handshake, Plugin-Interface, PluginContext. KEINE
# Link-Abhängigkeit auf den Kern (die Kern-Header model/ports/driving
# sind header-only; die Symbole liefert das ladende Executable,
# ENABLE_EXPORTS). arch-check Regel P2 begrenzt die Includes dieses
# Verzeichnisses.
add_library(bcad_plugin_api INTERFACE)
target_include_directories(bcad_plugin_api
    INTERFACE ${PROJECT_SOURCE_DIR}/src)
target_compile_features(bcad_plugin_api INTERFACE cxx_std_20)
