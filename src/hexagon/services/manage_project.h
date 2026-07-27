#pragma once

#include <filesystem>

#include "hexagon/model/building.h"
#include "hexagon/model/layer.h"  // LayerId
#include "hexagon/model/wall.h"   // StoreyId
#include "hexagon/ports/driven/project_repository_port.h"
#include "hexagon/ports/driving/manage_project_port.h"
#include "hexagon/ports/driving/project_session_port.h"
#include "hexagon/services/structure_edit_service.h"

namespace bcad::hexagon::services {

// slice-054: `DrawingTargetSinks` und `DrawingTargetResolution` sind mit dem
// `ManageProjectPort` in die Port-Schicht gezogen — sie stehen im Port-Vertrag.
// Die Aliase halten die bestehenden Aufrufer und Orakel **wortgleich**; das ist
// der Invarianz-Beleg des Slice, nicht Bequemlichkeit.
using ports::driving::DrawingTargetResolution;
using ports::driving::DrawingTargetSinks;

// Save-Use-Case (slice-047a; seit slice-054 die Implementierung hinter
// `ManageProjectPort::saveProject`): baut die
// **kern-abgeleiteten** write-derived Persistenz-Skalare (`PersistedDerivations`:
// `rise` je Treppe, aus `resolveStoreyHeight` + `stairRiseMm`) aus dem `Building`
// und speichert **atomar** über den `ProjectRepositoryPort` (der Adapter
// serialisiert nur, ADR-0020). **Fail-closed:** ein danglendes `from_storey`
// wirft neutral **vor** dem `save` — **kein** Teil-Speichern, Zieldatei unverändert.
//
// Promotet aus dem früheren Test-Helfer (`save_project_test_helper.h`) in die
// Produktion (framework-frei; nutzt nur den Port + Kern-Geometrie, ADR-0001) —
// die **einzige** Save-Ableitungs-Logik, von CLI (slice-047a) und GUI (047b)
// **geteilt**, damit sie testbar außerhalb des coverage-ausgenommenen main/GUI liegt.
void saveProject(const ports::driven::ProjectRepositoryPort& repository,
                 const model::Building& building,
                 const std::filesystem::path& path);

// Open-Use-Case (slice-047b) — symmetrische Gegen-Naht zu `saveProject`: lädt
// über den `ProjectRepositoryPort` und übergibt das Ergebnis dem Service, der
// das Modell **samt abgeleiteter Zustände** ersetzt (`replaceBuilding`).
// Fehler (fehlende/korrupte Datei, unbaubare Geometrie) kommen als neutrale
// `std::runtime_error` durch — der Aufrufer entscheidet über die Anzeige;
// bei einem Fehler bleibt der bisherige Modell-Stand unverändert.
//
// Zweck der Naht (wie bei `saveProject`): der Lade-Pfad ist damit **testbar
// außerhalb** des coverage-ausgenommenen `main`/GUI — der Datei-Dialog bleibt
// die einzige untestbare Schicht darüber.
//
// **Teil des Use-Case, nicht des Aufrufers** (slice-047, Verify-Finding B4): nach
// dem Tausch löst `openProject` das **Zeichen-Ziel** neu auf und meldet es über
// `sinks`. Vorher lag dieser Schritt als Lambda im coverage-ausgenommenen `main`
// und wurde von **keinem** Sensor ausgeführt — sein Verlust blieb unbemerkt
// (Gegenprobe des Verifiers: Aufruf entfernt → 280/280 grün). Jetzt fällt er mit
// dem Use-Case zusammen und ist damit orakel-gedeckt.
DrawingTargetResolution openProject(
    StructureEditService& service,
    const ports::driven::ProjectRepositoryPort& repository,
    const std::filesystem::path& path, const DrawingTargetSinks& sinks = {});

// Baut ein **neues, leeres Projekt** (slice-052b, LH-FA-BLD-001) — reine
// Kern-Funktion, damit die fachliche Regel nicht im coverage-ausgenommenen
// Composition-Root landet:
//
// - **genau ein Geschoss** mit `model::kDefaultStoreyHeightMm` (spez. §3),
// - **eine Zeichen-Ebene**, damit die einzige heute erreichbare
//   Benutzer-Mutation (Hilfslinie) möglich ist — ein neues Projekt darf keine
//   Sackgasse sein (L3-Entscheidung des Slice).
//
// Kein Widerspruch zu „Öffnen ist lesend": jene Regel schützt den Inhalt einer
// **fremden Datei** vor stiller Ergänzung. Hier gibt es keine Datei — b-cad
// definiert, was ein neues Projekt enthält.
model::Building newProjectModel();

// Erfüllt den `ManageProjectPort` (slice-054) — die seit dem Bootstrap in
// `spec/architecture.md` §1.1 deklarierte Ziel-Form der Projekt-Use-Cases.
//
// **Was diese Klasse hinzufügt, ist ausschließlich die Naht:** die
// Infrastruktur (Repository, Struktur-Service) wird hier **einmal** verdrahtet,
// damit der Vertrag darüber sie nicht mehr führen muss. Ein Treiber-Adapter
// (`adapters/ui/command/`) kann die Use-Cases dadurch über den Port rufen, ohne
// `services/` oder `ports/driven/` zu sehen — die Kante, die
// `.a-check.yml` ihm verbietet. Vorher blieb nur der Composition-Root, und
// jede Entscheidung an diesen Aufrufen landete im coverage-ausgenommenen
// `main.cpp` (slice-047-B4 · 052-MED-2/3 · 052a-HIGH-1 · 053-HIGH-1).
//
// **Kein neues Verhalten:** beide Methoden delegieren an die freien Funktionen
// oben, die die geteilte Ableitungs-Logik bleiben (CLI, GUI, Persistenz-Tests).
//
// **Lebensdauer:** hält Referenzen — der Composition-Root muss Service und
// Repository überleben lassen (Muster der übrigen Kern-Services).
class ManageProjectService : public ports::driving::ManageProjectPort {
public:
    // `session` darf `nullptr` sein (CLI/Tests ohne Sitzung) — aber **ohne
    // Default** (Lehre aus slice-053: ein Default-Argument kann ein Orakel
    // aushebeln; hier soll jede Aufrufstelle sichtbar sagen, ob sie eine
    // Sitzung führt).
    ManageProjectService(StructureEditService& service,
                         const ports::driven::ProjectRepositoryPort& repository,
                         ports::driving::ProjectSessionPort* session)
        : service_(service), repository_(repository), session_(session) {}

    DrawingTargetResolution openProject(
        const std::filesystem::path& path,
        const DrawingTargetSinks& sinks) override;

    // slice-052b: legt `newProjectModel()` als Sitzungs-Stand an, setzt die
    // Sitzung zurueck (Pfad weg) und loest das Zeichen-Ziel neu auf.
    DrawingTargetResolution newProject(
        const DrawingTargetSinks& sinks) override;

    // Speichert den Stand, den der **Struktur-Service** hält — deshalb braucht
    // der Vertrag kein `Building` und ein `ui_command`-Handler keinen
    // `StructureEditService` (slice-054, Plan §2.1).
    void saveProject(const std::filesystem::path& path) override;

private:
    StructureEditService& service_;
    const ports::driven::ProjectRepositoryPort& repository_;
    // slice-052a: nach **erfolgreichem** Öffnen/Speichern ist der neue Stand der
    // persistierte. Der Use-Case meldet es selbst — läge die Meldung beim
    // Aufrufer, könnte er sie vergessen, und der Sitzungs-Zustand wäre falsch,
    // ohne dass ein Orakel es sieht.
    ports::driving::ProjectSessionPort* session_;
};

}  // namespace bcad::hexagon::services
