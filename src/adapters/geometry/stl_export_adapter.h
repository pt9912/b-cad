#pragma once

#include <filesystem>

#include "hexagon/model/building.h"
#include "hexagon/ports/driven/geometry_kernel_port.h"
#include "hexagon/ports/driven/model_exporter_port.h"

namespace bcad::adapters::geometry {

// IO/Export (ADR-0014): **geometrie-residenter** `ModelExporterPort` für STL.
// Assembliert das Dreiecksnetz aller 3D-Bauteile — Wände/Decken über den
// `GeometryKernelPort` (Footprint-Extrusion + Cutouts, tesselliert), Dächer/
// Treppen analytisch (`roof_/stair_geometry`) — und schreibt eine **binäre
// STL-Datei** (atomar, Temp + Rename). Lebt in `src/adapters/geometry/`
// (OCC-nah, Regel C); nutzt nur Kern-Ports/-Geometrie-Funktionen, **kein**
// anderer Adapter (Regel B). Bei nicht beschreibbarem Zielpfad neutrale
// `std::runtime_error` mit vorangestelltem `E-IO-001` (kein Teil-Export);
// 3D-leeres Modell → gültige, leere STL (Totalität).
class StlExportAdapter final : public hexagon::ports::driven::ModelExporterPort {
public:
    explicit StlExportAdapter(
        const hexagon::ports::driven::GeometryKernelPort& geometry);

    void write(const hexagon::model::Building& building,
               const hexagon::model::DerivedGeometry& derived,
               const std::filesystem::path& path) const override;

private:
    const hexagon::ports::driven::GeometryKernelPort& geometry_;
};

}  // namespace bcad::adapters::geometry
