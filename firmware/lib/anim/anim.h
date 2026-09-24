// Plays a face's animation: loops its frames and drops in a blink now and then.
// Plain C++ (no Arduino), tested with `pio test -e native`.
#pragma once
#include <stdint.h>

namespace anim {

// One face's animation. Frames are numbered first .. first+count-1 in the sprite table.
struct Clip {
  uint16_t first;
  uint8_t count;
  int16_t blink;       // frame to show during a blink, or -1 for "this face doesn't blink"
  const uint16_t* ms;  // how long each loop frame shows
};

class Player {
 public:
  explicit Player(uint32_t seed = 0x6A09E667u) : rng_(seed ? seed : 1) {}

  // Switch to a clip. Starts it from frame 0 only if it's a different clip, so calling
  // this every loop with the same face doesn't restart it.
  void play(const Clip* clip, int64_t now_ms);

  // The frame number to draw right now.
  uint16_t frame(int64_t now_ms);

  static constexpr int64_t kBlinkMs = 150;
  static constexpr int64_t kBlinkGapMinMs = 2500;
  static constexpr int64_t kBlinkGapMaxMs = 6000;

 private:
  int64_t nextGap();

  const Clip* clip_ = nullptr;
  int64_t start_ms_ = 0;
  int64_t next_blink_ms_ = 0;
  int64_t blink_until_ms_ = -1;
  uint32_t rng_;
};

}  // namespace anim
