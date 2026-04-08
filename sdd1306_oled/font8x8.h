#ifndef FONT8X8_H_
#define FONT8X8_H_

#include <Arduino.h>

namespace Font8x8 {

const uint8_t* Lookup(uint16_t codepoint);
uint8_t GlyphWidth();
uint8_t GlyphHeight();
uint8_t GlyphAdvance();

}  // namespace Font8x8

#endif  // FONT8X8_H_
