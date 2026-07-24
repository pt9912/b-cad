// Hand-gerollter Raster-PNG-Encoder (ADR-0016 Option D, slice-025c). Reine
// PNG-Syntax (Signatur + Chunks + CRC-32; IDAT = zlib-Header + stored-DEFLATE +
// Adler-32) — kein Domänen-Wissen, keine externe Bibliothek, kein Qt/OCC/zlib.

#include "adapters/io/png_writer.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace bcad::adapters::io {
namespace {

constexpr std::uint32_t kCrcPolynom = 0xEDB88320U;
constexpr std::uint32_t kAdlerMod = 65521U;
constexpr std::size_t kMaxStoredBlock = 65535U;

// CRC-32 (IEEE, PNG-Chunk) über Typ+Daten. Eigenständige Herleitung (keine
// Tabelle) — der Test implementiert dieselbe Spec unabhängig (Review-MED-1).
std::uint32_t crc32(const std::string& data) {
    std::uint32_t crc = 0xFFFFFFFFU;
    for (const char ch : data) {
        crc ^= static_cast<unsigned char>(ch);
        for (int k = 0; k < 8; ++k) {
            crc = (crc >> 1U) ^ ((crc & 1U) != 0U ? kCrcPolynom : 0U);
        }
    }
    return crc ^ 0xFFFFFFFFU;
}

// Adler-32 (zlib) über die Roh-Scanlines.
std::uint32_t adler32(const std::string& data) {
    std::uint32_t a = 1U;
    std::uint32_t b = 0U;
    for (const char ch : data) {
        a = (a + static_cast<unsigned char>(ch)) % kAdlerMod;
        b = (b + a) % kAdlerMod;
    }
    return (b << 16U) | a;
}

void putBe32(std::string& out, std::uint32_t value) {
    out += static_cast<char>((value >> 24U) & 0xFFU);
    out += static_cast<char>((value >> 16U) & 0xFFU);
    out += static_cast<char>((value >> 8U) & 0xFFU);
    out += static_cast<char>(value & 0xFFU);
}

// Ein PNG-Chunk: Länge (be32) + Typ (4) + Daten + CRC-32 (be32) über Typ+Daten.
void emitChunk(std::string& out, const std::string& type, const std::string& data) {
    putBe32(out, static_cast<std::uint32_t>(data.size()));
    const std::string typed_data = type + data;
    out += typed_data;
    putBe32(out, crc32(typed_data));
}

// zlib-Strom aus `raw`: 2-Byte-Header + stored-DEFLATE-Blöcke (≤65535 B, nur der
// letzte BFINAL) + Adler-32 (be32) der Roh-Daten.
std::string zlibStored(const std::string& raw) {
    std::string out;
    out += static_cast<char>(0x78);  // CMF: CM=8, CINFO=7
    out += static_cast<char>(0x01);  // FLG: (0x78<<8|0x01)%31==0, FDICT=0
    std::size_t pos = 0;
    do {
        const std::size_t len =
            raw.size() - pos < kMaxStoredBlock ? raw.size() - pos : kMaxStoredBlock;
        const bool is_final = pos + len >= raw.size();
        out += static_cast<char>(is_final ? 0x01 : 0x00);  // BFINAL + BTYPE=00
        const auto len16 = static_cast<std::uint16_t>(len);
        const auto nlen16 = static_cast<std::uint16_t>(~len16);
        out += static_cast<char>(len16 & 0xFFU);
        out += static_cast<char>((len16 >> 8U) & 0xFFU);
        out += static_cast<char>(nlen16 & 0xFFU);
        out += static_cast<char>((nlen16 >> 8U) & 0xFFU);
        out += raw.substr(pos, len);
        pos += len;
    } while (pos < raw.size());
    putBe32(out, adler32(raw));
    return out;
}

}  // namespace

Bitmap::Bitmap(RasterSize size, Rgb background)
    : width_(size.width_px), height_(size.height_px),
      buf_(static_cast<std::size_t>(size.width_px) *
               static_cast<std::size_t>(size.height_px) * 3U,
           0) {
    const std::size_t pixels = static_cast<std::size_t>(size.width_px) *
                               static_cast<std::size_t>(size.height_px);
    for (std::size_t i = 0; i < pixels; ++i) {
        buf_[(i * 3U)] = background.r;
        buf_[(i * 3U) + 1] = background.g;
        buf_[(i * 3U) + 2] = background.b;
    }
}

void Bitmap::setPixel(PixelPoint p, Rgb color) {
    if (p.x < 0 || p.x >= width_ || p.y < 0 || p.y >= height_) {
        return;  // geklemmt (kein Out-of-Bounds-Schreiben)
    }
    const auto idx = (static_cast<std::size_t>(p.y) * static_cast<std::size_t>(width_) +
                      static_cast<std::size_t>(p.x)) *
                     3U;
    buf_[idx] = color.r;
    buf_[idx + 1] = color.g;
    buf_[idx + 2] = color.b;
}

void Bitmap::drawLine(PixelSegment seg, Rgb color) {
    int x = seg.a.x;
    int y = seg.a.y;
    const int dx = seg.b.x > x ? seg.b.x - x : x - seg.b.x;
    const int dy = -(seg.b.y > y ? seg.b.y - y : y - seg.b.y);
    const int sx = x < seg.b.x ? 1 : -1;
    const int sy = y < seg.b.y ? 1 : -1;
    int err = dx + dy;
    while (true) {
        setPixel({x, y}, color);
        if (x == seg.b.x && y == seg.b.y) {
            break;
        }
        const int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y += sy;
        }
    }
}

std::string encodePng(const Bitmap& bitmap) {
    const int w = bitmap.width();
    const int h = bitmap.height();
    const std::vector<unsigned char>& px = bitmap.rgb();

    // Roh-Scanlines: je Zeile ein Filterbyte 0 (None) + width*3 RGB-Bytes.
    std::string raw;
    raw.reserve(static_cast<std::size_t>(h) *
                (1U + static_cast<std::size_t>(w) * 3U));
    for (int y = 0; y < h; ++y) {
        raw += static_cast<char>(0);
        const auto row = static_cast<std::size_t>(y) * static_cast<std::size_t>(w) * 3U;
        for (std::size_t i = 0; i < static_cast<std::size_t>(w) * 3U; ++i) {
            raw += static_cast<char>(px[row + i]);
        }
    }

    std::string ihdr;
    putBe32(ihdr, static_cast<std::uint32_t>(w));
    putBe32(ihdr, static_cast<std::uint32_t>(h));
    ihdr += static_cast<char>(8);  // bit depth
    ihdr += static_cast<char>(2);  // color type: RGB
    ihdr += static_cast<char>(0);  // compression
    ihdr += static_cast<char>(0);  // filter
    ihdr += static_cast<char>(0);  // interlace

    constexpr std::array<unsigned char, 8> kSignature = {137, 80, 78, 71,
                                                         13,  10, 26, 10};
    std::string out;
    for (const unsigned char c : kSignature) {
        out += static_cast<char>(c);
    }
    emitChunk(out, "IHDR", ihdr);
    // Statische Text-Metadaten (slice-045): tEXt-Chunks (keyword \0 text, Latin-1)
    // direkt nach IHDR, vor IDAT. BEWUSST kein tIME-Chunk (dynamisch →
    // nicht-reproduzierbar); die Werte sind konstant → byte-deterministisch (der
    // Byte-Golden bleibt tragfähig). Konsistent mit STEP/IFC ('b-cad').
    emitChunk(out, "tEXt",
              std::string("Software").append(1, '\0').append("b-cad"));
    emitChunk(out, "tEXt",
              std::string("Title").append(1, '\0').append("b-cad Plan-Export"));
    emitChunk(out, "IDAT", zlibStored(raw));
    emitChunk(out, "IEND", "");
    return out;
}

}  // namespace bcad::adapters::io
