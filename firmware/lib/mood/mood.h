// The frog's mood: what face to show, how the button LED behaves, and when to make a
// sound. Plain C++ with no Arduino code, so it runs in desktop tests
// (`pio test -e native`) as well as on the board.
//
// Two layers (see docs/firmware-notes.md):
//   mood      = the resting face, from the pot and the room:
//               sleeping > soggy > thirsty > sunny > cloudy > happy > content
//   reaction  = a few seconds on top of any mood: celebrate > love
// Rules that come from who it's for: the button always gets an answer, the frog never
// asks at night, thirsty never gets worse, and watering is rewarded once per watering
// (not once per pour), so pouring more never earns more.
#pragma once
#include <stdint.h>

namespace mood {

enum class Face : uint8_t {
  Content, Happy, Love, Thirsty, Soggy, Sunny, Cloudy, Sleeping, Celebrate,
  SleepyLove,  // a press at night: one eye peeks open, a small heart
};

enum class Led : uint8_t {
  Off,      // asleep
  Calm,     // slow breath
  Asking,   // faster pulse: "come see me" (thirsty)
  Excited,  // quick flutter during a reaction
};

enum class Sound : uint8_t { None, Chirp, Fanfare };

// Every tunable number. Moisture and light values are PLACEHOLDERS until the
// calibration week (docs/calibration.md).
struct Config {
  // Soil (raw seesaw reading, roughly 200 dry air to 2000 in water)
  float moisture_dry = 450;      // thirsty below this
  float moisture_full = 800;     // "well watered": pouring more earns no celebration
  float moisture_soggy = 1100;   // too wet above this...
  float moisture_margin = 40;    // hysteresis so the face doesn't flicker at a line
  int64_t soggy_after_ms = 3LL * 3600 * 1000;  // ...but only if it stays wet this long
  int64_t soggy_max_ms = 6LL * 3600 * 1000;    // then the frog forgives anyway
  float water_jump = 80;         // a rise this big, fast, means she watered
  float drift_per_s = 80.0f / 600;  // slow rises (drift, humidity) under this are ignored
  int64_t watering_cooldown_ms = 20LL * 60 * 1000;  // one celebration per watering

  // Light (lux)
  float dark_lux = 5;            // below this counts as dark
  float light_lux = 20;          // above this counts as light (gap = hysteresis)
  int64_t dark_after_ms = 10LL * 60 * 1000;   // dark this long -> sleep
  int64_t light_after_ms = 10LL * 60 * 1000;  // light this long -> wake
  float sunny_lux = 8000;
  float cloudy_lux = 1500;
  float lux_smoothing = 0.2f;    // 0-1, higher follows the sensor faster

  // Reactions
  int64_t love_ms = 3000;
  int64_t sleepy_love_ms = 2000;
  int64_t celebrate_ms = 6000;

  // Affection: button presses and waterings fill it, it fades by itself.
  float love_bump = 0.25f;
  float celebrate_bump = 0.5f;
  float happy_at = 0.4f;         // above this the resting face is Happy, not Content
  int64_t affection_half_life_ms = 3LL * 3600 * 1000;
};

struct Sensors {
  bool soil_ok = false;
  float moisture = 0;
  bool light_ok = false;
  float lux = 0;
};

struct Output {
  Face face = Face::Content;
  Led led = Led::Calm;
  Sound sound = Sound::None;  // one-shot: play it once, the next tick() says None
};

class Engine {
 public:
  explicit Engine(const Config& cfg = Config());

  // Feed a new sensor reading (every few seconds is plenty).
  void onSensors(int64_t now_ms, const Sensors& s);
  // The button went down.
  void onPress(int64_t now_ms);
  // Call every loop. Times are milliseconds from any steady clock.
  Output tick(int64_t now_ms);

  // For logging and tests.
  bool asleep() const { return asleep_; }
  bool thirsty() const { return thirsty_; }
  float affection(int64_t now_ms) const;
  uint32_t celebrations() const { return celebrations_; }

 private:
  Face moodFace(int64_t now_ms) const;
  void bumpAffection(int64_t now_ms, float amount);
  void updateLight(int64_t now_ms, const Sensors& s);
  void updateSoil(int64_t now_ms, const Sensors& s);

  Config cfg_;

  // light
  bool asleep_ = false;
  int64_t dark_since_ = -1;
  int64_t light_since_ = -1;
  bool have_lux_ = false;
  float lux_smooth_ = 0;

  // soil
  bool soil_ok_ = false;
  bool thirsty_ = false;
  int64_t wet_since_ = -1;
  bool have_baseline_ = false;
  float baseline_ = 0;
  int64_t last_soil_ms_ = 0;
  int64_t last_celebrate_ms_ = -1;
  uint32_t celebrations_ = 0;

  // reactions
  Face reaction_ = Face::Love;
  int64_t reaction_until_ = -1;

  // affection
  float affection_ = 0;
  int64_t affection_at_ = 0;

  Sound pending_sound_ = Sound::None;
};

}  // namespace mood
