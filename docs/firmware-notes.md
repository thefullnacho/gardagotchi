# Firmware design notes

## State precedence: two layers (built 2026-09-24 in `firmware/lib/mood`)

Original: sleeping > soggy > thirsty > celebrate > love > sunny > cloudy > happy > content

The problem with one list: **love sits below thirsty**. If the frog is thirsty and she
presses the button, nothing visible happens. For a 4-year-old that reads as "broken",
and it breaks the under-1-second rule. The same goes for pressing it at night.

Proposal: two layers.

**Mood** (the resting face; what the pot and room are like right now):
`sleeping > soggy > thirsty > sunny > cloudy > happy > content`

**Reactions** (a few seconds, drawn on top of any mood, then back to the mood):
`celebrate > love`
- **Love** always answers the button, in every mood. Hearts for ~3 s, then back to
  thirsty or whatever the mood is. The frog still wants water, but it loves her anyway.
- **At night**, a press gets a sleepy version (one eye opens, a small heart, back to
  sleep) rather than nothing, and no chirp.
- **Celebrate** fires when moisture jumps (she watered). It beats love, and it must
  also beat **soggy**: freshly watered soil reads very wet for a while, and she shouldn't
  get a "too much!" face straight after doing the right thing.

**Soggy** should need the soil to *stay* wet for a while (hours, not minutes) before it
shows. The calibration data will tell us how long this pot takes to drain. It's also
the one mood where the fix is "wait". That's fine to teach, but it should be mild and
short.

**Thirsty** needs no delay: water fixes it, and the reward comes fast.

**One celebration per watering, not per pour.** Otherwise the fastest way to more
fanfare is to keep pouring, which teaches overwatering. The frog celebrates when the
soil jumps, but not again within 20 minutes, and never when the pot was already well
watered. A small splash on very dry soil still counts: she did the right thing.

All thresholds live in `mood::Config` (`firmware/lib/mood/mood.h`) and are placeholders
until the calibration week. The desktop tests (`pio test -e native`) describe each rule
as a scenario, so changing a number and re-running shows what behavior moved.

## Night mode: light-based (decided 2026-09-24)

She can understand "the frog wakes up with the sun and sleeps when it's dark", so the
sun is the frog's clock, not her bedtime. Tune by watching her use it.

Plan: the BH1750, with a slow trigger. Sleep after it has been dark for
~10 minutes, wake after ~10 minutes of light. The delay stops a passing shadow or a
lamp flicked on for a moment from waking it.

Why light, not WiFi + internet time:
- No WiFi means no passwords, no router changes, nothing for anyone to maintain, and
  it keeps the toy fully standalone as the brief says.
- It fits the lesson: the frog, like the seed, lives by the sun.

The trade-offs:
- **Winter:** it gets dark around 4:30 to 5 PM, so the frog sleeps while she's still
  up. The sleepy-press reaction above softens that. Or the rule can be "dark AND it's
  been at least N hours since sunrise", which the device can learn by counting from
  the last dawn it saw.
- **A lamp in the room at night** keeps it awake. That depends on where the light
  sensor points. Facing out the window it sees real daylight, not the room lamp; that
  choice is part of the case design.

The calibration week will show what "dark" is on that windowsill, since the lux column
records every dusk and dawn.

## Awareness, not habit yet

She already knows the mechanics (seed, water, sun, time). What she doesn't have yet is
the habit of checking. So the frog's real job is to be the reminder she can't be for
herself yet, and to make keeping up feel good:

- **The ask is visible from across the room.** When thirsty, the button LED pulses
  faster. That is the "come see me" signal, and it goes calm again once she waters.
- **Thirsty stays one gentle face.** No worse stage for going longer without water.
  Missing a day costs nothing on screen.
- **The reward is the big moment.** Celebrate gets the best animation and a voice clip
  in Grandpa's voice. That is the part that builds the habit.
- **The real plant is the real stakes.** The frog forgives, but a dead seedling doesn't.
  Pick a sturdy, fast seed (bush bean, pea, sunflower), keep spare seeds, and treat
  overwatering as the likelier killer, since she will overwater. A pot with a drainage
  hole and a saucer helps more than any firmware.

## She's a gamer (proposal, 2026-09-24)

She has finished Astro Bot and three-starred every Sackboy level in co-op with the
family. So she already speaks the language of games: stars, collectibles, completion,
and lots of feedback for every action. That's something to build on, with one trap.

- **Use the language she knows.** Stars, gems, and a crown are icons, not text, so
  they don't break "faces, not text". The frog already wears a crown with one gem.
- **Earn, never lose.** A completionist notices every empty slot. A row of 3 stars
  with one missing is a guilt mechanic in game form. So nothing on screen may show a
  missed day or a gap, and nothing she's earned can ever be taken away.
- **Collection idea:** each time a plant reaches flower, the frog gets something to
  keep, like a new gem in its crown or a flower on a little shelf. That's one reward
  per plant, tied to *time + care*, which is the lesson. The count only ever grows.
- **Feel matters.** Games have taught her to expect instant, bouncy feedback. So
  animation (blink, breathe, wiggle) and the celebrate moment are worth doing well,
  not last.
- **Free extra input, maybe:** the board has a motion sensor built in (the QMI8658).
  Patting or gently tilting the case could make the frog wiggle, with no second
  button. Worth trying once there's a case to hold. Skip it if it muddies "one button".

## Rendering and animation
Each face is three layers in `faces/frog.py`: a backdrop that never moves, the frog,
and props in front (hearts, drops, clouds, Zs). `ANIM` in frog.py lists each face's
loop: how far the frog and the props shift on each frame, and for how long. So the frog
can breathe or bounce while the rainbow stays put, or the Zs drift up while it sleeps.

Blinks are not part of the loop. The firmware (`firmware/lib/anim`) drops a 150 ms
blink in at random every 2.5 to 6 seconds, for the faces whose eyes are open, so it
never looks mechanical.

`python3 faces/frog.py` writes previews to `faces/out/anim/` (one GIF per face, plus
`all_mint.gif` / `all_lilac.gif` with every face playing at once).
`python3 faces/export_sprites.py` packs every frame for the firmware: 60x60, one byte
per pixel, scaled 4x on the device, and each frame goes to the screen in one push, so
there's no flicker. 38 frames x 2 palettes is about 270 KB of the ~6 MB program space.
