#pragma once

#include <filesystem>
#include <optional>

#include "hexagon/model/building.h"

namespace bcad::hexagon::ports::driving {

// Was aus einer Rückfrage folgen kann. Die **Antwort** gibt der Benutzer, die
// **Auswertung** macht der Kern — sonst läge die schärfste Zusage des Slice
// („abbrechen ⇒ unterlassen") im coverage-ausgenommenen Composition-Root.
enum class DiscardAnswer {
    Save,     // erst speichern, dann ausführen
    Discard,  // ohne Speichern ausführen
    Cancel,   // die auslösende Aktion UNTERLASSEN
};

// Ergebnis der Auswertung: was der Aufrufer jetzt tun soll.
enum class DiscardOutcome {
    Proceed,      // die auslösende Aktion ausführen
    SaveThenProceed,  // erst speichern; gelingt das nicht → Abort
    Abort,        // die auslösende Aktion unterlassen
};

// Verdikt vor einer Aktion, die den Sitzungs-Stand verwirft.
enum class DiscardVerdict {
    Proceed,   // nichts Ungesichertes — direkt ausführen
    AskFirst,  // ungesichert — erst fragen
};

// Wohin „Speichern" schreibt.
enum class SaveTargetKind {
    KnownPath,  // die geöffnete/zuletzt gespeicherte Datei
    AskUser,    // noch kein Pfad — Ziel erfragen („Speichern unter…")
};

struct SaveTarget {
    SaveTargetKind kind{SaveTargetKind::AskUser};
    std::filesystem::path path{};  // nur bei `KnownPath` gesetzt
};

// Driving Port (ADR-0001): Use-Case „Sitzungs-Zustand" (slice-052a).
//
// **Warum ein eigener Port neben `ManageProjectPort`** (slice-052a §2.1): der
// `ManageProjectPort` trägt die **Datei**-Use-Cases (öffnen, speichern). Der
// Sitzungs-Zustand ist eine andere Verantwortung — Baseline halten, Verdikt
// bilden, Antwort auswerten — mit eigener Lebensdauer. Zwei Ports, zwei
// Zuständigkeiten.
//
// **Warum überhaupt ein Port:** ohne ihn dürfte nur der Composition-Root die
// Abfragen rufen (`.a-check.yml` verbietet jedem Adapter den
// `hexagon/services/`-Import) — und dort ist alles orakel-los. Genau diese
// Klasse Finding hat die Kette 047 → 052 → 052a → 053 erzeugt.
//
// **Im Vertrag steht keine Entscheidung des Aufrufers:** er fragt das Verdikt
// ab, zeigt den Dialog, reicht die Antwort zurück und **führt aus**. Was aus
// der Antwort folgt, sagt `evaluate`.
class ProjectSessionPort {
public:
    virtual ~ProjectSessionPort() = default;

    // Weicht der aktuelle Stand vom zuletzt persistierten ab? (Der Vergleich
    // ist der Zustand — es gibt kein Flag, das ein Mutator setzen müsste.)
    virtual bool isDirty(const model::Building& current) const = 0;

    // Verdikt vor einer verwerfenden Aktion (Öffnen, Schließen, später „Neu").
    virtual DiscardVerdict verdictForDiscard(
        const model::Building& current) const = 0;

    // Wohin „Speichern" schreibt — bekannte Datei oder Ziel-Abfrage.
    virtual SaveTarget saveTarget() const = 0;

    // Was aus der Benutzer-Antwort folgt. **Die schärfste Zusage des Slice:**
    // `Cancel` ⇒ `Abort` ⇒ die auslösende Aktion unterbleibt.
    virtual DiscardOutcome evaluate(DiscardAnswer answer) const = 0;

    // Nach **erfolgreichem** Öffnen oder Speichern: dieser Stand ist jetzt der
    // persistierte. **Nach** dem Erfolg zu rufen — ein Rücksetzen davor würde
    // einen gescheiterten Schreibvorgang als „gesichert" ausweisen.
    virtual void markPersisted(const std::filesystem::path& path,
                               const model::Building& persisted) = 0;

    // Der zuletzt persistierte Pfad, falls es einen gibt.
    virtual std::optional<std::filesystem::path> path() const = 0;
};

}  // namespace bcad::hexagon::ports::driving
