// Hand-gerollter Vektor-PDF-Writer (ADR-0016 Option D, slice-025b). Reine
// PDF-Syntax (Objektgraph + Content-Stream-Operatoren + xref) — kein
// Domänen-Wissen, keine externe Bibliothek, kein Qt/OCC.

#include "adapters/io/pdf_writer.h"

#include <array>
#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>

namespace bcad::adapters::io {
namespace {

// 10-stellig null-aufgefüllter Byte-Offset (xref-Eintrag).
std::string pad10(std::size_t value) {
    std::string text = std::to_string(value);
    if (text.size() < 10) {
        text.insert(0, 10 - text.size(), '0');
    }
    return text;
}

// PDF-String-Literal-Escaping ('\', '(', ')').
std::string escapePdfString(const std::string& in) {
    std::string out;
    out.reserve(in.size());
    for (const char c : in) {
        if (c == '\\' || c == '(' || c == ')') {
            out += '\\';
        }
        out += c;
    }
    return out;
}

}  // namespace

std::string pdfReal(double value) {
    std::array<char, 32> buf{};
    std::snprintf(buf.data(), buf.size(), "%.3f", value);
    std::string text(buf.data());
    for (char& c : text) {  // defensiv locale-frei (Muster dxfReal)
        if (c == ',') {
            c = '.';
        }
    }
    return text;
}

PdfWriter::PdfWriter(PdfPageSize page_size) : page_size_(page_size) {}

void PdfWriter::beginPage() {
    current_.clear();
    in_page_ = true;
}

void PdfWriter::setLineWidth(double width) {
    current_ += pdfReal(width) + " w\n";
}

void PdfWriter::line(double x1, double y1, double x2, double y2) {
    current_ += pdfReal(x1) + " " + pdfReal(y1) + " m\n";
    current_ += pdfReal(x2) + " " + pdfReal(y2) + " l\n";
    current_ += "S\n";
}

void PdfWriter::rect(double x, double y, double width, double height) {
    current_ += pdfReal(x) + " " + pdfReal(y) + " " + pdfReal(width) + " " +
                pdfReal(height) + " re\n";
    current_ += "S\n";
}

void PdfWriter::text(double x, double y, double font_size,
                     const std::string& value) {
    current_ += "BT\n";
    current_ += "/F1 " + pdfReal(font_size) + " Tf\n";
    current_ += pdfReal(x) + " " + pdfReal(y) + " Td\n";
    current_ += "(" + escapePdfString(value) + ") Tj\n";
    current_ += "ET\n";
}

void PdfWriter::endPage() {
    pages_.push_back(current_);
    current_.clear();
    in_page_ = false;
}

std::string PdfWriter::build() const {
    const int page_count = static_cast<int>(pages_.size());
    // Objektnummern: 1 Katalog, 2 Seitenbaum, 3 Font; je Seite 2 (Page +
    // Content), Page = 4+2k, Content = 5+2k.
    const int object_count = 3 + (2 * page_count);

    std::string out = "%PDF-1.7\n";
    out += '%';  // Binär-Marker-Kommentar (Reader-Robustheit), byte-explizit
    out += static_cast<char>(0xE2);
    out += static_cast<char>(0xE3);
    out += static_cast<char>(0xCF);
    out += static_cast<char>(0xD3);
    out += '\n';

    std::vector<std::size_t> offset(static_cast<std::size_t>(object_count) + 1,
                                    0);
    const auto emit = [&](int number, const std::string& body) {
        offset[static_cast<std::size_t>(number)] = out.size();
        out += std::to_string(number) + " 0 obj\n" + body + "\nendobj\n";
    };

    emit(1, "<< /Type /Catalog /Pages 2 0 R >>");

    std::string kids;
    for (int k = 0; k < page_count; ++k) {
        if (k != 0) {
            kids += " ";
        }
        kids += std::to_string(4 + (2 * k)) + " 0 R";
    }
    emit(2, "<< /Type /Pages /Kids [" + kids + "] /Count " +
                std::to_string(page_count) + " >>");

    emit(3, "<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica >>");

    const std::string media_box = "[0 0 " + pdfReal(page_size_.width_pt) + " " +
                                  pdfReal(page_size_.height_pt) + "]";

    for (int k = 0; k < page_count; ++k) {
        const int page_obj = 4 + (2 * k);
        const int content_obj = 5 + (2 * k);
        emit(page_obj,
             "<< /Type /Page /Parent 2 0 R /MediaBox " + media_box +
                 " /Resources << /Font << /F1 3 0 R >> >> /Contents " +
                 std::to_string(content_obj) + " 0 R >>");

        // Content-Stream: /Length == exakte Stream-Byte-Zahl (der Stream liegt
        // zwischen "stream\n" und "\nendstream").
        const std::string& content = pages_[static_cast<std::size_t>(k)];
        offset[static_cast<std::size_t>(content_obj)] = out.size();
        out += std::to_string(content_obj) + " 0 obj\n";
        out += "<< /Length " + std::to_string(content.size()) + " >>\n";
        out += "stream\n";
        out += content;
        out += "\nendstream\nendobj\n";
    }

    const std::size_t xref_offset = out.size();
    out += "xref\n";
    out += "0 " + std::to_string(object_count + 1) + "\n";
    out += "0000000000 65535 f\r\n";
    for (int i = 1; i <= object_count; ++i) {
        out += pad10(offset[static_cast<std::size_t>(i)]) + " 00000 n\r\n";
    }

    out += "trailer\n";
    out += "<< /Size " + std::to_string(object_count + 1) + " /Root 1 0 R >>\n";
    out += "startxref\n";
    out += std::to_string(xref_offset) + "\n";
    out += "%%EOF\n";
    return out;
}

}  // namespace bcad::adapters::io
