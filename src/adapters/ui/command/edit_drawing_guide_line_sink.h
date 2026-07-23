#pragma once

#include <optional>

#include "hexagon/model/guide_line.h"
#include "hexagon/model/layer.h"      // LayerId
#include "hexagon/model/point2d.h"
#include "hexagon/model/segment.h"
#include "hexagon/model/wall.h"       // StoreyId
#include "hexagon/ports/driving/edit_drawing_port.h"

namespace bcad::adapters::ui::command {

// driving-Seite der ui (ADR-0019, Option A) + **der erste UI-Mutator**: kapselt
// den `EditDrawingPort`-Schreibpfad der 2D-Zeichenfläche. Baut aus dem gezogenen
// Segment + dem **aktiven Geschoss/der aktiven Ebene** das `GuideLine`-Prototyp
// und ruft `addGuideLine` (der Service vergibt die `GuideLineId`; Entartung
// [Anfang == Ende] / unbekannte Ebene / unbekanntes Geschoss → Ablehnung, kein
// Wert — Modell unverändert). Der Composition-Root verdrahtet `addGuideLine` als
// `std::function` in den `view/`-Canvas — **kein** `command/ → view/`-Include; der
// `EditDrawingPort`-Include lebt hier in `command/` (Richtungs-Trennung, ADR-0019
// Entscheidung 5). Aktives Geschoss/Ebene sind v1 fix (Composition-Root injiziert;
// interaktive Auswahl ist ein ADR-0019-Re-Eval, eigener Slice).
class EditDrawingGuideLineSink {
public:
    EditDrawingGuideLineSink(hexagon::ports::driving::EditDrawingPort& port,
                             hexagon::model::StoreyId storey,
                             hexagon::model::LayerId layer)
        : port_(port), storey_(storey), layer_(layer) {}

    std::optional<hexagon::model::GuideLineId> addGuideLine(
        hexagon::model::Point2D start, hexagon::model::Point2D end) const {
        hexagon::model::GuideLine prototype;
        prototype.storey_id = storey_;
        prototype.layer_id = layer_;
        prototype.segment = hexagon::model::Segment{start, end};
        return port_.addGuideLine(prototype);
    }

private:
    hexagon::ports::driving::EditDrawingPort& port_;
    hexagon::model::StoreyId storey_;
    hexagon::model::LayerId layer_;
};

}  // namespace bcad::adapters::ui::command
