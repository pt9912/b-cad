#pragma once

#include <optional>

#include "hexagon/model/building.h"
#include "hexagon/model/wall.h"  // StoreyId

namespace bcad::hexagon::model {

// Geteilte Roh-Auflösung der Geschoss-Höhe je `StoreyId` (ADR-0020,
// slice-042d): liefert `nullopt` bei danglendem/unbekanntem Geschoss und
// überlässt die **Semantik dem Aufrufer** — der Export nutzt
// `.value_or(kDefaultStoreyHeightMm)` (Totalität), der Save-Pfad wirft E-IO
// (Datenintegrität, kein Teil-Save). Löst das frühere Duplikat auf (die
// `ExchangeService`-lokale `storeyHeight` [total] vs. die Adapter-lokale
// `fromStoreyHeight` [werfend]) — **eine** Wahrheit. Pure Query, `model/`-only.
inline std::optional<double> resolveStoreyHeight(const Building& building,
                                                 StoreyId id) {
    for (const Storey& s : building.storeys) {
        if (s.id == id) {
            return s.height_mm;
        }
    }
    return std::nullopt;
}

}  // namespace bcad::hexagon::model
