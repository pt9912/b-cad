#pragma once

#include <optional>

#include "hexagon/model/wall.h"         // WallId
#include "hexagon/model/wall_params.h"  // WallParams
#include "hexagon/ports/driving/plan_view_port.h"

namespace bcad::adapters::ui::command {

// driving-Seite der ui (ADR-0019 Option A): kapselt die **schmale** Abfrage der
// 2D-Lese-Naht (`wallParams`, slice-057). Der Composition-Root verdrahtet sie
// als `std::function` ins Fenster — **kein** `command/ → view/`-Include; der
// Driving-Port-Include lebt hier in `command/` (Richtungs-Trennung).
//
// **Warum überhaupt eine eigene Abfrage** (ADR-0021 E15): der Grundriss-Werttyp
// reist im Ableitungs-Bündel zu den Export-Adaptern und darf nicht um
// Anzeige-Felder wachsen; das Domänen-Objekt direkt zu lesen hebelte die
// Lese-Naht aus. Beides ist **Bauvorschrift, kein Orakel** — eine
// Implementierung, die die Werte anders bezöge, lieferte dieselben Zahlen.
//
// **Total:** unbekannte Id ⇒ **kein Wert** (nicht: Default-Werte, nicht: Wurf).
// Das ist der Gegensatz zum Bearbeitungs-Weg, der wirft — wer diese Naht als
// Vorbild für den Schreib-Weg nimmt, baut die fehlende Barriere ein.
class WallParamsSource {
public:
    explicit WallParamsSource(
        const hexagon::ports::driving::PlanViewPort& port)
        : port_(port) {}

    std::optional<hexagon::model::WallParams> wallParams(
        hexagon::model::WallId id) const {
        return port_.wallParams(id);
    }

private:
    const hexagon::ports::driving::PlanViewPort& port_;
};

}  // namespace bcad::adapters::ui::command
