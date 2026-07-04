// Hand-gerollter ISO-10303-21 (SPF) Subset-Leser (ADR-0013 Option D).
// Generischer Tokenizer + Entitäts-Graph; kennt KEINE IFC-Domänen-Entität
// (das Mapping lebt in ifc_import_adapter). Total gegen Syntaxfehler: jeder
// nicht-parsebare Zustand wirft `std::runtime_error` (Adapter -> E-IO-003).

#include "adapters/io/ifc_spf_reader.h"

#include <cctype>
#include <cstddef>
#include <stdexcept>
#include <string>

namespace bcad::adapters::io {
namespace {

// Zeichen-Cursor über den (kommentar-bereinigten) SPF-Text.
class Cursor {
public:
    explicit Cursor(const std::string& text) : text_(text) {}

    bool eof() const { return pos_ >= text_.size(); }
    char peek() const { return eof() ? '\0' : text_[pos_]; }
    char get() { return eof() ? '\0' : text_[pos_++]; }
    std::size_t pos() const { return pos_; }
    void seek(std::size_t pos) { pos_ = pos; }

    void skipWhitespace() {
        while (!eof()) {
            const char c = text_[pos_];
            if (std::isspace(static_cast<unsigned char>(c)) == 0) {
                break;
            }
            ++pos_;
        }
    }

    // Erwartet ein bestimmtes Zeichen; wirft sonst.
    void expect(char want) {
        if (eof() || text_[pos_] != want) {
            throw std::runtime_error(std::string("SPF: erwartet '") + want +
                                     "' bei Position " + std::to_string(pos_));
        }
        ++pos_;
    }

private:
    const std::string& text_;
    std::size_t pos_{0};
};

// Kopiert ein 'string'-Literal ab `i` (auf der öffnenden Quote) nach `out`,
// inkl. '' -> '' (escaptes Quote bleibt im String). `i` zeigt danach auf die
// schließende Quote (Schleifenkopf inkrementiert weiter).
void copyStringLiteral(const std::string& in, std::size_t& i, std::string& out) {
    out.push_back(in[i]);  // öffnendes '
    for (++i; i < in.size(); ++i) {
        const char c = in[i];
        out.push_back(c);
        if (c != '\'') {
            continue;
        }
        if (i + 1 < in.size() && in[i + 1] == '\'') {
            out.push_back(in[++i]);  // escaptes Quote, im String bleiben
            continue;
        }
        return;  // schließende Quote
    }
}

// Überspringt einen /* ... */-Kommentar ab `i` (auf '/'); `i` zeigt danach auf
// das letzte Kommentar-Zeichen.
void skipBlockComment(const std::string& in, std::size_t& i) {
    i += 2;  // über '/*'
    while (i + 1 < in.size() && (in[i] != '*' || in[i + 1] != '/')) {
        ++i;
    }
    ++i;  // auf das schließende '/'
}

// Entfernt /* ... */-Kommentare (ISO 10303-21) string-bewusst (ein "/*" in
// einem 'string'-Literal ist kein Kommentar).
std::string stripComments(const std::string& in) {
    std::string out;
    out.reserve(in.size());
    for (std::size_t i = 0; i < in.size(); ++i) {
        if (in[i] == '\'') {
            copyStringLiteral(in, i, out);
            continue;
        }
        if (in[i] == '/' && i + 1 < in.size() && in[i + 1] == '*') {
            skipBlockComment(in, i);
            continue;
        }
        out.push_back(in[i]);
    }
    return out;
}

bool isIdentChar(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '_' ||
           c == '-';
}

// Liest ein 'string'-Literal (Cursor steht auf der öffnenden Quote).
// '' wird zu einem einzelnen '.
SpfValue parseString(Cursor& cur) {
    cur.expect('\'');
    SpfValue value;
    value.kind = SpfValue::Kind::String;
    while (!cur.eof()) {
        const char c = cur.get();
        if (c == '\'') {
            if (cur.peek() == '\'') {  // '' -> '
                cur.get();
                value.text.push_back('\'');
                continue;
            }
            return value;  // schließende Quote
        }
        value.text.push_back(c);
    }
    throw std::runtime_error("SPF: nicht abgeschlossenes String-Literal");
}

// Liest eine .ENUM.-Form (Cursor steht auf dem ersten Punkt).
SpfValue parseEnum(Cursor& cur) {
    cur.expect('.');
    SpfValue value;
    value.kind = SpfValue::Kind::Enum;
    while (!cur.eof()) {
        const char c = cur.get();
        if (c == '.') {
            return value;
        }
        value.text.push_back(c);
    }
    throw std::runtime_error("SPF: nicht abgeschlossenes Enum-Literal");
}

// Liest #123 (Cursor steht auf '#').
SpfValue parseRef(Cursor& cur) {
    cur.expect('#');
    std::string digits;
    while (std::isdigit(static_cast<unsigned char>(cur.peek())) != 0) {
        digits.push_back(cur.get());
    }
    if (digits.empty()) {
        throw std::runtime_error("SPF: leere Referenz '#'");
    }
    SpfValue value;
    value.kind = SpfValue::Kind::Ref;
    value.ref = std::stoi(digits);
    return value;
}

// Liest eine Zahl (Real/Integer). Cursor steht auf Vorzeichen/Ziffer.
SpfValue parseNumber(Cursor& cur) {
    std::string token;
    bool is_real = false;
    while (!cur.eof()) {
        const char c = cur.peek();
        if (std::isdigit(static_cast<unsigned char>(c)) != 0 || c == '+' ||
            c == '-') {
            token.push_back(cur.get());
        } else if (c == '.' || c == 'e' || c == 'E') {
            is_real = true;
            token.push_back(cur.get());
        } else {
            break;
        }
    }
    if (token.empty()) {
        throw std::runtime_error("SPF: erwartete Zahl");
    }
    SpfValue value;
    value.number = std::stod(token);
    if (is_real) {
        value.kind = SpfValue::Kind::Real;
    } else {
        value.kind = SpfValue::Kind::Integer;
        value.integer = std::stoll(token);
    }
    return value;
}

SpfValue parseValue(Cursor& cur);  // vorwärts (List/Typed-Rekursion)

// Liest eine ( ... )-Liste (Cursor steht auf '('). Leere Liste erlaubt.
SpfValue parseList(Cursor& cur) {
    cur.expect('(');
    SpfValue value;
    value.kind = SpfValue::Kind::List;
    cur.skipWhitespace();
    if (cur.peek() == ')') {
        cur.get();
        return value;
    }
    while (true) {
        value.items.push_back(parseValue(cur));
        cur.skipWhitespace();
        const char c = cur.get();
        if (c == ')') {
            return value;
        }
        if (c != ',') {
            throw std::runtime_error("SPF: erwartet ',' oder ')' in Liste");
        }
        cur.skipWhitespace();
    }
}

// Liest ein Bezeichner-Token (Keyword); für Typed-Konstruktoren.
SpfValue parseKeywordValue(Cursor& cur) {
    std::string ident;
    while (isIdentChar(cur.peek())) {
        ident.push_back(cur.get());
    }
    cur.skipWhitespace();
    SpfValue value;
    value.text = ident;
    if (cur.peek() == '(') {  // KEYWORD( ... ) -> Typed
        value.kind = SpfValue::Kind::Typed;
        value.items = parseList(cur).items;
    } else {  // nacktes Token (selten); als Enum-artigen Text führen
        value.kind = SpfValue::Kind::Enum;
    }
    return value;
}

SpfValue parseValue(Cursor& cur) {
    cur.skipWhitespace();
    const char c = cur.peek();
    switch (c) {
        case '\'':
            return parseString(cur);
        case '.':
            return parseEnum(cur);
        case '#':
            return parseRef(cur);
        case '(':
            return parseList(cur);
        case '$': {
            cur.get();
            SpfValue v;
            v.kind = SpfValue::Kind::Null;
            return v;
        }
        case '*': {
            cur.get();
            SpfValue v;
            v.kind = SpfValue::Kind::Derived;
            return v;
        }
        default:
            break;
    }
    if (std::isdigit(static_cast<unsigned char>(c)) != 0 || c == '+' ||
        c == '-') {
        return parseNumber(cur);
    }
    if (std::isalpha(static_cast<unsigned char>(c)) != 0) {
        return parseKeywordValue(cur);
    }
    throw std::runtime_error(std::string("SPF: unerwartetes Zeichen '") + c +
                             "' bei Position " + std::to_string(cur.pos()));
}

// Liest ein führendes Bezeichner-Token (Sektions-Marke / HEADER-Keyword),
// ohne den Rest des Statements zu konsumieren. Leer, wenn das erste Zeichen
// kein Bezeichner-Zeichen ist.
std::string readWord(Cursor& cur) {
    std::string word;
    while (isIdentChar(cur.peek())) {
        word.push_back(cur.get());
    }
    return word;
}

// Überspringt ein Nicht-Entitäts-Statement (HEADER-Zeilen, Sektions-Marken
// wie HEADER;/DATA;/ENDSEC;/END-ISO-10303-21;) bis zum nächsten ';' außerhalb
// von Strings.
void skipStatement(Cursor& cur) {
    while (!cur.eof()) {
        const char c = cur.get();
        if (c == '\'') {  // String string-bewusst überspringen
            while (!cur.eof()) {
                const char s = cur.get();
                if (s == '\'') {
                    if (cur.peek() == '\'') {
                        cur.get();
                        continue;
                    }
                    break;
                }
            }
            continue;
        }
        if (c == ';') {
            return;
        }
    }
}

// Parst ein Entitäts-Statement `#id = KEYWORD ( attrs ) ;` (Cursor auf '#').
SpfEntity parseEntity(Cursor& cur) {
    const SpfValue id_value = parseRef(cur);
    cur.skipWhitespace();
    cur.expect('=');
    cur.skipWhitespace();
    SpfEntity entity;
    entity.id = id_value.ref;
    while (isIdentChar(cur.peek())) {
        entity.keyword.push_back(cur.get());
    }
    if (entity.keyword.empty()) {
        throw std::runtime_error("SPF: Entität ohne Keyword (#" +
                                 std::to_string(entity.id) + ")");
    }
    cur.skipWhitespace();
    entity.attributes = parseList(cur).items;
    cur.skipWhitespace();
    cur.expect(';');
    return entity;
}

}  // namespace

SpfReader SpfReader::parse(const std::string& text) {
    const std::string clean = stripComments(text);

    // ISO-Kopf-Pflicht: erstes nicht-leeres Token muss ISO-10303-21 sein.
    Cursor head(clean);
    head.skipWhitespace();
    if (clean.compare(head.pos(), std::string("ISO-10303-21").size(),
                      "ISO-10303-21") != 0) {
        throw std::runtime_error(
            "SPF: kein ISO-10303-21-Kopf (Format nicht erkannt)");
    }

    SpfReader reader;
    Cursor cur(clean);
    bool in_data = false;  // Sektions-Zustand: nur in DATA leben Entitäten.
    while (true) {
        cur.skipWhitespace();
        if (cur.eof()) {
            break;
        }
        if (cur.peek() == '#') {
            SpfEntity entity = parseEntity(cur);
            reader.entities_[entity.id] = std::move(entity);
            continue;
        }
        // Nicht-Entitäts-Statement: Sektions-Marke oder HEADER-Eintrag.
        const std::string word = readWord(cur);
        if (word == "DATA") {
            in_data = true;
        } else if (word == "ENDSEC") {
            in_data = false;
        } else if (in_data) {
            // In DATA ist nur eine #-Entität (oder ENDSEC) zulässig. Jedes
            // andere Token ist inhaltlich kaputt -> Wurf statt stillem Drop
            // (LH-FA-IO-001 Negative / atomarer Import, kein Unter-Zählen).
            throw std::runtime_error("SPF: ungültiges Token '" + word +
                                     "' in DATA-Sektion");
        }
        skipStatement(cur);  // Rest des Statements bis ';' (HEADER/Marken)
    }
    return reader;
}

std::vector<const SpfEntity*> SpfReader::byKeyword(
    const std::string& keyword) const {
    std::vector<const SpfEntity*> result;
    for (const auto& [id, entity] : entities_) {  // std::map: id-aufsteigend
        if (entity.keyword == keyword) {
            result.push_back(&entity);
        }
    }
    return result;
}

const SpfEntity* SpfReader::byId(int id) const {
    const auto it = entities_.find(id);
    return it == entities_.end() ? nullptr : &it->second;
}

const SpfValue* attributeAt(const SpfEntity& entity, std::size_t index) {
    if (index >= entity.attributes.size()) {
        return nullptr;
    }
    return &entity.attributes[index];
}

std::optional<int> asRef(const SpfValue* value) {
    if (value == nullptr || value->kind != SpfValue::Kind::Ref) {
        return std::nullopt;
    }
    return value->ref;
}

std::optional<double> asNumber(const SpfValue* value) {
    if (value == nullptr) {
        return std::nullopt;
    }
    if (value->kind == SpfValue::Kind::Real ||
        value->kind == SpfValue::Kind::Integer) {
        return value->number;
    }
    return std::nullopt;
}

std::optional<std::string> asString(const SpfValue* value) {
    if (value == nullptr || value->kind != SpfValue::Kind::String) {
        return std::nullopt;
    }
    return value->text;
}

}  // namespace bcad::adapters::io
