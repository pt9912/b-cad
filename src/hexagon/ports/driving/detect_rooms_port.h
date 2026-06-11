#pragma once

#include <vector>

#include "hexagon/model/room.h"

namespace bcad::hexagon::ports::driving {

// Driving Port (ADR-0001): Abfrage der automatisch erkannten Räume
// eines Geschosses (LH-FA-ROM-001, ADR-0007). Reine Query — die
// Erkennung selbst läuft bei Modell-Mutation im Service
// (spez. §1 LH-FA-ROM-001.a §Auslösung), nicht beim Abruf.
class DetectRoomsPort {
public:
    virtual ~DetectRoomsPort() = default;

    // Zuletzt erkannte Räume des Geschosses; leer, wenn keine
    // geschlossenen Wandzüge existieren (oder das Geschoss unbekannt ist).
    virtual std::vector<model::Room> rooms(model::StoreyId storey) const = 0;
};

}  // namespace bcad::hexagon::ports::driving
