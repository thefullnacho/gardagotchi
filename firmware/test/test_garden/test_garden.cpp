// Desktop tests for growth and the collection:  cd firmware && pio test -e native
#include <unity.h>

#include "garden.h"

using garden::Reveal;
using garden::Stage;

namespace {

constexpr int64_t kMin = 60 * 1000;
constexpr int64_t kHour = 60 * kMin;

// Small stage numbers so tests stay short.
garden::Config quickConfig() {
  garden::Config c;
  c.sprout_day = 2;
  c.leaves_day = 4;
  c.bud_day = 6;
  c.flower_day = 8;
  return c;
}

struct Sim {
  garden::Garden g;
  int64_t now = 0;
  int saves = 0;

  explicit Sim(const garden::Config& c = quickConfig(), const garden::Saved& s = garden::Saved())
      : g(c, s) {
    g.update(now, false, false, true);
  }

  void run(int64_t ms, bool asleep, bool thirsty, bool light_ok = true) {
    int64_t end = now + ms;
    while (now < end) {
      now += kMin;
      if (g.update(now, asleep, thirsty, light_ok)) ++saves;
    }
  }

  // One day and night: 14 h awake (thirsty for `thirsty_h` of them), 10 h asleep,
  // then the frog wakes at dawn (which is when the day counts).
  void day(int thirsty_h = 0) {
    run(thirsty_h * kHour, false, true);
    run((14 - thirsty_h) * kHour, false, false);
    run(10 * kHour, true, false);
    run(kMin, false, false);
  }
};

void test_a_day_is_one_sleep() {
  Sim sim;
  sim.run(14 * kHour, false, false);
  TEST_ASSERT_EQUAL_UINT16(0, sim.g.saved().growth_days);  // awake all day: no new day yet
  sim.run(10 * kHour, true, false);
  sim.run(1 * kMin, false, false);  // the frog wakes: dawn
  TEST_ASSERT_EQUAL_UINT16(1, sim.g.saved().growth_days);
}

void test_grows_through_every_stage() {
  Sim sim;
  Stage seen[5] = {};
  for (int d = 0; d < 9; ++d) {
    sim.day();
    seen[(int)sim.g.stage()] = sim.g.stage();
  }
  TEST_ASSERT_EQUAL(Stage::Flower, sim.g.stage());
  TEST_ASSERT_EQUAL(Stage::Sprout, seen[1]);
  TEST_ASSERT_EQUAL(Stage::Leaves, seen[2]);
  TEST_ASSERT_EQUAL(Stage::Bud, seen[3]);
}

void test_first_dawn_after_a_power_cut_counts() {
  Sim sim;  // "plugged in" in the evening
  sim.run(2 * kHour, false, false);
  sim.run(10 * kHour, true, false);
  sim.run(1 * kMin, false, false);
  TEST_ASSERT_EQUAL_UINT16(1, sim.g.saved().growth_days);
}

void test_lamp_at_night_does_not_add_days() {
  Sim sim;
  sim.day();
  TEST_ASSERT_EQUAL_UINT16(1, sim.g.saved().growth_days);
  sim.run(14 * kHour, false, false);
  sim.run(2 * kHour, true, false);
  sim.run(30 * kMin, false, false);  // lamp on at 11 PM wakes the frog
  sim.run(8 * kHour, true, false);
  sim.run(1 * kMin, false, false);   // real dawn
  TEST_ASSERT_EQUAL_UINT16(2, sim.g.saved().growth_days);  // one day, not two
}

void test_thirsty_all_day_waits_but_loses_nothing() {
  Sim sim;
  sim.day();
  sim.day();
  TEST_ASSERT_EQUAL_UINT16(2, sim.g.saved().growth_days);
  sim.day(14);  // thirsty the whole day
  TEST_ASSERT_EQUAL_UINT16(2, sim.g.saved().growth_days);  // waited, nothing went back
  sim.day();
  TEST_ASSERT_EQUAL_UINT16(3, sim.g.saved().growth_days);
}

void test_watered_by_afternoon_still_grows() {
  Sim sim;
  sim.day(10);  // thirsty all morning, watered in the afternoon
  TEST_ASSERT_EQUAL_UINT16(1, sim.g.saved().growth_days);
}

void test_care_rule_can_be_turned_off() {
  garden::Config c = quickConfig();
  c.growth_needs_care = false;
  Sim sim(c);
  sim.day(14);
  TEST_ASSERT_EQUAL_UINT16(1, sim.g.saved().growth_days);
}

void test_new_stage_leaves_a_surprise_for_her_press() {
  Sim sim;
  sim.day();
  TEST_ASSERT_FALSE(sim.g.revealWaiting());  // day 1: still a seed
  sim.day();
  TEST_ASSERT_TRUE(sim.g.revealWaiting());   // day 2: sprouted overnight
  TEST_ASSERT_EQUAL(Reveal::Grew, sim.g.takeReveal());
  TEST_ASSERT_FALSE(sim.g.revealWaiting());
  TEST_ASSERT_EQUAL(Reveal::None, sim.g.takeReveal());
}

void test_surprise_waits_however_long_she_takes() {
  Sim sim;
  for (int d = 0; d < 5; ++d) sim.day();  // grew twice, she never pressed
  TEST_ASSERT_EQUAL(Reveal::Grew, sim.g.takeReveal());  // still there, shown once
}

void test_blooming_earns_a_keepsake_once() {
  Sim sim;
  for (int d = 0; d < 20; ++d) sim.day();
  TEST_ASSERT_EQUAL(Stage::Flower, sim.g.stage());
  TEST_ASSERT_EQUAL_UINT16(1, sim.g.saved().keepsakes);  // blooming more days adds nothing
  TEST_ASSERT_EQUAL(Reveal::Bloomed, sim.g.takeReveal());
}

void test_bloom_surprise_is_not_hidden_by_a_later_one() {
  garden::Config c = quickConfig();
  Sim sim(c);
  for (int d = 0; d < 8; ++d) sim.day();  // blooms on day 8
  TEST_ASSERT_EQUAL(Reveal::Bloomed, sim.g.takeReveal());
}

void test_replant_keeps_the_collection() {
  Sim sim;
  for (int d = 0; d < 9; ++d) sim.day();
  sim.g.takeReveal();
  sim.g.replant();
  TEST_ASSERT_EQUAL(Stage::Seed, sim.g.stage());
  TEST_ASSERT_EQUAL_UINT16(2, sim.g.saved().plant_number);
  TEST_ASSERT_EQUAL_UINT16(1, sim.g.saved().keepsakes);  // earned, never lost
  TEST_ASSERT_EQUAL(Reveal::Grew, sim.g.takeReveal());   // she gets to see the new seed
  for (int d = 0; d < 9; ++d) sim.day();
  TEST_ASSERT_EQUAL_UINT16(2, sim.g.saved().keepsakes);  // and it compounds
}

void test_state_survives_a_power_cut() {
  Sim sim;
  for (int d = 0; d < 3; ++d) sim.day();
  garden::Saved kept = sim.g.saved();  // what the firmware wrote to flash
  Sim after(quickConfig(), kept);      // power comes back
  TEST_ASSERT_EQUAL_UINT16(3, after.g.saved().growth_days);
  TEST_ASSERT_EQUAL(Stage::Sprout, after.g.stage());
}

void test_first_ever_shows_her_seed() {
  garden::Saved s = garden::Garden::firstEver();
  Sim sim(quickConfig(), s);
  TEST_ASSERT_EQUAL(Stage::Seed, sim.g.stage());
  TEST_ASSERT_EQUAL(Reveal::Grew, sim.g.takeReveal());
}

void test_saves_only_when_something_changes() {
  Sim sim;
  for (int d = 0; d < 3; ++d) sim.day();
  TEST_ASSERT_TRUE(sim.saves >= 3 && sim.saves <= 4);  // about once a day, not every minute
}

void test_no_light_sensor_still_grows() {
  Sim sim;
  sim.run(3 * 24 * kHour, false, false, false);
  TEST_ASSERT_TRUE(sim.g.saved().growth_days >= 2);
}

}  // namespace

void setUp() {}
void tearDown() {}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_a_day_is_one_sleep);
  RUN_TEST(test_grows_through_every_stage);
  RUN_TEST(test_first_dawn_after_a_power_cut_counts);
  RUN_TEST(test_lamp_at_night_does_not_add_days);
  RUN_TEST(test_thirsty_all_day_waits_but_loses_nothing);
  RUN_TEST(test_watered_by_afternoon_still_grows);
  RUN_TEST(test_care_rule_can_be_turned_off);
  RUN_TEST(test_new_stage_leaves_a_surprise_for_her_press);
  RUN_TEST(test_surprise_waits_however_long_she_takes);
  RUN_TEST(test_blooming_earns_a_keepsake_once);
  RUN_TEST(test_bloom_surprise_is_not_hidden_by_a_later_one);
  RUN_TEST(test_replant_keeps_the_collection);
  RUN_TEST(test_state_survives_a_power_cut);
  RUN_TEST(test_first_ever_shows_her_seed);
  RUN_TEST(test_saves_only_when_something_changes);
  RUN_TEST(test_no_light_sensor_still_grows);
  return UNITY_END();
}
