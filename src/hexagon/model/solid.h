#pragma once

namespace bcad::hexagon::model {

// Neutraler Geometrie-Wert: das Ergebnis einer Extrusion über den
// `GeometryKernelPort`. Bewusst OHNE OCC-Typ — kein
// OpenCascade-Datentyp verlässt den Geometrie-Adapter (ADR-0001/0002).
// Für ein gerades Wand-Segment gilt analytisch
// volume = länge · stärke · höhe; das macht den OCC-Adapter (003b) über
// das gemessene Volumen testbar.
struct Solid {
    double volume_mm3{};
};

}  // namespace bcad::hexagon::model
