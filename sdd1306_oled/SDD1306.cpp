#include "SDD1306.h"

#include "font5x7.h"
#include "font6x8.h"
#include "font8x8.h"
#include "font8x12.h"
#include "font_engine.h"

namespace {
FontEngine::Align ToEngineAlign(Sdd1306::TextAlign align) {
  if (align == Sdd1306::AlignCenter) return FontEngine::AlignCenter;
  if (align == Sdd1306::AlignRight) return FontEngine::AlignRight;
  return FontEngine::AlignLeft;
}

FontEngine::GlyphLookupFn SelectLookup(Sdd1306::FontType font) {
  if (font == Sdd1306::FontMedium) return Font6x8::Lookup;
  if (font == Sdd1306::FontLarge) return Font8x12::Lookup;
  return Font5x7::Lookup;
}

uint8_t FontBaseScale(Sdd1306::FontType font) {
  (void)font;
  return 1U;
}
}  // namespace

Sdd1306::Sdd1306(TwoWire& wireIn, uint8_t addressIn)
    : wire(wireIn),
      address_7bit(addressIn),
      buffer{0},
      active_font(FontSmall),
      text_scale(1U),
      text_wrap(false),
      text_align(AlignLeft),
      rotation(0U),
      last_i2c_error(0U),
      has_dirty(true),
      dirty_min_x(0U),
      dirty_max_x(width_px - 1U),
      dirty_min_page(0U),
      dirty_max_page(page_count - 1U) {}

bool Sdd1306::Begin(uint32_t pinSda, uint32_t pinScl, uint32_t i2cClockHz) {
  wire.setSDA(pinSda);
  wire.setSCL(pinScl);
  wire.begin();
  wire.setClock(i2cClockHz);

  // Basic init sequence
  const bool ok = SendCommand(0xAE) &&  // display off
                  SendCommand(0xD5) && SendCommand(0x80) && SendCommand(0xA8) &&
                  SendCommand(0x3F) &&  // 64 lines
                  SendCommand(0xD3) && SendCommand(0x00) && SendCommand(0x40) &&
                  SendCommand(0x8D) && SendCommand(0x14) &&  // charge pump on
                  SendCommand(0xA1) &&                       // segment remap
                  SendCommand(0xC8) &&                       // com scan dec
                  SendCommand(0xDA) && SendCommand(0x12) && SendCommand(0x81) &&
                  SendCommand(0xCF) && SendCommand(0xD9) && SendCommand(0xF1) &&
                  SendCommand(0xDB) && SendCommand(0x40) && SendCommand(0xA4) &&
                  SendCommand(0xA6) && SendCommand(0xAF);  // display on

  Clear();
  return ok && Display();
}

void Sdd1306::Clear() {
  memset(buffer, 0, sizeof(buffer));
  has_dirty = true;
  dirty_min_x = 0U;
  dirty_max_x = static_cast<uint8_t>(width_px - 1U);
  dirty_min_page = 0U;
  dirty_max_page = static_cast<uint8_t>(page_count - 1U);
}

bool Sdd1306::Display() {
  if (!has_dirty) {
    return true;
  }

  const uint8_t start_x = dirty_min_x;
  const uint8_t end_x = dirty_max_x;
  const uint8_t length = static_cast<uint8_t>(end_x - start_x + 1U);
  bool ok = true;

  for (uint8_t page = dirty_min_page; page <= dirty_max_page; page++) {
    SendCommand(static_cast<uint8_t>(0xB0U + page));
    const uint8_t col = static_cast<uint8_t>(page_offset + start_x);
    ok = ok && SendCommand(static_cast<uint8_t>(0x10U + ((col >> 4) & 0x0FU)));
    ok = ok && SendCommand(static_cast<uint8_t>(col & 0x0FU));
    ok = ok && SendData(&buffer[(page * width_px) + start_x], length);
    if (page == 255U) {
      break;
    }
  }
  has_dirty = false;
  return ok;
}

void Sdd1306::DrawPixel(uint8_t x, uint8_t y, bool color) {
  if ((x >= width_px) || (y >= height_px)) {
    return;
  }
  uint8_t tx = x;
  uint8_t ty = y;
  if (rotation == 1U) {
    tx = static_cast<uint8_t>((height_px - 1U) - y);
    ty = x;
  } else if (rotation == 2U) {
    tx = static_cast<uint8_t>((width_px - 1U) - x);
    ty = static_cast<uint8_t>((height_px - 1U) - y);
  } else if (rotation == 3U) {
    tx = y;
    ty = static_cast<uint8_t>((width_px - 1U) - x);
  }
  if ((tx >= width_px) || (ty >= height_px)) {
    return;
  }
  const uint16_t index = static_cast<uint16_t>((ty / 8U) * width_px + tx);
  const uint8_t bit = static_cast<uint8_t>(1U << (ty % 8U));
  if (color) {
    buffer[index] = static_cast<uint8_t>(buffer[index] | bit);
  } else {
    buffer[index] = static_cast<uint8_t>(buffer[index] & static_cast<uint8_t>(~bit));
  }
  MarkDirty(tx, ty);
}

void Sdd1306::DrawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool color) {
  int16_t dx = abs(static_cast<int16_t>(x1) - static_cast<int16_t>(x0));
  int16_t sx = (x0 < x1) ? 1 : -1;
  int16_t dy = -abs(static_cast<int16_t>(y1) - static_cast<int16_t>(y0));
  int16_t sy = (y0 < y1) ? 1 : -1;
  int16_t err = dx + dy;
  int16_t cx = x0;
  int16_t cy = y0;

  while (true) {
    if ((cx >= 0) && (cy >= 0) && (cx < width_px) && (cy < height_px)) {
      DrawPixel(static_cast<uint8_t>(cx), static_cast<uint8_t>(cy), color);
    }
    if ((cx == x1) && (cy == y1)) {
      break;
    }
    const int16_t e2 = static_cast<int16_t>(2 * err);
    if (e2 >= dy) {
      err = static_cast<int16_t>(err + dy);
      cx = static_cast<int16_t>(cx + sx);
    }
    if (e2 <= dx) {
      err = static_cast<int16_t>(err + dx);
      cy = static_cast<int16_t>(cy + sy);
    }
  }
}

void Sdd1306::DrawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool color) {
  if ((w == 0U) || (h == 0U)) {
    return;
  }
  DrawLine(x, y, static_cast<uint8_t>(x + w - 1U), y, color);
  DrawLine(x, static_cast<uint8_t>(y + h - 1U), static_cast<uint8_t>(x + w - 1U),
           static_cast<uint8_t>(y + h - 1U), color);
  DrawLine(x, y, x, static_cast<uint8_t>(y + h - 1U), color);
  DrawLine(static_cast<uint8_t>(x + w - 1U), y, static_cast<uint8_t>(x + w - 1U),
           static_cast<uint8_t>(y + h - 1U), color);
}

void Sdd1306::FillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool color) {
  for (uint8_t yy = 0; yy < h; yy++) {
    DrawLine(x, static_cast<uint8_t>(y + yy), static_cast<uint8_t>(x + w - 1U),
             static_cast<uint8_t>(y + yy), color);
  }
}

void Sdd1306::DrawCircle(uint8_t x0, uint8_t y0, uint8_t r, bool color) {
  int16_t f = static_cast<int16_t>(1 - r);
  int16_t ddF_x = 1;
  int16_t ddF_y = static_cast<int16_t>(-2 * r);
  int16_t x = 0;
  int16_t y = r;
  DrawPixel(x0, static_cast<uint8_t>(y0 + r), color);
  DrawPixel(x0, static_cast<uint8_t>(y0 - r), color);
  DrawPixel(static_cast<uint8_t>(x0 + r), y0, color);
  DrawPixel(static_cast<uint8_t>(x0 - r), y0, color);
  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y = static_cast<int16_t>(ddF_y + 2);
      f = static_cast<int16_t>(f + ddF_y);
    }
    x++;
    ddF_x = static_cast<int16_t>(ddF_x + 2);
    f = static_cast<int16_t>(f + ddF_x);
    DrawPixel(static_cast<uint8_t>(x0 + x), static_cast<uint8_t>(y0 + y), color);
    DrawPixel(static_cast<uint8_t>(x0 - x), static_cast<uint8_t>(y0 + y), color);
    DrawPixel(static_cast<uint8_t>(x0 + x), static_cast<uint8_t>(y0 - y), color);
    DrawPixel(static_cast<uint8_t>(x0 - x), static_cast<uint8_t>(y0 - y), color);
    DrawPixel(static_cast<uint8_t>(x0 + y), static_cast<uint8_t>(y0 + x), color);
    DrawPixel(static_cast<uint8_t>(x0 - y), static_cast<uint8_t>(y0 + x), color);
    DrawPixel(static_cast<uint8_t>(x0 + y), static_cast<uint8_t>(y0 - x), color);
    DrawPixel(static_cast<uint8_t>(x0 - y), static_cast<uint8_t>(y0 - x), color);
  }
}

void Sdd1306::FillCircle(uint8_t x0, uint8_t y0, uint8_t r, bool color) {
  DrawLine(x0, static_cast<uint8_t>(y0 - r), x0, static_cast<uint8_t>(y0 + r), color);
  int16_t f = static_cast<int16_t>(1 - r);
  int16_t ddF_x = 1;
  int16_t ddF_y = static_cast<int16_t>(-2 * r);
  int16_t x = 0;
  int16_t y = r;
  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y = static_cast<int16_t>(ddF_y + 2);
      f = static_cast<int16_t>(f + ddF_y);
    }
    x++;
    ddF_x = static_cast<int16_t>(ddF_x + 2);
    f = static_cast<int16_t>(f + ddF_x);
    DrawLine(static_cast<uint8_t>(x0 - x), static_cast<uint8_t>(y0 + y),
             static_cast<uint8_t>(x0 + x), static_cast<uint8_t>(y0 + y), color);
    DrawLine(static_cast<uint8_t>(x0 - x), static_cast<uint8_t>(y0 - y),
             static_cast<uint8_t>(x0 + x), static_cast<uint8_t>(y0 - y), color);
    DrawLine(static_cast<uint8_t>(x0 - y), static_cast<uint8_t>(y0 + x),
             static_cast<uint8_t>(x0 + y), static_cast<uint8_t>(y0 + x), color);
    DrawLine(static_cast<uint8_t>(x0 - y), static_cast<uint8_t>(y0 - x),
             static_cast<uint8_t>(x0 + y), static_cast<uint8_t>(y0 - x), color);
  }
}

void Sdd1306::DrawTriangle(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2,
                          bool color) {
  DrawLine(x0, y0, x1, y1, color);
  DrawLine(x1, y1, x2, y2, color);
  DrawLine(x2, y2, x0, y0, color);
}

void Sdd1306::FillTriangle(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2,
                          bool color) {
  if (y0 > y1) {
    const uint8_t ty = y0;
    y0 = y1;
    y1 = ty;
    const uint8_t tx = x0;
    x0 = x1;
    x1 = tx;
  }
  if (y1 > y2) {
    const uint8_t ty = y1;
    y1 = y2;
    y2 = ty;
    const uint8_t tx = x1;
    x1 = x2;
    x2 = tx;
  }
  if (y0 > y1) {
    const uint8_t ty = y0;
    y0 = y1;
    y1 = ty;
    const uint8_t tx = x0;
    x0 = x1;
    x1 = tx;
  }

  const int16_t total_height = static_cast<int16_t>(y2 - y0);
  for (int16_t i = 0; i <= total_height; i++) {
    const bool second_half = i > static_cast<int16_t>(y1 - y0) || (y1 == y0);
    const int16_t segment_height =
        second_half ? static_cast<int16_t>(y2 - y1) : static_cast<int16_t>(y1 - y0);
    if (segment_height == 0) {
      continue;
    }
    const float alpha = static_cast<float>(i) / static_cast<float>(total_height == 0 ? 1 : total_height);
    const float beta = static_cast<float>(i - (second_half ? static_cast<int16_t>(y1 - y0) : 0)) /
                       static_cast<float>(segment_height);
    int16_t ax = static_cast<int16_t>(x0 + (x2 - x0) * alpha);
    int16_t bx = second_half ? static_cast<int16_t>(x1 + (x2 - x1) * beta)
                             : static_cast<int16_t>(x0 + (x1 - x0) * beta);
    if (ax > bx) {
      const int16_t t = ax;
      ax = bx;
      bx = t;
    }
    DrawLine(static_cast<uint8_t>(ax), static_cast<uint8_t>(y0 + i), static_cast<uint8_t>(bx),
             static_cast<uint8_t>(y0 + i), color);
  }
}

void Sdd1306::DrawBitmap(uint8_t x, uint8_t y, const uint8_t* bitmap, uint8_t w, uint8_t h, bool color) {
  if (bitmap == nullptr) {
    return;
  }
  const uint8_t row_bytes = static_cast<uint8_t>((w + 7U) / 8U);
  for (uint8_t yy = 0; yy < h; yy++) {
    for (uint8_t xx = 0; xx < w; xx++) {
      const uint8_t b = bitmap[static_cast<uint16_t>(yy) * row_bytes + (xx / 8U)];
      const bool on = (b & static_cast<uint8_t>(0x80U >> (xx % 8U))) != 0U;
      if (on) {
        DrawPixel(static_cast<uint8_t>(x + xx), static_cast<uint8_t>(y + yy), color);
      }
    }
  }
}

void Sdd1306::DrawLogo(uint8_t x, uint8_t y, const Logo& logo, bool color) {
  if (logo.data == nullptr || logo.width == 0U || logo.height == 0U) {
    return;
  }
  DrawBitmap(x, y, logo.data, logo.width, logo.height, color);
}

void Sdd1306::DrawLogoCentered(const Logo& logo, bool color) {
  if (logo.data == nullptr || logo.width == 0U || logo.height == 0U) {
    return;
  }
  const uint8_t x = (logo.width >= width_px) ? 0U : static_cast<uint8_t>((width_px - logo.width) / 2U);
  const uint8_t y = (logo.height >= height_px) ? 0U : static_cast<uint8_t>((height_px - logo.height) / 2U);
  DrawBitmap(x, y, logo.data, logo.width, logo.height, color);
}

void Sdd1306::InvertRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
  for (uint8_t yy = 0; yy < h; yy++) {
    for (uint8_t xx = 0; xx < w; xx++) {
      const uint8_t px = static_cast<uint8_t>(x + xx);
      const uint8_t py = static_cast<uint8_t>(y + yy);
      if (px >= width_px || py >= height_px) {
        continue;
      }
      const uint16_t index = static_cast<uint16_t>((py / 8U) * width_px + px);
      const uint8_t bit = static_cast<uint8_t>(1U << (py % 8U));
      buffer[index] ^= bit;
      MarkDirty(px, py);
    }
  }
}

void Sdd1306::SetContrast(uint8_t contrast) {
  (void)SendCommand(0x81);
  (void)SendCommand(contrast);
}

void Sdd1306::SetFont(FontType font) { active_font = font; }

void Sdd1306::SetTextScale(uint8_t scale) {
  if (scale < 1U) {
    text_scale = 1U;
  } else if (scale > 3U) {
    text_scale = 3U;
  } else {
    text_scale = scale;
  }
}

void Sdd1306::SetTextWrap(bool enabled) { text_wrap = enabled; }

void Sdd1306::SetTextAlign(TextAlign align) { text_align = align; }

void Sdd1306::SetRotation(uint8_t value) { rotation = static_cast<uint8_t>(value % 4U); }

void Sdd1306::SetDisplayOn(bool on) { (void)SendCommand(on ? 0xAFU : 0xAEU); }

void Sdd1306::SetInvertDisplay(bool invert) { (void)SendCommand(invert ? 0xA7U : 0xA6U); }

void Sdd1306::SetDim(bool dimmed) {
  const uint8_t contrast = dimmed ? 0x10U : 0xCFU;
  SetContrast(contrast);
}

void Sdd1306::StartScrollHorizontal(bool toLeft, uint8_t startPage, uint8_t endPage, uint8_t interval) {
  (void)SendCommand(toLeft ? 0x27U : 0x26U);
  (void)SendCommand(0x00U);
  (void)SendCommand(static_cast<uint8_t>(startPage & 0x07U));
  (void)SendCommand(static_cast<uint8_t>(interval & 0x07U));
  (void)SendCommand(static_cast<uint8_t>(endPage & 0x07U));
  (void)SendCommand(0x00U);
  (void)SendCommand(0xFFU);
  (void)SendCommand(0x2FU);
}

void Sdd1306::StopScroll() { (void)SendCommand(0x2EU); }

uint8_t Sdd1306::CurrentGlyphWidth() const {
  if (active_font == FontMedium) return Font6x8::GlyphWidth();
  if (active_font == FontLarge) return Font8x12::GlyphWidth();
  return Font5x7::GlyphWidth();
}

uint8_t Sdd1306::CurrentGlyphHeight() const {
  if (active_font == FontMedium) return Font6x8::GlyphHeight();
  if (active_font == FontLarge) return Font8x12::GlyphHeight();
  return Font5x7::GlyphHeight();
}

uint8_t Sdd1306::CurrentGlyphAdvance() const {
  if (active_font == FontMedium) return Font6x8::GlyphAdvance();
  if (active_font == FontLarge) return Font8x12::GlyphAdvance();
  return Font5x7::GlyphAdvance();
}

void Sdd1306::DrawChar(uint8_t x, uint8_t y, char c) {
  const uint8_t effective_scale = static_cast<uint8_t>(FontBaseScale(active_font) * text_scale);
  const FontEngine::Style style = {CurrentGlyphWidth(), CurrentGlyphHeight(), CurrentGlyphAdvance(),
                                   effective_scale, text_wrap, ToEngineAlign(text_align)};
  char text[2] = {c, '\0'};
  FontEngine::DrawText(*this, x, y, text, false, SelectLookup(active_font), style);
}

void Sdd1306::DrawText(uint8_t x, uint8_t y, const char* text) {
  const uint8_t effective_scale = static_cast<uint8_t>(FontBaseScale(active_font) * text_scale);
  const FontEngine::Style style = {CurrentGlyphWidth(), CurrentGlyphHeight(), CurrentGlyphAdvance(),
                                   effective_scale, text_wrap, ToEngineAlign(text_align)};
  FontEngine::DrawText(*this, x, y, text, false, SelectLookup(active_font), style);
}

void Sdd1306::DrawTextUtf8(uint8_t x, uint8_t y, const char* text) {
  const uint8_t effective_scale = static_cast<uint8_t>(FontBaseScale(active_font) * text_scale);
  const FontEngine::Style style = {CurrentGlyphWidth(), CurrentGlyphHeight(), CurrentGlyphAdvance(),
                                   effective_scale, text_wrap, ToEngineAlign(text_align)};
  FontEngine::DrawText(*this, x, y, text, true, SelectLookup(active_font), style);
}

void Sdd1306::DrawTextAligned(uint8_t x, uint8_t y, uint8_t w, const char* text) {
  const uint8_t effective_scale = static_cast<uint8_t>(FontBaseScale(active_font) * text_scale);
  const FontEngine::Style style = {CurrentGlyphWidth(), CurrentGlyphHeight(), CurrentGlyphAdvance(),
                                   effective_scale, text_wrap, ToEngineAlign(text_align)};
  FontEngine::DrawTextAligned(*this, x, y, w, text, SelectLookup(active_font), style);
}

void Sdd1306::DrawTextBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const char* text) {
  const uint8_t effective_scale = static_cast<uint8_t>(FontBaseScale(active_font) * text_scale);
  const FontEngine::Style style = {CurrentGlyphWidth(), CurrentGlyphHeight(), CurrentGlyphAdvance(),
                                   effective_scale, text_wrap, ToEngineAlign(text_align)};
  FontEngine::DrawTextBox(*this, x, y, w, h, text, SelectLookup(active_font), style);
}

void Sdd1306::GetTextBounds(const char* text, uint8_t* outW, uint8_t* outH) const {
  const uint8_t effective_scale = static_cast<uint8_t>(FontBaseScale(active_font) * text_scale);
  const FontEngine::Style style = {CurrentGlyphWidth(), CurrentGlyphHeight(), CurrentGlyphAdvance(),
                                   effective_scale, text_wrap, ToEngineAlign(text_align)};
  FontEngine::GetTextBounds(text, style, outW, outH);
}

uint8_t Sdd1306::Width() const { return width_px; }
uint8_t Sdd1306::Height() const { return height_px; }
uint8_t Sdd1306::LastI2cError() const { return last_i2c_error; }

bool Sdd1306::SendCommand(uint8_t cmd) {
  wire.beginTransmission(address_7bit);
  wire.write(0x00U);  // control: command
  wire.write(cmd);
  const uint8_t err = wire.endTransmission();
  last_i2c_error = err;
  return err == 0U;
}

bool Sdd1306::SendData(const uint8_t* data, uint8_t len) {
  // Write in small chunks to stay safe on Wire buffers.
  uint8_t offset = 0U;
  bool ok = true;
  while (offset < len) {
    const uint8_t chunk = static_cast<uint8_t>((len - offset) > 16U ? 16U : (len - offset));
    wire.beginTransmission(address_7bit);
    wire.write(0x40U);  // control: data
    wire.write(&data[offset], chunk);
    const uint8_t err = wire.endTransmission();
    last_i2c_error = err;
    ok = ok && (err == 0U);
    offset = static_cast<uint8_t>(offset + chunk);
  }
  return ok;
}

void Sdd1306::MarkDirty(uint8_t x, uint8_t y) {
  const uint8_t page = static_cast<uint8_t>(y / 8U);
  if (!has_dirty) {
    has_dirty = true;
    dirty_min_x = x;
    dirty_max_x = x;
    dirty_min_page = page;
    dirty_max_page = page;
    return;
  }
  if (x < dirty_min_x) {
    dirty_min_x = x;
  }
  if (x > dirty_max_x) {
    dirty_max_x = x;
  }
  if (page < dirty_min_page) {
    dirty_min_page = page;
  }
  if (page > dirty_max_page) {
    dirty_max_page = page;
  }
}

void Sdd1306::MarkDirtyRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
  if (w == 0U || h == 0U) {
    return;
  }
  const uint8_t y2 = static_cast<uint8_t>(y + h - 1U);
  MarkDirty(x, y);
  MarkDirty(static_cast<uint8_t>(x + w - 1U), y2);
}

