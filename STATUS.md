# Status

## 2026-10-04

**Changed.** One board arrived, the planned Waveshare ESP32-S3-LCD-1.28. Its two header
blocks are 1.27 mm **sockets**, not pins, so wires go on 1.27 mm male pins plugged in.
Waveshare's schematic and pinout diagram confirm every pin we use, including the
CC1101 pins from 2026-10-03, and answer that entry's open question: VSYS is USB 5 V
through a Schottky diode, and the 3.3 V regulator runs from VBAT. So VSYS is very likely
dead on battery, and the button LED and speaker amp take power from the battery lead
instead (`docs/wiring.md`: the H1/H2 map and the checks). **First flash ever:** the frog
booted on the real screen with the right colors. (This checkout had missed the
2026-10-03 wireless pivot; it was merged at wrap.)

The frog is now hand-painted art: Alex's original leaf frog, a strip of 64x64 cells per
face in `faces/art/`, all ten faces drawn. `faces/frog.py` puts it on an 80x80 grid at
3x and still draws the backdrops, props (re-placed and enlarged), plant cards and flower
bed. Blinks play three frames now (half, closed, half; `lib/anim`, 44 tests pass). New
builds: `env:artcheck` puts one strip on the screen (`faces/art_check.py` also checks
it against the brief), and `env:demo` tours every face with nothing wired (BOOT button =
hearts). `docs/sprite-brief.md` is the paste-in brief for drawing faces. Stage timing
set from the seed bag (French marigold Sparky Mix): sprout day 10, leaves 21, bud 50,
flower 65. `docs/calibration.md` notes marigolds like to dry out, so thirsty belongs at
the dry end and soggy matters more.

**Decided.** Green only: at this size the color says what the character is, and pink
or purple read as alien; mint/lilac dropped. 3x on an 80 grid (4x cuts the frog off).
Thirsty shows a watering can pouring on the frog instead of a thought bubble. Stage
cards err late: a card must never show a stage the real pot hasn't reached.

**Planned (portable frog).** She wants to carry it off every time she sees it, so the
plan for living with the 2026-10-03 pivot is in `docs/firmware-notes.md` ("Portable
frog"): the rainbow waits for her press (the WH51 reports only every ~70 s), a lily-pad
charging dock, low battery shown as a sleepy frog, doze when still and wake when picked
up, a protected LiPo and a TPU bumper, and a new replant gesture.

**First user test.** She saw the demo on the board. She loved the rainbow (celebrate,
the watering reward). She didn't understand the watering prop (then a thought bubble)
and one other face, not yet known. At the end: "all of the faces are cute." She doesn't
know yet that the frog goes with her plant; that was kept surface level on purpose.

**In flight / unverified.** The watering can is Alex's pick, untested with her. The
other face she missed. Props are flat placeholders next to the painted frog. VSYS on
battery (the schematic says dead; measure when the LiPo is in). Nothing is wired yet,
and the WH51 decoder isn't started. The board is running `env:demo`.

**Next concrete action.** 1.27 mm male pins into the sockets, the USB multimeter check,
solder button + LED onto those pins, flash `env:frog`. Then the CC1101 and the WH51
decoder (2026-10-03 plan).

**[non-production]** (in `~/me/queue.md`): pins + bench list; CC1101 module + LiPo;
the multimeter check; solder button + LED; next visit, the thirsty test and the missing
face.

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
