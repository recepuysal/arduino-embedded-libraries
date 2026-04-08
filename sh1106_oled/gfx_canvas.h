#ifndef GFX_CANVAS_H_
#define GFX_CANVAS_H_

#include <Arduino.h>

class GfxCanvas {
 public:
  virtual ~GfxCanvas() {}
  virtual void DrawPixel(uint8_t x, uint8_t y, bool color) = 0;
  virtual uint8_t Width() const = 0;
  virtual uint8_t Height() const = 0;
};

#endif  // GFX_CANVAS_H_
