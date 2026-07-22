#pragma once

#include <optional>
#include <string>

namespace bcad::hexagon::model {

// Starker Id-Typ (enum class, Muster MaterialId/WallId): nicht implizit nach
// int konvertierbar (clang-tidy bugprone-easily-swappable).
enum class LayerId : int {};

// Ebene (Layer, LH-FA-DRW-006): benannte, projekt-eindeutige Organisations-
// Ebene mit Sichtbarkeit (optional Sperre und Farbe). Die `visible`-Eigenschaft
// steuert die Export-Sichtbarkeit der zugeordneten Hilfslinien (2D-Grundriss,
// spez. §1 LH-FA-DRW-005.a) — NICHT die 3D-Viewer-Sicht. `locked` ist
// daten-durabel, in dieser Ausbaustufe aber nicht verhaltenswirksam (kein
// interaktiver 2D-Canvas). Pure Domänen-Werte, framework-frei (ADR-0001).
struct Layer {
    LayerId id{};
    std::string name;                      // Pflicht (DRW-006: leer → abgelehnt), projekt-eindeutig
    bool visible{true};
    bool locked{false};
    std::optional<std::string> color_hex;  // optional (DRW-006)
};

}  // namespace bcad::hexagon::model
