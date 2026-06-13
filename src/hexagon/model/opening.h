#pragma once

#include "hexagon/model/wall.h"  // WallId

namespace bcad::hexagon::model {

// Starker Id-Typ (enum class): nicht implizit nach int/double konvertierbar
// (clang-tidy bugprone-easily-swappable; Muster WallId/StoreyId).
enum class OpeningId : int {};

// Bauteil-Art einer Wandöffnung (Class-Table-Inheritance im Schema:
// `openings` → `doors`/`windows`, ADR-0006).
enum class OpeningKind { Door, Window };

// Anschlag einer Tür (Öffnungsseite, LH-FA-DOR-003). In welle-2 eine
// gespeicherte, abfragbare Eigenschaft ohne eigenes Türblatt-Solid
// (ADR-0011 Re-Eval-Trigger).
enum class SwingDirection { Left, Right };

// Wand-gehostete Öffnung (Tür/Fenster, ADR-0011 (1)): eigenständiges
// Element mit Referenz auf seine Wirtswand. Position parametrisch entlang
// der Wandachse: `offset_mm` = Abstand vom Wand-Startpunkt zur Nahkante;
// die Öffnung belegt `[offset, offset+width]` entlang der Achse,
// `[sill, sill+height]` über der Standfläche, quer die volle Wandstärke
// (spez. §1 LH-FA-DOR-004.a/WIN-005.a). Pure Werte, framework-frei.
struct Opening {
    OpeningId id{};
    WallId wall_id{};
    OpeningKind kind{OpeningKind::Door};
    double offset_mm{};
    double width_mm{};
    double height_mm{};
    double sill_height_mm{};  // 0 für Türen; Brüstung für Fenster (LH-FA-WIN-004)
    SwingDirection swing{SwingDirection::Left};  // nur Tür (LH-FA-DOR-003)
};

}  // namespace bcad::hexagon::model
