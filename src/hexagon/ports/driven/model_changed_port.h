#pragma once

#include "hexagon/model/wall.h"  // WallId, StoreyId

namespace bcad::hexagon::ports::driven {

// Operations-Art einer Modell-Änderung. Vokabular deckungsgleich mit
// dem OTel-Span `bcad.geometry.rebuild` (`element_id`, `op` —
// spec/spezifikation.md §5).
enum class ModelChangeOp {
    StoreyAdded,
    WallAdded,
    WallThicknessChanged,
    WallHeightChanged,
    // Footprint-Folgeänderung einer NICHT direkt mutierten Nachbar-Wand
    // (Eckenschluss LH-FA-WAL-006: Wand-Anlage/Stärke-Änderung ändert
    // die Eck-Geometrie der Nachbarn) — Mehr-Element-Update gemäß
    // ADR-0008 §3; Reihenfolge: auslösende Op → Nachbarn einzeln →
    // RoomsChanged (spez. §1 D3-002.a).
    WallGeometryChanged,
    RoomsChanged,  // Raum-Stand des Geschosses neu erkannt (ADR-0007/0008)
    // Dach eines Geschosses angelegt/geändert/entfernt (LH-FA-ROF-*,
    // ADR-0011 #6 neuer Bauteil-Typ → neuer `op`). Storey-bezogen: der
    // Beobachter lädt die Dächer des Stands neu (spez. §1 D3-002.a).
    RoofChanged,
};

// Push-Notify-Meldung (ADR-0008): WAS sich geändert hat (`op`) und
// WELCHES Element. Kein Zustands-Payload — den aktualisierten Stand
// holt der Beobachter über die Abfrage-Ports (Pull-State).
struct ModelChange {
    ModelChangeOp op{};
    model::StoreyId storey_id{};  // betroffenes Geschoss (immer gesetzt)
    model::WallId wall_id{};      // gesetzt bei Wall*-Ops
};

// Driven Port (ADR-0001/0008): Beobachter der Modell-Änderungen
// (2D-/3D-Sicht, OBJ-003). Aufruf erfolgt synchron NACH dem
// transaktionalen Commit und allen Post-Commit-Schritten
// (Raum-Re-Detektion) — ein Pull im Callback sieht einen konsistenten
// Stand. Vertrags-Pflichten (ADR-0008 #6): Implementierungen sollen
// nicht werfen (der Service kapselt trotzdem) und dürfen im Callback
// abfragen, aber keine Mutationen auslösen (Re-Entranz-Verbot).
class ModelChangedPort {
public:
    virtual ~ModelChangedPort() = default;
    virtual void onModelChanged(const ModelChange& change) = 0;
};

}  // namespace bcad::hexagon::ports::driven
