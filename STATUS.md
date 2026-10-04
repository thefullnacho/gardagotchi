# Status

## 2026-10-03

**Changed.** Architecture pivot: the frog is portable and battery-powered now. Soil
moisture comes from the Ecowitt WH51 already in the beds (and already in HA), heard
directly over 915 MHz by a CC1101 module on SPI — no DIY soil node to build, power, or
weatherproof, and no gateway needed on the frog. 3.7V LiPo into the MX1.25 header; the
onboard charger tops it up over USB-C. The wired STEMMA soil sensor is out (bench
spare). BH1750 stays for the light-based night mode. Docs updated to match: README,
brief, wiring, calibration.

**Decided.** CC1101 on SPI: SCK 13, MISO 14, MOSI 4, CS 5, GDO0 2 (all confirmed free;
15/16/17 stay with button, LED, speaker amp). WH51 filtered by sensor ID so the frog
hears her pot, not the beds. WH51 reports moisture as 0–100%, so thresholds start from
HA history instead of a from-scratch calibration week.

**Open.** Button LED + speaker amp want 5 V (VSYS), which may be USB-only. Check 2 in
wiring.md will tell us on the real board; fallback is 3V3 (dimmer LED) or a boost.

**Next.** Buy CC1101 module + LiPo; breadboard the frog (board + CC1101 + LiPo); port
the WH51 decoder; first 915 MHz packet on the serial monitor. Then the enclosure.

## 2026-09-24

**Changed.** Direction reset: standalone frog reading its own pot (soil + light), not
the homestead MQTT concept. Old README/spec replaced; the brief is `docs/brief.md`.
Built the PlatformIO firmware scaffold (display, button, LED pulse, sensors), a
calibration logger (serial + flash, button = "watered" marker), the sprite exporter,
and the mood engine in `firmware/lib/mood` with 19 passing desktop tests. The frog
build is driven by the mood engine. Everything compiles; **nothing has run on the real
board yet** (parts ordered 2026-09-23, not arrived).

**Decided.** Two-layer mood (resting mood + short reactions), so the button always
answers. Light-based night mode ("the frog follows the sun"). One celebration per
watering, not per pour. Earn-never-lose rewards.

**In flight / unverified.** GPIO picks 15/16/17/18/21 and VSYS as a 5 V source need
checking on the physical board. Moisture and light thresholds are placeholders until
the calibration week. Sleepy-love reaction has no art. Speaker code not started.

**Next.** Animation frames (blink, breathe, happy wiggle), then growth stages +
the collection that compounds (one keepsake per plant that reaches flower).

**[non-production]** (in `~/me/queue.md`): board checks with a multimeter when the
boards arrive; solder button + LED and first flash; start the calibration week.

## 2026-09-24 (later)

**Changed.** Animation. Each face is now three layers (backdrop, frog, props) with a
short loop per face in `faces/frog.py` (`ANIM`): breathing, wiggle, sigh, shake-off,
sway, drifting clouds, rising Zs, a celebration bounce. Random blinks come from the
new player in `firmware/lib/anim` (6 desktop tests, 25 in total). Added the sleepy-love
face for a press at night. 38 frames per palette, ~270 KB. Previews look smooth as
GIFs; **not yet seen on the real screen.**

**Next.** Growth stages (days since planting) and the collection that compounds (one
keepsake per plant that reaches flower), saved to flash so a power cut loses nothing.

## 2026-09-24 (evening)

**Changed.** Growth and the collection (`firmware/lib/garden`, 16 tests; 43 in total).
A day is one sleep (dawns counted, lamp-proof). Growth waits when the plant was thirsty
nearly all day, never goes back. A new stage is a surprise: the LED twinkles and her
next press shows the plant card. Each plant that blooms becomes a flower in the frog's
garden, saved to flash. Grown-up replant: hold the button while plugging in, 3 s.

**Decided.** The plant appears only on the surprise card, so the real pot stays the
plant she watches. The collection is a flower bed at the frog's feet (9 spots), not
crown gems. Stage timing set for a dwarf French marigold (beans dropped: climbing,
and she loves flowers).

**Next.** Idea logged: a full round (9 flowers) changes the background for good.
Firmware still waits on the boards for its first real run. After that: the speaker
(voice clips), then the case.

**[non-production]** Buy dwarf French marigold seeds, a small pot with a drainage
hole, and a saucer (in `~/me/queue.md`).

## 2026-09-24 (wrap)

**Changed.** Nothing further in code since the growth + collection commit. Plant
confirmed: marigold from the bag in the greenhouse. Pot + saucer on hand (plain pot,
not self-watering: wicking from below hides her watering from the sensor and the
frog would never celebrate or get thirsty).

**In flight.** Boards in the mail. Firmware has never run on hardware. Stage timing is
set for a dwarf French marigold until the bag's numbers come in.

**Next concrete action.** Read the seed bag (type, days to sprout, days to flower),
send the numbers, set `garden::Config`. Then, when the boards land: the two
multimeter checks, solder button + LED, first flash.

**[non-production]** Check the marigold seed bag; board checks; solder + first
flash; start the calibration week. All already in `~/me/queue.md`.

**Spun off.** A character for Alex's own home and garden, reading hestia's watches.
Separate project, not this repo; stub at `~/hestia-face/README.md`.
