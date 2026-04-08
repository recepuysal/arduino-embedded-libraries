# arduino-embedded-libraries

**TR:** Arduino ve STM32 projelerinde tekrar kullanilabilir gomulu C++ kutuphaneleri.  
**EN:** Reusable embedded C++ libraries for Arduino and STM32 projects.

## Libraries / Kutuphaneler

### `DcFan`
- PWM (`0..255`) ile fan surme
- Tach kesmesi ile pulse sayma
- RPM olcumu:
  - pencere tabanli (`ReadRpm`)
  - anlik/periyot tabanli (`ReadRpmInstant`)

### `FanControl`
- Hedef RPM takibi (`SetFanSpeed`)
- PID + feedforward tabanli kapali cevrim kontrol
- Ters PWM surus destegi (`SetPwmInverted`)
- Salinim azaltma:
  - adim limiti (`SetMaxPwmStep`)
  - PID trim limiti (`SetMaxPidTrim`)

### `Mlx90614`
- MLX90614 I2C sensor surucusu
- Ortam sicakligi (`ReadAmbientC`)
- Nesne sicakligi (`ReadObjectC`)
- Adres yonetimi (`SetAddress`)

## Repository Layout / Dizin Yapisi

Mevcut kaynak kodlar `src/` altindadir:

- `src/dc_fan.h`, `src/dc_fan.cpp`
- `src/fan_control.h`, `src/fan_control.cpp`
- `src/mlx90614.h`, `src/mlx90614.cpp`

Ek dokuman dosyalari:
- `dc_fan_description.txt`
- `fan_control_description.txt`
- `mlx90614_description.txt`

## Quick Start / Hizli Baslangic

```bash
pio run
```

Varsayilan test ortami:
- Board: `nucleo_g474re`
- Framework: Arduino (PlatformIO)

## Why this project? / Neden bu proje?

- Dinamik bellek kullanmadan hafif tasarim
- Gömülü sistem dostu API
- Gercek donanim odakli fan + sicaklik kontrol ornegi

## License

MIT (onerilen)

## Roadmap

- Kutuphaneleri `lib/` veya ayri repo formatina gecirmek
- Her kutuphane icin `examples/` eklemek
- `library.json` metadatasini tamamlamak
