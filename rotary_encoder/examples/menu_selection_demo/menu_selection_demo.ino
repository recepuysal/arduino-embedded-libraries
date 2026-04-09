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
  enc.SetBehaviorMode(RotaryEncoder::BehaviorMode::MenuSelection);  // 1 tik ~= +1
  enc.SetOutputLimits(0, 5);  // 6 satirlik menu
  enc.SetValue(0);

  Serial.println("Menu demo basladi");
}

void loop() {
  enc.Update();

  static int32_t last_line = -1;
  const int32_t line = enc.Value();
  if (line != last_line) {
    last_line = line;
    Serial.print("Secili satir: ");
    Serial.println(line);
  }

  if (enc.ButtonPressedEdge()) {
    Serial.print("Secim onaylandi -> satir ");
    Serial.println(enc.Value());
  }
}
