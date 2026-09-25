#include "garden.h"

namespace garden {

Garden::Garden(const Config& cfg, const Saved& saved) : cfg_(cfg), saved_(saved) {}

Saved Garden::firstEver() {
  Saved s;
  s.reveal = (uint8_t)Reveal::Grew;  // first press shows her seed
  return s;
}

Stage Garden::stageFor(uint16_t days) const {
  if (days >= cfg_.flower_day) return Stage::Flower;
  if (days >= cfg_.bud_day) return Stage::Bud;
  if (days >= cfg_.leaves_day) return Stage::Leaves;
  if (days >= cfg_.sprout_day) return Stage::Sprout;
  return Stage::Seed;
}

bool Garden::update(int64_t now_ms, bool asleep, bool thirsty, bool light_ok) {
  if (!started_) {
    started_ = true;
    last_ms_ = now_ms;
    // Just powered on: the first dawn always counts, even after an evening power cut.
    last_day_ms_ = now_ms - cfg_.min_day_ms;
    was_asleep_ = asleep;
    return false;
  }
  int64_t dt = now_ms - last_ms_;
  last_ms_ = now_ms;
  if (!asleep && dt > 0) {
    awake_ms_ += dt;
    if (thirsty) thirsty_ms_ += dt;
  }

  uint16_t before = saved_.growth_days;
  uint8_t reveal_before = saved_.reveal;

  bool woke = was_asleep_ && !asleep;
  was_asleep_ = asleep;
  if (light_ok) {
    // A dawn too soon after the last one (a lamp at night) is ignored.
    if (woke && now_ms - last_day_ms_ >= cfg_.min_day_ms) countDay();
  } else if (now_ms - last_day_ms_ >= cfg_.no_light_day_ms) {
    countDay();  // no light sensor: a day per 24 hours instead
  }
  return saved_.growth_days != before || saved_.reveal != reveal_before;
}

void Garden::countDay() {
  bool cared_for = !cfg_.growth_needs_care || awake_ms_ == 0 ||
                   (float)thirsty_ms_ < cfg_.thirsty_all_day * (float)awake_ms_;
  last_day_ms_ = last_ms_;
  awake_ms_ = thirsty_ms_ = 0;
  if (!cared_for) return;  // growth waits a day; nothing is lost
  if (stage() == Stage::Flower) return;  // in full bloom until a new seed is planted

  Stage old_stage = stage();
  ++saved_.growth_days;
  Stage new_stage = stage();
  if (new_stage != old_stage) {
    if (new_stage == Stage::Flower) {
      ++saved_.keepsakes;
      saved_.reveal = (uint8_t)Reveal::Bloomed;
    } else if (saved_.reveal != (uint8_t)Reveal::Bloomed) {
      saved_.reveal = (uint8_t)Reveal::Grew;
    }
  }
}

void Garden::replant() {
  saved_.growth_days = 0;
  ++saved_.plant_number;
  saved_.reveal = (uint8_t)Reveal::Grew;  // her next press shows the new seed
  awake_ms_ = thirsty_ms_ = 0;
  last_day_ms_ = last_ms_;
}

Reveal Garden::takeReveal() {
  Reveal r = (Reveal)saved_.reveal;
  saved_.reveal = 0;
  return r;
}

}  // namespace garden
