#include "font_engine.h"

void FontEngine::DrawGlyph(GfxCanvas& canvas, uint8_t x, uint8_t y, const uint8_t* glyph,
                           const Style& style) {
  if (style.glyph_height <= 8U) {
    for (uint8_t col = 0U; col < style.glyph_width; col++) {
      const uint8_t bits = glyph[col];
      for (uint8_t row = 0U; row < style.glyph_height; row++) {
        if (((bits >> row) & 0x01U) == 0U) {
          continue;
        }
        for (uint8_t sx = 0U; sx < style.scale; sx++) {
          for (uint8_t sy = 0U; sy < style.scale; sy++) {
            canvas.DrawPixel(static_cast<uint8_t>(x + (col * style.scale) + sx),
                             static_cast<uint8_t>(y + (row * style.scale) + sy), true);
          }
        }
      }
    }
    return;
  }

  // Tall glyphs (e.g. 8x12): each column uses 16-bit packed rows.
  for (uint8_t col = 0U; col < style.glyph_width; col++) {
    const uint8_t lo = glyph[static_cast<uint8_t>(col * 2U)];
    const uint8_t hi = glyph[static_cast<uint8_t>(col * 2U + 1U)];
    const uint16_t bits = static_cast<uint16_t>(lo | (static_cast<uint16_t>(hi) << 8U));
    for (uint8_t row = 0U; row < style.glyph_height && row < 16U; row++) {
      if (((bits >> row) & 0x01U) == 0U) {
        continue;
      }
      for (uint8_t sx = 0U; sx < style.scale; sx++) {
        for (uint8_t sy = 0U; sy < style.scale; sy++) {
          canvas.DrawPixel(static_cast<uint8_t>(x + (col * style.scale) + sx),
                           static_cast<uint8_t>(y + (row * style.scale) + sy), true);
        }
      }
    }
  }
}

uint16_t FontEngine::DecodeUtf8Char(const char* text, size_t* index) {
  const uint8_t b0 = static_cast<uint8_t>(text[*index]);
  if ((b0 & 0x80U) == 0U) {
    return b0;
  }
  const uint8_t b1 = static_cast<uint8_t>(text[*index + 1U]);
  if (((b0 & 0xE0U) == 0xC0U) && ((b1 & 0xC0U) == 0x80U)) {
    *index += 1U;
    return static_cast<uint16_t>(((b0 & 0x1FU) << 6U) | (b1 & 0x3FU));
  }
  return '?';
}

void FontEngine::DrawText(GfxCanvas& canvas, uint8_t x, uint8_t y, const char* text, bool utf8,
                          GlyphLookupFn glyph_lookup, const Style& style) {
  if (text == nullptr || glyph_lookup == nullptr) {
    return;
  }
  const uint8_t advance = static_cast<uint8_t>(style.glyph_advance * style.scale);
  const uint8_t line_h = static_cast<uint8_t>(style.glyph_height * style.scale);
  uint8_t cursor_x = x;
  uint8_t cursor_y = y;

  for (size_t i = 0U; text[i] != '\0'; i++) {
    if (text[i] == '\n') {
      cursor_x = x;
      cursor_y = static_cast<uint8_t>(cursor_y + line_h);
      continue;
    }
    if (cursor_x > static_cast<uint8_t>(canvas.Width() - advance)) {
      if (style.wrap) {
        cursor_x = x;
        cursor_y = static_cast<uint8_t>(cursor_y + line_h);
      } else {
        break;
      }
    }
    const uint16_t cp = utf8 ? DecodeUtf8Char(text, &i) : static_cast<uint8_t>(text[i]);
    const uint8_t* glyph = glyph_lookup(cp);
    DrawGlyph(canvas, cursor_x, cursor_y, glyph, style);
    cursor_x = static_cast<uint8_t>(cursor_x + advance);
  }
}

void FontEngine::GetTextBounds(const char* text, const Style& style, uint8_t* out_w, uint8_t* out_h) {
  const uint8_t advance = static_cast<uint8_t>(style.glyph_advance * style.scale);
  const uint8_t line_h = static_cast<uint8_t>(style.glyph_height * style.scale);
  uint8_t current_w = 0U;
  uint8_t max_w = 0U;
  uint8_t lines = 1U;
  for (size_t i = 0U; text[i] != '\0'; i++) {
    if (text[i] == '\n') {
      if (current_w > max_w) {
        max_w = current_w;
      }
      current_w = 0U;
      lines++;
      continue;
    }
    current_w = static_cast<uint8_t>(current_w + advance);
  }
  if (current_w > max_w) {
    max_w = current_w;
  }
  if (out_w != nullptr) {
    *out_w = max_w;
  }
  if (out_h != nullptr) {
    *out_h = static_cast<uint8_t>(lines * line_h);
  }
}

void FontEngine::DrawTextAligned(GfxCanvas& canvas, uint8_t x, uint8_t y, uint8_t w, const char* text,
                                 GlyphLookupFn glyph_lookup, const Style& style) {
  uint8_t tw = 0U;
  uint8_t th = 0U;
  GetTextBounds(text, style, &tw, &th);
  (void)th;
  uint8_t sx = x;
  if (style.align == AlignCenter && tw < w) {
    sx = static_cast<uint8_t>(x + ((w - tw) / 2U));
  } else if (style.align == AlignRight && tw < w) {
    sx = static_cast<uint8_t>(x + (w - tw));
  }
  DrawText(canvas, sx, y, text, false, glyph_lookup, style);
}

void FontEngine::DrawTextBox(GfxCanvas& canvas, uint8_t x, uint8_t y, uint8_t w, uint8_t h, const char* text,
                             GlyphLookupFn glyph_lookup, const Style& style) {
  const uint8_t line_h = static_cast<uint8_t>(style.glyph_height * style.scale);
  uint8_t cy = y;
  const char* p = text;
  char line[32];
  while (*p != '\0' && (cy + line_h) <= static_cast<uint8_t>(y + h)) {
    uint8_t idx = 0U;
    while (*p != '\0' && *p != '\n' && idx < static_cast<uint8_t>(sizeof(line) - 1U)) {
      line[idx++] = *p++;
    }
    line[idx] = '\0';
    DrawTextAligned(canvas, x, cy, w, line, glyph_lookup, style);
    cy = static_cast<uint8_t>(cy + line_h);
    if (*p == '\n') {
      p++;
    }
  }
}
