// Every GPIO the firmware touches, in one place. See docs/wiring.md.
#pragma once
#include <stdint.h>

// ---- Onboard, fixed by the Waveshare board (from the Waveshare wiki) ----
constexpr int PIN_LCD_DC   = 8;
constexpr int PIN_LCD_CS   = 9;
constexpr int PIN_LCD_SCLK = 10;
constexpr int PIN_LCD_MOSI = 11;
constexpr int PIN_LCD_RST  = 12;
constexpr int PIN_LCD_BL   = 40;
constexpr int PIN_I2C_SDA  = 6;   // shared: onboard IMU + soil sensor + light sensor
constexpr int PIN_I2C_SCL  = 7;

// ---- Ours, chosen from free non-strapping pins. Confirm against the board silkscreen. ----
constexpr int PIN_BUTTON   = 15;  // arcade button switch -> GND (internal pull-up)
constexpr int PIN_LED      = 16;  // -> 1k -> PN2222 base (LED runs from 5V)
constexpr int PIN_I2S_BCLK = 17;  // MAX98357A BCLK  (wired now, used later)
constexpr int PIN_I2S_LRC  = 18;  // MAX98357A LRC
constexpr int PIN_I2S_DIN  = 21;  // MAX98357A DIN

// ---- I2C addresses ----
constexpr uint8_t ADDR_SOIL  = 0x36;  // Adafruit STEMMA soil sensor (seesaw)
constexpr uint8_t ADDR_LIGHT = 0x23;  // BH1750 with ADDR pin low/floating
// Onboard QMI8658 IMU sits at 0x6B (or 0x6A), so no clash.
