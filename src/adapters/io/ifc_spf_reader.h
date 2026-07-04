#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

// IO-Adapter (ADR-0013 Option D): hand-gerollter ISO-10303-21 (STEP Physical
// File, `.ifc`-Klartext) Subset-Leser. Lebt AUSSCHLIESSLICH in src/adapters/io/
// — der Kern bleibt format-frei (ADR-0001, a-check Regel A). Keine externe
// IFC-Bibliothek (ADR-0013: kein vcpkg/Conan, kein OCC-Zwang).
//
// Trennung der Verantwortung (Muster Persistenz: Codec vs. Mapping):
//   - DIESE Datei  = generischer SPF-Tokenizer + Entitäts-Graph (#id -> Entität),
//                    format-agnostisch (kennt keine IFC-Domänen-Entität).
//   - ifc_import_adapter = IFC-Domänen-Mapping auf model::Building.
// Damit ist der Codec isoliert (ohne Adapter/Building) testbar (slice-019b
// Zwei-Commit-Split i).

namespace bcad::adapters::io {

// Ein geparster SPF-Attributwert. Subset-tauglicher Werttyp; deckt die im
// welle-4-Entitäts-Subset vorkommenden Formen ab.
struct SpfValue {
    enum class Kind {
        String,   // 'text' (mit '' -> ' entschärft)
        Enum,     // .T. / .ELEMENT. (text trägt den Inhalt ohne Punkte)
        Ref,      // #123 (ref trägt die Id)
        Real,     // 2500. / -1.5E2 (number)
        Integer,  // 42 (number; integer zusätzlich exakt)
        Null,     // $ (unbelegt)
        Derived,  // * (abgeleitet)
        List,     // ( ... ) (items)
        Typed,    // KEYWORD( ... ) (inline-Konstruktor; text=KEYWORD, items=Args)
    };

    Kind kind{Kind::Null};
    std::string text;             // String/Enum/Typed-Keyword
    int ref{0};                   // Ref-Id
    double number{0.0};           // Real/Integer-Wert
    long long integer{0};         // Integer-Wert (exakt)
    std::vector<SpfValue> items;  // List/Typed-Argumente
};

// Eine Entitäts-Instanz der DATA-Sektion: `#id = KEYWORD ( attrs ) ;`.
struct SpfEntity {
    int id{0};
    std::string keyword;  // GROSSBUCHSTABEN, z. B. "IFCWALL"
    std::vector<SpfValue> attributes;
};

// Geparster Entitäts-Graph einer SPF-Datei.
class SpfReader {
public:
    // Parst den gesamten SPF-Text (DATA-Sektion). Wirft `std::runtime_error`
    // bei fehlendem ISO-10303-21-Kopf oder Syntaxfehler — der Adapter mappt das
    // auf E-IO-003 (Format nicht erkannt / invalide). Eine inhaltlich gültige,
    // aber daten-leere Datei ergibt einen leeren Graphen (kein Wurf).
    static SpfReader parse(const std::string& text);

    // Alle Entitäten eines Keywords, aufsteigend nach #id (deterministisch).
    std::vector<const SpfEntity*> byKeyword(const std::string& keyword) const;

    // Entität per #id; nullptr, wenn unbekannt (dangling Reference).
    const SpfEntity* byId(int id) const;

    const std::map<int, SpfEntity>& entities() const { return entities_; }

private:
    std::map<int, SpfEntity> entities_;
};

// --- Attribut-Zugriffshelfer (frei, damit Codec wie Mapping sie teilen) ---

// Attribut an Position `index` einer Entität; nullptr wenn außerhalb.
const SpfValue* attributeAt(const SpfEntity& entity, std::size_t index);

// Typisierte Lese-Helfer: leeres optional, wenn Wert fehlt/anderer Kind.
std::optional<int> asRef(const SpfValue* value);
std::optional<double> asNumber(const SpfValue* value);
std::optional<std::string> asString(const SpfValue* value);

}  // namespace bcad::adapters::io
