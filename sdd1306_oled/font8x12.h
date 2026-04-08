#ifndef FONT8X12_H_
#define FONT8X12_H_

#include <Arduino.h>

namespace Font8x12 {

const uint8_t* Lookup(uint16_t codepoint);
uint8_t GlyphWidth();
uint8_t GlyphHeight();
uint8_t GlyphAdvance();

}  // namespace Font8x12

#endif  // FONT8X12_H_
