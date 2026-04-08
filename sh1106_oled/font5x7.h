#ifndef FONT5X7_H_
#define FONT5X7_H_

#include <Arduino.h>

namespace Font5x7 {

const uint8_t* Lookup(uint16_t codepoint);
uint8_t GlyphWidth();
uint8_t GlyphHeight();
uint8_t GlyphAdvance();

}  // namespace Font5x7

#endif  // FONT5X7_H_
