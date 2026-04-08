#ifndef MLX90614_H_
#define MLX90614_H_

#include <Arduino.h>
#include <Wire.h>

// MLX90614 icin hafif, dinamik bellek kullanmayan I2C surucusu.
class Mlx90614 {
 public:
  // Sensorun yaygin varsayilan I2C adresi.
  static const uint8_t DefaultAddr = 0x5AU;
  // Bazi kartlarda kullanilan alternatif adres.
  static const uint8_t AltAddr = 0x5BU;

  // Disaridan Wire nesnesini alir; adres verilmezse DefaultAddr kullanir.
  explicit Mlx90614(TwoWire& wire, uint8_t address = DefaultAddr);

  // Cihaz bu adreste cevap veriyor mu kontrol eder.
  bool Begin();
  // Cihaz adresini degistirir ve yeni adreste tekrar test eder.
  bool SetAddress(uint8_t address);

  // Ortam sicakligini (Ta) Celsius cinsinden okur.
  bool ReadAmbientC(float& tempC);
  // Nesne sicakligini (To1) Celsius cinsinden okur.
  bool ReadObjectC(float& tempC);

 private:
  // MLX90614 datasheet register adresleri.
  static const uint8_t RegAmbient = 0x06U;
  static const uint8_t RegObject1 = 0x07U;

  // Kullanilan I2C hatti referansi (Wire).
  TwoWire& wire;
  // Aktif sensor adresi.
  uint8_t address;

  // Verilen register'dan 16-bit ham deger okur.
  bool ReadRegister16(uint8_t reg, uint16_t& value);
  // Ham register degerini Celsius'a cevirir.
  static float RawToCelsius(uint16_t raw);
};

#endif  // MLX90614_H_

