// ASCII-DXF (R12-Subset) Tokenizer (ADR-0015 Option D). Zerlegt den Gruppencode-
// Strom in Struktureinheiten; format-agnostisch (kein Domänen-Wissen). Die
// Wohlgeformtheit (jede Code-Zeile eine Ganzzahl, gerade Paarung) ist die
// Negative/Boundary-Grenze: unwohlgeformt → Wurf (Adapter → E-IO-003);
// wohlgeformt-aber-strukturlos → leere Liste (kein Wurf).

#include "adapters/io/dxf_reader.h"

#include <cctype>
#include <cstddef>
#include <sstream>
#include <stdexcept>
#include <string>

namespace bcad::adapters::io {
namespace {

std::string strip(const std::string& text) {
    std::size_t begin = 0;
    std::size_t end = text.size();
    while (begin < end && std::isspace(static_cast<unsigned char>(text[begin])) != 0) {
        ++begin;
    }
    while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1])) != 0) {
        --end;
    }
    return text.substr(begin, end - begin);
}

// Ganzzahl-Gruppencode (optionales Vorzeichen + Ziffern). Kein Float, kein Text.
bool parseGroupCode(const std::string& text, int& out) {
    if (text.empty()) {
        return false;
    }
    std::size_t i = 0;
    if (text[i] == '+' || text[i] == '-') {
        ++i;
    }
    if (i >= text.size()) {
        return false;
    }
    for (std::size_t j = i; j < text.size(); ++j) {
        if (std::isdigit(static_cast<unsigned char>(text[j])) == 0) {
            return false;
        }
    }
    try {
        out = std::stoi(text);
    } catch (const std::exception&) {
        return false;
    }
    return true;
}

std::optional<double> toNumber(const std::string& text) {
    try {
        return std::stod(text);
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

// Zeilen (ohne CR), führende/abschließende Leerzeilen verworfen.
std::vector<std::string> contentLines(const std::string& text) {
    std::vector<std::string> lines;
    std::istringstream in(text);
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines.push_back(line);
    }
    std::size_t begin = 0;
    std::size_t end = lines.size();
    while (begin < end && strip(lines[begin]).empty()) {
        ++begin;
    }
    while (end > begin && strip(lines[end - 1]).empty()) {
        --end;
    }
    return std::vector<std::string>(lines.begin() + static_cast<std::ptrdiff_t>(begin),
                                    lines.begin() + static_cast<std::ptrdiff_t>(end));
}

}  // namespace

std::optional<std::string> DxfEntity::str(int code) const {
    for (const DxfPair& p : attributes) {
        if (p.code == code) {
            return p.value;
        }
    }
    return std::nullopt;
}

std::optional<double> DxfEntity::num(int code) const {
    for (const DxfPair& p : attributes) {
        if (p.code == code) {
            return toNumber(p.value);
        }
    }
    return std::nullopt;
}

std::vector<std::pair<double, double>> DxfEntity::points() const {
    std::vector<std::pair<double, double>> pts;
    std::optional<double> x;
    for (const DxfPair& p : attributes) {
        if (p.code == 10) {
            x = toNumber(p.value);
        } else if (p.code == 20 && x) {
            if (const auto y = toNumber(p.value)) {
                pts.emplace_back(*x, *y);
            }
            x = std::nullopt;
        }
    }
    return pts;
}

DxfReader DxfReader::parse(const std::string& text) {
    const std::vector<std::string> lines = contentLines(text);
    if (lines.size() % 2 != 0) {
        throw std::runtime_error("DXF: ungerade Gruppencode-Paarung");
    }

    DxfReader reader;
    DxfEntity* current = nullptr;
    for (std::size_t i = 0; i + 1 < lines.size(); i += 2) {
        int code = 0;
        if (!parseGroupCode(strip(lines[i]), code)) {
            throw std::runtime_error("DXF: nicht-numerische Gruppencode-Zeile '" +
                                     strip(lines[i]) + "'");
        }
        const std::string value = strip(lines[i + 1]);
        if (code == 0) {
            reader.entities_.push_back(DxfEntity{value, {}});
            current = &reader.entities_.back();
        } else if (current != nullptr) {
            current->attributes.push_back(DxfPair{code, value});
        }
        // Attribut-Paare vor dem ersten Gruppencode 0 werden ignoriert.
    }
    return reader;
}

std::vector<const DxfEntity*> DxfReader::sectionEntities(
    const std::string& section) const {
    std::vector<const DxfEntity*> out;
    bool in_section = false;
    for (const DxfEntity& e : entities_) {
        if (e.type == "SECTION") {
            const auto name = e.str(2);
            in_section = name.has_value() && *name == section;
        } else if (e.type == "ENDSEC") {
            in_section = false;
        } else if (in_section) {
            out.push_back(&e);
        }
    }
    return out;
}

}  // namespace bcad::adapters::io
