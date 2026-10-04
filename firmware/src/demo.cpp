// env:demo  shows every face in turn, each with its own animation, for showing the frog
//           off before the sensors and button are wired. The BOOT button on the back
//           (GPIO0, safe to press once it's running) jumps to the love face.
#ifdef GG_DEMO
#include <Arduino.h>

#include "anim.h"
#include "display.h"

namespace {
struct Step {
  sprites::State state;
  uint32_t ms;
};
const Step kTour[] = {
    {sprites::CONTENT, 7000},   {sprites::LOVE, 4000},      {sprites::HAPPY, 4000},
    {sprites::THIRSTY, 5000},   {sprites::CELEBRATE, 5000}, {sprites::SUNNY, 4000},
    {sprites::CLOUDY, 4000},    {sprites::SOGGY, 4000},     {sprites::SLEEPING, 5000},
    {sprites::SLEEPY_LOVE, 3000}, {sprites::CARD_SPROUT, 3000}, {sprites::CARD_FLOWER, 4000},
};
constexpr int kSteps = sizeof(kTour) / sizeof(kTour[0]);
constexpr int kBootPin = 0;

anim::Player player;
int step = 0;
uint32_t step_started = 0;
uint32_t love_until = 0;
int shown = -1;
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\ngardagotchi demo");
  pinMode(kBootPin, INPUT_PULLUP);
  display::begin();
}

void loop() {
  uint32_t now = millis();
  if (digitalRead(kBootPin) == LOW && (int32_t)(love_until - now) <= 0) love_until = now + 3000;
  if (now - step_started >= kTour[step].ms) {
    step = (step + 1) % kSteps;
    step_started = now;
  }
  sprites::State s = (int32_t)(love_until - now) > 0 ? sprites::LOVE : kTour[step].state;
  player.play(&sprites::kClips[s], now);
  int f = player.frame(now);
  if (f != shown) {
    shown = f;
    display::drawFrame(sprites::GREEN, f);
    display::present();
  }
  delay(5);
}
#endif
