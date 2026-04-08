#include "fan_control.h"

namespace {
static const uint8_t PwmMin = 0U;
static const uint8_t PwmMax = 255U;
static const float MsToSecond = 0.001f;
static const float IntegralClamp = 5000.0f;
static const uint32_t InstantRpmTimeoutMs = 1200U;
static const float RpmFilterAlpha = 0.25f;
}  // namespace

FanControl::FanControl(DcFan& fanIn)
    : fan(fanIn),
      sampleMs(200U),
      targetRpm(0U),
      currentRpm(0U),
      lastUpdateMs(0U),
      kp(0.08f),
      ki(0.03f),
      kd(0.01f),
      integral(0.0f),
      prevError(0.0f),
      rpmFiltered(0.0f),
      maxPwmStep(4U),
      isPwmInverted(false),
      rpmMinLimit(0U),
      rpmMaxLimit(65535U),
      ffMaxRpm(5000U),
      feedforwardEnabled(false),
      feedforwardRpmScale(1.0f),
      maxPidTrim(24U),
      lastTargetRpm(0xFFFFFFFFU) {}

void FanControl::Begin(uint32_t sampleMsIn, uint8_t initialPwmDuty) {
  sampleMs = sampleMsIn;
  fan.SetPwmDuty(initialPwmDuty);
  lastUpdateMs = millis();
  integral = 0.0f;
  prevError = 0.0f;
  rpmFiltered = 0.0f;
  lastTargetRpm = 0xFFFFFFFFU;
}

void FanControl::SetRpmLimits(uint32_t minRpm, uint32_t maxRpm) {
  rpmMinLimit = minRpm;
  if (maxRpm < minRpm) {
    rpmMaxLimit = minRpm;
  } else {
    rpmMaxLimit = maxRpm;
  }
  ffMaxRpm = (rpmMaxLimit > 0U) ? rpmMaxLimit : 1U;
}

void FanControl::SetFeedforwardEnabled(bool isEnabled) {
  feedforwardEnabled = isEnabled;
}

void FanControl::SetFeedforwardRpmScale(float scale) {
  if (scale < 0.85f) {
    feedforwardRpmScale = 0.85f;
  } else if (scale > 1.2f) {
    feedforwardRpmScale = 1.2f;
  } else {
    feedforwardRpmScale = scale;
  }
}

void FanControl::SetMaxPidTrim(uint8_t trim) {
  if (trim == 0U) {
    maxPidTrim = 1U;
  } else {
    maxPidTrim = trim;
  }
}

void FanControl::SetFanSpeed(uint32_t targetRpmIn) {
  uint32_t clamped = targetRpmIn;
  if (clamped < rpmMinLimit) {
    clamped = rpmMinLimit;
  } else if (clamped > rpmMaxLimit) {
    clamped = rpmMaxLimit;
  }
  if (clamped != lastTargetRpm) {
    integral = 0.0f;
    prevError = 0.0f;
    lastTargetRpm = clamped;
  }
  targetRpm = clamped;
}

void FanControl::SetPidGains(float kpIn, float kiIn, float kdIn) {
  kp = kpIn;
  ki = kiIn;
  kd = kdIn;
}

void FanControl::SetPwmInverted(bool isInvertedIn) {
  isPwmInverted = isInvertedIn;
}

void FanControl::SetMaxPwmStep(uint8_t stepPerUpdateIn) {
  if (stepPerUpdateIn == 0U) {
    maxPwmStep = 1U;
  } else {
    maxPwmStep = stepPerUpdateIn;
  }
}

void FanControl::Update() {
  const uint32_t nowMs = millis();
  if ((nowMs - lastUpdateMs) < sampleMs) {
    return;
  }

  const uint32_t rpmRaw = fan.ReadRpmInstant(InstantRpmTimeoutMs);
  if (rpmFiltered <= 0.0f) {
    rpmFiltered = static_cast<float>(rpmRaw);
  } else {
    rpmFiltered = (RpmFilterAlpha * static_cast<float>(rpmRaw)) +
                  ((1.0f - RpmFilterAlpha) * rpmFiltered);
  }
  currentRpm = static_cast<uint32_t>(rpmFiltered);

  const float error = static_cast<float>(targetRpm) - static_cast<float>(currentRpm);
  const float dt = static_cast<float>(sampleMs) * MsToSecond;

  integral += error * dt;
  if (integral > IntegralClamp) {
    integral = IntegralClamp;
  } else if (integral < -IntegralClamp) {
    integral = -IntegralClamp;
  } else {
    // no action
  }

  float derivative = 0.0f;
  if (dt > 0.0f) {
    derivative = (error - prevError) / dt;
  }

  const float control = (kp * error) + (ki * integral) + (kd * derivative);

  int32_t pwmStep = static_cast<int32_t>(control);
  if (pwmStep > static_cast<int32_t>(maxPwmStep)) {
    pwmStep = static_cast<int32_t>(maxPwmStep);
  } else if (pwmStep < -static_cast<int32_t>(maxPwmStep)) {
    pwmStep = -static_cast<int32_t>(maxPwmStep);
  } else {
    // no action
  }
  if (pwmStep > static_cast<int32_t>(maxPidTrim)) {
    pwmStep = static_cast<int32_t>(maxPidTrim);
  } else if (pwmStep < -static_cast<int32_t>(maxPidTrim)) {
    pwmStep = -static_cast<int32_t>(maxPidTrim);
  }

  int32_t pwm = static_cast<int32_t>(fan.GetPwmDuty());
  if (feedforwardEnabled && (ffMaxRpm > 0U)) {
    float tFf = static_cast<float>(targetRpm) * feedforwardRpmScale;
    if (tFf > static_cast<float>(ffMaxRpm)) {
      tFf = static_cast<float>(ffMaxRpm);
    }
    const uint32_t t =
        static_cast<uint32_t>(tFf + 0.5f);  // feedforward icin olceklenmis hedef
    uint32_t pwmFf = 0U;
    if (isPwmInverted) {
      // Yuksek RPM -> dusuk duty (ters surum).
      pwmFf = static_cast<uint32_t>(PwmMax) -
              ((t * static_cast<uint32_t>(PwmMax)) / ffMaxRpm);
    } else {
      pwmFf = (t * static_cast<uint32_t>(PwmMax)) / ffMaxRpm;
    }
    if (pwmFf > static_cast<uint32_t>(PwmMax)) {
      pwmFf = static_cast<uint32_t>(PwmMax);
    }
    pwm = static_cast<int32_t>(pwmFf);
    if (isPwmInverted) {
      pwm -= pwmStep;
    } else {
      pwm += pwmStep;
    }
  } else {
    if (isPwmInverted) {
      pwm -= pwmStep;
    } else {
      pwm += pwmStep;
    }
  }

  if (pwm > static_cast<int32_t>(PwmMax)) {
    pwm = static_cast<int32_t>(PwmMax);
  } else if (pwm < static_cast<int32_t>(PwmMin)) {
    pwm = static_cast<int32_t>(PwmMin);
  } else {
    // no action
  }

  fan.SetPwmDuty(static_cast<uint8_t>(pwm));

  prevError = error;
  lastUpdateMs = nowMs;
}

uint32_t FanControl::GetFanRpm() const { return currentRpm; }

uint32_t FanControl::GetTargetRpm() const { return targetRpm; }

uint8_t FanControl::GetPwmDuty() const { return fan.GetPwmDuty(); }

float FanControl::GetPwmPercent() const { return fan.GetPwmPercent(); }

