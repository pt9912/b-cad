#pragma once

#include <optional>
#include <string>

#include "hexagon/model/guide_line.h"  // GuideLine, GuideLineId
#include "hexagon/model/layer.h"       // Layer, LayerId

namespace bcad::hexagon::ports::driving {

// Driving Port (ADR-0001, ADR-0018 §Entscheidung 2): Use-Case „2D-Zeichnen"
// (Hilfslinien + Ebenen, LH-FA-DRW-005/006). EIGENER Port — eine eigene
// Use-Case-Familie (Annotation/Zeichnung), NICHT eine Erweiterung des Bauteil-
// Editier-Ports EditStructurePort (Muster EvaluatePort: eigener Port für eine
// eigene Familie). Zeichen-Mutationen melden KEINEN `op` (Material-Präzedenz,
// ADR-0008; die Beobachtung läuft über Persistenz-Round-Trip + 2D-Export).
// Ablehnungen sind die E-VAL-001-Rejection-Lesart (Modell UNVERÄNDERT), KEINE
// Klemmung (spec/spezifikation.md §4).
class EditDrawingPort {
public:
    virtual ~EditDrawingPort() = default;

    // --- Ebenen (LH-FA-DRW-006) ---

    // Legt eine Ebene an (`id` vom Service vergeben). Abgelehnt (kein Wert),
    // wenn der Name leer/whitespace-only ist ODER projekt-weit bereits vergeben
    // (uq_layers_project_name) — Modell unverändert.
    virtual std::optional<model::LayerId> addLayer(
        const model::Layer& prototype) = 0;

    // Benennt eine Ebene um. `false` bei unbekannter Id, leerem/whitespace-Namen
    // oder Namens-Kollision mit einer anderen Ebene (Modell unverändert).
    virtual bool renameLayer(model::LayerId id, const std::string& name) = 0;

    // Schaltet die Sichtbarkeit einer Ebene (Export-Filter für ihre
    // Hilfslinien). `false` bei unbekannter Id.
    virtual bool setLayerVisible(model::LayerId id, bool visible) = 0;

    // Entfernt eine Ebene. `on_delete: restrict` (ADR-0018): eine noch von einer
    // Hilfslinie referenzierte Ebene ist NICHT löschbar → `false`, Modell
    // unverändert (kein stiller Verlust); ebenso bei unbekannter Id.
    virtual bool removeLayer(model::LayerId id) = 0;

    // --- Hilfslinien (LH-FA-DRW-005) ---

    // Legt eine Hilfslinie an (`id` vom Service vergeben). Abgelehnt (kein Wert)
    // bei Entartung (Anfang = Ende), unbekannter Ebene oder unbekanntem Geschoss
    // — Modell unverändert.
    virtual std::optional<model::GuideLineId> addGuideLine(
        const model::GuideLine& prototype) = 0;

    // Entfernt eine Hilfslinie. `false` bei unbekannter Id.
    virtual bool removeGuideLine(model::GuideLineId id) = 0;
};

}  // namespace bcad::hexagon::ports::driving
