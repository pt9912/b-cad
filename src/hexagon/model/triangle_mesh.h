#pragma once

#include <vector>

namespace bcad::hexagon::model {

// Neutraler Darstellungs-Wert (ADR-0009 (b)): tesselliertes Dreiecksnetz
// eines Solids. Bewusst OHNE OCC-/Qt-Typ — die Tessellation entsteht im
// Geometrie-Adapter, gerendert wird im UI-Adapter; dazwischen fließen
// nur diese puren Daten (kein OCC verlässt den Geometrie-Adapter,
// ADR-0001/0002).
//
// Layout: flache Tripel-Arrays. `positions`/`normals` tragen je Vertex
// x,y,z (mm bzw. Einheitsvektor); `indices` referenziert Vertices in
// Dreier-Gruppen (ein Dreieck je Tripel). Flat-Shading-Konvention: je
// Dreieck eigene Vertices mit Flächennormale (keine geteilten Kanten) —
// passend für kantige Bauteil-Solids.
struct TriangleMesh {
    std::vector<double> positions;
    std::vector<double> normals;
    std::vector<int> indices;

    bool empty() const { return indices.empty(); }
    int vertexCount() const { return static_cast<int>(positions.size() / 3); }
    int triangleCount() const { return static_cast<int>(indices.size() / 3); }
};

}  // namespace bcad::hexagon::model
