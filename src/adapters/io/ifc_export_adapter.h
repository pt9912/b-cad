#pragma once

#include <filesystem>

#include "hexagon/model/building.h"
#include "hexagon/ports/driven/model_exporter_port.h"

namespace bcad::adapters::io {

// IO-Adapter (ADR-0013 Option D): implementiert den Driven Port
// `ModelExporterPort` für IFC. Bildet das welle-4-Subset (Geschosse + gerade
// Wände, spez. §1 `LH-FA-IO-001.a`) auf IFC4-Entitäten ab und serialisiert über
// den hand-gerollten `IfcSpfWriter` (`.ifc`-Klartext, ISO 10303-21). Der
// SPF-/IFC-Code lebt ausschließlich hier (a-check Regel A/B); der Kern bleibt
// format-frei (ADR-0001). Keine externe IFC-Bibliothek.
//
// **Atomar** (Temp + Rename, Muster Persistenz): vollständige Datei oder Wurf,
// kein Teil-Export; bei nicht beschreibbarem Zielpfad neutrale
// `std::runtime_error` mit vorangestelltem `E-IO-001` (`event=io_no_permission`),
// Zielpfad unverändert (`LH-FA-IO-002` Negative).
//
// **Roundtrip-Partner:** schreibt genau die Pflicht-Referenzen, die der
// 019b-Importer zum Zählen braucht — `IfcShapeRepresentation` mit
// `RepresentationIdentifier` **byte-exakt** `'Axis'` (+ `IfcPolyline`) und
// `IfcRelContainedInSpatialStructure` je Wand —, sodass Export → Import die
// Geschoss-/Wand-Anzahl erhält.
class IfcExportAdapter final : public hexagon::ports::driven::ModelExporterPort {
public:
    void write(const hexagon::model::Building& building,
               const std::filesystem::path& path) const override;
};

}  // namespace bcad::adapters::io
