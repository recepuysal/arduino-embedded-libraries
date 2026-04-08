#ifndef FAN_CONTROL_H_
#define FAN_CONTROL_H_

#include <Arduino.h>
#include "dc_fan.h"

class FanControl {
 public:
  explicit FanControl(DcFan& fan);

  // sampleMs: PID guncelleme periyodu (tipik 100..250 ms)
  void Begin(uint32_t sampleMs, uint8_t initialPwmDuty);

  // Hedef RPM atamasi.
  void SetFanSpeed(uint32_t targetRpm);

  // PID kazanc ayari (istege bagli).
  void SetPidGains(float kp, float ki, float kd);
  void SetPwmInverted(bool isInverted);
  void SetMaxPwmStep(uint8_t stepPerUpdate);
  // Hedef RPM ve feedforward icin ust sinir (olcum Max RPM). Min genelde 0.
  void SetRpmLimits(uint32_t minRpm, uint32_t maxRpm);
  void SetFeedforwardEnabled(bool isEnabled);
  // Feedforward'da hedef RPM'i carp (1.0=dogrusal; >1.0 biraz daha agresif PWM, geride kalma azalir).
  void SetFeedforwardRpmScale(float scale);
  // PID cikisini taban PWM etrafinda sinirlar (salinimi azaltir).
  void SetMaxPidTrim(uint8_t trim);

  // PID dongusunu calistirir; loop() icinde sik cagrilmalidir.
  void Update();

  uint32_t GetFanRpm() const;
  uint32_t GetTargetRpm() const;
  uint8_t GetPwmDuty() const;
  float GetPwmPercent() const;

 private:
  DcFan& fan;
  uint32_t sampleMs;
  uint32_t targetRpm;
  uint32_t currentRpm;
  uint32_t lastUpdateMs;

  float kp;
  float ki;
  float kd;
  float integral;
  float prevError;
  float rpmFiltered;
  uint8_t maxPwmStep;
  bool isPwmInverted;
  uint32_t rpmMinLimit;
  uint32_t rpmMaxLimit;
  uint32_t ffMaxRpm;
  bool feedforwardEnabled;
  float feedforwardRpmScale;
  uint8_t maxPidTrim;
  uint32_t lastTargetRpm;
};

#endif  // FAN_CONTROL_H_

