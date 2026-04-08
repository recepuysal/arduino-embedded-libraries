#ifndef SDD1306_H_
#define SDD1306_H_

#include <Arduino.h>
#include <Wire.h>

#include "gfx_canvas.h"

class Sdd1306 : public GfxCanvas {
 public:
  enum FontType : uint8_t {
    FontSmall = 0U,   // 5x7
    FontMedium = 1U,  // 6x8
    FontLarge = 2U    // 8x12
  };

  enum TextAlign : uint8_t {
    AlignLeft = 0U,
    AlignCenter = 1U,
    AlignRight = 2U,
  };

  struct Logo {
    const uint8_t* data;
    uint8_t width;
    uint8_t height;
  };

  Sdd1306(TwoWire& wire, uint8_t address7bit = 0x3C);

  bool Begin(uint32_t pinSda, uint32_t pinScl, uint32_t i2cClockHz = 400000U);
  void Clear();
  bool Display();

  void DrawPixel(uint8_t x, uint8_t y, bool color) override;
  void DrawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool color = true);
  void DrawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool color = true);
  void FillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool color = true);
  void DrawCircle(uint8_t x0, uint8_t y0, uint8_t r, bool color = true);
  void FillCircle(uint8_t x0, uint8_t y0, uint8_t r, bool color = true);
  void DrawTriangle(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2,
                    bool color = true);
  void FillTriangle(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2,
                    bool color = true);
  void DrawBitmap(uint8_t x, uint8_t y, const uint8_t* bitmap, uint8_t w, uint8_t h, bool color = true);
  void DrawLogo(uint8_t x, uint8_t y, const Logo& logo, bool color = true);
  void DrawLogoCentered(const Logo& logo, bool color = true);
  void InvertRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h);

  void SetContrast(uint8_t contrast);
  void SetFont(FontType font);
  void SetTextScale(uint8_t scale);  // 1..3
  void SetTextWrap(bool enabled);
  void SetTextAlign(TextAlign align);
  void SetRotation(uint8_t rotation);
  void SetDisplayOn(bool on);
  void SetInvertDisplay(bool invert);
  void SetDim(bool dimmed);
  void StartScrollHorizontal(bool toLeft, uint8_t startPage, uint8_t endPage, uint8_t interval);
  void StopScroll();

  void DrawChar(uint8_t x, uint8_t y, char c);
  void DrawText(uint8_t x, uint8_t y, const char* text);
  void DrawTextUtf8(uint8_t x, uint8_t y, const char* text);
  void DrawTextAligned(uint8_t x, uint8_t y, uint8_t w, const char* text);
  void DrawTextBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const char* text);
  void GetTextBounds(const char* text, uint8_t* outW, uint8_t* outH) const;

  uint8_t Width() const override;
  uint8_t Height() const override;
  uint8_t LastI2cError() const;

 private:
  static const uint8_t width_px = 128U;
  static const uint8_t height_px = 64U;
  static const uint8_t page_count = 8U;
  static const uint8_t page_offset = 0U;  // SSD1306 has no RAM column offset

  TwoWire& wire;
  uint8_t address_7bit;
  uint8_t buffer[width_px * page_count];
  FontType active_font;
  uint8_t text_scale;
  bool text_wrap;
  TextAlign text_align;
  uint8_t rotation;
  uint8_t last_i2c_error;
  bool has_dirty;
  uint8_t dirty_min_x;
  uint8_t dirty_max_x;
  uint8_t dirty_min_page;
  uint8_t dirty_max_page;

  bool SendCommand(uint8_t cmd);
  bool SendData(const uint8_t* data, uint8_t len);
  void MarkDirty(uint8_t x, uint8_t y);
  void MarkDirtyRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
  uint8_t CurrentGlyphWidth() const;
  uint8_t CurrentGlyphHeight() const;
  uint8_t CurrentGlyphAdvance() const;
};

#endif  // SDD1306_H_
