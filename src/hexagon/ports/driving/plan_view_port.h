#pragma once

#include <optional>

#include "hexagon/model/plan_view.h"
#include "hexagon/model/wall.h"         // WallId
#include "hexagon/model/wall_params.h"  // WallParams

namespace bcad::hexagon::ports::driving {

// Driving Port (ADR-0001, ADR-0019): read-only 2D-Lese-Naht — liefert die
// 2D-Grundriss-Projektion (Wand-Achsen + sichtbare Hilfslinien je Geschoss +
// gemeinsame Bounding-Box) aus dem committeten Modell. Das **2D-Analog zum
// `ViewModelPort`** (der die 3D-Tessellation liefert): reine Query (pull on
// demand, kein …Changed-`op`, keine Mutation), framework-freier Werttyp
// (`model::PlanView`). Eine Quelle für den interaktiven 2D-Canvas (zieht diesen
// Port) UND den 2D-Export (bekommt dieselbe Projektion im `DerivedGeometry`-
// Bündel) — kern-berechnet, kein Adapter leitet 2D-Geometrie ab (ADR-0020).
//
// **Der Vertrag ist mit slice-057 (ADR-0021 E15) ausdrücklich geweitet:** die
// Naht liefert, **was die 2D-Zeichenfläche lesen muss** — die Projektion **und**,
// zu einer benannten Wand, deren **änderbare** Parameter. Letztere sind KEINE
// 2D-Projektion; die Weitung steht hier, statt stillschweigend zu geschehen. Die
// Alternative (ein dritter Read-Port für zwei Zahlen) hätte dasselbe
// Bedeutungs-Problem mit mehr Teilen.
//
// **Beide Abfragen sind read-only und TOTAL** — sie werfen nie. Das unterscheidet
// diese Naht vom Bearbeitungs-Port, der bei unbekannten Bezügen wirft; ein
// geerbtes Wurf-Verhalten wäre eine stille Vertrags-Änderung.
class PlanViewPort {
public:
    virtual ~PlanViewPort() = default;

    // Der projizierte 2D-Grundriss des aktuellen Modells (total; leeres Modell →
    // `has_geometry == false`).
    virtual model::PlanView planView() const = 0;

    // Die **änderbaren** Parameter einer benannten Wand (ADR-0021 E13/E15).
    // Unbekannte Id ⇒ **kein Wert** (nicht: Default-Werte, nicht: Wurf).
    virtual std::optional<model::WallParams> wallParams(
        model::WallId id) const = 0;
};

}  // namespace bcad::hexagon::ports::driving
