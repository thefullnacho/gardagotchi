// Soil moisture (Adafruit STEMMA seesaw) and light (BH1750), both on the shared I2C bus.
#pragma once
#include <stdint.h>

namespace sensors {

struct Reading {
  bool soil_ok = false;
  uint16_t moisture = 0;  // raw capacitance, roughly 200 (dry air) to 2000 (wet)
  float soil_temp_c = 0;  // the seesaw's chip temperature, only roughly the soil's
  bool light_ok = false;
  float lux = 0;          // tops out near 54,600 lux in this mode (direct sun saturates)
};

void begin();
// Reads both sensors. A sensor that is missing or unplugged comes back with ok=false,
// and the next call tries to find it again.
Reading read();

}  // namespace sensors
