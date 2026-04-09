#ifndef DS18B20_H_
#define DS18B20_H_

#include <Arduino.h>

class Ds18b20 {
 public:
  explicit Ds18b20(uint8_t dataPin);

  void Begin(bool useInternalPullup);
  bool StartConversion(void);
  bool IsConversionReady(uint32_t nowMs) const;
  bool ReadLastConversion(float& temperatureC);

 private:
  uint8_t dataPin_;
  uint32_t conversionStartMs_;
  bool conversionActive_;

  bool ResetPulse(void);
  void WriteBit(uint8_t bitValue);
  uint8_t ReadBit(void);
  void WriteByte(uint8_t value);
  uint8_t ReadByte(void);

  static uint8_t Crc8(const uint8_t* data, uint8_t length);
};

#endif  // DS18B20_H_
