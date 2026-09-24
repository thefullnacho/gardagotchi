#include "mood.h"

#include <math.h>

namespace mood {

Engine::Engine(const Config& cfg) : cfg_(cfg) {}

void Engine::onSensors(int64_t now_ms, const Sensors& s) {
  updateLight(now_ms, s);
  updateSoil(now_ms, s);
}

void Engine::updateLight(int64_t now_ms, const Sensors& s) {
  if (!s.light_ok) {
    // No light sensor: stay awake rather than guess.
    asleep_ = false;
    dark_since_ = light_since_ = -1;
    return;
  }
  lux_smooth_ = have_lux_ ? lux_smooth_ + cfg_.lux_smoothing * (s.lux - lux_smooth_) : s.lux;
  have_lux_ = true;

  // Sleep and wake use the raw reading with a long timer: a passing shadow or a lamp
  // flicked on for a moment does nothing.
  if (s.lux < cfg_.dark_lux) {
    light_since_ = -1;
    if (dark_since_ < 0) dark_since_ = now_ms;
    if (!asleep_ && now_ms - dark_since_ >= cfg_.dark_after_ms) asleep_ = true;
  } else if (s.lux > cfg_.light_lux) {
    dark_since_ = -1;
    if (light_since_ < 0) light_since_ = now_ms;
    if (asleep_ && now_ms - light_since_ >= cfg_.light_after_ms) asleep_ = false;
  } else {
    dark_since_ = light_since_ = -1;  // in between: neither timer runs
  }
}

void Engine::updateSoil(int64_t now_ms, const Sensors& s) {
  soil_ok_ = s.soil_ok;
  if (!s.soil_ok) {
    // Never ask for water because of a broken or unplugged sensor.
    thirsty_ = false;
    wet_since_ = -1;
    have_baseline_ = false;
    return;
  }
  const float m = s.moisture;

  // Thirsty, with hysteresis. It never escalates: one gentle face, however long.
  if (m < cfg_.moisture_dry) thirsty_ = true;
  else if (m > cfg_.moisture_dry + cfg_.moisture_margin) thirsty_ = false;

  // Too wet: start the clock. Soggy only shows once it has stayed wet a while.
  if (m > cfg_.moisture_soggy) {
    if (wet_since_ < 0) wet_since_ = now_ms;
  } else if (m < cfg_.moisture_soggy - cfg_.moisture_margin) {
    wet_since_ = -1;
  }

  // Watering detection. The baseline follows falls at once but rises only slowly,
  // so drift never looks like watering and a real pour does.
  if (!have_baseline_) {
    baseline_ = m;
    have_baseline_ = true;
    last_soil_ms_ = now_ms;
    return;
  }
  float dt_s = (now_ms - last_soil_ms_) / 1000.0f;
  last_soil_ms_ = now_ms;
  if (m < baseline_) {
    baseline_ = m;
  } else {
    float creep = cfg_.drift_per_s * dt_s;
    baseline_ += (m - baseline_ < creep) ? (m - baseline_) : creep;
  }

  if (m - baseline_ >= cfg_.water_jump) {
    bool was_already_full = baseline_ >= cfg_.moisture_full;
    bool in_cooldown =
        last_celebrate_ms_ >= 0 && now_ms - last_celebrate_ms_ < cfg_.watering_cooldown_ms;
    if (!was_already_full && !in_cooldown) {
      reaction_ = Face::Celebrate;
      reaction_until_ = now_ms + cfg_.celebrate_ms;
      pending_sound_ = Sound::Fanfare;
      bumpAffection(now_ms, cfg_.celebrate_bump);
      last_celebrate_ms_ = now_ms;
      ++celebrations_;
    }
    baseline_ = m;  // this pour is counted (or ignored); don't count it twice
  }
}

void Engine::onPress(int64_t now_ms) {
  // A celebration in progress keeps going; the press still counts as love.
  bool celebrating = reaction_ == Face::Celebrate && now_ms < reaction_until_;
  if (asleep_) {
    if (!celebrating) {
      reaction_ = Face::SleepyLove;
      reaction_until_ = now_ms + cfg_.sleepy_love_ms;
    }
    // No sound at night.
  } else {
    if (!celebrating) {
      reaction_ = Face::Love;
      reaction_until_ = now_ms + cfg_.love_ms;
    }
    if (pending_sound_ == Sound::None) pending_sound_ = Sound::Chirp;
  }
  bumpAffection(now_ms, cfg_.love_bump);
}

Output Engine::tick(int64_t now_ms) {
  Output out;
  bool reacting = reaction_until_ >= 0 && now_ms < reaction_until_;
  out.face = reacting ? reaction_ : moodFace(now_ms);
  if (reacting) out.led = Led::Excited;
  else if (asleep_) out.led = Led::Off;
  else if (out.face == Face::Thirsty) out.led = Led::Asking;
  else out.led = Led::Calm;
  out.sound = pending_sound_;
  pending_sound_ = Sound::None;
  return out;
}

Face Engine::moodFace(int64_t now_ms) const {
  if (asleep_) return Face::Sleeping;
  if (wet_since_ >= 0) {
    int64_t wet_for = now_ms - wet_since_;
    if (wet_for >= cfg_.soggy_after_ms && wet_for < cfg_.soggy_after_ms + cfg_.soggy_max_ms)
      return Face::Soggy;
  }
  if (thirsty_) return Face::Thirsty;
  if (have_lux_ && lux_smooth_ >= cfg_.sunny_lux) return Face::Sunny;
  if (have_lux_ && lux_smooth_ <= cfg_.cloudy_lux) return Face::Cloudy;
  if (affection(now_ms) >= cfg_.happy_at) return Face::Happy;
  return Face::Content;
}

float Engine::affection(int64_t now_ms) const {
  float dt = (float)(now_ms - affection_at_);
  if (dt <= 0) return affection_;
  return affection_ * powf(0.5f, dt / (float)cfg_.affection_half_life_ms);
}

void Engine::bumpAffection(int64_t now_ms, float amount) {
  float a = affection(now_ms) + amount;
  affection_ = a > 1.0f ? 1.0f : a;
  affection_at_ = now_ms;
}

}  // namespace mood
