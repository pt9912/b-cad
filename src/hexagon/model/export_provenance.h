#pragma once

#include <string>

namespace bcad::hexagon::model {

// Export-Herkunft (slice-046): vom Composition-Root **vorformatierte**, injizierte
// Provenance-Strings — damit ein Benutzer, der ein Export-Artefakt anschaut,
// erkennt, aus welchem **Stand** es stammt (Exporte verschiedener Modellstände sind
// sonst byte-identisch, siehe slice-044a/045). Framework-frei (ADR-0001): der Kern/
// Adapter kennt **kein** `<chrono>` und keinen Pfad-Lookup — die Werte kommen fertig
// aus `main.cpp` (die einzige Uhr-/Umgebungs-Berührung).
//
// **Determinismus (das „SOURCE_DATE_EPOCH"-Muster):** die Writer rufen nie die Uhr;
// sie rendern nur diese Strings. Produktion füllt echte Werte, Tests/Golden **feste**
// → das Byte-Golden bleibt deterministisch. Ein **leeres** Feld ist zulässig und lässt
// den Adapter auf sein deterministisches Sentinel-Verhalten (044a/045) zurückfallen;
// **gefüllt** wird der Wert sichtbar/eingebettet.
struct ExportProvenance {
    std::string date;     // Export-Zeitpunkt, anzeige-fertig (z. B. "2026-07-24 14:32"); leer → Sentinel
    std::string source;   // Projektdatei-Basename (z. B. "haus.bcad"); leer → keine Quelle
    std::string version;  // Erzeuger, z. B. "b-cad 0.1.0" (application_banner()); leer → keine Version

    // True, wenn keinerlei Provenance injiziert wurde (→ Sentinel-/Alt-Verhalten).
    bool empty() const { return date.empty() && source.empty() && version.empty(); }
};

}  // namespace bcad::hexagon::model
