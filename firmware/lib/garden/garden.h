// The plant on the screen and the collection she earns. Plain C++ (no Arduino),
// tested with `pio test -e native`.
//
// Rules (see docs/firmware-notes.md):
// - A day is one sleep: the frog counts dawns (it has no clock). Dawns closer together
//   than min_day_ms don't count, so a lamp flicked on and off at night can't add days.
// - Growth only ever moves forward. A day counts unless the plant was thirsty for
//   nearly the whole of it; then growth just waits. Nothing on screen shows a missed day.
// - When the plant reaches a new stage, a surprise waits for her next press.
// - Each plant that reaches flower adds one keepsake to the collection, for good.
#pragma once
#include <stdint.h>

namespace garden {

enum class Stage : uint8_t { Seed, Sprout, Leaves, Bud, Flower };

enum class Reveal : uint8_t { None, Grew, Bloomed };

struct Config {
  // Growth days to reach each stage, roughly a dwarf French marigold sown indoors
  // (sprouts in about a week, flowers around 7 weeks). Adjust from the actual packet.
  uint16_t sprout_day = 6;
  uint16_t leaves_day = 14;
  uint16_t bud_day = 40;
  uint16_t flower_day = 50;

  int64_t min_day_ms = 20LL * 3600 * 1000;       // dawns closer than this don't count
  int64_t no_light_day_ms = 24LL * 3600 * 1000;  // no light sensor: count by awake time
  bool growth_needs_care = true;                 // false = pure days since planting
  float thirsty_all_day = 0.9f;                  // thirsty this share of the day = no growth
};

// Everything that must survive a power cut. The firmware saves it to flash whenever
// update() says it changed.
struct Saved {
  uint16_t growth_days = 0;
  uint16_t plant_number = 1;   // which plant this is (1 = her first)
  uint16_t keepsakes = 0;      // flowers bloomed, ever. Only goes up.
  uint8_t reveal = 0;          // Reveal waiting for her next press (as a number)
  uint8_t reserved = 0;
};

class Garden {
 public:
  explicit Garden(const Config& cfg = Config(), const Saved& saved = Saved());

  // A brand-new toy: the first seed, with a surprise waiting (the seed card) for the
  // first press.
  static Saved firstEver();

  // Call every loop with the frog's state. Returns true when Saved changed and should
  // be written to flash.
  bool update(int64_t now_ms, bool asleep, bool thirsty, bool light_ok);

  // A grown-up planted a new seed. Growth restarts; keepsakes are kept.
  void replant();

  Stage stage() const { return stageFor(saved_.growth_days); }
  Stage stageFor(uint16_t days) const;
  const Saved& saved() const { return saved_; }
  bool revealWaiting() const { return saved_.reveal != 0; }
  // Her press, while awake: returns what to show and clears it.
  Reveal takeReveal();

 private:
  void countDay();

  Config cfg_;
  Saved saved_;
  bool was_asleep_ = false;
  bool started_ = false;
  int64_t last_ms_ = 0;
  int64_t last_day_ms_ = 0;  // when the last day was counted (or boot)
  int64_t awake_ms_ = 0;     // today so far
  int64_t thirsty_ms_ = 0;
};

}  // namespace garden
