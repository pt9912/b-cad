#pragma once

#include <functional>
#include <optional>

#include <QPoint>
#include <QWidget>

#include "adapters/ui/view/view_transform.h"
#include "hexagon/model/guide_line.h"  // GuideLineId
#include "hexagon/model/plan_view.h"
#include "hexagon/model/point2d.h"
#include "hexagon/ports/driven/model_changed_port.h"

class QEvent;
class QFocusEvent;
class QKeyEvent;
class QMouseEvent;
class QPaintEvent;
class QResizeEvent;
class QWheelEvent;

namespace bcad::adapters::ui::view {

// Interaktive 2D-Zeichenfläche (ADR-0019): ein eigenes `view/`-`QWidget`
// (**nicht** der orbit-verdrahtete 3D-`ViewerWidget`), das die 2D-Grundriss-
// Projektion (`PlanView`) des **aktiven Geschosses** zeichnet und Hilfslinien
// per **Links-Zug** ziehen lässt (Press = Anfang, Release = Ende, gemappte mm).
//
// **Port-frei (Plan-Review-Entscheidung Option A):** das Widget hält **keinen**
// Driving-Port und **keinen** `view/`-Sink-Header, sondern
// zwei `std::function`-Callables (Read-Pull `PlanView`, Schreib `addGuideLine`),
// die der Composition-Root aus `ui/command/`-Objekten verdrahtet — **kein**
// `command/ → view/`-Include, der Regel-B-`adapter_sink` bleibt unverändert.
//
// **Refresh:** Beobachter des `ModelChangedPort` (Repaint + Neu-Einrahmen nach
// `op`-Mutationen, z. B. Wand-Änderung); nach dem **eigenen** erfolgreichen
// `addGuideLine` repaintet er **selbst** (Selbst-Refresh, **kein** `op` —
// ADR-0018 §2 „kein op" bleibt unrevidiert).
//
// **Fangen** (LH-FA-DRW-001, slice-048b): Press und Release quantisieren die
// geklickte Bildschirmposition über `snapTarget` auf die exakten mm eines
// nahen Fang-Punktes; außerhalb der Fang-Nähe wird **frei** gezeichnet
// (Raster/Winkel bleiben spätere Slices).
//
// **Fang-Anzeige** (slice-055): das Widget verfolgt die Maus (`setMouseTracking`)
// und hält den aktuellen Fang-Kandidaten als **reinen Widget-Zustand**
// (`snap_preview_`) — kein Modell-Datum, kein `op`, kein Schema. Er entsteht aus
// **demselben** `snapTarget`-Aufruf mit **derselben** Konstante wie die Eingabe-
// Quantisierung (sonst zeigte der Marker auf A und die Linie landete auf B) und
// wird in **beiden** Phasen gepflegt: ohne gedrückte Taste (Anfang) und während
// des Zugs (Ende). Er wird **verworfen**, sobald der Zeiger die Fläche verlässt
// oder sich die Abbildung ändert (Zoom/Resize/Geschoss/Modell-Meldung) — ein in
// mm gehaltener Kandidat zeigte danach auf die falsche Bildschirmstelle.
//
// **Werkzeug-Modus** (ADR-0021 E1, slice-058): derselbe Links-Zug erzeugt je
// nach Modus eine **Hilfslinie** oder eine **Wand**. Der Modus ist reiner
// Widget-Zustand (kein Modell-Datum, nicht persistiert) und als Eigenschaft
// lesbar — die Nachweis-Naht aus E14. **Default ist `GuideLine`**, damit die
// bestehende Bedienung unverändert weiterläuft. Die BEDIENUNG (Aktions-Gruppe)
// liegt im Fenster; der Composition-Root ruft `setToolMode`.
class CanvasWidget final : public QWidget,
                           public hexagon::ports::driven::ModelChangedPort {
public:
    using PlanPull = std::function<hexagon::model::PlanView()>;
    using GuideLineDraw =
        std::function<std::optional<hexagon::model::GuideLineId>(
            hexagon::model::Point2D, hexagon::model::Point2D)>;
    // Wand-Zug: **ohne Rückgabe**. Der Ausgang wird in der `ui/command/`-Senke
    // festgestellt und von DORT gemeldet (ADR-0021 E5/E10, slice-058 §2.3) —
    // der Canvas kennt weder den Ausgangs-Typ noch den Hinweis-Text. Die
    // Entartungs-Grenze liegt beim Kern (0,1 mm), nicht bei den Pixeln.
    using WallDraw =
        std::function<void(hexagon::model::Point2D, hexagon::model::Point2D)>;

    enum class ToolMode { GuideLine, Wall };

    // **Kein Default für `draw_wall`:** ein vergessenes Callable wäre ein
    // Wand-Modus, der still nichts tut. Muster `DrawingTargetSinks` (slice-053):
    // Vergessen ist ein Compile-Fehler.
    CanvasWidget(PlanPull pull, GuideLineDraw draw, WallDraw draw_wall,
                 int active_storey_id, QWidget* parent = nullptr);

    // Werkzeug-Modus setzen/lesen (ADR-0021 E1/E14). Ein Modus-Wechsel bricht
    // eine laufende Geste ab — sonst entstünde aus einem Zug, der als Hilfslinie
    // begann, eine Wand.
    void setToolMode(ToolMode mode);
    ToolMode toolMode() const { return tool_mode_; }

    // Aktives Geschoss neu setzen (slice-047b): nach einem Projekt-Laden ist
    // die beim Demo-Bau eingefrorene Geschoss-Id i. d. R. ungültig — der Canvas
    // filterte dann jede Plan-Zeile weg und bliebe LEER. Der Composition-Root
    // löst das Geschoss nach dem Laden neu auf und setzt es hier nach.
    void setActiveStorey(int active_storey_id);

    // ADR-0008-Callback: `op`-Mutation → neu einrahmen + Repaint einplanen.
    void onModelChanged(
        const hexagon::ports::driven::ModelChange& change) override;

    // Der aktuell angezeigte Fang-Kandidat in Modell-mm, oder `nullopt`.
    // Display-freie Testnaht der Anzeige-AK (Muster `screenToModel`, ADR-0019
    // E3/E7). **Sie belegt nur, WELCHER Punkt angezeigt würde** — dass überhaupt
    // etwas gezeichnet wird, prüft die Tinten-Sonde auf dem offscreen gerenderten
    // Widget (slice-055 §4-8); der Surrogat allein bliebe grün, wenn nie ein
    // Pixel gesetzt würde.
    std::optional<hexagon::model::Point2D> snapPreview() const {
        return snap_preview_;
    }

    // Testbare, display-freie Transformations-Naht (ADR-0019 E3/E7).
    hexagon::model::Point2D screenToModel(const QPoint& p) const {
        return transform_.screenToModel(p);
    }
    const ViewTransform& transform() const { return transform_; }

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void leaveEvent(QEvent* event) override;
    // Gesten-Abbruch, ADR-0021 E12 nennt **zwei** Auslöser: Escape und
    // Fokusverlust. Beide brechen ohne Modell-Mutation ab.
    void keyPressEvent(QKeyEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

private:
    // Bildschirm-Pixel → Modell-mm **mit Fang** (LH-FA-DRW-001): liegt ein
    // Fang-Punkt der frisch gepullten `PlanView` in Fang-Nähe, sind es dessen
    // exakte mm — sonst die freie Abbildung. Der Plan wird **an Ort und Stelle**
    // gepullt (wie im Paint-Pfad bei jedem Repaint); ein zwischengespeicherter
    // Plan wäre neuer Zustand und könnte veralten (Pull-Widget, ADR-0019).
    hexagon::model::Point2D snappedModelPos(const QPoint& cursor_px) const;

    // Setzt `snap_preview_` auf den Fang-Kandidaten unter `cursor_px` — oder
    // löscht ihn, wenn keiner in Reichweite liegt bzw. der Zeiger außerhalb der
    // Fläche steht (bei gedrückter Taste stellt Qt KEIN `leaveEvent` zu, die
    // Move-Ereignisse laufen mit Koordinaten außerhalb `rect()` weiter).
    // Plant bei Änderung einen Repaint ein.
    void updateSnapPreview(const QPoint& cursor_px);

    // Verwirft die Anzeige, weil die Abbildung sich geändert hat und die in mm
    // gehaltene Position keinem bekannten Zeiger-Pixel mehr entspricht.
    void invalidateSnapPreview();

    // Bricht eine laufende Geste ab: **keine** Mutation, nur der Zug-Zustand
    // fällt und die in-Arbeit-Linie verschwindet (ADR-0021 E12). Ohne laufende
    // Geste ein No-op — auch das ist Teil der Zusage („es entsteht keine Wand").
    void cancelDrag();

    PlanPull pull_;
    GuideLineDraw draw_;
    WallDraw draw_wall_;
    ToolMode tool_mode_{ToolMode::GuideLine};  // E1: Default bleibt Hilfslinie
    int active_storey_id_{};
    ViewTransform transform_{};
    bool fitted_{false};  // Fit-to-Bounds beim nächsten Paint nötig?
    bool dragging_{false};
    QPoint drag_start_px_{};   // nur für die visuelle in-Arbeit-Linie
    QPoint drag_current_px_{};
    // Der Zug-Startpunkt in Modell-mm (bei Press gemappt) — überlebt eine
    // Transformations-Änderung (Zoom/Resize) mitten im Zug; die committete
    // Hilfslinie nutzt IHN, nicht die Neu-Abbildung des alten Pixels (MR-009-LOW-2).
    hexagon::model::Point2D drag_start_mm_{};
    // Der angezeigte Fang-Kandidat in Modell-mm (slice-055). Bewusst in mm und
    // nicht in Pixeln: er ist derselbe Wert, der beim Klick übergeben würde.
    // Der Preis ist die Invalidierung bei jeder Transformations-Änderung.
    std::optional<hexagon::model::Point2D> snap_preview_{};
};

}  // namespace bcad::adapters::ui::view
