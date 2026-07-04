#pragma once

#include <filesystem>

#include "hexagon/model/building.h"
#include "hexagon/ports/driven/model_importer_port.h"

namespace bcad::adapters::io {

// IO-Adapter (ADR-0013 Option D): implementiert den Driven Port
// `ModelImporterPort` für IFC. Liest eine IFC-SPF-Datei (`.ifc`-Klartext,
// ISO 10303-21) über den hand-gerollten `SpfReader` und mappt das welle-4-
// Entitäts-Subset (Geschosse + gerade Wände, spez. §1 `LH-FA-IO-001.a`) auf
// ein `model::Building`. Der SPF-/IFC-Code lebt ausschließlich hier
// (a-check Regel A/B); der Kern bleibt format-frei (ADR-0001).
//
// Fehler-Totalität (`LH-FA-IO-001` Negative): nicht erkanntes/invalides Format
// oder eine fehlende tragende Pflicht-Referenz (Wand ohne Geschoss-Verortung
// oder ohne Achs-Repräsentation) -> neutrale `std::runtime_error` mit
// vorangestelltem `E-IO-003` und `event=import_rejected`, kein Teil-Import.
// Eine leere/strukturlose Datei -> leeres `Building` (kein Wurf, Totalität).
class IfcImportAdapter final : public hexagon::ports::driven::ModelImporterPort {
public:
    hexagon::model::Building read(
        const std::filesystem::path& path) const override;
};

}  // namespace bcad::adapters::io
