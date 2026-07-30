#pragma once

#include <charconv>
#include <functional>
#include <optional>
#include <string_view>
#include <system_error>
#include <utility>

#include "hexagon/model/wall.h"  // WallId
#include "hexagon/ports/driving/edit_structure_port.h"

namespace bcad::adapters::ui::command {

// Ausgang einer Parameter-Änderung — **der Typ lebt hier, in `command/`**
// (ADR-0021 E5, slice-059b §2.2): eine Ausgangs-Autorität, nicht zwei.
enum class ParamEditOutcome {
    Accepted,      // übernommen wie eingegeben
    Clamped,       // auf den Grenzwert geklemmt — `applied_mm` ist der Wert,
                   // der dem Benutzer GENANNT werden muss (abnahmebindend)
    Rejected,      // vom Kern abgelehnt (nicht-endlich) — Modell unverändert
    NotANumber,    // die Eingabe ist kein vollständiger Zahlwert — der Kern
                   // sieht sie nie, die Entscheidung fällt HIER
    Failed,        // Wurf gefangen (unbekannte Wand-Id) — Modell unverändert
};

// Der vollständige Ausgang: **Status und übernommener Wert**. Der Wert ist der
// Teil, den das Lastenheft abnahmebindend macht („der tatsächlich übernommene
// Wert wird dem Nutzer **genannt**") — deshalb reist er als **Zahl** weiter und
// nicht in einem fertigen Text (§2.3).
struct ParamEditResult {
    ParamEditOutcome outcome{};
    double applied_mm{};  // bei NotANumber/Failed: der unveränderte Ist-Wert
                          // ist unbekannt, dann 0 — der Empfänger nennt ihn
                          // in diesen Fällen ohnehin nicht
};

// driving-Seite der ui: der **dritte** UI-Mutator und der erste, der **Text**
// entgegennimmt (slice-059b §2.2).
//
// **Warum Text und nicht `double`:** eine nicht-numerische Eingabe erreicht den
// Kern **nie** — sie muss also irgendwo in der Oberfläche entschieden werden.
// Im Composition-Root wäre diese Entscheidung **orakel-los** (`src/main.cpp` ist
// in kein Testbinary gelinkt), im Fenster entstünde eine **zweite**
// Ausgangs-Autorität neben dieser Senke. Also hier — und damit ist die
// Umwandlung selbst Teil des Vertrags.
//
// **Die Umwandlung ist Bauvorschrift, nicht Implementierungs-Freiheit:**
// `std::from_chars` bei **voller Konsumption**. Gemessen im Plan-Review:
// `std::stod("abc")` **wirft** (statt still 0 zu liefern), `stod("50 mm")`
// liefert **50** und ignoriert den Rest, `QString::toDouble("50,0")` lehnt ab
// und ist locale-abhängig. `from_chars` wirft nicht, kennt keine Locale, und
// der Rest-Zeiger macht „50 mm" zu einer **Ablehnung** statt zu einer stillen
// Falsch-Übernahme. Nicht-endliche Werte (`inf`/`nan`) reicht es durch — sie
// werden im **Kern** abgelehnt, und diese Zweiteilung ist gewollt.
//
// **Schließbedingung** (§2.2): die **Anzeige-Form** der Werte muss von
// **dieser** Umwandlung wieder lesbar sein. Sonst bekäme ein Benutzer, der ein
// zurückgeschriebenes Feld nur bestätigt, eine Ablehnung für einen Wert, den er
// nie geändert hat.
class WallParamSink {
public:
    using Report = std::function<void(ParamEditResult)>;

    WallParamSink(hexagon::ports::driving::EditStructurePort& port,
                  Report report)
        : port_(port), report_(std::move(report)) {}

    ParamEditResult setThickness(hexagon::model::WallId wall,
                                 std::string_view text) const {
        return apply(text, [this, wall](double mm) {
            return port_.setWallThickness(wall, mm);
        });
    }

    ParamEditResult setHeight(hexagon::model::WallId wall,
                              std::string_view text) const {
        return apply(text, [this, wall](double mm) {
            return port_.setWallHeight(wall, mm);
        });
    }

    // Die Umwandlung als **eigene**, prüfbare Naht: dieselbe Funktion entscheidet
    // über Eingabe UND über die Lesbarkeit der Anzeige-Form (§4-13).
    [[nodiscard]] static std::optional<double> parse(std::string_view text) {
        double value = 0.0;
        const char* const begin = text.data();
        const char* const end = begin + text.size();
        const std::from_chars_result result = std::from_chars(begin, end, value);
        if (result.ec != std::errc{} || result.ptr != end) {
            return std::nullopt;  // kein Zahlwert ODER Rest übrig ("50 mm")
        }
        return value;
    }

private:
    template <typename Mutate>
    ParamEditResult apply(std::string_view text, Mutate mutate) const {
        const std::optional<double> value = parse(text);
        if (!value.has_value()) {
            // Der Kern sieht diese Eingabe NIE — ohne diese Prüfung würde
            // `0` übernommen und auf den Grenzwert geklemmt: eine stille
            // Falsch-Übernahme statt einer Ablehnung.
            return reported({ParamEditOutcome::NotANumber, 0.0});
        }
        hexagon::ports::driving::ParamResult result{};
        try {
            result = mutate(*value);
        } catch (...) {
            // Unbekannte Wand-Id. **Bewusst `...`:** die Zusage lautet „kein
            // Wurf verlässt den Ereignis-Pfad" — eine Barriere, die nur die
            // dokumentierten Typen fängt, ist keine (Muster slice-058).
            return reported({ParamEditOutcome::Failed, 0.0});
        }
        switch (result.status) {
            case hexagon::ports::driving::ParamStatus::Accepted:
                return reported({ParamEditOutcome::Accepted, result.applied_mm});
            case hexagon::ports::driving::ParamStatus::Clamped:
                return reported({ParamEditOutcome::Clamped, result.applied_mm});
            case hexagon::ports::driving::ParamStatus::Rejected:
                break;
        }
        return reported({ParamEditOutcome::Rejected, result.applied_mm});
    }

    ParamEditResult reported(ParamEditResult result) const {
        if (report_) {
            report_(result);
        }
        return result;
    }

    hexagon::ports::driving::EditStructurePort& port_;
    Report report_;
};

}  // namespace bcad::adapters::ui::command
