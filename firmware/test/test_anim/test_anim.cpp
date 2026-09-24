// Desktop tests for the animation player:  cd firmware && pio test -e native
#include <unity.h>

#include "anim.h"

namespace {

const uint16_t kBreatheMs[] = {900, 900};
const anim::Clip kBreathe = {10, 2, 12, kBreatheMs};  // frames 10, 11; blink is frame 12

const uint16_t kWiggleMs[] = {200, 200, 200, 200};
const anim::Clip kWiggle = {20, 4, -1, kWiggleMs};  // no blink

void test_loops_through_frames_on_time() {
  anim::Player p;
  p.play(&kWiggle, 0);
  TEST_ASSERT_EQUAL_UINT16(20, p.frame(0));
  TEST_ASSERT_EQUAL_UINT16(20, p.frame(199));
  TEST_ASSERT_EQUAL_UINT16(21, p.frame(200));
  TEST_ASSERT_EQUAL_UINT16(23, p.frame(799));
  TEST_ASSERT_EQUAL_UINT16(20, p.frame(800));  // loops back
}

void test_same_clip_does_not_restart() {
  anim::Player p;
  p.play(&kWiggle, 0);
  p.play(&kWiggle, 500);  // called every loop with the same face
  TEST_ASSERT_EQUAL_UINT16(22, p.frame(500));
}

void test_new_clip_starts_at_its_first_frame() {
  anim::Player p;
  p.play(&kWiggle, 0);
  p.play(&kBreathe, 1234);
  TEST_ASSERT_EQUAL_UINT16(10, p.frame(1234));  // immediately, no leftover frame
}

void test_blinks_now_and_then_briefly() {
  anim::Player p;
  p.play(&kBreathe, 0);
  int blinks = 0;
  int64_t blink_start = -1, longest = 0;
  bool in_blink = false;
  for (int64_t t = 0; t < 60000; t += 10) {  // one minute, checked every 10 ms
    bool b = p.frame(t) == 12;
    if (b && !in_blink) { ++blinks; blink_start = t; }
    if (!b && in_blink && t - blink_start > longest) longest = t - blink_start;
    in_blink = b;
  }
  // Gaps of 2.5-6 s means roughly 10-24 blinks a minute.
  TEST_ASSERT_TRUE(blinks >= 9 && blinks <= 25);
  TEST_ASSERT_TRUE(longest <= anim::Player::kBlinkMs + 10);
}

void test_blinks_are_not_evenly_spaced() {
  anim::Player p;
  p.play(&kBreathe, 0);
  int64_t last = -1, first_gap = -1;
  bool varied = false, in_blink = false;
  for (int64_t t = 0; t < 60000; t += 10) {
    bool b = p.frame(t) == 12;
    if (b && !in_blink) {
      if (last >= 0) {
        int64_t gap = t - last;
        if (first_gap < 0) first_gap = gap;
        else if (gap != first_gap) varied = true;
      }
      last = t;
    }
    in_blink = b;
  }
  TEST_ASSERT_TRUE(varied);
}

void test_face_without_blink_never_blinks() {
  anim::Player p;
  p.play(&kWiggle, 0);
  for (int64_t t = 0; t < 60000; t += 10) {
    uint16_t f = p.frame(t);
    TEST_ASSERT_TRUE(f >= 20 && f <= 23);
  }
}

}  // namespace

void setUp() {}
void tearDown() {}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_loops_through_frames_on_time);
  RUN_TEST(test_same_clip_does_not_restart);
  RUN_TEST(test_new_clip_starts_at_its_first_frame);
  RUN_TEST(test_blinks_now_and_then_briefly);
  RUN_TEST(test_blinks_are_not_evenly_spaced);
  RUN_TEST(test_face_without_blink_never_blinks);
  return UNITY_END();
}
