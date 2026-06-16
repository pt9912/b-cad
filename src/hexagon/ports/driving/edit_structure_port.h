#pragma once

#include <optional>

#include "hexagon/model/footprint.h"  // Footprint (Slab-Ausschnitt)
#include "hexagon/model/material.h"  // Material, MaterialId
#include "hexagon/model/opening.h"  // OpeningId, OpeningKind, SwingDirection
#include "hexagon/model/roof.h"     // Roof, RoofId, RoofType
#include "hexagon/model/segment.h"
#include "hexagon/model/slab.h"     // Slab, SlabId
#include "hexagon/model/stair.h"    // Stair, StairId
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

    // --- Dächer: Sattel/Walm/Pult (LH-FA-ROF-*, ADR-0011) ---

    // Legt ein Dach über dem rechteckigen Grundriss des `prototype` an
    // (`type`, `origin`, `width_mm`, `depth_mm`, `base_z_mm`,
    // `pitch_deg`, `overhang_mm` — `id` wird vom Service vergeben).
    // Neigung/Überstand werden auf §3 geklemmt; ein degenerierter
    // Grundriss (Seite < Toleranz) wird abgelehnt (kein Wert).
    virtual std::optional<model::RoofId> addRoof(const model::Roof& prototype) = 0;

    // Setzt Neigung/Überstand; klemmt auf §3 (`E-VAL-001`); meldet
    // `RoofChanged` (LH-FA-ROF-004/005).
    virtual ParamResult setRoofPitch(model::RoofId roof, double deg) = 0;
    virtual ParamResult setRoofOverhang(model::RoofId roof, double mm) = 0;

    // Wechselt die Dachform (LH-FA-ROF-001..003); meldet `RoofChanged`.
    virtual void setRoofType(model::RoofId roof, model::RoofType type) = 0;

    // Entfernt ein Dach; meldet `RoofChanged`. `false` bei unbekanntem Dach.
    virtual bool removeRoof(model::RoofId roof) = 0;

    // --- Platten: Decken/Fundament (LH-FA-SLB-*/FND-*, ADR-0011) ---

    // Legt eine Platte über dem Grundriss des `prototype` an (`type`,
    // `footprint`, `thickness_mm`, optionale `cutouts`; `id`/`storey_id`
    // aus dem Prototyp). Dicke/Tiefe gegen §3 geklemmt (typabhängig);
    // ein degenerierter Grundriss oder ein unbekanntes Geschoss wird
    // abgelehnt (kein Wert). Meldet `SlabChanged`.
    virtual std::optional<model::SlabId> addSlab(const model::Slab& prototype) = 0;

    // Setzt die Platten-Dicke/-Tiefe; klemmt typabhängig auf §3
    // (`E-VAL-001`); meldet `SlabChanged` (LH-FA-SLB-002/FND-002).
    virtual ParamResult setSlabThickness(model::SlabId slab, double mm) = 0;

    // Fügt einen Ausschnitt hinzu (LH-FA-SLB-003); auf den Platten-Umriss
    // begrenzt; meldet `SlabChanged`. `false` bei unbekannter Platte.
    virtual bool addSlabCutout(model::SlabId slab,
                               const model::Footprint& cutout) = 0;

    // Entfernt eine Platte; meldet `SlabChanged`. `false` bei unbekannt.
    virtual bool removeSlab(model::SlabId slab) = 0;

    // --- Treppen: gerade einläufig (LH-FA-STR-*, ADR-0011) ---

    // Legt eine gerade Treppe aus dem `prototype` an (`from_storey_id`/
    // `to_storey_id`, `start`, `width_mm`, `step_count`, `tread_mm`; `id` vom
    // Service). width/step_count/tread werden gegen §3 geklemmt. **Abgelehnt**
    // (kein Wert), wenn die Zwei-Geschoss-Spanne ungültig ist: `from == to`,
    // ein Geschoss unbekannt, oder die `from_storey`-Höhe ≤ Toleranz. Meldet
    // `StairChanged` (an `from_storey`).
    virtual std::optional<model::StairId> addStair(
        const model::Stair& prototype) = 0;

    // Setzt die Laufbreite; klemmt auf §3 (`E-VAL-001`); meldet `StairChanged`
    // (LH-FA-STR-003).
    virtual ParamResult setStairWidth(model::StairId stair, double mm) = 0;

    // Setzt die Stufenanzahl (LH-FA-STR-002); klemmt auf den ganzzahligen
    // §3-Bereich. `applied_mm` trägt die geklemmte Stufenzahl; Status
    // `Clamped`/`Accepted` (nie `Rejected` — `int` ist endlich). Meldet
    // `StairChanged`.
    virtual ParamResult setStairStepCount(model::StairId stair, int count) = 0;

    // Entfernt eine Treppe; meldet `StairChanged`. `false` bei unbekannt.
    virtual bool removeStair(model::StairId stair) = 0;

    // --- Material: projekt-eigene Materialien (LH-FA-MAT-*, ADR-0006/0012) ---
    // Material ist eine Bauteil-Eigenschaft, KEINE Geometrie; Material-
    // Mutationen melden KEINEN `op` (kein gerendertes Szenen-Korrelat in
    // welle-3 — Pull-Konsum durch die Auswertung, ADR-0012-Geist).

    // Legt ein Material an (LH-FA-MAT-001); `id` vom Service vergeben.
    // Abgelehnt (kein Wert), wenn der Name leer/whitespace-only ist
    // (MAT-001 Negative).
    virtual std::optional<model::MaterialId> addMaterial(
        const model::Material& prototype) = 0;

    // Ändert Name/Kategorie/Kennwerte eines Materials (LH-FA-MAT-001/005/006).
    // `false` bei unbekannter Id oder leerem/whitespace-Namen (Modell
    // unverändert).
    virtual bool updateMaterial(model::MaterialId id,
                                const model::Material& values) = 0;

    // Entfernt ein Material (LH-FA-MAT-001). `on_delete: restrict`-Treue
    // (ADR-0006): ein **noch zugewiesenes** Material ist NICHT löschbar →
    // `false`, Modell unverändert (kein stiller Verlust der Zuweisung); ebenso
    // bei unbekannter Id. Erst löschbar, wenn kein Bauteil es referenziert.
    virtual bool removeMaterial(model::MaterialId id) = 0;

    // Weist einem Bauteil (Wand/Dach/Decke) ein Material zu bzw. ab
    // (LH-FA-MAT-003, Override). `std::nullopt` = „kein Material" (immer ok).
    // `false` bei unbekanntem Bauteil oder unbekannter Material-Id (Modell
    // unverändert). Kein Fehler bei „kein Material" (MAT-003 Negative).
    virtual bool setWallMaterial(model::WallId wall,
                                 std::optional<model::MaterialId> material) = 0;
    virtual bool setRoofMaterial(model::RoofId roof,
                                 std::optional<model::MaterialId> material) = 0;
    virtual bool setSlabMaterial(model::SlabId slab,
                                 std::optional<model::MaterialId> material) = 0;
};

}  // namespace bcad::hexagon::ports::driving
