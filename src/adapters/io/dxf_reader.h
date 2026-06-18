#pragma once

#include <optional>
#include <string>
#include <utility>
#include <vector>

// IO-Adapter (ADR-0015 Option D): hand-gerollter ASCII-DXF (R12-Subset)
// Tokenizer. Lebt AUSSCHLIESSLICH in `src/adapters/io/` — der Kern bleibt
// format-frei (ADR-0001, arch-check Regel A). Keine externe DXF-Bibliothek.
//
// Trennung der Verantwortung (Muster IFC-Codec: Codec vs. Mapping):
//   - DIESE Datei  = generischer DXF-Gruppencode-Tokenizer + Struktureinheiten,
//                    format-agnostisch (kennt keine b-cad-Domäne).
//   - dxf_import_adapter = DXF-Domänen-Mapping auf `model::Building`.

namespace bcad::adapters::io {

// Ein geparstes Gruppencode/Wert-Paar (Attribut einer Struktureinheit).
struct DxfPair {
    int code{0};
    std::string value;
};

// Eine DXF-Struktureinheit: beginnt an einem Gruppencode 0 (Typ = dessen Wert,
// z. B. "LINE"/"SECTION"/"ENDSEC"), gefolgt von ihren Attribut-Paaren
// (Codes != 0) bis zum nächsten Gruppencode 0.
struct DxfEntity {
    std::string type;
    std::vector<DxfPair> attributes;

    // Erstes Attribut mit `code` als String / Zahl; leeres optional sonst.
    std::optional<std::string> str(int code) const;
    std::optional<double> num(int code) const;
    // Alle (x,y) aus aufeinanderfolgenden 10/20-Paaren (LWPOLYLINE-Vertices),
    // in Quell-Reihenfolge.
    std::vector<std::pair<double, double>> points() const;
};

// Geparste Struktureinheiten einer DXF-Datei.
class DxfReader {
public:
    // Parst den DXF-Text in Struktureinheiten. Wirft `std::runtime_error` bei
    // **nicht wohlgeformtem** Gruppencode-Strom (nicht-numerische Code-Zeile /
    // ungerade Paarung) — der Adapter mappt das auf E-IO-003. Eine
    // wohlgeformte, aber strukturlose (z. B. ENTITIES-lose) oder leere Datei
    // ergibt eine entsprechend leere Struktureinheiten-Liste (**kein** Wurf).
    static DxfReader parse(const std::string& text);

    // Struktureinheiten **innerhalb** der benannten Sektion (`SECTION`/Code 2 ==
    // `section` … `ENDSEC`), in Quell-Reihenfolge — HEADER/TABLES werden so
    // übersprungen.
    std::vector<const DxfEntity*> sectionEntities(const std::string& section) const;

    const std::vector<DxfEntity>& entities() const { return entities_; }

private:
    std::vector<DxfEntity> entities_;
};

}  // namespace bcad::adapters::io
