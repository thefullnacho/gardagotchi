// env:artcheck  loops one hand-drawn sprite strip on the real screen, so new art can be
//               judged at real size and color before it joins the frog. The strip comes
//               from faces/art_check.py, which writes include/art_check.h.
#ifdef GG_ARTCHECK
#include <Arduino.h>

#include "art_check.h"
#include "display.h"

namespace {
int step = 0;
uint32_t step_started = 0;

void show(int s) {
  display::drawIndexed(art_check::kCellPx[art_check::kSeq[s][0]], art_check::kColors, art_check::kW);
  display::present();
}
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.printf("\nart check: %d cells, %d steps\n", art_check::kCells, art_check::kSteps);
  display::begin();
  show(0);
  step_started = millis();
}

void loop() {
  if (millis() - step_started >= art_check::kSeq[step][1]) {
    step = (step + 1) % art_check::kSteps;
    step_started = millis();
    show(step);
  }
  delay(5);
}
#endif
