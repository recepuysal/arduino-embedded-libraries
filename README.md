# Arduino Embedded Libraries

Arduino ve STM32 projeleri icin tekrar kullanilabilir, hafif ve acik kaynak gomulu C++ kutuphaneleri.

English: Reusable, lightweight and open-source embedded C++ libraries for Arduino and STM32 projects.

## Icerik

- `dc_fan`: PWM ile fan surme ve tach geri bildirimi ile RPM olcumu
- `fan_control`: PID + feedforward tabanli hedef RPM kontrolu
- `mlx90614`: MLX90614 I2C kizilotesi sicaklik sensor surucusu
- `sh1106_oled`: SH1106 1.3" 128x64 OLED surucusu, cizim ve metin motoru

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
- `dc_fan/dc_fan_description.txt`

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
- `fan_control/fan_control_description.txt`

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
- `mlx90614/mlx90614_description.txt`

### 4) `sh1106_oled`

Amac:
- SH1106 tabanli 1.3" 128x64 OLED ekranlari STM32/Arduino ile surmek
- Tek kutuphanede cizim, metin ve bitmap/logo ihtiyaclarini karsilamak

Temel ozellikler:
- Cizim primitifleri: pixel, line, rect, fill rect, circle, fill circle, triangle, fill triangle
- Metin motoru: `FontSmall`, `FontMedium`, `FontLarge`, text align, text wrap, UTF-8 derece (`°`) destegi
- Bitmap/logo cizimi: `DrawBitmap`, `DrawLogo`, `DrawLogoCentered`
- Ekran kontrolu: contrast, invert, display on/off, dim, rotation, horizontal scroll
- Performans: dirty-rectangle ile kismi `Display()` guncellemesi
- Hata geri bildirimi: `LastI2cError()`

Klasor:
- `sh1106_oled/SH1106.h`
- `sh1106_oled/SH1106.cpp`
- `sh1106_oled/gfx_canvas.h`
- `sh1106_oled/font_engine.h`
- `sh1106_oled/font_engine.cpp`
- `sh1106_oled/font5x7.h`
- `sh1106_oled/font5x7.cpp`
- `sh1106_oled/font6x8.h`
- `sh1106_oled/font6x8.cpp`
- `sh1106_oled/font8x8.h`
- `sh1106_oled/font8x8.cpp`
- `sh1106_oled/font8x12.h`
- `sh1106_oled/font8x12.cpp`
- `sh1106_oled/library.json`

## Dizin Yapisi

```text
arduino-embedded-libraries/
|-- dc_fan/
|   |-- dc_fan.h
|   |-- dc_fan.cpp
|   `-- dc_fan_description.txt
|-- fan_control/
|   |-- fan_control.h
|   |-- fan_control.cpp
|   `-- fan_control_description.txt
|-- mlx90614/
|   |-- mlx90614.h
|   |-- mlx90614.cpp
|   `-- mlx90614_description.txt
|-- sh1106_oled/
|   |-- SH1106.h
|   |-- SH1106.cpp
|   |-- gfx_canvas.h
|   |-- font_engine.h
|   |-- font_engine.cpp
|   |-- font5x7.h
|   |-- font5x7.cpp
|   |-- font6x8.h
|   |-- font6x8.cpp
|   |-- font8x8.h
|   |-- font8x8.cpp
|   |-- font8x12.h
|   |-- font8x12.cpp
|   `-- library.json
`-- README.md
```

## Hizli Baslangic

Bu repo bir kutuphane koleksiyonu oldugu icin dosyalari kendi PlatformIO projenize ekleyerek kullanabilirsiniz.

Ornek derleme komutu:

```bash
pio run
```

## Neden Bu Yapi?

- Gömülü sistem odakli sade API
- Dinamik bellek kullanmadan hafif tasarim
- Gercek donanim senaryolarina uygun fan + sicaklik + OLED altyapisi

## Lisans

MIT

## Gelistirme Yol Haritasi

- Her kutuphane icin `examples/` klasoru ekleme
- PlatformIO icin `library.json` metadatasi ekleme
- Ileri test ornekleri ve dokumantasyon genisletme
