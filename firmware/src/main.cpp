// Gardagotchi.
//
// env:frog       the frog, driven by the mood engine (lib/mood) and the garden
//                (lib/garden). Reads soil + light every few seconds; the button always
//                gets a reaction; the LED breathes calmly, pulses faster when thirsty,
//                twinkles when a surprise is waiting, and is off while the frog sleeps.
//                Grown-up gesture: hold the button while plugging in, for 3 seconds,
//                to plant a new seed (the collection is kept).
// env:calibrate  logging week: a soil + light reading every minute, printed to serial
//                and saved to flash. Press the button when you water the test pot:
//                that writes a "watered" marker row and shows the celebrate face.

#include <Arduino.h>
#include <esp_timer.h>

#include "anim.h"
#include "controls.h"
#include "display.h"
#include "sensors.h"
#ifdef GG_CALIBRATE
#include "calib_log.h"
#else
#include "garden.h"
#include "garden_store.h"
#include "mood.h"
#endif

namespace {
constexpr sprites::Palette kPalette = sprites::MINT;  // she picks mint or lilac later

int shown_frame = -1;  // nothing drawn yet
anim::Player player;
int bed_flowers = 0;   // her collection, drawn over frog faces
bool bed_night = false;
int shown_bed = -1;

void showFrame(uint16_t f, bool force = false);

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
constexpr uint32_t kReplantHoldMs = 3000;
mood::Engine engine;
garden::Garden plant;
bool light_ok = false;
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
    case mood::Face::SleepyLove: return sprites::SLEEPY_LOVE;
    case mood::Face::PlantCard: return (sprites::State)(sprites::CARD_SEED + (int)plant.stage());
  }
  return sprites::CONTENT;
}

void driveLed(mood::Led led) {
  switch (led) {
    case mood::Led::Off: controls::setLed(0); break;
    case mood::Led::Calm: controls::breatheLed(4000); break;
    case mood::Led::Asking: controls::breatheLed(1200); break;
    case mood::Led::Excited: controls::breatheLed(400); break;
    case mood::Led::Surprise: controls::twinkleLed(); break;
  }
}

// Grown-up gesture at power-on: button held for 3 s plants a new seed.
void checkReplantGesture() {
  if (!controls::held()) return;
  uint32_t start = millis();
  while (controls::held() && millis() - start < kReplantHoldMs) {
    controls::setLed(1.0f);
    delay(10);
  }
  if (millis() - start >= kReplantHoldMs) {
    plant.replant();
    garden_store::save(plant.saved());
    Serial.println("garden: new seed planted");
    showFrame(sprites::kClips[sprites::CARD_SEED].first);
  }
  while (controls::held()) delay(10);  // don't count this hold as a press
  controls::setLed(0);
}
#endif

// Draw a frame, but only if it isn't already on screen (a redraw takes ~25 ms).
void showFrame(uint16_t f, bool force) {
  int bed = bed_flowers * 2 + (bed_night ? 1 : 0);
  if ((int)f == shown_frame && bed == shown_bed && !force) return;
  shown_frame = f;
  shown_bed = bed;
  display::drawFrame(kPalette, f);
  if (bed_flowers > 0) display::drawFlowerBed(bed_flowers, bed_night);
#ifdef GG_CALIBRATE
  char footer[40];
  snprintf(footer, sizeof(footer), "soil %s  lux %s", last.soil_ok ? String(last.moisture).c_str() : "--",
           last.light_ok ? String(last.lux, 0).c_str() : "--");
  display::drawFooter(footer);
#endif
  display::present();
}

#ifdef GG_CALIBRATE
// The calibration build shows still faces: each face's first frame.
void show(sprites::State s, bool force = false) { showFrame(sprites::kClips[s].first, force); }
#endif
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\ngardagotchi booting");
  controls::begin();
  display::begin();
  showFrame(sprites::kClips[sprites::CONTENT].first);
  sensors::begin();
#ifndef GG_CALIBRATE
  plant = garden::Garden(garden::Config(), garden_store::load());
  checkReplantGesture();
#endif
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
    showFrame(shown_frame, true);  // refresh the footer numbers
  }
  calib_log::pollSerial();
  delay(5);
}
#else
void loop() {
  int64_t now = nowMs();

  // Button first, so a press is answered on this very loop.
  if (controls::pressed()) {
    if (!engine.asleep() && plant.revealWaiting()) {
      plant.takeReveal();  // the plant card for its current stage
      garden_store::save(plant.saved());
      engine.reveal(now);
    } else {
      engine.onPress(now);
    }
  }

  if (now - last_sense_ms >= kSenseEveryMs) {
    last_sense_ms = now;
    sensors::Reading r = sensors::read();
    mood::Sensors s;
    s.soil_ok = r.soil_ok;
    s.moisture = r.moisture;
    s.light_ok = r.light_ok;
    s.lux = r.lux;
    light_ok = r.light_ok;
    engine.onSensors(now, s);
  }

  if (plant.update(now, engine.asleep(), engine.thirsty(), light_ok)) {
    garden_store::save(plant.saved());
  }
  engine.setSurpriseWaiting(plant.revealWaiting());

  mood::Output out = engine.tick(now);
  static mood::Face last_face = mood::Face::Content;
  if (out.face != last_face) {
    Serial.printf("face %d\n", (int)out.face);
    last_face = out.face;
  }
  bool on_card = out.face == mood::Face::PlantCard;
  bed_flowers = on_card ? 0 : plant.saved().keepsakes;  // the card is the plant itself
  bed_night = out.face == mood::Face::Sleeping || out.face == mood::Face::SleepyLove;
  player.play(&sprites::kClips[spriteFor(out.face)], now);
  showFrame(player.frame(now));
  driveLed(out.led);
  if (out.sound != mood::Sound::None) {
    // The speaker comes later; for now just say what it would play.
    Serial.println(out.sound == mood::Sound::Fanfare ? "sound: fanfare" : "sound: chirp");
  }
  delay(5);
}
#endif
