#pragma once

#include <optional>
#include <vector>

#include "hexagon/model/triangle_mesh.h"
#include "hexagon/model/wall.h"  // WallId

namespace bcad::hexagon::ports::driving {

// Tessellierter Stand einer Wand für die Darstellung (ADR-0009 (b)).
struct WallMesh {
    model::WallId wall_id{};
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
};

}  // namespace bcad::hexagon::ports::driving
