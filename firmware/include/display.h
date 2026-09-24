// The round screen. Draws a 60x60 frog face scaled 4x to fill 240x240.
#pragma once
#include <LovyanGFX.hpp>
#include "sprites.h"

namespace display {
void begin();
// Draw a face into the off-screen buffer (call present() to show it).
void drawFace(sprites::Palette p, sprites::State s);
// Small debug text at the bottom of the circle. Grown-up modes only.
void drawFooter(const char* text);
// Send the buffer to the screen in one go (no flicker).
void present();
}  // namespace display
