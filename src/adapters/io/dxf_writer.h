#pragma once

#include <string>

// IO-Adapter (ADR-0015 Option D): hand-gerollter ASCII-DXF (R12/AC1009)
// Subset-**Writer** — Spiegel des `dxf_reader` (symmetrisch Lesen UND Schreiben
// in einer Code-Basis). Generisch/format-agnostisch: kennt die DXF-
// Gruppencode-**Syntax** (Paar-Formatierung, locale-freie Reals), aber keine
// b-cad-Domänen-Entität. Das Domänen→DXF-Mapping (welche Sektionen/Entitäten)
// lebt im `dxf_export_adapter`. Lebt ausschließlich in `src/adapters/io/`
// (a-check Regel A/B); keine externe DXF-Bibliothek (ADR-0015 Option D).

namespace bcad::adapters::io {

// Akkumuliert DXF-Gruppencode/Wert-Paare und emittiert den vollständigen Text.
// Der Aufrufer schreibt die Sektions-/Entitäts-Struktur über `pair`
// (`0/SECTION` … `0/ENDSEC` … `0/EOF`) — der Writer kennt sie nicht.
class DxfWriter {
public:
    // Ein Gruppencode/Wert-Paar (`code`\n`value`\n).
    void pair(int code, const std::string& value);
    // Real-Wert locale-frei (kein Komma), round-trip-stabil.
    void pair(int code, double value);
    // Vollständiger DXF-Text (alle bisher geschriebenen Paare).
    std::string build() const;

private:
    std::string out_;
};

// Locale-freie DXF-Real-Formatierung (frei; Adapter wie Tests teilen sie):
// garantiert ein Dezimalzeichen, kein Komma-Separator. Round-trip-stabil für
// endliche mm-Koordinaten (`%.12g`).
std::string dxfReal(double value);

}  // namespace bcad::adapters::io
