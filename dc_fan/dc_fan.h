#ifndef DC_FAN_H_
#define DC_FAN_H_

#include <Arduino.h>

class DcFan {
 public:
  // tachPulsePerRev: fanin 1 turda urettigi tach pulse sayisi (tipik 2).
  DcFan(uint32_t pinPwm, uint32_t pinTach, uint8_t tachPulsePerRev);

  // Pinleri baslatir, kesmeyi baglar ve ilk PWM degerini yazar.
  void Begin(uint8_t pwmDuty);

  // PWM gorev oranini 0..255 araliginda ayarlar.
  void SetPwmDuty(uint8_t pwmDuty);
  uint8_t GetPwmDuty() const;
  float GetPwmPercent() const;

  // Son ornekleme penceresine gore RPM hesaplar.
  // sampleMs tipik 1000 ms secilir.
  uint32_t ReadRpm(uint32_t sampleMs);
  // Pulse periyodundan anlik RPM hesaplar (daha hassas).
  // timeoutMs boyunca pulse gelmezse 0 doner.
  uint32_t ReadRpmInstant(uint32_t timeoutMs) const;

 private:
  static DcFan* instance;
  static void TachIsr();

  uint32_t pinPwm;
  uint32_t pinTach;
  uint8_t tachPulsePerRev;
  uint8_t pwmDuty;
  volatile uint32_t tachPulses;
  uint32_t lastSampleMs;
  volatile uint32_t lastPulseUs;
  volatile uint32_t pulsePeriodUsFiltered;
};

#endif  // DC_FAN_H_

