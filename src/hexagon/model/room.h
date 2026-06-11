#pragma once

#include <vector>

#include "hexagon/model/point2d.h"
#include "hexagon/model/wall.h"  // StoreyId

namespace bcad::hexagon::model {

// Starke Raum-Id (Konvention aus slice-003a: Bauteil-Ids sind enum class,
// nie implizit gegen Messwerte vertauschbar). Räume sind in welle-1
// abgeleitete Laufzeit-Daten — die Ids sind pro Erkennungslauf vergeben;
// Persistenz-Identität entscheidet der Raum-Persistenz-Slice (ADR-0007
// §Konsequenzen).
enum class RoomId : int {};

// Polygon-Ring als geschlossene Punktfolge (letzter Punkt verbindet
// implizit zurück zum ersten).
using Ring = std::vector<Point2D>;

// Raum (LH-FA-ROM-001, ADR-0007): aus einem geschlossenen Wandzug
// abgeleitet. Polygon auf INNENKANTEN-Basis im RING-Modell — äußerer
// Ring plus 0..n Loch-Ringe (Außenkonturen innen liegender Wandzüge).
// `net_area_mm2` ist die NETTO-Fläche (äußerer Ring minus Löcher) —
// damit ist „keine Doppelzählung der Fläche" (LH-FA-ROM-001 Boundary)
// direkt ablesbar.
struct Room {
    RoomId id{};
    StoreyId storey_id{};
    Ring outer;
    std::vector<Ring> holes;
    double net_area_mm2{};
};

}  // namespace bcad::hexagon::model
