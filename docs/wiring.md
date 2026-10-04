# Wiring: GPIO map and solder checklist

Board: Waveshare ESP32-S3-LCD-1.28 (non-touch). Headers are 1.27 mm pitch; wires
are soldered straight to the pads. Pin numbers live in `firmware/include/pins.h`.
If a wire moves, change that one file.

## Before you solder: two checks (5 minutes, a multimeter)

1. **Confirm the pin labels.** The Waveshare wiki lists what the board uses on its own,
   but it doesn't publish a full list of the header pins. Read the silkscreen and
   confirm **GPIO15, 16, 17, 18, 21** and **GPIO13, 14, 4, 5, 2** and **VSYS, 3V3, GND**
   are all broken out. If one is missing, any pin from the spare list below works.
   Tell me which one and I'll change `pins.h`.
2. **Is VSYS a usable 5 V output on battery?** Plug the board into USB and measure VSYS
   to GND: about 4.6 to 5.1 V means the LED and speaker amp can run from it on USB.
   Then unplug USB, plug in the LiPo, and measure again. If VSYS is dead on battery,
   the button LED needs a new home (3V3, dimmer — or a boost). Stop and we'll decide.

## GPIO map

| GPIO | Use | Notes |
|---|---|---|
| 6 | I2C SDA | Onboard, shared with the IMU. BH1750 connects here too |
| 7 | I2C SCL | Onboard, shared |
| 8, 9, 10, 11, 12 | Display SPI (DC, CS, CLK, MOSI, RST) | Onboard, don't touch |
| 40 | Display backlight | Onboard |
| 1 | Battery ADC | Onboard: battery voltage via divider, read it in firmware |
| 43, 44 | UART to the USB chip | Onboard. This is the serial monitor |
| 47, 48 | IMU interrupts | Onboard |
| **15** | **Button** | Switch between GPIO15 and GND. No resistor needed (internal pull-up) |
| **16** | **Button LED** | Through a 1 kΩ resistor to the PN2222 base. See power note below |
| **17** | **I2S BCLK** | MAX98357A BCLK |
| **18** | **I2S LRC** | MAX98357A LRC (word select) |
| **21** | **I2S DIN** | MAX98357A DIN |
| **13** | **CC1101 SCK** | 915 MHz radio SPI |
| **14** | **CC1101 MISO** | |
| **4** | **CC1101 MOSI** | |
| **5** | **CC1101 CS** | |
| **2** | **CC1101 GDO0** | Packet-RX interrupt |

**Avoid:** 0, 3, 45, 46 (strapping pins: they decide how the chip boots), 19/20 (USB),
26 to 37 (flash and PSRAM).
**Spares if needed:** 38, 39, 41, 42.

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
**Battery caveat:** this assumes check 2 passes on battery power. If VSYS is dead
without USB, the LED moves (see check 2).

### 3. Light sensor on I2C (test: calibrate build prints `light found`)
**Power the BH1750 from 3V3, not 5 V.** Its board has pull-up resistors tied
to its supply. At 5 V it would pull the ESP32's I2C pins to 5 V, and those pins
only take 3.3 V.
- [ ] BH1750: VCC → **3V3**, GND → **GND**, SDA → **GPIO6**, SCL → **GPIO7**,
  ADDR → leave unconnected (address 0x23)
- [ ] The code runs I2C at the slow 100 kHz to be forgiving

### 4. CC1101 915 MHz radio (test: serial prints `radio: WH51 found` + packets)
The module runs at 1.8–3.6 V: **power it from 3V3, no level shifting.** All logic
is 3.3 V both ways.
- [ ] CC1101 **VCC** → **3V3**, **GND** → **GND**
- [ ] **SCK** → **GPIO13**, **MISO** → **GPIO14**, **MOSI** → **GPIO4**
- [ ] **CS** → **GPIO5**, **GDO0** → **GPIO2**
- [ ] Keep the spring antenna clear of the case walls and away from the LiPo.
  A 915 MHz whip is a later upgrade if range disappoints; the stock spring hears
  the garden fine.

### 5. Speaker amp (wire now; firmware for sound comes later)
- [ ] MAX98357A **VIN** → **VSYS (5 V)**, **GND** → **GND**
- [ ] **BCLK** → **GPIO17**, **LRC** → **GPIO18**, **DIN** → **GPIO21**
- [ ] **GAIN**: tie to **VIN** for the quietest fixed gain (6 dB). This is a kid's toy near her ears; volume can go up in software, not down in hardware
- [ ] **SD**: leave unconnected (mono, amp on)
- [ ] Speaker + / − to the amp's output terminals (4 Ω)

### Power
- 3.7V LiPo into the **MX1.25** header. The onboard charger tops it up over USB-C.
  **Check polarity with the multimeter before the first plug-in** — red must land on +.
- Bench power / charging: USB-C, **at least 1 A**. The speaker amp pulls current in
  bursts at 5 V; a weak phone charger can brown out the board mid-chirp (it reboots).
- Battery voltage is readable in firmware on GPIO1 (onboard divider, see wiki).

## Things I'm not sure of (flagged, not guessed)
- **Full header pin list:** see check 1 above.
- **VSYS on battery power:** see check 2 above. The button LED and speaker amp both
  want 5 V; if VSYS is USB-only, we need a plan B for battery operation.
- **Display color settings:** `invert = true`, `rgb_order = false` is the usual GC9A01
  setup. If the frog looks like a photo negative or red and blue are swapped, it's a
  one-word change in `firmware/src/display.cpp`.
- **Serial port:** the wiki says USB goes through a CH343P chip. If the serial monitor
  stays silent, we may need a different USB setting; tell me what you see.
- **WH51 sensor ID:** the frog must filter by her pot's sensor ID or it'll hear the
  beds too. The ID prints in the `radio` line on first reception; hardcode it after
  the first run.
