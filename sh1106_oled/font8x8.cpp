#include "font8x8.h"

#include "font5x7.h"

namespace Font8x8 {

const uint8_t* Lookup(uint16_t codepoint) {
  static uint8_t glyph8[8];
  const uint8_t* src = Font5x7::Lookup(codepoint);
  glyph8[0] = src[0];
  for (uint8_t i = 0U; i < 5U; i++) {
    glyph8[i + 1U] = src[i];
  }
  glyph8[6] = src[4];
  glyph8[7] = 0x00U;
  return glyph8;
}

uint8_t GlyphWidth() { return 8U; }
uint8_t GlyphHeight() { return 8U; }
uint8_t GlyphAdvance() { return 9U; }

}  // namespace Font8x8
