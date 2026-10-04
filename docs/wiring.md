# Wiring: GPIO map and solder checklist

Board: Waveshare ESP32-S3-LCD-1.28 (non-touch). It ships with two 2x10 header blocks
on the back, H1 and H2, at 1.27 mm pitch (half the usual breadboard spacing). The
blocks are **sockets**, not pins: wires go onto 1.27 mm male pins plugged into them,
and nothing gets soldered to the board itself. Pin numbers live in
`firmware/include/pins.h`. If a wire moves, change that one file.

## The two headers

Hold the board display down, USB-C at the top. H1 is the block on the left, H2 on the
right. Rows count down from the USB-C end. This is Waveshare's pinout diagram,
cross-checked against their schematic (`Esp32-s3-lcd-.128-sch.pdf` on the wiki).

**H1 (left): the button, LED, speaker amp, and one radio pin.**

| Row | Left column | Right column |
|---|---|---|
| 1 | GPIO36 | GPIO46 |
| 2 | GPIO35 | GPIO45 |
| 3 | GPIO34 | GPIO42 |
| 4 | GPIO33 | GPIO41 |
| 5 | **GPIO21** (I2S DIN) | GPIO40 (backlight, don't touch) |
| 6 | **GPIO18** (I2S LRC) | GPIO39 |
| 7 | **GPIO17** (I2S BCLK) | GPIO38 |
| 8 | **GPIO16** (LED) | GPIO37 |
| 9 | **GPIO15** (button) | **VSYS** |
| 10 | **GPIO14** (CC1101 MISO) | **GND** |

**H2 (right): power, the light sensor, and the radio.**

| Row | Left column | Right column |
|---|---|---|
| 1 | **GND** | **GND** |
| 2 | **VSYS** | **3V3** (the diagram says ADC_AVDD; the schematic says 3V3) |
| 3 | **GPIO6** (I2C SDA) | BOOT, don't touch |
| 4 | **GPIO7** (I2C SCL) | RUN (this is reset), don't touch |
| 5 | GPIO8 (display) | GPIO0 (strapping, avoid) |
| 6 | GPIO9 (display) | GPIO1 (battery voltage, onboard) |
| 7 | GPIO10 (display) | **GPIO2** (CC1101 GDO0) |
| 8 | GPIO11 (display) | GPIO3 (strapping, avoid) |
| 9 | GPIO12 (display) | **GPIO4** (CC1101 MOSI) |
| 10 | **GPIO13** (CC1101 SCK) | **GPIO5** (CC1101 CS) |

Watch rows 3 and 4 of H2: a solder bridge from SDA or SCL to BOOT or RUN will hold the
board in reset or in download mode.

## Before you solder: one check now, one when the LiPo arrives (a multimeter)

The schematic settles which pins exist: every pin above is on a header. It also shows
how power flows: **VSYS is USB 5 V through a Schottky diode** (VBUS → MBR230 → VSYS),
and the **3.3 V regulator runs from VBAT** (the battery side of the charger). So VSYS
is there on USB and very likely dead on battery.

Probe tips don't fit 1.27 mm sockets, so do these once the male pins are plugged in.

1. Board on USB, display down, USB-C at the top. On **H1, right column, rows 9 and
   10** (VSYS and GND, the bottom two pins on the right), expect **about 4.6 to 4.8 V**.
   Near 0 V means the board is rotated from the drawing or you are on the wrong
   column. Stop there and tell me what you see. While it's plugged in: **H2, right
   column, row 2 against row 1** (3V3 and GND), expect **about 3.3 V**.
2. **When the LiPo is in:** unplug USB and measure H1 rows 9 and 10 again. The schematic
   says about 0 V. If so, the button LED and the speaker amp take power from the
   battery's + lead (3.7 to 4.2 V) instead of VSYS, on USB and on battery alike: the
   MAX98357A runs from 2.5 to 5.5 V, and the LED will be a little dimmer. VBAT isn't on
   either header, so it's tapped at the battery lead.

The pins are 1.27 mm apart and VSYS sits right next to GND. Hold the probes steady and
never let one tip touch two pins.

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

Every wire lands on a 1.27 mm male pin plugged into H1 or H2, never on the board. Work
one group at a time, then flash `env:frog` or `env:calibrate` to test before the next
group.

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
- [ ] Button **LED +** → **VSYS** for bench testing on USB; the battery + lead once
  check 2 confirms VSYS is dead on battery
- [ ] Optional: 10 kΩ from base to GND, so the LED stays fully off while the board boots

The LED has its own resistor inside, so nothing else is needed on the supply side.

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
- [ ] MAX98357A **VIN** → **VSYS** for bench testing on USB; the battery + lead once
  check 2 confirms VSYS is dead on battery. **GND** → **GND**
- [ ] **BCLK** → **GPIO17**, **LRC** → **GPIO18**, **DIN** → **GPIO21**
- [ ] **GAIN**: tie to **VIN** for the quietest fixed gain (6 dB). This is a kid's toy near her ears; volume can go up in software, not down in hardware
- [ ] **SD**: leave unconnected (mono, amp on)
- [ ] Speaker + / − to the amp's output terminals: a small 8 Ω speaker (28 to 40 mm). It
  draws less from the battery than 4 Ω and is plenty loud near her ears

### Power
- 3.7V LiPo into the **MX1.25** header. The onboard charger tops it up over USB-C.
  **Check polarity with the multimeter before the first plug-in** — red must land on +.
- Bench power / charging: USB-C, **at least 1 A**. The speaker amp pulls current in
  bursts; a weak phone charger can brown out the board mid-chirp (it reboots).
- Battery voltage is readable in firmware on GPIO1 (onboard divider, see wiki).

## Things I'm not sure of (flagged, not guessed)
- **VSYS on battery power:** the schematic says USB only (see the check section).
  Check 2 confirms it on the real board.
- **WH51 sensor ID:** the frog must filter by her pot's sensor ID or it'll hear the
  beds too. The ID prints in the `radio` line on first reception; hardcode it after
  the first run.

Confirmed on the real board 2026-10-04: the display colors (`invert = true`,
`rgb_order = false`) and the serial port (CH343 USB chip, `/dev/ttyACM0`).
