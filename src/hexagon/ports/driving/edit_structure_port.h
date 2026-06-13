#pragma once

#include <optional>

#include "hexagon/model/opening.h"  // OpeningId, OpeningKind, SwingDirection
#include "hexagon/model/segment.h"
#include "hexagon/model/wall.h"  // WallId, StoreyId

namespace bcad::hexagon::ports::driving {

// Ausgang einer parametrischen Setzung (E-VAL-001-Familie, siehe
// spec/spezifikation.md §4).
enum class ParamStatus {
    Accepted,  // im Wertebereich übernommen — Modell geändert
    Clamped,   // außerhalb [min, max] → auf Grenzwert geklemmt (E-VAL-001), Modell geändert
    Rejected,  // ungültige (nicht-endliche) Eingabe → Modell UNVERÄNDERT (E-VAL-001)
};

struct ParamResult {
    double applied_mm{};  // gesetzter Wert; bei Rejected der unveränderte Ist-Wert
    ParamStatus status{ParamStatus::Accepted};
};

// Driving Port (ADR-0001): Use-Case „Gebäudestruktur bearbeiten".
// Bauteil-Parametrik für Geschosse und Wände (OBJ-002).
class EditStructurePort {
public:
    virtual ~EditStructurePort() = default;

    // Legt ein Geschoss mit gegebener Höhe (mm) an, gibt dessen Id zurück
    // (LH-FA-FLR-001).
    virtual model::StoreyId addStorey(double height_mm) = 0;

    // Erzeugt eine Wand entlang `seg` mit Default-Stärke/-Höhe
    // (LH-FA-WAL-001). Ein Null-Längen-Segment (Länge < Toleranz) wird
    // verworfen (WAL-001 Boundary) — Rückgabe ohne Wert.
    virtual std::optional<model::WallId> addWall(model::StoreyId storey,
                                                 model::Segment seg) = 0;

    // Setzt die Wandstärke; klemmt auf [50, 1000] mm (LH-FA-WAL-002).
    virtual ParamResult setWallThickness(model::WallId wall, double mm) = 0;

    // Setzt die Wandhöhe; klemmt auf [500, 10000] mm (LH-FA-WAL-003).
    virtual ParamResult setWallHeight(model::WallId wall, double mm) = 0;

    // --- Wandöffnungen: Türen/Fenster (LH-FA-DOR-*/WIN-*, ADR-0011) ---

    // Platziert eine Tür (LH-FA-DOR-001) bzw. ein Fenster (LH-FA-WIN-001)
    // an Wand `wall`, `offset_mm` entlang der Wandachse (Nahkante).
    // Default-Maße (spez. §3); die Position wird so geklemmt, dass die
    // Öffnung in die Wand passt (LH-FA-DOR-001 Boundary). Rückgabe ohne
    // Wert, wenn die Wand unbekannt ist oder selbst die Mindestbreite
    // nicht passt (Negative) bzw. die Geometrie scheitert (E-GEO-002).
    virtual std::optional<model::OpeningId> addDoor(model::WallId wall,
                                                    double offset_mm) = 0;
    virtual std::optional<model::OpeningId> addWindow(model::WallId wall,
                                                      double offset_mm) = 0;

    // Setzt Öffnungs-Breite/-Höhe; klemmt auf den kind-spezifischen
    // Bereich (Tür/Fenster, spez. §3, `E-VAL-001`). Breite zusätzlich so
    // begrenzt, dass die Öffnung in der Wand bleibt.
    virtual ParamResult setOpeningWidth(model::OpeningId opening, double mm) = 0;
    virtual ParamResult setOpeningHeight(model::OpeningId opening, double mm) = 0;

    // Setzt die Brüstungshöhe eines Fensters (LH-FA-WIN-004); klemmt auf
    // [0, 2000] mm. Für eine Tür: `Rejected` (Türen haben keine Brüstung).
    virtual ParamResult setWindowSill(model::OpeningId opening, double mm) = 0;

    // Verschiebt eine Öffnung entlang ihrer Wand (LH-FA-DOR-002/WIN-002);
    // Position auf den gültigen Bereich geklemmt. `false`, wenn die
    // Öffnung unbekannt ist oder die Geometrie scheitert.
    virtual bool moveOpening(model::OpeningId opening, double offset_mm) = 0;

    // Setzt den Tür-Anschlag (LH-FA-DOR-003) — gespeicherte Eigenschaft,
    // keine Geometrie-Änderung in welle-2 (ADR-0011 Re-Eval).
    virtual void setDoorSwing(model::OpeningId opening,
                              model::SwingDirection swing) = 0;

    // Entfernt eine Öffnung; die Wand schließt sich wieder
    // (LH-FA-DOR-004/WIN-005 Negative). `false` bei unbekannter Öffnung.
    virtual bool removeOpening(model::OpeningId opening) = 0;
};

}  // namespace bcad::hexagon::ports::driving
