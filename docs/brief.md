# Gardagotchi brief (2026-09-24)

This brief replaces the original concept (ESPHome, MQTT, greenhouse moods, piezo), which is in the git history.

## Concept
- She has her own pot on a dedicated windowsill. The frog's mood comes from that pot's soil moisture and light. It is standalone and does NOT connect to the homestead MQTT/relay system (that's a separate future project; don't mix them).
- Teaching goal: seeds need soil, water, sun, and time. The core loop: soil dries out → frog gets thirsty → she waters → the sensor sees it → frog celebrates on its own (no button needed to confirm).
- Design rules (she's 4): faces, not text. One button. Reactions in under 1 s. The frog only asks for things she can actually fix, and negative moods fade/forgive quickly; no guilt mechanics. USB wall power only, no batteries. All electronics enclosed, no small parts, rounded edges.
- Lore: we had a real bullfrog named Jeremiah at our pond. She names the frog herself (naming ceremony on day one).

## Hardware (ordered)
- Waveshare ESP32-S3-LCD-1.28 (NON-touch): 240x240 round GC9A01 display, onboard QMI8658 IMU. Headers are 1.27mm pitch; wires soldered directly. The onboard I2C bus is GPIO6 (SDA) / GPIO7 (SCL); external sensors share it.
- Adafruit STEMMA I2C capacitive soil sensor (#4026). Electronics at the top aren't waterproof; they sit above the soil line and get a conformal coat.
- BH1750 light sensor (I2C)
- MAX98357A I2S amp + small 4Ω speaker, for voice clips recorded in Grandpa's voice
- Adafruit 30mm yellow LED arcade button (#3488). LEDs need 5V: driven via PN2222 NPN (GPIO → 1k → base, emitter → GND, collector → LED−, LED+ → 5V). PWM on that GPIO pulses the LED.
- USB-C 5V wall power. Case printed on a Bambu A1 (multicolor).
- GPIO map: see `wiring.md`.

## Art
- `faces/frog.py` generates everything procedurally: a 60x60 grid scaled 4x to 240x240.
- 9 states: content, happy, love (button press), thirsty, soggy (overwatered), sunny, cloudy, sleeping, celebrate.
- Two palettes, mint and lilac. She hasn't picked one yet.
- 5 plant growth stages: seed, sprout, leaves, bud, flower.
- Single still frames. Animation (2–3 frames per state: blink, breathing, happy wiggle) is NOT done.

## Firmware direction
- Arduino framework via PlatformIO, LovyanGFX. Not ESPHome.
- Keep OTA if we end up joining WiFi.
- State precedence (original): sleeping > soggy > thirsty > celebrate > love > sunny > cloudy > happy > content. Replaced by a two-layer mood + reaction model, see `firmware-notes.md`.
- Button press = love (hearts, chirp, small mood bump that decays over hours).
- Growth stages: based on days since planting to start, not watering totals.

## Open questions
- Moisture thresholds: log a week of readings from a test pot while watering normally, then set dry/happy/soggy from the data (`calibration.md`).
- Night mode: DECIDED 2026-09-24, light-based. "The frog follows the sun" is an idea she can hold. Expect trial and error once she uses it.
- Scene layout: DECIDED 2026-09-24. The plant appears only on the surprise card when it grows; the collection is a flower bed at the frog's feet (see `firmware-notes.md`).
- Case: Waveshare publishes a 3D model of the board. Display recessed behind a printed bezel with a clear cover; the 30mm button hole dominates one face. Electronics away from the pot (she will overwater), accessible with 4 screws, USB cable strain-relieved.
