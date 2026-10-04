# Gardagotchi brief (2026-09-24, revised 2026-10-03)

This brief replaces the original concept (ESPHome, MQTT, greenhouse moods, piezo), which is in the git history.

## Concept
- She has her own pot. The frog's mood comes from that pot's soil moisture and light.
  The frog is **portable** — she picks it up and carries it, and it hears its pot over
  915 MHz from the Ecowitt WH51 already in the beds. It is standalone and does NOT
  connect to the homestead MQTT/relay system (that's a separate future project; don't
  mix them).
- Teaching goal: seeds need soil, water, sun, and time. The core loop: soil dries out → frog gets thirsty → she waters → the sensor sees it → frog celebrates on its own (no button needed to confirm).
- Design rules (she's 4): faces, not text. One button. Reactions in under 1 s. The frog only asks for things she can actually fix, and negative moods fade/forgive quickly; no guilt mechanics. Battery-powered and portable: 3.7V LiPo, USB-C charging. All electronics enclosed, no small parts, rounded edges.
- Lore: we had a real bullfrog named Jeremiah at our pond. She names the frog herself (naming ceremony on day one).

## Hardware (ordered)
- Waveshare ESP32-S3-LCD-1.28 (NON-touch): 240x240 round GC9A01 display, onboard QMI8658 IMU. Its two header blocks are 1.27 mm sockets; wires go on 1.27 mm male pins plugged in, never on the board. The onboard I2C bus is GPIO6 (SDA) / GPIO7 (SCL); the BH1750 shares it.
- CC1101 915 MHz module on SPI (SCK 13, MISO 14, MOSI 4, CS 5, GDO0 2), hearing the Ecowitt WH51 in her pot. WH51 runs a year on its AA battery; no soil node to build, power, or weatherproof. Filter by her pot's sensor ID so the frog ignores the bed sensors.
- 3.7V LiPo into the MX1.25 header; onboard charger tops it up over USB-C. Check polarity before the first plug-in.
- BH1750 light sensor (I2C)
- MAX98357A I2S amp + small 4Ω speaker, for voice clips recorded in Grandpa's voice
- Adafruit 30mm yellow LED arcade button (#3488). LEDs need 5V: driven via PN2222 NPN (GPIO → 1k → base, emitter → GND, collector → LED−, LED+ → 5V). PWM on that GPIO pulses the LED. VSYS is USB-only per the schematic, so on battery the LED (and the amp) run from the battery + lead instead; wiring.md check 2 confirms it.
- Printed case on a Bambu A1 (multicolor). Room for the LiPo, CC1101 + antenna placement away from the battery.
- GPIO map: see `wiring.md`.

## Art
- The frog is hand-painted pixel art (a leaf on its head), one strip of 64x64 cells per
  face in `faces/art/`. `docs/sprite-brief.md` is the format and the list of faces.
- `faces/frog.py` puts the frog on an 80x80 grid scaled 3x to 240x240 and draws the
  backdrops, props, plant cards and flower bed around it.
- 9 states: content, happy, love (button press), thirsty, soggy (overwatered), sunny, cloudy, sleeping, celebrate.
- Green only, decided 2026-10-04. At this size the color tells you what the character
  is: green reads as a frog that belongs with a plant, while pink or purple scaled down
  reads as alien. The mint/lilac choice is dropped.
- 5 plant growth stages: seed, sprout, leaves, bud, flower.

## Firmware direction
- Arduino framework via PlatformIO, LovyanGFX. Not ESPHome.
- WH51 decoder: port from rtl_433's Ecowitt implementation; CC1101 via SPI, GDO0 as packet interrupt. WH51 transmits ~every 70 s.
- State precedence (original): sleeping > soggy > thirsty > celebrate > love > sunny > cloudy > happy > content. Replaced by a two-layer mood + reaction model, see `firmware-notes.md`.
- Button press = love (hearts, chirp, small mood bump that decays over hours).
- Growth stages: based on days since planting to start, not watering totals.

## Open questions
- Moisture thresholds: WH51 reports 0–100% and the beds have HA history — start from that, tune by watching her pot (`calibration.md`).
- Night mode: DECIDED 2026-09-24, light-based. "The frog follows the sun" is an idea she can hold. Expect trial and error once she uses it.
- Scene layout: DECIDED 2026-09-24. The plant appears only on the surprise card when it grows; the collection is a flower bed at the frog's feet (see `firmware-notes.md`).
- Case: Waveshare publishes a 3D model of the board. Display recessed behind a printed bezel with a clear cover; the 30mm button hole dominates one face. Battery compartment + CC1101 antenna keep-out are part of the layout now. Electronics away from the pot (she will overwater), accessible with 4 screws, USB-C port reachable for charging with strain relief.
