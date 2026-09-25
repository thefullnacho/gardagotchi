// The one button and its LED.
#pragma once
#include <stdint.h>

namespace controls {
void begin();
// Call every loop. Returns true exactly once per press (debounced).
bool pressed();
// LED brightness 0.0-1.0 (gamma-corrected so it looks even to the eye).
void setLed(float level);
// Slow "breathing" pulse; call every loop. period_ms = one full breath.
void breatheLed(uint32_t period_ms);
// Two quick twinkles every two seconds: "something new is waiting". Call every loop.
void twinkleLed();
// True while the button is held down right now (debounce not needed for long holds).
bool held();
}  // namespace controls
