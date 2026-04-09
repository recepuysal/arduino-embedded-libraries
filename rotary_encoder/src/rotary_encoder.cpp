#include "rotary_encoder.h"

namespace {

// (onceki_AB << 2) | yeni_AB -> -1, 0, +1 (Gray kod gecis tablosu)
const int8_t kQuadTable[16] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};

// Iki ardışık okuma aynı degilse pin gecisinde veya gurultude; bu kareyi yok say.
uint8_t ReadStableAB(uint32_t pin_clk, uint32_t pin_dt) {
  const uint8_t a1 = digitalRead(pin_clk) == HIGH ? 1U : 0U;
  const uint8_t b1 = digitalRead(pin_dt) == HIGH ? 1U : 0U;
  const uint8_t a2 = digitalRead(pin_clk) == HIGH ? 1U : 0U;
  const uint8_t b2 = digitalRead(pin_dt) == HIGH ? 1U : 0U;
  if (a1 != a2 || b1 != b2) {
    return 0xFFU;
  }
  return static_cast<uint8_t>((a1 << 1U) | b1);
}

}  // namespace

RotaryEncoder::RotaryEncoder(uint32_t pin_clk, uint32_t pin_dt, uint32_t pin_sw)
    : pin_clk_(pin_clk),
      pin_dt_(pin_dt),
      pin_sw_(pin_sw),
      state_ab_(0U),
      position_(0),
      last_delta_(0),
      invert_dir_(false),
      sw_valid_(pin_sw != 0xFFFFFFFFU),
      sw_pullup_(true),
      sw_last_sample_(true),
      sw_stable_down_(false),
      debounce_ms_(30U),
      sw_last_debounce_ms_(0U),
      pending_press_(false),
      pending_release_(false),
      last_quad_step_(0),
      last_quad_step_ms_(0U),
      scale_num_(1),
      scale_den_(1),
      profile_(InputProfile::Custom),
      behavior_mode_(BehaviorMode::MenuSelection),
      limit_enable_(false),
      limit_min_(0),
      limit_max_(0) {}

void RotaryEncoder::SetRawPosition(int32_t p) {
  position_ = p;
  ClampRawToOutputLimits();
}

void RotaryEncoder::SetPosition(int32_t p) {
  SetRawPosition(p);
}

void RotaryEncoder::Begin(bool use_pullup_clk_dt, bool use_pullup_sw) {
  if (use_pullup_clk_dt) {
    pinMode(pin_clk_, INPUT_PULLUP);
    pinMode(pin_dt_, INPUT_PULLUP);
  } else {
    pinMode(pin_clk_, INPUT);
    pinMode(pin_dt_, INPUT);
  }
  sw_pullup_ = use_pullup_sw;
  if (sw_valid_) {
    if (use_pullup_sw) {
      pinMode(pin_sw_, INPUT_PULLUP);
    } else {
      pinMode(pin_sw_, INPUT);
    }
    sw_last_sample_ = digitalRead(pin_sw_) == LOW;
    sw_stable_down_ = sw_last_sample_;
    sw_last_debounce_ms_ = millis();
  }
  uint8_t ab = ReadStableAB(pin_clk_, pin_dt_);
  if (ab == 0xFFU) {
    ab = static_cast<uint8_t>((digitalRead(pin_clk_) == HIGH ? 1U : 0U) << 1U |
                              (digitalRead(pin_dt_) == HIGH ? 1U : 0U));
  }
  state_ab_ = ab;
  last_delta_ = 0;
  last_quad_step_ = 0;
  last_quad_step_ms_ = millis();
  ClampRawToOutputLimits();
}

void RotaryEncoder::Update() {
  last_delta_ = 0;
  const uint8_t curr = ReadStableAB(pin_clk_, pin_dt_);
  if (curr != 0xFFU) {
    const uint8_t idx = static_cast<uint8_t>((state_ab_ << 2) | curr);
    int8_t d = kQuadTable[idx];
    if (invert_dir_) {
      d = static_cast<int8_t>(-d);
    }
    if (d != 0) {
      const uint32_t now_ms = millis();
      const bool bounce_back =
          (last_quad_step_ != 0) &&
          (d == static_cast<int8_t>(-last_quad_step_)) &&
          ((now_ms - last_quad_step_ms_) < kQuadBounceRejectMs);
      if (bounce_back) {
        last_delta_ = 0;
      } else {
        last_delta_ = d;
        position_ += static_cast<int32_t>(d);
        last_quad_step_ = d;
        last_quad_step_ms_ = now_ms;
      }
    }
    state_ab_ = curr;
  }

  ClampRawToOutputLimits();

  if (!sw_valid_) {
    return;
  }
  const uint32_t now = millis();
  const bool raw = digitalRead(pin_sw_) == LOW;
  if (raw != sw_last_sample_) {
    sw_last_debounce_ms_ = now;
  }
  if ((now - sw_last_debounce_ms_) >= debounce_ms_) {
    if (raw != sw_stable_down_) {
      sw_stable_down_ = raw;
      if (sw_stable_down_) {
        pending_press_ = true;
      } else {
        pending_release_ = true;
      }
    }
  }
  sw_last_sample_ = raw;
}

bool RotaryEncoder::ButtonIsDown() const {
  if (!sw_valid_) {
    return false;
  }
  return sw_stable_down_;
}

bool RotaryEncoder::ButtonPressedEdge() {
  if (pending_press_) {
    pending_press_ = false;
    return true;
  }
  return false;
}

bool RotaryEncoder::ButtonReleasedEdge() {
  if (pending_release_) {
    pending_release_ = false;
    return true;
  }
  return false;
}

void RotaryEncoder::SetButtonDebounceMs(uint32_t ms) {
  // Pratik bir alt limit: cok dusuk degerler mekanik titreşimi gecirir.
  if (ms < 5U) {
    debounce_ms_ = 5U;
    return;
  }
  debounce_ms_ = ms;
}

int32_t RotaryEncoder::Value() const {
  int32_t v;
  if (scale_den_ <= 0) {
    v = position_;
  } else {
    const int64_t n = static_cast<int64_t>(position_) * static_cast<int64_t>(scale_num_);
    v = static_cast<int32_t>(n / static_cast<int64_t>(scale_den_));
  }
  if (limit_enable_) {
    if (v < limit_min_) {
      v = limit_min_;
    }
    if (v > limit_max_) {
      v = limit_max_;
    }
  }
  return v;
}

void RotaryEncoder::SetValue(int32_t v) {
  if (limit_enable_) {
    if (v < limit_min_) {
      v = limit_min_;
    }
    if (v > limit_max_) {
      v = limit_max_;
    }
  }
  if (scale_den_ <= 0 || scale_num_ <= 0) {
    position_ = v;
    return;
  }
  const int64_t r =
      (static_cast<int64_t>(v) * static_cast<int64_t>(scale_den_)) / static_cast<int64_t>(scale_num_);
  position_ = static_cast<int32_t>(r);
  ClampRawToOutputLimits();
}

void RotaryEncoder::ConfigureForMenuNavigation(uint8_t quadratureStepsPerDetent) {
  uint8_t q = quadratureStepsPerDetent;
  if (q == 0U) {
    q = 4U;
  }
  scale_num_ = 1;
  scale_den_ = static_cast<int32_t>(q);
  profile_ = InputProfile::MenuNavigation;
  behavior_mode_ = BehaviorMode::MenuSelection;
  ClampRawToOutputLimits();
}

void RotaryEncoder::ConfigureForValueRangeSelection(uint8_t quadratureStepsPerDetent,
                                                    int32_t valueStepsPerDetent) {
  uint8_t q = quadratureStepsPerDetent;
  if (q == 0U) {
    q = 4U;
  }
  int32_t vs = valueStepsPerDetent;
  if (vs < 1) {
    vs = 1;
  }
  scale_num_ = vs;
  scale_den_ = static_cast<int32_t>(q);
  profile_ = InputProfile::ValueRangeSelection;
  behavior_mode_ = BehaviorMode::ValueSelection;
  ClampRawToOutputLimits();
}

void RotaryEncoder::SetBehaviorMode(BehaviorMode mode) {
  behavior_mode_ = mode;
  if (mode == BehaviorMode::MenuSelection) {
    ConfigureForMenuNavigation(4);
  } else {
    ConfigureForValueRangeSelection(4, 2);
  }
}

void RotaryEncoder::ConfigureScaling(int32_t numerator, int32_t denominator) {
  int32_t d = denominator;
  if (d <= 0) {
    d = 1;
  }
  int32_t n = numerator;
  if (n <= 0) {
    n = 1;
  }
  scale_num_ = n;
  scale_den_ = d;
  profile_ = InputProfile::Custom;
  behavior_mode_ = BehaviorMode::Custom;
  ClampRawToOutputLimits();
}

void RotaryEncoder::SetOutputLimits(int32_t minValue, int32_t maxValue) {
  if (minValue > maxValue) {
    const int32_t tmp = minValue;
    minValue = maxValue;
    maxValue = tmp;
  }
  limit_min_ = minValue;
  limit_max_ = maxValue;
  limit_enable_ = true;
  ClampRawToOutputLimits();
}

void RotaryEncoder::ClearOutputLimits() {
  limit_enable_ = false;
}

void RotaryEncoder::ClampRawToOutputLimits() {
  if (!limit_enable_ || scale_den_ <= 0 || scale_num_ <= 0) {
    return;
  }
  const int64_t n = static_cast<int64_t>(position_) * static_cast<int64_t>(scale_num_);
  int32_t scaled = static_cast<int32_t>(n / static_cast<int64_t>(scale_den_));
  if (scaled > limit_max_) {
    position_ = static_cast<int32_t>((static_cast<int64_t>(limit_max_) * static_cast<int64_t>(scale_den_)) /
                                     static_cast<int64_t>(scale_num_));
  } else if (scaled < limit_min_) {
    position_ = static_cast<int32_t>((static_cast<int64_t>(limit_min_) * static_cast<int64_t>(scale_den_)) /
                                     static_cast<int64_t>(scale_num_));
  }
}
