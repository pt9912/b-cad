#pragma once

#include <functional>
#include <optional>
#include <utility>

#include "hexagon/model/point2d.h"
#include "hexagon/model/segment.h"
#include "hexagon/model/wall.h"  // WallId, StoreyId
#include "hexagon/ports/driving/edit_structure_port.h"

namespace bcad::adapters::ui::command {

// Ausgang eines Wand-Zugs — **der Typ lebt hier, in `command/`** (ADR-0021 E5,
// slice-058 §2.3): der `view/`-Canvas sieht ihn NIE. Der Hinweis-TEXT liegt im
// Composition-Root (benannte Grenze des Fenster-Adapters); diese Senke meldet
// den Ausgang als **Wert** über ein eigenes injiziertes Callable.
//
// **Drei Ausgänge, nicht vier:** die beiden Wurf-Quellen (unbekanntes Geschoss ·
// Geometrie-Fehlschlag vor dem Commit) fallen bewusst zusammen — sie wären nur
// über den Ausnahme-TYP unterscheidbar, und den dokumentiert der Port für diesen
// Aufruf nicht. Für den Benutzer ist die Aussage dieselbe: nichts entstanden,
// Modell unverändert.
enum class WallDrawOutcome {
    Created,          // Wand angelegt — die Wand ist der Beleg, kein Hinweis
    RejectedNoWall,   // kein Wert zurück (Null-Länge / nicht-endlich)
    Failed,           // Wurf gefangen — Modell unverändert (transaktional)
};

// driving-Seite der ui (ADR-0019 Option A / ADR-0021 E7): der **zweite**
// UI-Mutator, und er ist **gegenläufig zum ersten**.
//
// `EditDrawingGuideLineSink` lehnt rein **wertbasiert** ab. Der Bauteil-Weg
// **wirft**: `addWall` bei unbekannter Geschoss-Id, und die Geometrie-Berechnung
// **vor** dem Commit bei Fehlschlag (`E-GEO-002`, transaktional — das Modell
// bleibt unverändert). **Diese Senke ist die Fehler-Barriere** (ADR-0021 E10):
// sie fängt den Wurf, statt ihn aus einem Qt-Event-Handler laufen zu lassen.
// **Wer die Bauform der ersten Senke kopiert, baut den Fehler ein.**
//
// **Die Entartung stellt SIE fest, nicht der Canvas** (slice-058 §2.3): der Kern
// verwirft unterhalb der Geometrie-Toleranz (0,1 mm), während bei Maximal-Zoom
// (100 px/mm) benachbarte Pixel 0,01 mm auseinanderliegen — ein Canvas, der an
// seinen eigenen zwei Punkten urteilte, hielte den Zug für gültig, während der
// Kern ihn verwirft: ein **falscher** Hinweis.
class EditStructureWallSink {
public:
    // Meldung des Ausgangs an den Composition-Root (der die Texte hält).
    // Optional: nicht gesetzt heißt „kein Empfänger", nicht „kein Ausgang".
    using OutcomeReport = std::function<void(WallDrawOutcome)>;

    EditStructureWallSink(hexagon::ports::driving::EditStructurePort& port,
                          hexagon::model::StoreyId storey,
                          OutcomeReport report)
        : port_(port), storey_(storey), report_(std::move(report)) {}

    // Ziel-Geschoss neu setzen — dieselbe Notwendigkeit wie beim Zeichen-Ziel
    // (slice-047b): nach einem Projekt-Laden ist die beim Demo-Bau eingefrorene
    // Geschoss-Id i. d. R. ungültig, und `addWall` **wirft** darauf. Der
    // Composition-Root löst sie nach dem Laden neu auf.
    void setTarget(hexagon::model::StoreyId storey) { storey_ = storey; }

    // Legt die Wand über den Bearbeitungs-Port an und meldet den Ausgang.
    // **Total:** kein Wurf verlässt diesen Aufruf (ADR-0021 E10) — der Rückgabe-
    // wert ist für Aufrufer, die ihn brauchen (Tests, spätere Auswahl), der
    // Canvas ignoriert ihn.
    std::optional<hexagon::model::WallId> addWall(
        hexagon::model::Point2D start, hexagon::model::Point2D end) const {
        std::optional<hexagon::model::WallId> created;
        try {
            created = port_.addWall(storey_,
                                    hexagon::model::Segment{start, end});
        } catch (...) {
            // Unbekannte Geschoss-Id ODER Geometrie-Fehlschlag vor dem Commit.
            // Das Modell ist in beiden Fällen unverändert (transaktional).
            // **Bewusst `...` und nicht `const std::exception&`:** die Zusage
            // lautet „kein Wurf verlässt den Ereignis-Pfad" — eine Barriere, die
            // nur die dokumentierten Typen fängt, ist keine Barriere, und was ein
            // aus einem Qt-Event-Handler entweichender Wurf tut, ist nicht unsere
            // Entscheidung. Der Typ trägt hier ohnehin keine Information: beide
            // Quellen bekommen denselben Ausgang (s. o.).
            report(WallDrawOutcome::Failed);
            return std::nullopt;
        }
        report(created.has_value() ? WallDrawOutcome::Created
                                   : WallDrawOutcome::RejectedNoWall);
        return created;
    }

private:
    void report(WallDrawOutcome outcome) const {
        if (report_) {
            report_(outcome);
        }
    }

    hexagon::ports::driving::EditStructurePort& port_;
    hexagon::model::StoreyId storey_;
    OutcomeReport report_;
};

}  // namespace bcad::adapters::ui::command
