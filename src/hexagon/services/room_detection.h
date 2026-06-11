#pragma once

#include <vector>

#include "hexagon/model/building.h"
#include "hexagon/model/room.h"

namespace bcad::hexagon::services {

// Reine Raumerkennung (LH-FA-ROM-001, ADR-0007; Algorithmus:
// spez. §1 LH-FA-ROM-001.a). Framework-frei, kein OCC — 2D-Geometrie
// auf dem Domänen-Modell.
//
// Die Erkennung ist TOTAL und wirft kein E-GEO-002: degenerierte
// Zyklen (kollabierter Innenkanten-Offset, Netto-Fläche <= 0) erzeugen
// keinen Raum — gleiches Verhalten wie offene Wandzüge (spez. §1
// §Degenerierte Zyklen).
//
// Welle-1-Einschränkung: Graph-Knoten sind toleranz-verschmolzene
// Segment-Endpunkte (kGeometryToleranceMm); Schnittpunkte/T-Stöße
// werden erst mit der Wandverschneidung (LH-FA-WAL-006) zu Knoten.
std::vector<model::Room> detectRooms(const model::Building& building,
                                     model::StoreyId storey);

}  // namespace bcad::hexagon::services
