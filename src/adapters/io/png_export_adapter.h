#pragma once

#include <filesystem>

#include "hexagon/model/building.h"
#include "hexagon/ports/driven/model_exporter_port.h"

namespace bcad::adapters::io {

// IO-Adapter (ADR-0016 Option D, slice-025c): implementiert den Driven Port
// `ModelExporterPort` für **PNG** (export-only). Bildet den welle-4-Subset (gerade
// Wand-Achsen je Geschoss, spez. §1 `LH-FA-IO-007.a`) auf **ein** kombiniertes
// Rasterbild ab: feste Leinwand, **Fit-to-Canvas** (seitenverhältnis-erhaltend,
// zentriert, geguardet gegen degenerierte Bounding-Box), **je Geschoss eine Farbe**
// (Palette zyklisch) auf weißem Grund; serialisiert über den hand-gerollten
// `png_writer` (PlanView aus dem `DerivedGeometry`-Bündel, ADR-0020) und schreibt
// **atomar** ([`E-IO-001`], Reuse `io_atomic_write`). Der PNG-Code lebt ausschließlich hier + in `png_writer`
// (a-check Regel A/B — kein OCC/Qt, keine externe Bibliothek).
//
// Export-only: kein Import-Adapter; ein Import-Request → `E-IO-003` (Lookup-Miss).
class PngExportAdapter final : public hexagon::ports::driven::ModelExporterPort {
public:
    void write(const hexagon::model::Building& building,
               const hexagon::model::DerivedGeometry& derived,
               const std::filesystem::path& path) const override;
};

}  // namespace bcad::adapters::io
