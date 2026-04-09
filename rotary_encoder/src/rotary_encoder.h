#ifndef ROTARY_ENCODER_H_
#define ROTARY_ENCODER_H_

#include <Arduino.h>

// Quadrature rotary encoder (KY-040 style) with optional push button.
// Call Update() each loop iteration (possibly multiple times per loop).

class RotaryEncoder {
 public:
  enum class BehaviorMode : uint8_t {
    MenuSelection = 0,   // detent basina +1 (stabil menuler)
    ValueSelection = 1,  // detent basina +2 (hizli sayi secimi)
    Custom = 2,          // ozel ConfigureScaling(...)
  };

  /// Recognized UI scaling presets (see ConfigureForMenuNavigation / ConfigureForValueRangeSelection).
  enum class InputProfile : uint8_t {
    Custom = 0,
    MenuNavigation = 1,
    ValueRangeSelection = 2,
  };

  // pin_sw unused: pass 0xFFFFFFFF
  RotaryEncoder(uint32_t pin_clk, uint32_t pin_dt, uint32_t pin_sw = 0xFFFFFFFFU);

  void Begin(bool use_pullup_clk_dt = true, bool use_pullup_sw = true);

  void Update();

  // -------------------------------------------------------------------------
  // Raw quadrature steps (one count per valid edge transition). Advanced use.
  // -------------------------------------------------------------------------
  int32_t RawPosition() const { return position_; }
  void SetRawPosition(int32_t p);

  /** @deprecated Prefer RawPosition(); kept for compatibility */
  int32_t Position() const { return position_; }
  /** @deprecated Prefer SetRawPosition(); kept for compatibility */
  void SetPosition(int32_t p);

  int8_t Delta() const { return last_delta_; }

  void SetDirectionInverted(bool inv) { invert_dir_ = inv; }

  // -------------------------------------------------------------------------
  // Scaled value for UI: value = (raw * scale_num) / scale_den
  // Default 1:1 until you call Configure*.
  // -------------------------------------------------------------------------
  int32_t Value() const;
  void SetValue(int32_t v);

  /// KY-040-like modules: typically 4 quadrature edges per mechanical detent.
  /// Result: one detent moves the UI by 1 (stable menu lines).
  void ConfigureForMenuNavigation(uint8_t quadratureStepsPerDetent = 4);

  /// Faster numeric edit (e.g. 0–100): one detent adds @p valueStepsPerDetent to Value().
  void ConfigureForValueRangeSelection(uint8_t quadratureStepsPerDetent = 4,
                                       int32_t valueStepsPerDetent = 2);

  // High-level presets: "menude bu, sayida bu" davranisini tek noktada toplar.
  void SetBehaviorMode(BehaviorMode mode);

  /// Full control: Value = (RawPosition * numerator) / denominator (denominator > 0).
  void ConfigureScaling(int32_t numerator, int32_t denominator);

  int32_t ScaleNumerator() const { return scale_num_; }
  int32_t ScaleDenominator() const { return scale_den_; }
  InputProfile Profile() const { return profile_; }
  BehaviorMode Behavior() const { return behavior_mode_; }

  /// Optional clamp on Value() / SetValue() (default: disabled).
  void SetOutputLimits(int32_t minValue, int32_t maxValue);
  void ClearOutputLimits();

  bool ButtonIsDown() const;
  bool ButtonPressedEdge();
  bool ButtonReleasedEdge();
  bool HasButton() const { return sw_valid_; }
  void SetButtonDebounceMs(uint32_t ms);

 private:
  uint32_t pin_clk_;
  uint32_t pin_dt_;
  uint32_t pin_sw_;
  uint8_t state_ab_;
  int32_t position_;
  int8_t last_delta_;
  bool invert_dir_;
  bool sw_valid_;
  bool sw_pullup_;
  bool sw_last_sample_;
  bool sw_stable_down_;
  uint32_t debounce_ms_;
  uint32_t sw_last_debounce_ms_;
  bool pending_press_;
  bool pending_release_;

  int8_t last_quad_step_;
  uint32_t last_quad_step_ms_;
  static constexpr uint32_t kQuadBounceRejectMs = 55U;

  int32_t scale_num_;
  int32_t scale_den_;
  InputProfile profile_;
  BehaviorMode behavior_mode_;

  bool limit_enable_;
  int32_t limit_min_;
  int32_t limit_max_;

  void ClampRawToOutputLimits();
};

#endif  // ROTARY_ENCODER_H_
