#include "hexagon/services/project_session.h"

#include <utility>

namespace bcad::hexagon::services {

bool ProjectSessionService::isDirty(const model::Building& current) const {
    // DAS ist der Zustand: ein Wert-Vergleich, kein Flag. "Zurueck-geaendert auf
    // den Dateistand" wird dadurch wieder sauber (Orakel-Zeile 7).
    return !(current == baseline_);
}

DiscardVerdict ProjectSessionService::verdictForDiscard(
    const model::Building& current) const {
    return isDirty(current) ? DiscardVerdict::AskFirst : DiscardVerdict::Proceed;
}

SaveTarget ProjectSessionService::saveTarget() const {
    if (path_) {
        return {SaveTargetKind::KnownPath, *path_};
    }
    return {SaveTargetKind::AskUser, {}};
}

DiscardOutcome ProjectSessionService::evaluate(DiscardAnswer answer) const {
    switch (answer) {
        case DiscardAnswer::Cancel:
            // Die schaerfste Zusage des Slice: abbrechen UNTERLAESST die
            // ausloesende Aktion. Nicht "trotzdem ausfuehren, nur ohne
            // speichern" — das waere der Datenverlust, den der Slice verhindert.
            return DiscardOutcome::Abort;
        case DiscardAnswer::Save:
            return DiscardOutcome::SaveThenProceed;
        case DiscardAnswer::Discard:
            return DiscardOutcome::Proceed;
    }
    return DiscardOutcome::Abort;  // fail-closed bei unbekannter Antwort
}

void ProjectSessionService::markPersisted(const std::filesystem::path& path,
                                          const model::Building& persisted) {
    // NACH dem Erfolg zu rufen (§9 R2): ein Ruecksetzen vor dem Schreiben wuerde
    // einen gescheiterten Schreibvorgang als "gesichert" ausweisen — Orakel 6.
    path_ = path;
    baseline_ = persisted;
}

void ProjectSessionService::reset(const model::Building& baseline) {
    // Pfad WEG (nicht: unveraendert lassen) — sonst schriebe "Speichern" nach
    // "Neu" dialoglos in die vorige Datei (slice-052b, L1).
    path_.reset();
    baseline_ = baseline;
}

std::optional<std::filesystem::path> ProjectSessionService::path() const {
    return path_;
}

}  // namespace bcad::hexagon::services
