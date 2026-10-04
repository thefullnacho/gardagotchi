// The round screen. Draws an 80x80 frog frame scaled 3x to fill 240x240.
#pragma once
#include <LovyanGFX.hpp>
#include "sprites.h"

namespace display {
void begin();
// Draw one animation frame into the off-screen buffer (call present() to show it).
void drawFrame(sprites::Palette p, uint16_t index);
// Draw any square n x n color-indexed image, scaled to fill the screen (240 / n).
void drawIndexed(const uint8_t* px, const uint16_t* colors, int n);
// Draw the flower bed (her collection) on top of the current frame: n flowers.
void drawFlowerBed(int n, bool night);
// Small debug text at the bottom of the circle. Grown-up modes only.
void drawFooter(const char* text);
// Send the buffer to the screen in one go (no flicker).
void present();
}  // namespace display
