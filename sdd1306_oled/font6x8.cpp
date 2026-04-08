#include "font6x8.h"

#include "font5x7.h"

namespace Font6x8 {

const uint8_t* Lookup(uint16_t codepoint) {
  static uint8_t glyph6[6];
  const uint8_t* src = Font5x7::Lookup(codepoint);
  for (uint8_t i = 0U; i < 5U; i++) {
    glyph6[i] = src[i];
  }
  glyph6[5] = 0x00U;  // dedicated 6th column spacing
  return glyph6;
}

uint8_t GlyphWidth() { return 6U; }
uint8_t GlyphHeight() { return 8U; }
uint8_t GlyphAdvance() { return 7U; }

}  // namespace Font6x8
