#include "anim.h"

namespace anim {

void Player::play(const Clip* clip, int64_t now_ms) {
  if (clip == clip_) return;
  clip_ = clip;
  start_ms_ = now_ms;
  blink_until_ms_ = -1;
  next_blink_ms_ = now_ms + nextGap();
}

uint16_t Player::frame(int64_t now_ms) {
  if (!clip_ || clip_->count == 0) return 0;

  if (clip_->blink >= 0) {
    if (now_ms >= next_blink_ms_) {
      blink_until_ms_ = now_ms + kBlinkMs;
      next_blink_ms_ = blink_until_ms_ + nextGap();
    }
    if (now_ms < blink_until_ms_) return (uint16_t)clip_->blink;
  }

  int64_t total = 0;
  for (int i = 0; i < clip_->count; ++i) total += clip_->ms[i];
  if (total <= 0) return clip_->first;
  int64_t t = (now_ms - start_ms_) % total;
  for (int i = 0; i < clip_->count; ++i) {
    if (t < clip_->ms[i]) return (uint16_t)(clip_->first + i);
    t -= clip_->ms[i];
  }
  return clip_->first;
}

int64_t Player::nextGap() {
  // xorshift32: cheap, and random enough that blinks don't feel mechanical.
  rng_ ^= rng_ << 13;
  rng_ ^= rng_ >> 17;
  rng_ ^= rng_ << 5;
  return kBlinkGapMinMs + (int64_t)(rng_ % (uint32_t)(kBlinkGapMaxMs - kBlinkGapMinMs + 1));
}

}  // namespace anim
