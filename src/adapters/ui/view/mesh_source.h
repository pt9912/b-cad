// ui-interne, PORT-FREIE Pull-Schnittstelle (slice-029, MeshSource-Naht
// durch das ADR-0008-Push-Notify/Pull-State-Muster): deklariert beim
// Konsumenten (ui/view/, driven-Seite — Dependency-Inversion im
// Kleinen), implementiert auf der driving-Seite (ui/command/ über den
// ViewModelPort). Rückgaben sind reine Modelltypen — KEIN Port-Header
// in diesem Verzeichnis außer dem driven ModelChangedPort der
// Beobachter-Implementierungen (Richtungs-Reinheit je Verzeichnis).
#pragma once

#include <map>
#include <optional>

#include "hexagon/model/roof.h"
#include "hexagon/model/slab.h"
#include "hexagon/model/stair.h"
#include "hexagon/model/triangle_mesh.h"
#include "hexagon/model/wall.h"

namespace bcad::adapters::ui::view {

class MeshSource {
public:
    virtual ~MeshSource() = default;

    // Tessellierter Stand ALLER Wände (Initial-Laden, LH-FA-D3-001).
    virtual std::map<hexagon::model::WallId, hexagon::model::TriangleMesh>
    allWallMeshes() const = 0;

    // Stand EINER Wand — gezielter Pull nach einer ADR-0008-Meldung.
    // Leer bei unbekannter Id/fehlgeschlagener Tessellation (total).
    virtual std::optional<hexagon::model::TriangleMesh> wallMesh(
        hexagon::model::WallId id) const = 0;

    // Netze aller Dächer / Platten / Treppen (Pull nach RoofChanged/
    // SlabChanged/StairChanged; total, degeneriert → kein Eintrag).
    virtual std::map<hexagon::model::RoofId, hexagon::model::TriangleMesh>
    roofMeshes() const = 0;
    virtual std::map<hexagon::model::SlabId, hexagon::model::TriangleMesh>
    slabMeshes() const = 0;
    virtual std::map<hexagon::model::StairId, hexagon::model::TriangleMesh>
    stairMeshes() const = 0;
};

}  // namespace bcad::adapters::ui::view
