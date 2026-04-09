#include "ds18b20.h"

static const uint8_t kCmdSkipRom = 0xCCU;
static const uint8_t kCmdConvertT = 0x44U;
static const uint8_t kCmdReadScratchpad = 0xBEU;
static const uint32_t kConversionTimeMs = 750UL;

Ds18b20::Ds18b20(uint8_t dataPin)
    : dataPin_(dataPin), conversionStartMs_(0UL), conversionActive_(false) {}

void Ds18b20::Begin(bool useInternalPullup) {
  if (useInternalPullup) {
    pinMode(dataPin_, INPUT_PULLUP);
  } else {
    pinMode(dataPin_, INPUT);
  }
  conversionActive_ = false;
  conversionStartMs_ = 0UL;
}

bool Ds18b20::StartConversion(void) {
  if (!ResetPulse()) {
    conversionActive_ = false;
    return false;
  }

  WriteByte(kCmdSkipRom);
  WriteByte(kCmdConvertT);
  conversionStartMs_ = millis();
  conversionActive_ = true;
  return true;
}

bool Ds18b20::IsConversionReady(uint32_t nowMs) const {
  uint32_t elapsedMs;
  if (!conversionActive_) {
    return false;
  }
  elapsedMs = nowMs - conversionStartMs_;
  return (elapsedMs >= kConversionTimeMs);
}

bool Ds18b20::ReadLastConversion(float& temperatureC) {
  uint8_t scratchpad[9];
  uint8_t index;
  int16_t rawValue;
  uint16_t rawUnsigned;

  if (!conversionActive_) {
    return false;
  }

  if (!IsConversionReady(millis())) {
    return false;
  }

  if (!ResetPulse()) {
    conversionActive_ = false;
    return false;
  }

  WriteByte(kCmdSkipRom);
  WriteByte(kCmdReadScratchpad);

  for (index = 0U; index < 9U; index++) {
    scratchpad[index] = ReadByte();
  }

  if (Crc8(scratchpad, 8U) != scratchpad[8]) {
    conversionActive_ = false;
    return false;
  }

  rawUnsigned = (static_cast<uint16_t>(scratchpad[1]) << 8) | scratchpad[0];
  rawValue = static_cast<int16_t>(rawUnsigned);
  temperatureC = static_cast<float>(rawValue) / 16.0f;

  conversionActive_ = false;
  return true;
}

bool Ds18b20::ResetPulse(void) {
  bool presence;

  pinMode(dataPin_, OUTPUT);
  digitalWrite(dataPin_, LOW);
  delayMicroseconds(480);

  pinMode(dataPin_, INPUT_PULLUP);
  delayMicroseconds(70);
  presence = (digitalRead(dataPin_) == LOW);
  delayMicroseconds(410);
  return presence;
}

void Ds18b20::WriteBit(uint8_t bitValue) {
  noInterrupts();
  pinMode(dataPin_, OUTPUT);
  digitalWrite(dataPin_, LOW);
  if (bitValue != 0U) {
    delayMicroseconds(6);
    pinMode(dataPin_, INPUT_PULLUP);
    delayMicroseconds(64);
  } else {
    delayMicroseconds(60);
    pinMode(dataPin_, INPUT_PULLUP);
    delayMicroseconds(10);
  }
  interrupts();
}

uint8_t Ds18b20::ReadBit(void) {
  uint8_t bitValue;

  noInterrupts();
  pinMode(dataPin_, OUTPUT);
  digitalWrite(dataPin_, LOW);
  delayMicroseconds(6);
  pinMode(dataPin_, INPUT_PULLUP);
  delayMicroseconds(9);
  bitValue = static_cast<uint8_t>(digitalRead(dataPin_));
  delayMicroseconds(55);
  interrupts();
  return bitValue;
}

void Ds18b20::WriteByte(uint8_t value) {
  uint8_t index;
  for (index = 0U; index < 8U; index++) {
    WriteBit(value & 0x01U);
    value >>= 1U;
  }
}

uint8_t Ds18b20::ReadByte(void) {
  uint8_t value;
  uint8_t index;

  value = 0U;
  for (index = 0U; index < 8U; index++) {
    value >>= 1U;
    if (ReadBit() != 0U) {
      value |= 0x80U;
    }
  }
  return value;
}

uint8_t Ds18b20::Crc8(const uint8_t* data, uint8_t length) {
  uint8_t crc;
  uint8_t index;

  crc = 0U;
  for (index = 0U; index < length; index++) {
    uint8_t inByte;
    uint8_t bit;
    inByte = data[index];
    for (bit = 0U; bit < 8U; bit++) {
      const uint8_t mix = (crc ^ inByte) & 0x01U;
      crc >>= 1U;
      if (mix != 0U) {
        crc ^= 0x8CU;
      }
      inByte >>= 1U;
    }
  }
  return crc;
}
