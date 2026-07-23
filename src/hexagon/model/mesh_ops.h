#pragma once

#include <cstddef>

#include "hexagon/model/triangle_mesh.h"

namespace bcad::hexagon::model {

// Verschiebt ein Netz um `dz` in z (reine Wert-Operation auf dem
// `TriangleMesh`) — platziert eine bei z∈[0,Dicke] extrudierte Platte auf ihre
// Aufstandshöhe. **Keine** Modell-Ableitung: ADR-0020 hebt diese eine
// `services/geometry`-Funktion nach `model/`, weil die Slab-Mesh
// **adapter-seitig** (STL-Tessellation über den `GeometryKernelPort`) entsteht
// und **danach** um das kern-gelieferte `baseZ` verschoben wird — der Adapter
// wendet die pure Util auf sein **eigenes** Mesh an, der Kern hält kein Mesh.
// Header-only (adapter-erreichbar ohne `services/geometry`-Import).
inline TriangleMesh translateMeshZ(TriangleMesh mesh, double dz) {
    for (std::size_t i = 2; i < mesh.positions.size(); i += 3) {
        mesh.positions[i] += dz;
    }
    return mesh;
}

}  // namespace bcad::hexagon::model
