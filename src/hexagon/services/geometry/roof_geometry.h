#pragma once

#include "hexagon/model/roof.h"
#include "hexagon/model/triangle_mesh.h"

namespace bcad::hexagon::services {

// Analytisches Dach-Netz (Sattel/Walm/Pult) über dem rechteckigen
// Grundriss (spez. §1 `LH-FA-ROF-001.a`): Traufrechteck aus Überstand,
// Firsthöhe = Formel·tan(Neigung); die Dachflächen werden direkt als
// Dreiecke angegeben (das Dach ist ein Polyeder — kein OCC-Boolean/
// -Extrusion nötig, ADR-0001 Geometrie-Hoheit im Kern; das Netz fließt
// framework-frei über den `ViewModelPort`, ADR-0009). **Total:** ein
// degenerierter/zu kleiner Grundriss oder eine nicht-positive Neigung
// liefert ein **leeres** Netz (kein Wurf).
model::TriangleMesh roofMesh(const model::Roof& roof);

}  // namespace bcad::hexagon::services
