#include <Arduino.h>
#include <SDD1306.h>

static constexpr uint32_t kPinSda = PB9;
static constexpr uint32_t kPinScl = PB8;
static constexpr uint8_t kAddr = 0x3C;
static constexpr uint32_t kPage1Ms = 3500;
static constexpr uint32_t kPage2Ms = 3500;
static constexpr uint8_t kStarCount = 90;

Sdd1306 oled(Wire, kAddr);
uint8_t star_x[kStarCount];
uint8_t star_y[kStarCount];

void DrawWordSpace(uint8_t x, uint8_t y) {
  static const uint8_t kGlyphS[7] = {0x1E, 0x11, 0x10, 0x0E, 0x01, 0x11, 0x0E};
  static const uint8_t kGlyphP[7] = {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10};
  static const uint8_t kGlyphA[7] = {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11};
  static const uint8_t kGlyphC[7] = {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E};
  static const uint8_t kGlyphE[7] = {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F};

  const uint8_t* glyphs[5] = {kGlyphS, kGlyphP, kGlyphA, kGlyphC, kGlyphE};
  for (uint8_t g = 0; g < 5; g++) {
    for (uint8_t row = 0; row < 7; row++) {
      const uint8_t bits = glyphs[g][row];
      for (uint8_t col = 0; col < 5; col++) {
        if ((bits & static_cast<uint8_t>(1U << (4U - col))) != 0U) {
          oled.DrawPixel(static_cast<uint8_t>(x + g * 6U + col), static_cast<uint8_t>(y + row), true);
        }
      }
    }
  }
}

void DrawPageText() {
  oled.Clear();
  oled.SetFont(Sdd1306::FontSmall);
  oled.SetTextScale(1);
  oled.DrawText(0, 0, "ALFABE");
  oled.DrawText(0, 10, "ABCDEF GHIJKL");
  oled.DrawText(0, 20, "MNOPQR STUVWX");
  oled.DrawText(0, 30, "YZ");
  oled.DrawText(0, 40, "SAYILAR: 0123456789");
  oled.DrawTextUtf8(0, 50, "ISARET: !?@#%&+-*/ \xC2\xB0");
  oled.DrawText(0, 58, "()[]{}.,:;_=");
  oled.Display();
}

void DrawPageShapes() {
  oled.Clear();
  oled.DrawRect(2, 2, 24, 16, true);
  oled.FillRect(30, 2, 24, 16, true);
  oled.DrawCircle(72, 10, 8, true);
  oled.FillCircle(98, 10, 8, true);
  oled.DrawTriangle(8, 40, 24, 22, 40, 40, true);
  oled.FillTriangle(46, 40, 62, 22, 78, 40, true);
  oled.DrawLine(84, 22, 126, 22, true);
  oled.DrawLine(84, 24, 126, 40, true);
  oled.DrawLine(84, 40, 126, 24, true);
  oled.DrawText(2, 54, "Ucgen Kare Cember");
  oled.Display();
}

void DrawPageStars() {
  oled.Clear();
  for (uint8_t i = 0U; i < kStarCount; i++) {
    oled.DrawPixel(star_x[i], star_y[i], true);
  }
  DrawWordSpace(2U, 56U);
  oled.Display();
}

void setup() {
  const bool ok = oled.Begin(kPinSda, kPinScl, 400000U);
  if (!ok) return;

  randomSeed(analogRead(A0));
  for (uint8_t i = 0U; i < kStarCount; i++) {
    star_x[i] = static_cast<uint8_t>(random(0, 128));
    star_y[i] = static_cast<uint8_t>(random(0, 64));
  }
}

void loop() {
  static uint8_t phase = 0U;
  static uint32_t phase_start_ms = 0U;
  static uint32_t next_twinkle_ms = 0U;
  static bool twinkle_on = false;
  static uint8_t twinkle_x[3] = {0U, 0U, 0U};
  static uint8_t twinkle_y[3] = {0U, 0U, 0U};
  static uint16_t twinkle_duration_ms = 140U;
  const uint32_t now = millis();

  if (phase == 0U) {
    DrawPageText();
    phase_start_ms = now;
    phase = 1U;
    return;
  }
  if (phase == 1U) {
    if ((now - phase_start_ms) >= kPage1Ms) {
      DrawPageShapes();
      phase_start_ms = now;
      phase = 2U;
    }
    return;
  }
  if (phase == 2U) {
    if ((now - phase_start_ms) >= kPage2Ms) {
      DrawPageStars();
      next_twinkle_ms = now + static_cast<uint32_t>(random(600, 4500));
      phase = 3U;
    }
    return;
  }

  if (!twinkle_on) {
    if (now >= next_twinkle_ms) {
      for (uint8_t i = 0U; i < 3U; i++) {
        twinkle_x[i] = static_cast<uint8_t>(random(0, 128));
        twinkle_y[i] = static_cast<uint8_t>(random(0, 56));
        oled.DrawPixel(twinkle_x[i], twinkle_y[i], true);
      }
      oled.Display();
      twinkle_on = true;
      twinkle_duration_ms = static_cast<uint16_t>(random(120, 260));
      phase_start_ms = now;
    }
  } else if ((now - phase_start_ms) >= twinkle_duration_ms) {
    for (uint8_t i = 0U; i < 3U; i++) {
      oled.DrawPixel(twinkle_x[i], twinkle_y[i], false);
    }
    oled.Display();
    twinkle_on = false;
    next_twinkle_ms = now + static_cast<uint32_t>(random(600, 4500));
  }
}
