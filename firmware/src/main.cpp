// Gardagotchi.
//
// env:frog       the frog, driven by the mood engine (lib/mood). Reads soil + light every
//                few seconds; the button always gets a reaction; the LED breathes calmly,
//                pulses faster when thirsty, and is off while the frog sleeps.
// env:calibrate  logging week: a soil + light reading every minute, printed to serial
//                and saved to flash. Press the button when you water the test pot:
//                that writes a "watered" marker row and shows the celebrate face.

#include <Arduino.h>
#include <esp_timer.h>

#include "controls.h"
#include "display.h"
#include "sensors.h"
#ifdef GG_CALIBRATE
#include "calib_log.h"
#else
#include "mood.h"
#endif

namespace {
constexpr sprites::Palette kPalette = sprites::MINT;  // she picks mint or lilac later

sprites::State shown = sprites::STATE_COUNT;  // nothing drawn yet

// A millisecond clock that doesn't wrap after 49 days like millis() does.
int64_t nowMs() { return esp_timer_get_time() / 1000; }

#ifdef GG_CALIBRATE
constexpr uint32_t kReactionMs = 3000;
constexpr uint32_t kSampleEveryMs = 60 * 1000;
sensors::Reading last;
uint32_t last_sample_ms = 0;
uint32_t reaction_until = 0;
#else
constexpr int64_t kSenseEveryMs = 5000;
mood::Engine engine;
int64_t last_sense_ms = -kSenseEveryMs;

sprites::State spriteFor(mood::Face f) {
  switch (f) {
    case mood::Face::Content: return sprites::CONTENT;
    case mood::Face::Happy: return sprites::HAPPY;
    case mood::Face::Love: return sprites::LOVE;
    case mood::Face::Thirsty: return sprites::THIRSTY;
    case mood::Face::Soggy: return sprites::SOGGY;
    case mood::Face::Sunny: return sprites::SUNNY;
    case mood::Face::Cloudy: return sprites::CLOUDY;
    case mood::Face::Sleeping: return sprites::SLEEPING;
    case mood::Face::Celebrate: return sprites::CELEBRATE;
    case mood::Face::SleepyLove: return sprites::SLEEPING;  // TODO: needs its own art
  }
  return sprites::CONTENT;
}

void driveLed(mood::Led led) {
  switch (led) {
    case mood::Led::Off: controls::setLed(0); break;
    case mood::Led::Calm: controls::breatheLed(4000); break;
    case mood::Led::Asking: controls::breatheLed(1200); break;
    case mood::Led::Excited: controls::breatheLed(400); break;
  }
}
#endif

void show(sprites::State s, bool force = false) {
  if (s == shown && !force) return;  // only redraw when something changed
  shown = s;
  display::drawFace(kPalette, s);
#ifdef GG_CALIBRATE
  char footer[40];
  snprintf(footer, sizeof(footer), "soil %s  lux %s", last.soil_ok ? String(last.moisture).c_str() : "--",
           last.light_ok ? String(last.lux, 0).c_str() : "--");
  display::drawFooter(footer);
#endif
  display::present();
}
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\ngardagotchi booting");
  controls::begin();
  display::begin();
  show(sprites::CONTENT);
  sensors::begin();
#ifdef GG_CALIBRATE
  calib_log::begin();
  last = sensors::read();
  calib_log::append(last, "boot");
  last_sample_ms = millis();
  show(sprites::CONTENT, true);
#endif
}

#ifdef GG_CALIBRATE
void loop() {
  uint32_t now = millis();
  if (controls::pressed()) {
    last = sensors::read();
    calib_log::append(last, "watered");
    reaction_until = now + kReactionMs;
    show(sprites::CELEBRATE, true);
  }
  bool reacting = (int32_t)(reaction_until - now) > 0;
  if (!reacting) show(sprites::CONTENT);
  controls::breatheLed(reacting ? 400 : 4000);

  if (now - last_sample_ms >= kSampleEveryMs) {
    last_sample_ms = now;
    last = sensors::read();
    calib_log::append(last, "");
    show(shown, true);  // refresh the footer numbers
  }
  calib_log::pollSerial();
  delay(5);
}
#else
void loop() {
  int64_t now = nowMs();

  // Button first, so a press is answered on this very loop.
  if (controls::pressed()) engine.onPress(now);

  if (now - last_sense_ms >= kSenseEveryMs) {
    last_sense_ms = now;
    sensors::Reading r = sensors::read();
    mood::Sensors s;
    s.soil_ok = r.soil_ok;
    s.moisture = r.moisture;
    s.light_ok = r.light_ok;
    s.lux = r.lux;
    engine.onSensors(now, s);
  }

  mood::Output out = engine.tick(now);
  sprites::State want = spriteFor(out.face);
  if (want != shown) Serial.printf("face %d\n", (int)out.face);
  show(want);
  driveLed(out.led);
  if (out.sound != mood::Sound::None) {
    // The speaker comes later; for now just say what it would play.
    Serial.println(out.sound == mood::Sound::Fanfare ? "sound: fanfare" : "sound: chirp");
  }
  delay(5);
}
#endif
