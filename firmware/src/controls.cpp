#include "controls.h"

#include <Arduino.h>

#include "pins.h"

namespace {
constexpr uint32_t kDebounceMs = 25;
constexpr uint32_t kPwmHz = 5000;
constexpr uint8_t kPwmBits = 10;
constexpr uint8_t kPwmChannel = 0;  // display backlight uses channel 7

bool stable_down = false;    // debounced state
bool last_raw = false;
uint32_t last_change_ms = 0;
}  // namespace

namespace controls {

void begin() {
  pinMode(PIN_BUTTON, INPUT_PULLUP);  // switch pulls the pin to GND when pressed
  ledcAttachChannel(PIN_LED, kPwmHz, kPwmBits, kPwmChannel);
  setLed(0);
}

bool pressed() {
  bool raw = digitalRead(PIN_BUTTON) == LOW;
  uint32_t now = millis();
  if (raw != last_raw) {
    last_raw = raw;
    last_change_ms = now;
  }
  if (now - last_change_ms >= kDebounceMs && raw != stable_down) {
    stable_down = raw;
    return stable_down;  // report the moment it goes down, not on release
  }
  return false;
}

void setLed(float level) {
  level = constrain(level, 0.0f, 1.0f);
  uint32_t max_duty = (1u << kPwmBits) - 1;
  ledcWrite(PIN_LED, (uint32_t)(level * level * max_duty));  // squared = rough gamma
}

void breatheLed(uint32_t period_ms) {
  float phase = (millis() % period_ms) / (float)period_ms;
  setLed(0.08f + 0.92f * (0.5f - 0.5f * cosf(phase * 2 * PI)));
}

void twinkleLed() {
  uint32_t t = millis() % 2000;
  bool on = t < 120 || (t >= 240 && t < 360);
  setLed(on ? 1.0f : 0.1f);
}

bool held() { return digitalRead(PIN_BUTTON) == LOW; }

}  // namespace controls
