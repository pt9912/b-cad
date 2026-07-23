#pragma once

#include <filesystem>

#include "hexagon/model/building.h"
#include "hexagon/ports/driven/model_exporter_port.h"

namespace bcad::adapters::io {

// IO-Adapter (ADR-0015 Option D): implementiert den Driven Port
// `ModelExporterPort` für DXF. Bildet das welle-4-Subset (gerade Wand-Achsen je
// Geschoss-`LAYER`, spez. §1 `LH-FA-IO-003.a`) auf ASCII-DXF (R12/AC1009) ab und
// serialisiert über den hand-gerollten `DxfWriter`. **Atomar** (Temp + fsync +
// Rename, Muster `ifc_export_adapter`): vollständige Datei oder Wurf, kein
// Teil-Export. Der DXF-Code lebt ausschließlich hier (a-check Regel A/B).
//
// Roundtrip-Disziplin: je gerade Wand eine `LINE` mit dem Geschoss-`LAYER` als
// Gruppencode 8 (was der Import liest); z=0 (2D-Grundriss). Subset-Grenzen
// (Spiegel des Imports): nur Geschosse + gerade Wände; weitere Inhalte werden
// nicht geschrieben.
class DxfExportAdapter final : public hexagon::ports::driven::ModelExporterPort {
public:
    void write(const hexagon::model::Building& building,
               const hexagon::model::DerivedGeometry& derived,
               const std::filesystem::path& path) const override;
};

}  // namespace bcad::adapters::io
