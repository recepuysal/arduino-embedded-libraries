# Sdd1306Oled

Moduler SSD1306 OLED kutuphanesi (0.96" 128x64) - Arduino/STM32.

Sürüm: `1.1.0`

## Ozellikler

- I2C tabanli SSD1306 surucu (`0x3C` varsayilan) ve framebuffer yapi
- Dirty-rectangle ile parcali ekran guncellemesi (`Display()`)
- Grafik primitifleri:
  - `DrawPixel`, `DrawLine`
  - `DrawRect`, `FillRect`
  - `DrawCircle`, `FillCircle`
  - `DrawTriangle`, `FillTriangle`
  - `InvertRect`
- Bitmap / logo:
  - `DrawBitmap`
  - `DrawLogo`, `DrawLogoCentered`
- Metin motoru:
  - Fontlar: `FontSmall (5x7)`, `FontMedium (6x8)`, `FontLarge (8x12)`
  - `DrawText`, `DrawTextUtf8`, `DrawTextAligned`, `DrawTextBox`
  - `SetTextScale`, `SetTextAlign`, `SetTextWrap`, `GetTextBounds`
  - UTF-8 derece sembolu destegi (`°`)
- Ekran kontrol:
  - `SetDisplayOn`, `SetInvertDisplay`, `SetDim`, `SetContrast`
  - `SetRotation`, `StartScrollHorizontal`, `StopScroll`
- Hata geri bildirimi: `LastI2cError()`

## Kurulum

PlatformIO `lib_deps`:

```ini
lib_deps =
  https://github.com/recepuysal/arduino-embedded-libraries.git#main
```

veya kutuphaneyi proje `lib/` klasorune kopyalayin.

## Hızlı Baslangic

```cpp
#include <Arduino.h>
#include <SDD1306.h>

Sdd1306 oled(Wire, 0x3C);

void setup() {
  if (!oled.Begin(PB9, PB8, 400000U)) return;
  oled.Clear();
  oled.DrawTextUtf8(0, 0, "Temp: 36\xC2\xB0C");
  oled.Display();
}

void loop() {}
```

## Demo

Ornek demo dosyasi:

- `examples/ssd1306_showcase_demo/ssd1306_showcase_demo.ino`

Demo akisi:

1. Sayfa: alfabe + sayilar + isaretler
2. Sayfa: ucgen/kare/daire gibi primitifler
3. Sayfa: yildizli ekran + rastgele 3 yildiz yanip sonme efekti

## Logo Formati

`DrawLogo()` icin bitmap verisi su formatta olmali:

- 1-bit packed
- row-major
- MSB-first

Logo tip tanimi:

```cpp
static const Sdd1306::Logo kMyLogo = { logoData, width, height };
```

## Paket Icerigi

- `SDD1306.h`, `SDD1306.cpp`
- `gfx_canvas.h`
- `font_engine.*`
- `font5x7.*`, `font6x8.*`, `font8x8.*`, `font8x12.*`
- `examples/ssd1306_showcase_demo/`
- `library.json`
