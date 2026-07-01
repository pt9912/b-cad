#pragma once

#include <filesystem>
#include <string>

namespace bcad::adapters::io {

// Geteilter atomarer, **binär-treuer** Datei-Writer (slice-025b, ADR-0016) für
// die io-residenten Export-Adapter (PDF/PNG). Schreibt `content` in eine
// Temp-Datei `path + ".tmp"`, `fsync`t und ersetzt den Zielpfad erst nach Erfolg
// (Rename) — **kein** Teil-Export; bei Fehler bleibt der Zielpfad unverändert und
// der Temp wird entfernt. Byte-Ströme (PDF/PNG) werden roh über `::write`
// geschrieben (kein Text-Transcoding).
//
// Fehler: eine neutrale `std::runtime_error` mit vorangestelltem Spec-Fehlercode
// (eine Fehler-Zeichenkette ist kein Backend-Typ, ADR-0001): nicht beschreibbarer
// Zielpfad (EACCES/EPERM/EROFS/EISDIR/ENOTDIR/ENOENT) → `E-IO-001`
// (`event=io_no_permission`); sonst (Medium voll / IO) → `E-IO-002`
// (`event=persist_error`). **Byte-gleiches errno-/`.tmp`-Muster wie
// `dxf_export_adapter`/`ifc_export_adapter`** (extrahiert; Review-LOW-2) — die
// bestehenden IFC/DXF-Adapter behalten vorerst ihre lokalen Kopien (Refaktor
// out-of-scope). `format_label` erscheint in der Fehlermeldung (z. B. "PDF").
void writeFileAtomically(const std::filesystem::path& path,
                         const std::string& content, const char* format_label);

}  // namespace bcad::adapters::io
