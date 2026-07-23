#pragma once

#include <filesystem>

#include "hexagon/model/building.h"
#include "hexagon/ports/driven/model_exporter_port.h"

namespace bcad::adapters::io {

// IO-Adapter (ADR-0016 Option D, slice-025b): implementiert den Driven Port
// `ModelExporterPort` für **PDF** (export-only). Bildet den welle-4-Subset (gerade
// Wand-Achsen je Geschoss, spez. §1 `LH-FA-IO-007.a`) auf einen **maßstäblichen
// 2D-Grundriss** ab: je Geschoss eine A4-Seite mit Rahmen, den bei **festem
// Maßstab 1:100** transformierten Wand-Achsen und einem sichtbaren „M 1:100"-Label
// (Standard-Font Helvetica); serialisiert über den hand-gerollten `PdfWriter` und
// schreibt **atomar** ([`E-IO-001`], `io_atomic_write`). Der PDF-Code lebt
// ausschließlich hier + in `pdf_writer`/`plan_geometry` (a-check Regel A/B —
// kein OCC/Qt, keine externe Bibliothek).
//
// Export-only: kein Import-Adapter (aus einem PDF wird kein Modell gelesen); ein
// Import-Request → `E-IO-003` (export-only-Lookup-Miss im `ExchangeService`).
class PdfExportAdapter final : public hexagon::ports::driven::ModelExporterPort {
public:
    void write(const hexagon::model::Building& building,
               const hexagon::model::DerivedGeometry& derived,
               const std::filesystem::path& path) const override;
};

}  // namespace bcad::adapters::io
