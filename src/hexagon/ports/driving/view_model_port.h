#pragma once

#include <optional>
#include <vector>

#include "hexagon/model/roof.h"  // RoofId
#include "hexagon/model/slab.h"  // SlabId
#include "hexagon/model/stair.h"  // StairId
#include "hexagon/model/triangle_mesh.h"
#include "hexagon/model/wall.h"  // WallId

namespace bcad::hexagon::ports::driving {

// Tessellierter Stand einer Wand für die Darstellung (ADR-0009 (b)).
struct WallMesh {
    model::WallId wall_id{};
    model::TriangleMesh mesh;
};

// Analytisches Dach-Netz je Dach (LH-FA-ROF-*, slice-014b). Das Dach ist
// ein Polyeder; sein Netz entsteht im Kern (`roof_geometry`), nicht über
// OCC — der Vertrag „framework-freie Netze über `ViewModelPort`" bleibt
// erfüllt (ADR-0009).
struct RoofMesh {
    model::RoofId roof_id{};
    model::TriangleMesh mesh;
};

// Netz je Platte (Decke/Fundament, LH-FA-SLB-*/FND-*, slice-015b) —
// Footprint-Extrusion (über den Port) auf die Aufstandshöhe verschoben.
struct SlabMesh {
    model::SlabId slab_id{};
    model::TriangleMesh mesh;
};

// Netz je Treppe (gerade einläufig, LH-FA-STR-*, slice-016b) — analytisches
// Stufen-Polyeder + Geländer im Kern (`stair_geometry`), kein OCC.
struct StairMesh {
    model::StairId stair_id{};
    model::TriangleMesh mesh;
};

// Driving Port (ADR-0001): Use-Case „Modell-Sicht ableiten"
// (LH-FA-D3-001, ACC-002). Liefert der Darstellungs-Schicht
// framework-freie Dreiecksnetze (ADR-0009 (b): Tessellation über Port —
// kein OCC in der GUI). Reine Query: Pull-Hälfte des
// ADR-0008-Vertrags (Push-Notify über `ModelChangedPort`).
class ViewModelPort {
public:
    virtual ~ViewModelPort() = default;

    // Tessellierter Stand ALLER Wände des Gebäudes — Initial-Laden der
    // Szene (statische Darstellung, LH-FA-D3-001).
    virtual std::vector<WallMesh> wallMeshes() const = 0;

    // Tessellierter Stand EINER Wand — gezielter Pull nach einer
    // ADR-0008-Meldung (`element_id`). Leer bei unbekannter Id oder
    // fehlgeschlagener Tessellation (Query ist total, wirft nicht).
    virtual std::optional<model::TriangleMesh> wallMesh(model::WallId id) const = 0;

    // Netze ALLER Dächer (LH-FA-ROF-*); Pull nach einer `RoofChanged`-
    // Meldung. Total (degeneriertes Dach → kein Eintrag).
    virtual std::vector<RoofMesh> roofMeshes() const = 0;

    // Netze ALLER Platten (LH-FA-SLB-*/FND-*); Pull nach `SlabChanged`.
    // Total (degenerierte Platte → kein Eintrag).
    virtual std::vector<SlabMesh> slabMeshes() const = 0;

    // Netze ALLER Treppen (LH-FA-STR-*); Pull nach `StairChanged`. Total
    // (degenerierte Treppe → kein Eintrag). **Projektweit** (eine Treppe ist
    // geschossübergreifend — spez. §1 LH-FA-STR-001.a).
    virtual std::vector<StairMesh> stairMeshes() const = 0;
};

}  // namespace bcad::hexagon::ports::driving
