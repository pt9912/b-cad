#pragma once

#include <string>
#include <vector>

// IO-Adapter (ADR-0016 Option D): hand-gerollter, **format-agnostischer**
// Vektor-PDF-Writer (slice-025b). Kennt die PDF-**Syntax** (Objekt-Katalog,
// Seitenbaum, Content-Stream mit Grafik-Operatoren `m`/`l`/`S`/`re`, Text via
// Standard-Font Helvetica, `xref`/`trailer`/`startxref`/`%%EOF`), aber **keine**
// b-cad-Domäne — das Domänen→PDF-Mapping (Maßstab, Wand-Achsen) lebt im
// `pdf_export_adapter`. Lebt ausschließlich in `src/adapters/io/` (a-check
// Regel A/B); **keine** externe PDF-Bibliothek, **kein** Qt/OCC — reines C++/STL.

namespace bcad::adapters::io {

// Seitengröße in PDF-Punkten (1 pt = 1/72 Zoll). Ein Struct statt zweier
// benachbarter `double`-Parameter (bugprone-easily-swappable-parameters).
struct PdfPageSize {
    double width_pt{};
    double height_pt{};
};

// Baut ein valides mehrseitiges PDF 1.7. Objektgraph: Katalog → Seitenbaum →
// je Seite ein Page-Objekt (mit `/MediaBox`, `/Resources`→Helvetica-Font,
// `/Contents`→Stream) + ein Content-Stream-Objekt mit korrektem `/Length`. Reale
// Reader-Öffenbarkeit (nicht nur Byte-Konsistenz) ist der Vertrag (Review-MED-1).
class PdfWriter {
public:
    // Default A4 (595×842 pt).
    explicit PdfWriter(PdfPageSize page_size = {595.0, 842.0});

    // Beginnt eine neue Seite; folgende Zeichenbefehle füllen ihren Content-Stream.
    void beginPage();
    // Linienstärke (Operator `w`).
    void setLineWidth(double width);
    // Linie (Operatoren `m`/`l` + `S`), Koordinaten in Seiten-Punkten.
    void line(double x1, double y1, double x2, double y2);
    // Rechteck-Umriss (Operator `re` + `S`) — z. B. der Seitenrahmen.
    void rect(double x, double y, double width, double height);
    // Text in Helvetica (`BT`/`Tf`/`Td`/`Tj`/`ET`) — z. B. das Maßstabs-Label.
    void text(double x, double y, double font_size, const std::string& value);
    // Schließt die aktuelle Seite ab (ihr Content-Stream ist fertig).
    void endPage();

    // Serialisiert das vollständige PDF (alle Seiten). Deterministisch.
    std::string build() const;

    // Anzahl abgeschlossener Seiten.
    std::size_t pageCount() const { return pages_.size(); }

private:
    PdfPageSize page_size_;
    std::vector<std::string> pages_;  // fertiger Content-Stream je Seite
    std::string current_;             // Content-Stream der offenen Seite
    bool in_page_{false};
};

// Locale-freie PDF-Real-Formatierung (frei; Adapter wie Tests teilen sie):
// garantiert '.' als Dezimalzeichen (Muster `dxfReal`), feste 3 Nachkommastellen.
std::string pdfReal(double value);

}  // namespace bcad::adapters::io
