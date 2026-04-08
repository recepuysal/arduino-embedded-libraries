#include "mlx90614.h"

namespace {
// Okuma hatalarina karsi maksimum tekrar deneme sayisi.
static const uint8_t MaxReadAttempts = 3U;
// MLX90614 okumada beklenen byte sayisi: LSB + MSB + PEC.
static const uint8_t ReadLengthBytes = 3U;
// Byte birlestirme sirasinda acik maskeleme.
static const uint8_t ByteMask = 0xFFU;
// 16-bit birlestirme icin bit kaydirma miktari.
static const uint8_t Shift8 = 8U;
// Baslangic ham veri degeri.
static const uint16_t NoData = 0U;
// Basarisiz denemeler arasi kisa bekleme.
static const uint32_t RetryDelayMs = 2U;
// Datasheet katsayilari: Kelvin = raw * 0.02
static const float RawScale = 0.02f;
// Kelvin -> Celsius ofseti.
static const float KelvinOffset = 273.15f;
}  // namespace

// Kurucu: disaridan gelen Wire referansi ve adres saklanir.
Mlx90614::Mlx90614(TwoWire& wireIn, uint8_t addressIn)
    : wire(wireIn), address(addressIn) {}

bool Mlx90614::Begin() {
  // Cihaza bos bir I2C aktarimi gonderip ACK var mi kontrol eder.
  wire.beginTransmission(address);
  return (wire.endTransmission() == 0U);
}

bool Mlx90614::SetAddress(uint8_t addressIn) {
  // Runtime adres degisikligi.
  address = addressIn;
  // Yeni adreste cihaz var mi tekrar test.
  return Begin();
}

bool Mlx90614::ReadAmbientC(float& tempC) {
  uint16_t raw = NoData;
  // Ortam register'ini oku, basarisizsa false don.
  if (!ReadRegister16(RegAmbient, raw)) {
    return false;
  }
  // Ham degerden Celsius hesapla.
  tempC = RawToCelsius(raw);
  return true;
}

bool Mlx90614::ReadObjectC(float& tempC) {
  uint16_t raw = NoData;
  // Nesne register'ini oku, basarisizsa false don.
  if (!ReadRegister16(RegObject1, raw)) {
    return false;
  }
  // Ham degerden Celsius hesapla.
  tempC = RawToCelsius(raw);
  return true;
}

bool Mlx90614::ReadRegister16(uint8_t reg, uint16_t& value) {
  // Gecici I2C hatalari icin kontrollu retry.
  for (uint8_t attempt = 0U; attempt < MaxReadAttempts; ++attempt) {
    // 1) Once okunacak register adresini slave'e yaz.
    wire.beginTransmission(address);
    wire.write(reg);
    // false: stop gondermeden repeated-start ile okumaya gec.
    if (wire.endTransmission(false) != 0U) {
      // NACK / bus hatasi.
      delay(RetryDelayMs);
      continue;
    }

    // 2) Sensor 3 byte doner: LSB, MSB, PEC.
    const uint8_t readCount = wire.requestFrom(address, ReadLengthBytes);
    if (readCount != ReadLengthBytes) {
      // Eksik veri geldiyse bu deneme gecersiz.
      delay(RetryDelayMs);
      continue;
    }

    // 3) Byte'lari oku.
    const uint8_t lsb = wire.read();
    const uint8_t msb = wire.read();
    (void)wire.read();  // PEC byte (istersen CRC-8 dogrulamasi eklenebilir)

    // LSB/MSB birlestirerek 16-bit ham degeri olustur.
    value = (static_cast<uint16_t>(msb & ByteMask) << Shift8) |
            static_cast<uint16_t>(lsb & ByteMask);
    // Bu deneme basarili.
    return true;
  }

  // Tum denemeler basarisiz.
  return false;
}

float Mlx90614::RawToCelsius(uint16_t raw) {
  // Datasheet formulu:
  // Kelvin = raw * 0.02
  // Celsius = Kelvin - 273.15
  return (static_cast<float>(raw) * RawScale) - KelvinOffset;
}

