#include "font8x12.h"

#include "font5x7.h"

namespace {
// 8x12 glyph format: 8 columns, each column is 12 bits (little-endian in 16-bit).
// Stored as 16 bytes per glyph: [col0_lo, col0_hi, col1_lo, col1_hi, ...]
void BuildFrom5x7(uint16_t cp, uint8_t out[16]) {
  const uint8_t* src = Font5x7::Lookup(cp);
  // Adafruit-style crisp upscale: nearest-neighbor with fixed baseline.
  // Source is 5x8, target is 8x12.
  for (uint8_t dx = 0U; dx < 8U; dx++) {
    uint16_t bits12 = 0U;
    for (uint8_t dy = 0U; dy < 12U; dy++) {
      uint8_t src_col = static_cast<uint8_t>((static_cast<uint16_t>(dx) * 5U) / 8U);
      uint8_t src_row = static_cast<uint8_t>((static_cast<uint16_t>(dy + 1U) * 8U) / 12U);
      if (src_col > 4U) {
        src_col = 4U;
      }
      if (src_row > 7U) {
        src_row = 7U;
      }
      if (((src[src_col] >> src_row) & 0x01U) != 0U) {
        bits12 = static_cast<uint16_t>(bits12 | static_cast<uint16_t>(1U << dy));
      }
    }
    out[static_cast<uint8_t>(dx * 2U)] = static_cast<uint8_t>(bits12 & 0xFFU);
    out[static_cast<uint8_t>(dx * 2U + 1U)] = static_cast<uint8_t>((bits12 >> 8U) & 0x0FU);
  }
}
}  // namespace

namespace Font8x12 {

const uint8_t* Lookup(uint16_t codepoint) {
  static uint8_t glyph[16];
  BuildFrom5x7(codepoint, glyph);
  return glyph;
}

uint8_t GlyphWidth() { return 8U; }
uint8_t GlyphHeight() { return 12U; }
uint8_t GlyphAdvance() { return 9U; }

}  // namespace Font8x12
