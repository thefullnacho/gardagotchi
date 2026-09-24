#include "sensors.h"

#include <Adafruit_seesaw.h>
#include <BH1750.h>
#include <Wire.h>

#include "pins.h"

namespace {
Adafruit_seesaw soil(&Wire);
BH1750 light(ADDR_LIGHT);
bool soil_up = false;
bool light_up = false;
uint32_t last_try_ms = 0;
constexpr uint32_t kRetryMs = 30000;  // looking for a missing sensor takes a moment

void tryStart() {
  if (soil_up && light_up) return;
  uint32_t now = millis();
  if (last_try_ms != 0 && now - last_try_ms < kRetryMs) return;
  last_try_ms = now ? now : 1;
  if (!soil_up) soil_up = soil.begin(ADDR_SOIL);
  if (!light_up) light_up = light.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, ADDR_LIGHT, &Wire);
}
}  // namespace

namespace sensors {

void begin() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, 100000);  // 100 kHz: forgiving on longer wires
  Wire.setTimeOut(50);
  tryStart();
  Serial.printf("sensors: soil %s, light %s\n", soil_up ? "found" : "MISSING",
                light_up ? "found" : "MISSING");
}

Reading read() {
  tryStart();
  Reading r;
  if (soil_up) {
    // Average a few reads; single capacitive readings jitter.
    uint32_t sum = 0;
    int good = 0;
    for (int i = 0; i < 4; ++i) {
      uint16_t v = soil.touchRead(0);
      if (v != 65535) {  // 65535 = read failed
        sum += v;
        ++good;
      }
      delay(5);
    }
    if (good) {
      r.soil_ok = true;
      r.moisture = sum / good;
      r.soil_temp_c = soil.getTemp();
    } else {
      soil_up = false;  // look for it again next time
    }
  }
  if (light_up) {
    float lux = light.readLightLevel();
    if (lux >= 0) {
      r.light_ok = true;
      r.lux = lux;
    } else {
      light_up = false;
    }
  }
  return r;
}

}  // namespace sensors
