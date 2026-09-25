#include "garden_store.h"

#include <Arduino.h>
#include <Preferences.h>

namespace {
constexpr const char* kNamespace = "garden";
constexpr const char* kKey = "saved1";  // bump the number if Saved's layout changes
}  // namespace

namespace garden_store {

garden::Saved load() {
  Preferences prefs;
  prefs.begin(kNamespace, true);
  garden::Saved s;
  size_t got = prefs.isKey(kKey) ? prefs.getBytes(kKey, &s, sizeof(s)) : 0;
  prefs.end();
  if (got != sizeof(s)) {
    Serial.println("garden: nothing saved, planting her first seed");
    s = garden::Garden::firstEver();
    save(s);
  }
  return s;
}

void save(const garden::Saved& s) {
  Preferences prefs;
  prefs.begin(kNamespace, false);
  prefs.putBytes(kKey, &s, sizeof(s));
  prefs.end();
  Serial.printf("garden: saved (plant %u, day %u, keepsakes %u)\n", s.plant_number,
                s.growth_days, s.keepsakes);
}

}  // namespace garden_store
