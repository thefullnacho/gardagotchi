# Gardagotchi — Build Spec

**Date:** 2026-09-22 · **For:** Alex's step-granddaughter (4, very smart) · **Status:** spec, pre-build

## The concept

A little creature in a chunky, colorful case. Its mood is driven by *real homestead
data* — cozy when the greenhouse is warm, thirsty when the barrels run low,
panicking when the wind kicks the greenhouse door open. One big button. She
presses it to say hi, and the creature reacts. The garden has moods, and she can
take care of it.

The tech stays invisible. The creature is the interface.

## Design principles (she's 4)

- **Faces, not text.** She's pre-/early-reading. Emotion is carried by the
  creature's face, color, sound, and motion — not words. Minimal on-screen text.
- **One button.** A single big, satisfying arcade button. Press = love / say hi /
  "I did the thing." No menus, no modes.
- **Cause and effect, fast.** Every action gets an immediate reaction (<1 s).
  Four-year-olds live in the now; delayed feedback is no feedback.
- **Durable and safe.** Chunky printed case, no exposed electronics, no small
  swallowable parts, no sharp edges. USB-powered (wall adapter) — no LiPo in a
  kid's toy.
- **It sleeps when she sleeps.** Night mode ~8 PM–7 AM: creature sleeps (dim,
  slow breathing animation). Models healthy rhythms, and a sleeping creature is
  adorable.

## Parts

**On hand:** Bambu A1 multifilament (the case), filament, general tools.

**To snag (~$30 total):**

| Part | Why | ~Cost |
|---|---|---|
| ESP32 dev board (get 2–3, you'll want spares) | the brain; WiFi + MQTT | $8–10 ea |
| 1.28" round GC9A01 TFT, 240×240 | the creature's face — round display *is* the tamagotchi look | $9–12 |
| 30 mm arcade button with LED, colorful | the one button; LED pulses when the creature "wants attention" | $4–5 |
| Piezo buzzer (5 V) | happy chirps, sad boops | ~$2 |
| Jumper wires (F-F pack) | assembly | ~$7 |
| USB-C cable + 5 V wall adapter | power (you probably have these) | $0 |

The round display is the key pick: ESPHome supports the GC9A01 directly, and a
240×240 round face reads as "creature" instantly. Rectangular TFTs work but look
like a thermostat.

## Software architecture

Same pattern as everything else on the homestead — leaf node, brains on the server:

```
[sensors] → MQTT → hl-relay → MQTT → [gardagotchi ESP32] → round display
  (barrels, greenhouse…)         (mood computation lives here, easy to tweak)
```

- **Gardagotchi runs ESPHome.** Subscribes to MQTT topics
  (`homestead/greenhouse/temp`, `homestead/greenhouse/door`,
  `homestead/barrel1`, …), computes mood, draws the face.
- **Mood logic lives on the server**, not the device — same rule as the barrels.
  You'll tune what "thirsty" means; reflashing a kid's toy to tweak a threshold
  is nobody's idea of fun.
- **Faces:** 6–8 simple pixel-art faces drawn as images in ESPHome
  (happy, content, thirsty, cold, worried, scared, sleeping, celebrating).

## The mood model (v1 inputs → creature states)

| Input | Creature state |
|---|---|
| Greenhouse ≥ 50°F, door closed, barrels > 50% | **happy** |
| All nominal, nothing exciting | **content** |
| Either barrel < 20% | **thirsty** |
| Greenhouse 45–50°F | **chilly** (sweater-weather face) |
| Greenhouse < 45°F | **cold** (shivering) |
| Greenhouse door open | **scared** (wide eyes — the wind got in!) |
| Button LED pulses on scared/cold | "come tell Grandpa!" |
| 8 PM – 7 AM | **sleeping** |

New sensors (soil moisture, etc.) slot into the same table later. The model only
ever grows — never rewrites.

## Reward mechanics (the fun part)

1. **Press = love.** Button press → happy wiggle + chirp + small mood bump that
   gently decays over hours. Teaches, softly, that care is ongoing.
2. **The quest loop.** The creature *asks*: thirsty face + "the flowers are
   thirsty" (minimal words, mostly face). She waters the flowers, presses the
   button to say "did it!" → **celebration** (spinning happy face + fanfare
   chirp). Need → real-world action → reward. That's the core loop.
3. **Real-data magic (phase 3).** Soil moisture rises after a watering → the
   creature perks up *on its own*: "the garden drank! thank you!" No button
   needed. To a 4-year-old, that's indistinguishable from magic, and it's
   completely real.
4. **The naming ceremony (phase 1).** She names it. Ownership is the first
   reward.
5. **"It learned about the garden!"** Each phase upgrade is an event — the
   creature gets *smarter* over time, and she witnesses it.

## Phased build (each phase is a complete gift)

- **Phase 1 — now.** Creature + button + chirps, standalone (no WiFi needed).
  She names it, learns press = love. A complete toy on day one.
- **Phase 2 — with the greenhouse monitor.** WiFi + MQTT. Moods from greenhouse
  temp + door. The wind-past-the-rock incident becomes *her* early-warning
  system: scared face → she runs to tell Grandpa. She's part of the homestead
  crew.
- **Phase 3 — the full network.** Barrels, soil moisture, the quest loop, the
  real-data magic. The creature is now genuinely the mood of the land.

## Case design notes (Bambu)

- Chunky rounded form — think egg/potato with a face, or a little house. Big
  enough that the 30 mm button dominates one face.
- Round display bezel: recess the GC9A01 ~2 mm behind a printed bezel ring so
  little fingers can't pry the ribbon cable.
- Button hole sized for the arcade button's threaded collar + nut.
- Vent slots on the back (ESP32 runs warm-ish; also lets the chirps out).
- Multicolor: let *her* pick the colors. Body one color, ears/feet accent,
  button color her choice. Buy-in starts at the filament.
- Print a spare button cap and keep the electronics accessible with 4 screws —
  kids are durability testers.

## Safety

- USB wall power only. No batteries in the toy.
- All electronics fully enclosed; strain-relief the USB cable through the case.
- No parts < 1.25" accessible. Round every edge in the CAD.
- Volume: piezo at 3.3 V through a resistor if it's too enthusiastic.

## Open questions (ask *her*)

- What's its name? (Phase 1 ceremony.)
- What colors? (Filament buy-in.)
- What does its happy dance look like? (You'll be animating it — take notes.)
