#pragma once

#include <string>
#include <vector>

// IO-Adapter (ADR-0013 Option D): hand-gerollter ISO-10303-21 (STEP Physical
// File) Subset-**Writer** — Spiegel des `ifc_spf_reader` (symmetrisch Lesen UND
// Schreiben in einer Code-Basis). Generisch/format-agnostisch (kennt keine
// IFC-Domänen-Entität); das Domänen→IFC-Mapping lebt im `ifc_export_adapter`.
// Lebt ausschließlich in `src/adapters/io/` (arch-check Regel A/B); keine
// externe IFC-Bibliothek.

namespace bcad::adapters::io {

// Akkumuliert DATA-Entitäten und emittiert eine vollständige SPF-Datei. Der
// Aufrufer baut Parameter **bottom-up** (referenzierte Entität zuerst) und
// referenziert über die von `add` zurückgegebene `#id` — so entstehen keine
// Vorwärts-Referenzen.
class IfcSpfWriter {
public:
    // Fügt eine DATA-Entität hinzu; `entity` ist der Rumpf `KEYWORD(params)`
    // (ohne `#id=` und ohne `;`). Gibt die zugewiesene `#id` zurück.
    int add(const std::string& entity);

    // Vollständiger ISO-10303-21-Text: Kopf + HEADER (`FILE_SCHEMA(('IFC4'))`)
    // + DATA + ENDSEC + END-ISO. Deterministisch (Sentinel-Zeitstempel) —
    // byte-stabil für Roundtrip-Tests.
    std::string build() const;

private:
    int next_id_{1};
    std::vector<std::string> lines_;
};

// --- SPF-Wertformatierung (frei; Adapter baut Parameter-Strings damit) ---

// 'text' mit ' -> '' (SPF-String-Escape).
std::string spfString(const std::string& text);
// SPF-Real: garantiert ein Dezimalzeichen, locale-frei (kein Komma-Separator).
std::string spfReal(double value);
// #id-Referenz.
std::string spfRef(int id);
// (#a,#b,…)-Referenzliste.
std::string spfRefList(const std::vector<int>& ids);

}  // namespace bcad::adapters::io
