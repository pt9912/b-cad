#pragma once

#include <string>
#include <utility>
#include <vector>

// IO-Adapter (ADR-0016 Option D, slice-025c): hand-gerollter, **format-agnostischer**
// Raster-PNG-Encoder. Kennt die PNG-**Syntax** (8-Byte-Signatur, Chunks mit
// Länge+Typ+CRC-32, `IHDR`/`IDAT`/`IEND`; `IDAT` = zlib-Header + **stored-DEFLATE**-
// Blöcke + Adler-32), aber **keine** b-cad-Domäne — das Domänen→Raster-Mapping lebt im
// `png_export_adapter`. Lebt ausschließlich in `src/adapters/io/` (a-check Regel A/B);
// **keine** externe PNG-/zlib-Bibliothek, **kein** Qt/OCC — reines C++/STL.

namespace bcad::adapters::io {

// Structs statt benachbarter gleichtypiger Skalar-Parameter (bugprone-easily-
// swappable-parameters feuert bei separat gespeicherten Params — slice-025b-Beleg).
struct RasterSize {
    int width_px{};
    int height_px{};
};
struct Rgb {
    unsigned char r{};
    unsigned char g{};
    unsigned char b{};
};
struct PixelPoint {
    int x{};
    int y{};
};
struct PixelSegment {
    PixelPoint a{};
    PixelPoint b{};
};

// 8-bit-RGB-Rasterpuffer (Zeilen top-to-bottom, row-major). Zeichnet Linien
// (Bresenham, auf die Leinwand geklemmt).
class Bitmap {
public:
    Bitmap(RasterSize size, Rgb background);
    void drawLine(PixelSegment seg, Rgb color);
    // Zeichnet ASCII-Text (fester 5×7-Font, slice-046b) ab `origin` (obere linke
    // Ecke des ersten Glyphs), links→rechts. Off-Canvas-Pixel werden von `setPixel`
    // geklemmt (out-of-bounds-sicher by construction, unabhängig von der Textlänge).
    void drawText(PixelPoint origin, const std::string& text, Rgb color);
    int width() const { return width_; }
    int height() const { return height_; }
    const std::vector<unsigned char>& rgb() const { return buf_; }

private:
    void setPixel(PixelPoint p, Rgb color);
    int width_;
    int height_;
    std::vector<unsigned char> buf_;  // width*height*3
};

// Encodiert die Bitmap als valides PNG (8-bit RGB, `IDAT` unkomprimiert/stored-DEFLATE).
// `text_chunks` (slice-046b): zusätzliche `tEXt`-Chunks als `(keyword, text)`-Paare
// (nach IHDR, vor IDAT) — der Adapter komponiert sie (der Writer bleibt domänen-frei).
std::string encodePng(
    const Bitmap& bitmap,
    const std::vector<std::pair<std::string, std::string>>& text_chunks = {});

}  // namespace bcad::adapters::io
