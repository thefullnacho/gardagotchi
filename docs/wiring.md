# Wiring: GPIO map and solder checklist

Board: Waveshare ESP32-S3-LCD-1.28 (non-touch). Headers are 1.27 mm pitch; wires
are soldered straight to the pads. Pin numbers live in `firmware/include/pins.h`.
If a wire moves, change that one file.

## Before you solder: two checks (5 minutes, a multimeter)

1. **Confirm the pin labels.** The Waveshare wiki lists what the board uses on its own,
   but it doesn't publish a full list of the header pins. Read the silkscreen and
   confirm **GPIO15, 16, 17, 18, 21** and **VSYS, 3V3, GND** are all broken out. If one
   is missing, any pin from the spare list below works. Tell me which one and I'll
   change `pins.h`.
2. **Is VSYS a usable 5 V output?** Plug the board into USB and measure VSYS to GND.
   About 4.6 to 5.1 V means the LED and speaker amp can run from it. Close to 0 V,
   or around 3.7 to 4.2 V, means VSYS is not USB power. In that case stop and we'll
   take 5 V from the USB side another way.

## GPIO map

| GPIO | Use | Notes |
|---|---|---|
| 6 | I2C SDA | Onboard, shared with the IMU. Soil + light sensors connect here too |
| 7 | I2C SCL | Onboard, shared |
| 8, 9, 10, 11, 12 | Display SPI (DC, CS, CLK, MOSI, RST) | Onboard, don't touch |
| 40 | Display backlight | Onboard |
| 1 | Battery ADC | Onboard, unused (no battery) |
| 43, 44 | UART to the USB chip | Onboard. This is the serial monitor |
| 47, 48 | IMU interrupts | Onboard |
| **15** | **Button** | Switch between GPIO15 and GND. No resistor needed (internal pull-up) |
| **16** | **Button LED** | Through a 1 kΩ resistor to the PN2222 base |
| **17** | **I2S BCLK** | MAX98357A BCLK |
| **18** | **I2S LRC** | MAX98357A LRC (word select) |
| **21** | **I2S DIN** | MAX98357A DIN |

**Avoid:** 0, 3, 45, 46 (strapping pins: they decide how the chip boots), 19/20 (USB),
26 to 37 (flash and PSRAM).
**Spares if needed:** 2, 4, 5, 13, 14, 38, 39, 41, 42.

## Solder checklist

Work one group at a time, then flash `env:frog` or `env:calibrate` to test before the
next group.

### 1. Button (test: press → love face; serial prints `button: love`)
- [ ] Button switch terminal A → **GPIO15**
- [ ] Button switch terminal B → **GND**

The #3488 has four tabs: two for the switch (either way round) and two for the LED
(marked + / −). Check which pair is which with the multimeter's continuity beep
while pressing.

### 2. Button LED via PN2222 (test: LED breathes slowly)
Hold the PN2222 flat side toward you, legs down: left to right is **E, B, C**.
Check that against your part's datasheet, because some makers swap the order.
- [ ] **GPIO16** → 1 kΩ resistor → PN2222 **base** (middle)
- [ ] PN2222 **emitter** → **GND**
- [ ] PN2222 **collector** → button **LED −**
- [ ] Button **LED +** → **VSYS (5 V)**
- [ ] Optional: 10 kΩ from base to GND, so the LED stays fully off while the board boots

The LED has its own resistor inside, so nothing else is needed on the 5 V side.

### 3. Sensors on I2C (test: calibrate build prints `soil found, light found`)
**Power both sensors from 3V3, not 5 V.** Their boards have pull-up resistors tied
to their supply. At 5 V they would pull the ESP32's I2C pins to 5 V, and those pins
only take 3.3 V.
- [ ] Soil sensor (STEMMA JST-PH cable): red → **3V3**, black → **GND**, blue (SDA) → **GPIO6**, yellow (SCL) → **GPIO7**
- [ ] BH1750: VCC → **3V3**, GND → **GND**, SDA → **GPIO6**, SCL → **GPIO7**, ADDR → leave unconnected (address 0x23)
- [ ] Keep the soil sensor cable under ~50 cm. The code runs I2C at the slow 100 kHz to be forgiving

The Adafruit JST-PH cable colors are the usual ones, but check them against the labels
printed on the sensor board.

### 4. Speaker amp (wire now; firmware for sound comes later)
- [ ] MAX98357A **VIN** → **VSYS (5 V)**, **GND** → **GND**
- [ ] **BCLK** → **GPIO17**, **LRC** → **GPIO18**, **DIN** → **GPIO21**
- [ ] **GAIN**: tie to **VIN** for the quietest fixed gain (6 dB). This is a kid's toy near her ears; volume can go up in software, not down in hardware
- [ ] **SD**: leave unconnected (mono, amp on)
- [ ] Speaker + / − to the amp's output terminals (4 Ω)

### Power
- USB-C wall adapter: **at least 1 A**. The speaker amp pulls current in bursts at
  5 V; a weak phone charger can brown out the board mid-chirp (it reboots).
- No battery on the MX1.25 connector. Leave it empty.

## Things I'm not sure of (flagged, not guessed)
- **Full header pin list:** see check 1 above.
- **VSYS as a 5 V output:** see check 2 above.
- **Display color settings:** `invert = true`, `rgb_order = false` is the usual GC9A01
  setup. If the frog looks like a photo negative or red and blue are swapped, it's a
  one-word change in `firmware/src/display.cpp`.
- **Serial port:** the wiki says USB goes through a CH343P chip. If the serial monitor
  stays silent, we may need a different USB setting; tell me what you see.
