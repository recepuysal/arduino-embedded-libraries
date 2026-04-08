#include "dc_fan.h"

namespace {
static const uint8_t PwmMax = 255U;
static const uint32_t MsPerMinute = 60000UL;
static const uint32_t UsPerMinute = 60000000UL;
static const uint8_t FilterOldWeight = 3U;
static const uint8_t FilterNewWeight = 1U;
static const uint8_t FilterDiv = 4U;
}  // namespace

DcFan* DcFan::instance = NULL;

DcFan::DcFan(uint32_t pinPwmIn, uint32_t pinTachIn, uint8_t tachPulsePerRevIn)
    : pinPwm(pinPwmIn),
      pinTach(pinTachIn),
      tachPulsePerRev(tachPulsePerRevIn),
      pwmDuty(0U),
      tachPulses(0U),
      lastSampleMs(0U),
      lastPulseUs(0U),
      pulsePeriodUsFiltered(0U) {}

void DcFan::Begin(uint8_t pwmDutyIn) {
  pinMode(pinPwm, OUTPUT);
  pinMode(pinTach, INPUT_PULLUP);

  instance = this;
  attachInterrupt(digitalPinToInterrupt(pinTach), TachIsr, FALLING);

  SetPwmDuty(pwmDutyIn);
  lastSampleMs = millis();
}

void DcFan::SetPwmDuty(uint8_t pwmDutyIn) {
  pwmDuty = pwmDutyIn;
  analogWrite(pinPwm, pwmDuty);
}

uint8_t DcFan::GetPwmDuty() const { return pwmDuty; }

float DcFan::GetPwmPercent() const {
  return (static_cast<float>(pwmDuty) * 100.0f) / static_cast<float>(PwmMax);
}

uint32_t DcFan::ReadRpm(uint32_t sampleMs) {
  const uint32_t nowMs = millis();
  const uint32_t elapsedMs = nowMs - lastSampleMs;

  if (elapsedMs < sampleMs) {
    return 0U;
  }

  noInterrupts();
  const uint32_t pulses = tachPulses;
  tachPulses = 0U;
  interrupts();

  lastSampleMs = nowMs;

  // RPM = (pulse / elapsedMs) * 60000 / pulsePerRev
  const uint32_t numerator = pulses * MsPerMinute;
  const uint32_t denominator = elapsedMs * static_cast<uint32_t>(tachPulsePerRev);
  if (denominator == 0U) {
    return 0U;
  }
  return numerator / denominator;
}

uint32_t DcFan::ReadRpmInstant(uint32_t timeoutMs) const {
  uint32_t lastPulseUsLocal = 0U;
  uint32_t periodUsLocal = 0U;

  noInterrupts();
  lastPulseUsLocal = lastPulseUs;
  periodUsLocal = pulsePeriodUsFiltered;
  interrupts();

  if ((lastPulseUsLocal == 0U) || (periodUsLocal == 0U) || (tachPulsePerRev == 0U)) {
    return 0U;
  }

  const uint32_t nowUs = micros();
  const uint32_t ageUs = nowUs - lastPulseUsLocal;
  const uint32_t timeoutUs = timeoutMs * 1000UL;
  if (ageUs > timeoutUs) {
    return 0U;
  }

  const uint32_t denominator = periodUsLocal * static_cast<uint32_t>(tachPulsePerRev);
  if (denominator == 0U) {
    return 0U;
  }
  return UsPerMinute / denominator;
}

void DcFan::TachIsr() {
  if (instance != NULL) {
    instance->tachPulses++;

    const uint32_t nowUs = micros();
    if (instance->lastPulseUs != 0U) {
      const uint32_t deltaUs = nowUs - instance->lastPulseUs;
      if (instance->pulsePeriodUsFiltered == 0U) {
        instance->pulsePeriodUsFiltered = deltaUs;
      } else {
        const uint32_t filtered = (instance->pulsePeriodUsFiltered * FilterOldWeight) +
                                  (deltaUs * FilterNewWeight);
        instance->pulsePeriodUsFiltered = filtered / FilterDiv;
      }
    }
    instance->lastPulseUs = nowUs;
  }
}

