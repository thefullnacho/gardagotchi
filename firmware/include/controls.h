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
}  // namespace controls
