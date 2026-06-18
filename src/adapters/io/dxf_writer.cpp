// ASCII-DXF (R12/AC1009) Subset-Writer (ADR-0015 Option D). Reine Gruppencode-
// Syntax (Paar-Formatierung + locale-freie Reals) — kein Domänen-Wissen.

#include "adapters/io/dxf_writer.h"

#include <array>
#include <cstdio>
#include <string>

namespace bcad::adapters::io {

std::string dxfReal(double value) {
    std::array<char, 32> buf{};
    std::snprintf(buf.data(), buf.size(), "%.12g", value);
    std::string text(buf.data());
    // Defensiv locale-frei: ein etwaiges Dezimalkomma auf '.' normalisieren.
    for (char& c : text) {
        if (c == ',') {
            c = '.';
        }
    }
    // DXF-Reals tragen ein Dezimalzeichen; ganzzahlige Ausgaben (`5000`)
    // bekommen eines (Lesbarkeit + symmetrischer Roundtrip).
    const bool has_marker = text.find_first_of(".eEnN") != std::string::npos;
    if (!has_marker) {
        text += ".0";
    }
    return text;
}

void DxfWriter::pair(int code, const std::string& value) {
    out_ += std::to_string(code);
    out_ += '\n';
    out_ += value;
    out_ += '\n';
}

void DxfWriter::pair(int code, double value) { pair(code, dxfReal(value)); }

std::string DxfWriter::build() const { return out_; }

}  // namespace bcad::adapters::io
