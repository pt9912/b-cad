#pragma once

#include <optional>
#include <vector>

namespace bcad::hexagon::model {

// 2D-Grundriss-Sicht eines `Building` (ADR-0016, slice-025b; ADR-0019/ADR-0020:
// die reine 2D-Projektion ist Kern-Rechnung, ihre Werttypen leben im `model/`-
// Kern — so darf das kern-berechnete `DerivedGeometry`-Bündel sie tragen und der
// Kern-Read-Port `PlanViewPort` sie liefern, ohne dass ein Adapter die Projektion
// selbst ableitet). Reine Werttypen (kein OCC/Qt); Längeneinheit mm.

// Woher ein Segment stammt (ADR-0021 E11, slice-057). Die Projektion mischt
// Wand-Achsen und sichtbare Hilfslinien in EINE Liste; ohne diese Angabe hätte
// eine Treffer-Prüfung nur anonyme Koordinaten zu benennen.
enum class PlanSegmentKind {
    WallAxis,
    GuideLine,
};

// Art + Identität der Entität, aus der ein Segment entstanden ist. `id` ist der
// Zahlwert der starken Id (`WallId` bzw. `GuideLineId`) — welche der beiden,
// sagt `kind`. Bewusst nicht als Variante: der Werttyp reist im
// `DerivedGeometry`-Bündel zu den Export-Adaptern und bleibt darum trivial.
struct PlanSegmentOrigin {
    PlanSegmentKind kind{};
    int id{};

    friend bool operator==(const PlanSegmentOrigin&,
                           const PlanSegmentOrigin&) = default;
};

// Ein Achs-Segment in Modell-Millimetern.
//
// **Die Herkunft ist `optional`, und das ist eine Entscheidung** (slice-057 §2.1):
// `PlanSegment` wird an mehreren Stellen als Aggregat in `{x1, y1, x2, y2}`-
// Klammerform gebaut — auch in Bestands-Tests. Als *Wert* initialisierte die
// Klammerform ein vergessenes Feld still auf den Nullwert der Aufzählung; jedes
// Segment gälte dann als Wand-Achse, also **falsch beschriftet** statt leer. Als
// `optional` ist ein vergessenes Feld **leer** — und Leere ist prüfbar. Die
// Zusage „`projectPlan` liefert nie ein Segment ohne Herkunft" ist damit ein
// Orakel und keine Hoffnung.
struct PlanSegment {
    double x1_mm{};
    double y1_mm{};
    double x2_mm{};
    double y2_mm{};
    std::optional<PlanSegmentOrigin> origin{};
};

// Die Achsen eines Geschosses (Geschoss-Reihenfolge des Modells).
struct StoreyPlan {
    int storey_id{};
    std::vector<PlanSegment> segments;
};

// Der projizierte Grundriss: je Geschoss ein `StoreyPlan` (in Modell-Reihenfolge;
// ein Geschoss ohne Wände trägt eine leere Segment-Liste) + die gemeinsame
// Bounding-Box. `has_geometry == false` ⇔ kein Segment (leeres Modell) → die Box
// ist dann degeneriert (0,0,0,0) und der Aufrufer zeichnet eine leere Seite
// (Totalität).
struct PlanView {
    std::vector<StoreyPlan> storeys;
    double min_x_mm{};
    double min_y_mm{};
    double max_x_mm{};
    double max_y_mm{};
    bool has_geometry{false};
};

}  // namespace bcad::hexagon::model
