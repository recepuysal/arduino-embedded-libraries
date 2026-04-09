#include <Arduino.h>
#include <ds18b20.h>

// DS18B20 DATA pini (Arduino pin numarasi)
static const uint8_t kPinDs18b20 = 7U;

// Her 1 saniyede bir yeni donusum baslatilir
static const uint32_t kSamplePeriodMs = 1000UL;

static Ds18b20 sensor(kPinDs18b20);
static uint32_t lastTriggerMs = 0UL;
static bool conversionPending = false;

void setup(void) {
  Serial.begin(115200);
  delay(200);

  // Harici 4.7k pull-up yoksa true kullanilabilir (harici direnç tavsiye edilir)
  sensor.Begin(true);
  Serial.println(F("DS18B20 non-blocking demo basladi."));
}

void loop(void) {
  const uint32_t nowMs = millis();

  if (!conversionPending) {
    if ((nowMs - lastTriggerMs) >= kSamplePeriodMs) {
      lastTriggerMs = nowMs;
      conversionPending = sensor.StartConversion();
      if (!conversionPending) {
        Serial.println(F("Donusum baslatilamadi (sensor/yol kontrol edin)."));
      }
    }
    return;
  }

  if (!sensor.IsConversionReady(nowMs)) {
    return;
  }

  {
    float temperatureC = 0.0f;
    const bool ok = sensor.ReadLastConversion(temperatureC);
    conversionPending = false;

    if (ok) {
      Serial.print(F("Sicaklik: "));
      Serial.print(temperatureC, 2);
      Serial.println(F(" C"));
    } else {
      Serial.println(F("Okuma hatasi (CRC veya baglanti)."));
    }
  }
}
