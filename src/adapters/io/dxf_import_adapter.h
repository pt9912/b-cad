#pragma once

#include <filesystem>

#include "hexagon/model/building.h"
#include "hexagon/ports/driven/model_importer_port.h"

namespace bcad::adapters::io {

// IO-Adapter (ADR-0015 Option D): implementiert den Driven Port
// `ModelImporterPort` für DXF. Liest eine ASCII-DXF-Datei (2D-Grundriss) über
// den hand-gerollten `DxfReader` und mappt das welle-4-Subset (gerade
// Wand-Achsen je Geschoss-`LAYER`, spez. §1 `LH-FA-IO-003.a`) auf ein
// `model::Building`. Der DXF-Code lebt ausschließlich hier (a-check Regel
// A/B); der Kern bleibt format-frei (ADR-0001).
//
// Fehler-Totalität (`LH-FA-IO-003` Negative/Boundary): nicht wohlgeformter
// Gruppencode-Strom -> neutrale `std::runtime_error` mit vorangestelltem
// `E-IO-003` (`event=import_rejected`), kein Teil-Import. Eine leere/strukturlose
// (aber wohlgeformte) Datei -> leeres `Building` (kein Wurf, Totalität).
// DXF trägt keine Höhe/Dicke -> importierte Wände bekommen Default-Werte
// (benannte Lücke, spez. §1).
class DxfImportAdapter final : public hexagon::ports::driven::ModelImporterPort {
public:
    hexagon::model::Building read(
        const std::filesystem::path& path) const override;
};

}  // namespace bcad::adapters::io
