#ifndef FONT6X8_H_
#define FONT6X8_H_

#include <Arduino.h>

namespace Font6x8 {

const uint8_t* Lookup(uint16_t codepoint);
uint8_t GlyphWidth();
uint8_t GlyphHeight();
uint8_t GlyphAdvance();

}  // namespace Font6x8

#endif  // FONT6X8_H_
