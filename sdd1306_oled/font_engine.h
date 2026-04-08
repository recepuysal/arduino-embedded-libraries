#ifndef FONT_ENGINE_H_
#define FONT_ENGINE_H_

#include <Arduino.h>

#include "gfx_canvas.h"

class FontEngine {
 public:
  enum Align : uint8_t {
    AlignLeft = 0U,
    AlignCenter = 1U,
    AlignRight = 2U,
  };

  typedef const uint8_t* (*GlyphLookupFn)(uint16_t codepoint);

  struct Style {
    uint8_t glyph_width;
    uint8_t glyph_height;
    uint8_t glyph_advance;
    uint8_t scale;
    bool wrap;
    Align align;
  };

  static void DrawText(GfxCanvas& canvas, uint8_t x, uint8_t y, const char* text, bool utf8,
                       GlyphLookupFn glyph_lookup, const Style& style);
  static void DrawTextAligned(GfxCanvas& canvas, uint8_t x, uint8_t y, uint8_t w, const char* text,
                              GlyphLookupFn glyph_lookup, const Style& style);
  static void DrawTextBox(GfxCanvas& canvas, uint8_t x, uint8_t y, uint8_t w, uint8_t h, const char* text,
                          GlyphLookupFn glyph_lookup, const Style& style);
  static void GetTextBounds(const char* text, const Style& style, uint8_t* out_w, uint8_t* out_h);

 private:
  static uint16_t DecodeUtf8Char(const char* text, size_t* index);
  static void DrawGlyph(GfxCanvas& canvas, uint8_t x, uint8_t y, const uint8_t* glyph,
                        const Style& style);
};

#endif  // FONT_ENGINE_H_
