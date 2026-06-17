// Hand-gerollter ISO-10303-21 (SPF) Subset-Writer (ADR-0013 Option D).
// Generischer Serializer; kennt KEINE IFC-Domänen-Entität (das Mapping lebt im
// ifc_export_adapter). Deterministisch: fester Sentinel-Zeitstempel im HEADER
// (keine Wall-Clock) → byte-stabile Ausgabe für Roundtrip-Tests.

#include "adapters/io/ifc_spf_writer.h"

#include <locale>
#include <sstream>
#include <string>
#include <vector>

namespace bcad::adapters::io {
namespace {

// Deterministischer HEADER-Zeitstempel (Muster Persistenz `kSentinelTs`): das
// welle-4-Subset kennt keine Zeitstempel-Semantik; hält den Export byte-stabil.
constexpr const char* kSentinelTs = "1970-01-01T00:00:00";

}  // namespace

int IfcSpfWriter::add(const std::string& entity) {
    const int id = next_id_;
    lines_.push_back("#" + std::to_string(id) + "=" + entity + ";");
    ++next_id_;
    return id;
}

std::string IfcSpfWriter::build() const {
    std::ostringstream os;
    os << "ISO-10303-21;\n"
       << "HEADER;\n"
       << "FILE_DESCRIPTION(('b-cad IFC4 export subset'),'2;1');\n"
       << "FILE_NAME('','" << kSentinelTs
       << "',(''),(''),'b-cad','b-cad','');\n"
       << "FILE_SCHEMA(('IFC4'));\n"
       << "ENDSEC;\n"
       << "DATA;\n";
    for (const std::string& line : lines_) {
        os << line << '\n';
    }
    os << "ENDSEC;\n"
       << "END-ISO-10303-21;\n";
    return os.str();
}

std::string spfString(const std::string& text) {
    std::string out = "'";
    for (const char c : text) {
        if (c == '\'') {
            out += "''";  // SPF-Escape
        } else {
            out.push_back(c);
        }
    }
    out.push_back('\'');
    return out;
}

std::string spfReal(double value) {
    std::ostringstream os;
    os.imbue(std::locale::classic());  // kein Komma-Dezimalseparator
    os.precision(10);
    os << value;
    std::string out = os.str();
    if (out.find('.') == std::string::npos &&
        out.find('e') == std::string::npos &&
        out.find('E') == std::string::npos) {
        out.push_back('.');  // SPF-Real braucht ein Dezimalzeichen
    }
    return out;
}

std::string spfRef(int id) { return "#" + std::to_string(id); }

std::string spfRefList(const std::vector<int>& ids) {
    std::string out = "(";
    for (std::size_t i = 0; i < ids.size(); ++i) {
        if (i != 0) {
            out.push_back(',');
        }
        out += spfRef(ids[i]);
    }
    out.push_back(')');
    return out;
}

}  // namespace bcad::adapters::io
