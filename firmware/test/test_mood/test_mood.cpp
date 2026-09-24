// Desktop tests for the mood engine:  cd firmware && pio test -e native
#include <unity.h>

#include "mood.h"

using mood::Face;
using mood::Led;
using mood::Sound;

namespace {

constexpr int64_t kSec = 1000;
constexpr int64_t kMin = 60 * kSec;
constexpr int64_t kHour = 60 * kMin;

// A fake windowsill: the pot and the light, with a clock we can fast-forward.
struct Sim {
  mood::Engine engine;
  int64_t now = 0;
  mood::Sensors s;
  bool saw_celebrate = false;  // during the last water() call
  bool heard_fanfare = false;

  Sim() {
    s.soil_ok = true;
    s.moisture = 600;  // comfortable
    s.light_ok = true;
    s.lux = 3000;      // ordinary daylight: neither sunny nor cloudy
    engine.onSensors(now, s);
  }

  // Let time pass, taking a sensor reading every 10 s.
  void wait(int64_t ms) {
    int64_t end = now + ms;
    while (now < end) {
      now += 10 * kSec;
      engine.onSensors(now, s);
    }
  }

  // Pour water: moisture climbs to `to` over about a minute, while she watches.
  void water(float to) {
    float from = s.moisture;
    saw_celebrate = heard_fanfare = false;
    for (int i = 1; i <= 6; ++i) {
      s.moisture = from + (to - from) * i / 6.0f;
      now += 10 * kSec;
      engine.onSensors(now, s);
      auto out = engine.tick(now);
      saw_celebrate |= out.face == Face::Celebrate;
      heard_fanfare |= out.sound == Sound::Fanfare;
    }
  }

  mood::Output tick() { return engine.tick(now); }
  Face face() { return tick().face; }
};

void test_starts_content() {
  Sim sim;
  auto out = sim.tick();
  TEST_ASSERT_EQUAL(Face::Content, out.face);
  TEST_ASSERT_EQUAL(Led::Calm, out.led);
  TEST_ASSERT_EQUAL(Sound::None, out.sound);
}

void test_press_gives_love_immediately_then_returns() {
  Sim sim;
  sim.engine.onPress(sim.now);
  auto out = sim.tick();  // same instant: well under 1 s
  TEST_ASSERT_EQUAL(Face::Love, out.face);
  TEST_ASSERT_EQUAL(Led::Excited, out.led);
  TEST_ASSERT_EQUAL(Sound::Chirp, out.sound);
  TEST_ASSERT_EQUAL(Sound::None, sim.tick().sound);  // sound plays once
  sim.now += 4 * kSec;
  TEST_ASSERT_NOT_EQUAL(Face::Love, sim.face());
}

void test_press_while_thirsty_still_shows_love() {
  Sim sim;
  sim.s.moisture = 300;
  sim.wait(1 * kMin);
  TEST_ASSERT_EQUAL(Face::Thirsty, sim.face());
  sim.engine.onPress(sim.now);
  TEST_ASSERT_EQUAL(Face::Love, sim.face());  // the fix to the original ordering
  sim.now += 4 * kSec;
  TEST_ASSERT_EQUAL(Face::Thirsty, sim.face());  // still wants water, no guilt added
}

void test_thirsty_asks_with_led_and_never_escalates() {
  Sim sim;
  sim.s.moisture = 300;
  sim.wait(1 * kMin);
  auto out = sim.tick();
  TEST_ASSERT_EQUAL(Face::Thirsty, out.face);
  TEST_ASSERT_EQUAL(Led::Asking, out.led);
  sim.wait(3 * 24 * kHour);  // three days ignored
  out = sim.tick();
  TEST_ASSERT_EQUAL(Face::Thirsty, out.face);  // same gentle face
  TEST_ASSERT_EQUAL(Led::Asking, out.led);
}

void test_watering_celebrates_on_its_own() {
  Sim sim;
  sim.s.moisture = 300;
  sim.wait(1 * kMin);
  sim.water(700);
  TEST_ASSERT_TRUE(sim.saw_celebrate);
  TEST_ASSERT_TRUE(sim.heard_fanfare);
  TEST_ASSERT_EQUAL_UINT32(1, sim.engine.celebrations());
  sim.wait(1 * kMin);
  TEST_ASSERT_EQUAL(Face::Happy, sim.face());  // not thirsty, and she feels loved
}

void test_small_pour_still_celebrates_even_if_still_dry() {
  Sim sim;
  sim.s.moisture = 250;
  sim.wait(1 * kMin);
  sim.water(420);  // a little splash, still below the thirsty line
  TEST_ASSERT_TRUE(sim.saw_celebrate);  // she did the right thing
}

void test_slow_drift_is_not_watering() {
  Sim sim;
  sim.s.moisture = 500;
  sim.wait(1 * kMin);
  for (int i = 0; i < 200; ++i) {  // +200 over ~6 hours
    sim.s.moisture += 1;
    sim.wait(2 * kMin);
  }
  TEST_ASSERT_EQUAL_UINT32(0, sim.engine.celebrations());
}

void test_pouring_more_does_not_earn_more() {
  Sim sim;
  sim.s.moisture = 350;
  sim.wait(1 * kMin);
  sim.water(700);
  TEST_ASSERT_EQUAL_UINT32(1, sim.engine.celebrations());
  sim.wait(1 * kMin);
  sim.water(900);  // second pour, same watering
  TEST_ASSERT_EQUAL_UINT32(1, sim.engine.celebrations());
  sim.wait(1 * kHour);  // cooldown over, but the pot is already full
  sim.water(1000);
  TEST_ASSERT_EQUAL_UINT32(1, sim.engine.celebrations());
}

void test_next_day_watering_celebrates_again() {
  Sim sim;
  sim.s.moisture = 350;
  sim.wait(1 * kMin);
  sim.water(700);
  for (int h = 0; h < 24; ++h) {  // dries out over a day
    sim.s.moisture -= 15;
    sim.wait(1 * kHour);
  }
  sim.water(700);
  TEST_ASSERT_EQUAL_UINT32(2, sim.engine.celebrations());
}

void test_celebrate_beats_soggy_and_soggy_waits() {
  Sim sim;
  sim.s.moisture = 400;
  sim.wait(1 * kMin);
  sim.water(1300);  // a big overwatering
  TEST_ASSERT_TRUE(sim.saw_celebrate);  // still rewarded for watering
  sim.wait(1 * kHour);
  TEST_ASSERT_NOT_EQUAL(Face::Soggy, sim.face());  // not straight away
  sim.wait(3 * kHour);
  TEST_ASSERT_EQUAL(Face::Soggy, sim.face());  // stayed wet: now it shows
  sim.wait(7 * kHour);
  TEST_ASSERT_NOT_EQUAL(Face::Soggy, sim.face());  // and the frog forgives
}

void test_draining_clears_soggy() {
  Sim sim;
  sim.s.moisture = 1200;
  sim.wait(4 * kHour);
  TEST_ASSERT_EQUAL(Face::Soggy, sim.face());
  sim.s.moisture = 900;
  sim.wait(1 * kMin);
  TEST_ASSERT_NOT_EQUAL(Face::Soggy, sim.face());
}

void test_sleeps_after_sustained_dark_only() {
  Sim sim;
  sim.s.lux = 1;
  sim.wait(5 * kMin);
  TEST_ASSERT_FALSE(sim.engine.asleep());  // a shadow is not night
  sim.wait(6 * kMin);
  TEST_ASSERT_TRUE(sim.engine.asleep());
  auto out = sim.tick();
  TEST_ASSERT_EQUAL(Face::Sleeping, out.face);
  TEST_ASSERT_EQUAL(Led::Off, out.led);
}

void test_brief_lamp_does_not_wake() {
  Sim sim;
  sim.s.lux = 1;
  sim.wait(20 * kMin);
  sim.s.lux = 200;  // someone flicks a lamp on
  sim.wait(3 * kMin);
  sim.s.lux = 1;
  sim.wait(1 * kMin);
  TEST_ASSERT_TRUE(sim.engine.asleep());
  sim.s.lux = 500;  // morning
  sim.wait(11 * kMin);
  TEST_ASSERT_FALSE(sim.engine.asleep());
}

void test_no_asking_at_night() {
  Sim sim;
  sim.s.moisture = 300;
  sim.s.lux = 1;
  sim.wait(15 * kMin);
  auto out = sim.tick();
  TEST_ASSERT_EQUAL(Face::Sleeping, out.face);
  TEST_ASSERT_EQUAL(Led::Off, out.led);
}

void test_press_at_night_is_sleepy_and_silent() {
  Sim sim;
  sim.s.lux = 1;
  sim.wait(15 * kMin);
  sim.engine.onPress(sim.now);
  auto out = sim.tick();
  TEST_ASSERT_EQUAL(Face::SleepyLove, out.face);
  TEST_ASSERT_EQUAL(Sound::None, out.sound);
  sim.now += 3 * kSec;
  TEST_ASSERT_EQUAL(Face::Sleeping, sim.face());
}

void test_affection_makes_happy_then_fades() {
  Sim sim;
  sim.engine.onPress(sim.now);
  sim.engine.onPress(sim.now + 500);
  sim.now += 5 * kSec;
  TEST_ASSERT_EQUAL(Face::Happy, sim.face());
  sim.wait(6 * kHour);
  TEST_ASSERT_EQUAL(Face::Content, sim.face());  // fades on its own, nothing lost
}

void test_sunny_and_cloudy() {
  Sim sim;
  sim.s.lux = 20000;
  sim.wait(5 * kMin);
  TEST_ASSERT_EQUAL(Face::Sunny, sim.face());
  sim.s.lux = 800;
  sim.wait(5 * kMin);
  TEST_ASSERT_EQUAL(Face::Cloudy, sim.face());
}

void test_broken_soil_sensor_never_asks() {
  Sim sim;
  sim.s.soil_ok = false;
  sim.s.moisture = 0;
  sim.wait(1 * kHour);
  auto out = sim.tick();
  TEST_ASSERT_NOT_EQUAL(Face::Thirsty, out.face);
  TEST_ASSERT_NOT_EQUAL(Led::Asking, out.led);
}

void test_missing_light_sensor_stays_awake() {
  Sim sim;
  sim.s.light_ok = false;
  sim.s.lux = 0;
  sim.wait(1 * kHour);
  TEST_ASSERT_FALSE(sim.engine.asleep());
}

}  // namespace

void setUp() {}
void tearDown() {}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_starts_content);
  RUN_TEST(test_press_gives_love_immediately_then_returns);
  RUN_TEST(test_press_while_thirsty_still_shows_love);
  RUN_TEST(test_thirsty_asks_with_led_and_never_escalates);
  RUN_TEST(test_watering_celebrates_on_its_own);
  RUN_TEST(test_small_pour_still_celebrates_even_if_still_dry);
  RUN_TEST(test_slow_drift_is_not_watering);
  RUN_TEST(test_pouring_more_does_not_earn_more);
  RUN_TEST(test_next_day_watering_celebrates_again);
  RUN_TEST(test_celebrate_beats_soggy_and_soggy_waits);
  RUN_TEST(test_draining_clears_soggy);
  RUN_TEST(test_sleeps_after_sustained_dark_only);
  RUN_TEST(test_brief_lamp_does_not_wake);
  RUN_TEST(test_no_asking_at_night);
  RUN_TEST(test_press_at_night_is_sleepy_and_silent);
  RUN_TEST(test_affection_makes_happy_then_fades);
  RUN_TEST(test_sunny_and_cloudy);
  RUN_TEST(test_broken_soil_sensor_never_asks);
  RUN_TEST(test_missing_light_sensor_stays_awake);
  return UNITY_END();
}
