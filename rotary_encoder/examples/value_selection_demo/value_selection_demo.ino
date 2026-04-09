#include <Arduino.h>
#include <rotary_encoder.h>

// Nucleo G474RE ornek: CLK=D2, DT=D8, SW=D9
static constexpr uint32_t pin_enc_clk = 2U;
static constexpr uint32_t pin_enc_dt = 8U;
static constexpr uint32_t pin_enc_sw = 9U;

static RotaryEncoder enc(pin_enc_clk, pin_enc_dt, pin_enc_sw);

void setup() {
  Serial.begin(115200);
  delay(100);

  enc.Begin();
  enc.SetButtonDebounceMs(30U);
  enc.SetBehaviorMode(RotaryEncoder::BehaviorMode::ValueSelection);  // 1 tik ~= +2
  enc.SetOutputLimits(0, 100);
  enc.SetValue(50);

  Serial.println("Value demo basladi (0..100)");
}

void loop() {
  enc.Update();

  static int32_t last_value = -1;
  const int32_t v = enc.Value();
  if (v != last_value) {
    last_value = v;
    Serial.print("Deger: ");
    Serial.println(v);
  }

  if (enc.ButtonPressedEdge()) {
    enc.SetValue(50);
    Serial.println("Buton: deger 50'ye sifirlandi");
  }
}
