# Arduino Embedded Libraries

TR: Arduino ve STM32 icin moduler, hafif ve tekrar kullanilabilir gomulu C++ kutuphaneler. Bu repo SH1106/SSD1306 OLED suruculeri, fan surme-kontrol modulleri ve MLX90614 I2C sicaklik sensor surucusu sunar.

EN: A modular, lightweight, reusable embedded C++ library collection for Arduino and STM32. It provides SH1106/SSD1306 OLED drivers, fan drive/control modules, and an MLX90614 I2C temperature sensor driver.

## Icerik

- `dc_fan`: PWM ile fan surme ve tach geri bildirimi ile RPM olcumu
- `fan_control`: PID + feedforward tabanli hedef RPM kontrolu
- `mlx90614`: MLX90614 I2C kizilotesi sicaklik sensor surucusu
- `sh1106_oled`: SH1106 1.3" 128x64 OLED surucusu, cizim ve metin motoru
- `sdd1306_oled`: SSD1306 0.96" 128x64 OLED surucusu, moduler metin/graphics motoru

## Kutuphaneler

### 1) `dc_fan`

Amac:
- DC fani PWM ile surmek
- Tach pulse sinyalinden RPM hesaplamak

Temel ozellikler:
- `SetPwmDuty(0..255)` ile duty kontrolu
- `ReadRpm(sampleMs)` ile pencere tabanli RPM olcumu
- `ReadRpmInstant(timeoutMs)` ile anlik/periyot tabanli RPM olcumu

Klasor:
- `dc_fan/dc_fan.h`
- `dc_fan/dc_fan.cpp`

### 2) `fan_control`

Amac:
- Fan devrini belirlenen hedef RPM etrafinda sabit tutmak

Temel ozellikler:
- `SetFanSpeed(targetRpm)` ile hedef RPM
- `SetPidGains(kp, ki, kd)` ile PID ayari
- `SetFeedforwardEnabled(true/false)` ile feedforward ac/kapat
- `SetPwmInverted(true/false)` ile ters PWM surucu destegi
- `SetMaxPwmStep(...)` ve `SetMaxPidTrim(...)` ile salinim azaltma

Klasor:
- `fan_control/fan_control.h`
- `fan_control/fan_control.cpp`

### 3) `mlx90614`

Amac:
- MLX90614 sensorunden ortam ve nesne sicakligi okumak

Temel ozellikler:
- `Begin()` ile cihaz baglantisi kontrolu
- `ReadAmbientC(float&)` ile ortam sicakligi
- `ReadObjectC(float&)` ile nesne sicakligi
- `SetAddress(uint8_t)` ile I2C adres yonetimi

Klasor:
- `mlx90614/mlx90614.h`
- `mlx90614/mlx90614.cpp`

### 4) `sh1106_oled`

Amac:
- SH1106 tabanli 1.3" 128x64 OLED ekranlari STM32/Arduino ile surmek

Temel ozellikler:
- Cizim primitifleri: pixel, line, rect, fill rect, circle, fill circle, triangle, fill triangle
- Metin motoru: `FontSmall`, `FontMedium`, `FontLarge`, text align, text wrap, UTF-8 derece (`°`) destegi
- Bitmap/logo: `DrawBitmap`, `DrawLogo`, `DrawLogoCentered`
- Ekran kontrolu: contrast, invert, display on/off, dim, rotation, horizontal scroll
- Performans: dirty-rectangle ile kismi `Display()` guncellemesi
- Hata geri bildirimi: `LastI2cError()`

Klasor:
- `sh1106_oled/`

### 5) `sdd1306_oled`

Amac:
- SSD1306 tabanli 0.96" 128x64 OLED ekranlari STM32/Arduino ile surmek

Temel ozellikler:
- Moduler yapi: `gfx_canvas`, `font_engine`, coklu font paketleri
- Cizim primitifleri + bitmap/logo cizimi
- Metin motoru: `FontSmall`, `FontMedium`, `FontLarge`, UTF-8 derece (`°`) destegi
- Ekran kontrolu: contrast, invert, display on/off, dim, rotation, horizontal scroll
- Performans: dirty-rectangle ile kismi `Display()` guncellemesi

Klasor:
- `sdd1306_oled/`
- Ornek: `sdd1306_oled/examples/ssd1306_showcase_demo/ssd1306_showcase_demo.ino`

## Dizin Yapisi

```text
arduino-embedded-libraries/
|-- dc_fan/
|-- fan_control/
|-- mlx90614/
|-- sh1106_oled/
|-- sdd1306_oled/
`-- README.md
```

## Hizli Baslangic

Bu repo bir kutuphane koleksiyonudur. Ilgili klasoru projenize ekleyerek kullanabilirsiniz.

Ornek derleme:

```bash
pio run
```

## Lisans

MIT
