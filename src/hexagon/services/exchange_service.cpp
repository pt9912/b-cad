// Import-/Export-Use-Case (Hexagon-Kern). Format-Dispatch über ein
// Exporter-Registry + Atomaritäts-Grenze; framework-frei (ADR-0001) — kennt nur
// Ports und das Domänen-Modell.

#include "hexagon/services/exchange_service.h"

#include <stdexcept>
#include <utility>

#include "hexagon/model/building.h"
#include "hexagon/model/constants.h"
#include "hexagon/model/derived_geometry.h"
#include "hexagon/model/storey_query.h"                   // resolveStoreyHeight (geteilt, 042d)
#include "hexagon/services/geometry/opening_geometry.h"   // wallCutPrisms
#include "hexagon/services/geometry/plan_projection.h"    // projectPlan (2D, 042b)
#include "hexagon/services/geometry/roof_geometry.h"      // roofMesh
#include "hexagon/services/geometry/slab_geometry.h"      // slabBaseZ/slabCutPrisms
#include "hexagon/services/geometry/stair_geometry.h"     // stairStepBoxes/stairMesh
#include "hexagon/services/geometry/wall_footprint.h"     // wallFootprint

namespace bcad::hexagon::services {

// Höhen-Auflösung für die STEP/STL-Ableitung: der geteilte Kern-Helfer
// `model::resolveStoreyHeight` liefert die Roh-Höhe; der Export-Pfad ist
// **total** (unbekanntes/danglendes Geschoss → Default, kein Wurf) via
// `.value_or` (slice-042d-Konsolidierung — dieselbe Wahrheit nutzt der
// Save-Pfad, dort mit werfender Semantik).

ExchangeService::ExchangeService(ImporterMap importers, ExporterMap exporters)
    : importers_(std::move(importers)), exporters_(std::move(exporters)) {}

model::Building ExchangeService::importModel(
    const std::filesystem::path& path,
    ports::driving::ExchangeFormat format) const {
    const auto it = importers_.find(format);
    if (it == importers_.end() || it->second == nullptr) {
        // Format nicht für Import verdrahtet (z. B. STEP/STL export-only) →
        // wie nicht erkannt behandelt. **Beide** Token (E-IO-003 +
        // event=import_rejected), damit der export-only-Vertrag erhalten bleibt
        // (slice-021b MED-1).
        throw std::runtime_error(
            "E-IO-003: Format ist nicht für Import verdrahtet (export-only); "
            "event=import_rejected");
    }
    // Erfolg: vollständiges Building. Fehler: der Importer wirft E-IO-003
    // (event=import_rejected) — propagiert, kein Teil-Import.
    return it->second->read(path);
}

void ExchangeService::exportModel(const model::Building& building,
                                  const std::filesystem::path& path,
                                  ports::driving::ExchangeFormat format) const {
    const auto it = exporters_.find(format);
    if (it == exporters_.end() || it->second == nullptr) {
        // Format nicht verdrahtet → wie nicht beschreibbar behandelt.
        throw std::runtime_error(
            "E-IO-001: nicht unterstütztes/unverdrahtetes Export-Format; "
            "event=io_no_permission");
    }
    // slice-042a/042b/042c (ADR-0020): der Kern reicht das abgeleitete-Geometrie-
    // Bündel über den Port — driven Adapter serialisieren nur, sie leiten keine
    // Geometrie ab. **Format-selektiv befüllt:** die 2D-Grundriss-Projektion
    // (`PlanView`) für PDF/PNG (042b); die 3D-Bauteil-Ableitung als pre-OCC-Primitive
    // für STEP/STL (042c); IFC/DXF leiten nichts ab (leeres Bündel — IFC serialisiert
    // `model → SPF`, DXF iteriert direkt).
    using ports::driving::ExchangeFormat;
    model::DerivedGeometry derived;
    if (format == ExchangeFormat::Pdf || format == ExchangeFormat::Png) {
        derived.plan = projectPlan(building);
    } else if (format == ExchangeFormat::Step || format == ExchangeFormat::Stl) {
        // **Ein Eintrag je Bauteil in Modell-Reihenfolge** (Compound-/Netz-Reihenfolge
        // deterministisch); die reine Ableitung ist **total** (wirft nie). Den
        // fail-closed-Skip degenerierter Bauteile besorgt der Adapter **beim OCC-/
        // Tessellations-Bau** (Regel C) — hier **kein** Vor-Filtern, sonst verschöbe
        // sich, welches Bauteil im Export fehlt.
        derived.walls.reserve(building.walls.size());
        for (const model::Wall& w : building.walls) {
            derived.walls.push_back({wallFootprint(w, building.walls), w.height_mm,
                                     wallCutPrisms(w, building.openings)});
        }
        derived.slabs.reserve(building.slabs.size());
        for (const model::Slab& s : building.slabs) {
            derived.slabs.push_back(
                {s.footprint, s.thickness_mm, slabCutPrisms(s),
                 slabBaseZ(s, model::resolveStoreyHeight(building, s.storey_id)
                                  .value_or(model::kDefaultStoreyHeightMm))});
        }
        derived.roofs.reserve(building.roofs.size());
        for (const model::Roof& r : building.roofs) {
            derived.roofs.push_back({roofMesh(r)});
        }
        derived.stairs.reserve(building.stairs.size());
        for (const model::Stair& s : building.stairs) {
            const double h = model::resolveStoreyHeight(building, s.from_storey_id)
                                 .value_or(model::kDefaultStoreyHeightMm);
            derived.stairs.push_back({stairStepBoxes(s, h), stairMesh(s, h)});
        }
    }
    // Erfolg: vollständige Datei. Fehler: der Exporter wirft E-IO-001 (bzw.
    // E-IO-003) — propagiert, kein Teil-Export.
    it->second->write(building, derived, path);
}

}  // namespace bcad::hexagon::services
