# Sprite brief: the leaf frog's faces

Paste this into the chat where the emotes get drawn, and attach `faces/art/content.png`
as the reference. Every strip that follows these rules drops straight into the firmware.

## The frog

A chubby green frog with a leaf on its head, hand-painted pixel art. It lives on a
240x240 round screen beside a real potted plant, for a 4-year-old. The screen shows
each pixel 3x, so the art is drawn in 64x64 cells.

`content.png` is the idle and the reference for everything else: 5 cells, in order:
eyes open, half-lidded, closed, squash, stretch.

## Format

- **One PNG per face**, named after it: `happy.png`, `love.png`, and so on.
- **64x64 cells side by side** in one row, so the strip is 64 tall and 64, 192 or 320 wide.
- **Transparent background.** Alpha is 0 or 255 only: no soft edges, no glow, no shadow
  on the ground.
- **Same spot in every cell.** The feet's lowest pixel is on **row 61** (counting from 0
  at the top), and the body spans about **columns 8 to 55**. The frog should not
  jump when the face changes.
- **Same palette** as `content.png` (below). Add colors only where the face needs them
  (heart eyes, sunglasses, tongue, star eyes, the duller greens for thirsty).
- **Frog only.** No background, no hearts, Zs, raindrops or sun: the code draws those
  around the frog. No text, ever.

## Cell layout

- **1 cell:** the face.
- **3 cells:** face, half-lidded blink, closed blink (for faces with open eyes).
- **5 cells:** face, half blink, closed blink, squash, stretch. If the face doesn't
  blink, repeat the face in cells 2 and 3.

## Faces to draw

Already covered by `content.png`: content (the idle), cloudy (a dim day; the half-lidded
cell), sleeping (dark; the closed-eyes cell, which the code dims and adds Zs to).

| File | When it shows | Look | Cells |
|---|---|---|---|
| `love.png` | she presses the button | heart eyes, hot-pink cheeks, smile | 1 |
| `happy.png` | she has pressed the button a lot lately | ^ ^ closed eyes, big open smile | 1 |
| `celebrate.png` | she just watered the plant | star eyes, big open mouth, the most joyful face | 5, with a big squash and stretch |
| `thirsty.png` | the soil is dry | droopy half-lids, tongue out, slightly duller greens, **leaf wilting** | 1 or 3 |
| `soggy.png` | too much water | squeezed > < eyes, wavy mouth, silly not sick | 1 |
| `sunny.png` | bright light | sunglasses, relaxed smile | 1 |
| `sleepy_love.png` | a press at night | eyes closed except one peeking open, small smile | 1 |

## Tone rules (from the design)

- The frog only asks for what she can fix. Thirsty is gentle and asking, never sad,
  sick or upset. Nothing about the frog gets worse for waiting.
- Soggy is funny, not unwell.
- Celebrate is the payoff for watering. It should be the biggest, happiest face.

## Palette of content.png, dark to light

#141C1A #1C2C1A #293D1B #2E621C #4F792A #3A804E #5A922B #73A62F #7CAD32 #859A40 #86B530
#93BA35 #9EBD3B #A6CA3A #B9CF44 #BAD542 #C0DA47 #CCB75E #C8D949 #C8DF4C #D4D953 #F5947E
#D7E257 #F79D84 #D8E75B #E6E869 #EDED76 #E8D6A0 #F5F289 #C4E2D0 #FCFCF8
