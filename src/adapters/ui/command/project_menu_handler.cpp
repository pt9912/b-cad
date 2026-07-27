#include "adapters/ui/command/project_menu_handler.h"

#include <exception>

namespace bcad::adapters::ui::command {

hexagon::ports::driving::DrawingTargetResolution ProjectMenuHandler::open(
    const std::filesystem::path& path) {
    // Die gehaltenen Senken werden DURCHGEREICHT — nicht `{}`. Ohne sie bliebe
    // das Zeichen-Ziel auf den Ids des alten Modells stehen (slice-047-B4);
    // Orakel slice-053 §3-Zeile 6 prueft genau das.
    return project_.openProject(path, sinks_);
}

void ProjectMenuHandler::saveAs(const std::filesystem::path& path) {
    project_.saveProject(path);
}

hexagon::ports::driving::DiscardVerdict ProjectMenuHandler::verdictForDiscard()
    const {
    return session_.verdictForDiscard(current_());
}

hexagon::ports::driving::SaveTarget ProjectMenuHandler::saveTarget() const {
    return session_.saveTarget();
}

hexagon::ports::driving::DiscardOutcome ProjectMenuHandler::evaluate(
    hexagon::ports::driving::DiscardAnswer answer) const {
    return session_.evaluate(answer);
}

bool ProjectMenuHandler::mayDiscard(const DiscardAsk& ask,
                                   const SaveTargetAsk& ask_target) {
    using Verdict = hexagon::ports::driving::DiscardVerdict;
    using Outcome = hexagon::ports::driving::DiscardOutcome;

    if (session_.verdictForDiscard(current_()) == Verdict::Proceed) {
        return true;  // nichts Ungesichertes — keine Rueckfrage
    }
    switch (session_.evaluate(ask())) {
        case Outcome::Abort:
            // Die schaerfste Zusage des Slice: abbrechen UNTERLAESST.
            return false;
        case Outcome::Proceed:
            return true;
        case Outcome::SaveThenProceed:
            break;
    }
    // "erst speichern": gelingt das nicht (abgebrochene Ziel-Abfrage ODER
    // Schreibfehler), unterbleibt die ausloesende Aktion — Orakel-Zeile 12a.
    try {
        if (save()) {
            return true;
        }
        const std::optional<std::filesystem::path> target = ask_target();
        if (!target) {
            return false;  // Ziel-Abfrage abgebrochen
        }
        saveAs(*target);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool ProjectMenuHandler::save() {
    const hexagon::ports::driving::SaveTarget target = session_.saveTarget();
    if (target.kind != hexagon::ports::driving::SaveTargetKind::KnownPath) {
        return false;  // kein Pfad bekannt -> der Aufrufer fragt das Ziel ab
    }
    project_.saveProject(target.path);
    return true;
}

}  // namespace bcad::adapters::ui::command
