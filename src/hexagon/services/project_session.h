#pragma once

#include <filesystem>
#include <optional>

#include "hexagon/model/building.h"
#include "hexagon/ports/driving/project_session_port.h"

namespace bcad::hexagon::services {

using ports::driving::DiscardAnswer;
using ports::driving::DiscardOutcome;
using ports::driving::DiscardVerdict;
using ports::driving::SaveTarget;
using ports::driving::SaveTargetKind;

// Erfüllt den `ProjectSessionPort` (slice-052a): der Sitzungs-Zustand einer
// laufenden GUI-Sitzung.
//
// **Der Zustand kommt aus dem VERGLEICH, nicht aus Meldungen.** Der erste
// Entwurf wollte ihn aus `ModelChangedPort`-Meldungen ableiten; das erste
// Plan-Review hat belegt, dass das für **keine heute erreichbare
// GUI-Mutation** funktioniert — Hilfslinien, Ebenen und Materialien melden
// **nichts** („kein op" ist Entscheidung 2 der Accepted-ADR-0018). Die Warnung
// wäre nie ausgelöst worden, **während alle Orakel grün stehen**.
//
// Der Vergleich ist dagegen **strukturell**: er setzt am Ergebnis an, also ist
// jeder heutige und jeder künftige Schreibweg erfasst — und weil die
// `operator==` der Modelltypen `= default` sind, auch jedes künftig ergänzte
// **Feld**.
//
// **Framework-frei.** Der Träger vergleicht `model::Building`-Werte und kennt
// sonst nichts.
class ProjectSessionService : public ports::driving::ProjectSessionPort {
public:
    // Die Baseline ist der Stand **bei Sitzungs-Beginn** — nicht ein leeres
    // Modell. Ein frisch aufgebautes Demo-Projekt gilt damit als „nicht
    // ungesichert", solange der Benutzer nichts ändert.
    explicit ProjectSessionService(model::Building baseline)
        : baseline_(std::move(baseline)) {}

    bool isDirty(const model::Building& current) const override;
    DiscardVerdict verdictForDiscard(
        const model::Building& current) const override;
    SaveTarget saveTarget() const override;
    DiscardOutcome evaluate(DiscardAnswer answer) const override;
    void markPersisted(const std::filesystem::path& path,
                       const model::Building& persisted) override;
    std::optional<std::filesystem::path> path() const override;

private:
    model::Building baseline_;
    std::optional<std::filesystem::path> path_;
};

}  // namespace bcad::hexagon::services
