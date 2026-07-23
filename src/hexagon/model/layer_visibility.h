#pragma once

#include <unordered_set>

#include "hexagon/model/building.h"
#include "hexagon/model/layer.h"

namespace bcad::hexagon::model {

// Roh-Ids der SICHTBAREN Ebenen (`layer.visible`, LH-FA-DRW-006). Geteilte,
// format-agnostische Quelle des Export-Sichtbarkeits-Filters (ADR-0018 §3:
// unsichtbare Ebene → ihre Hilfslinien werden nicht gezeichnet). **Reiner
// Nicht-Geometrie-Filter** — kein Koordinaten-Wert. Header-only im `model/`-Kern,
// damit **beide** ihn ziehen dürfen (ADR-0020): die kern-residente 2D-Projektion
// (`services/geometry/plan_projection`) UND der io-DXF-Export — ohne Duplikat und
// ohne eine `io → services_geo`-Kante.
inline std::unordered_set<int> visibleLayerIds(const Building& building) {
    std::unordered_set<int> visible;
    for (const Layer& layer : building.layers) {
        if (layer.visible) {
            visible.insert(static_cast<int>(layer.id));
        }
    }
    return visible;
}

}  // namespace bcad::hexagon::model
