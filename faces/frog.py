"""Pixel-art frog faces for a 240x240 round display.

The frog itself is hand-painted art: one strip per face in faces/art/<face>.png, 64x64
cells (docs/sprite-brief.md). This file puts each cell on an 80x80 grid, scaled 3x
(nearest neighbor), and draws everything around it: the backdrop, the props in front,
the plant cards and the flower bed. A face with no strip yet borrows a cell of the
idle (content.png), so its backdrop and props still say which mood it is."""
import os

import numpy as np
from PIL import Image, ImageDraw, ImageFont

N = 80
SCALE = 3
yy, xx = np.mgrid[0:N, 0:N]
CX = CY = 40

def ell(x0, y0, x1, y1):
    cx, cy = (x0 + x1) / 2, (y0 + y1) / 2
    rx, ry = (x1 - x0) / 2, (y1 - y0) / 2
    return ((xx + .5 - cx) / rx) ** 2 + ((yy + .5 - cy) / ry) ** 2 <= 1

def circ(cx, cy, r):
    return (xx + .5 - cx) ** 2 + (yy + .5 - cy) ** 2 <= r * r

def edge(m):
    p = np.pad(m, 1)
    inner = p[:-2, 1:-1] & p[2:, 1:-1] & p[1:-1, :-2] & p[1:-1, 2:]
    return m & ~inner

def hexc(h):
    h = h.lstrip('#')
    return tuple(int(h[i:i + 2], 16) for i in (0, 2, 4))

# The frog is green, and stays green: at this size the color says what the character
# is, and pink or purple read as alien (docs/brief.md). A palette is a color swap for
# the frog, {"#art": "#new"}; the one palette swaps nothing.
PALETTES = {
    "green": {},
}
C = dict(white="#FFFFFF", gold="#FFD35C", golddark="#E0A526", gem="#FF4F9A",
         pink="#FF8DB9", hot="#FF4F9A", tongue="#FF7B9C", mouth="#5A2236",
         blue="#6EC6FF", bluedark="#3A8FD6", yellow="#FFE066", orange="#FFB347",
         cloud="#F4F1FA", cloudsh="#C9C3DA", star="#FFF3A8", purple="#B07CFF",
         red="#FF6B6B", green="#6BD66B", navy="#1E2148", glass="#2A1E3A")

BG = dict(content="#E9DDFF", happy="#FFD9EC", love="#FFD0E6", thirsty="#FFF0C9",
          soggy="#CDEBFF", sunny="#FFF1B8", cloudy="#DAD6E6", sleeping="#1E2148",
          celebrate="#FFE3F4", sleepy_love="#1E2148",
          card_seed="#E9DDFF", card_flower="#FFE3F4")

# ---------- ASCII sprite helper ----------
def stamp(img, x, y, rows, cmap):
    for j, row in enumerate(rows):
        for i, ch in enumerate(row):
            if ch in cmap and 0 <= y + j < N and 0 <= x + i < N:
                img[y + j, x + i] = hexc(cmap[ch])

HEART_L = [".###.###.", "#########", "#########", "#########", ".#######.", "..#####..", "...###...",
           "....#...."]
HEART = [".##.##.", "#######", "#######", ".#####.", "..###..", "...#..."]
HEART_S = [".#.#.", "#####", ".###.", "..#.."]
DROP = ["..#..", "..#..", ".###.", "#####", "#####", ".###."]
SUN = ["#..#..#", ".#####.", "#######", "#######", "#######", ".#####.", "#..#..#"]
CLOUD = ["....####......", "..########....", ".###########..", "##############", "##############",
         ".############."]
MOON = [".###.", "##...", "##...", "##...", ".###."]
STAR = [".#.", "###", ".#."]
Z_BIG = ["#######", ".....#.", "....#..", "...#...", "..#....", ".#.....", "#######"]
Z_SM = ["#####", "...#.", "..#..", ".#...", "#####"]
SPARK = ["..#..", "..#..", "##.##", "..#..", "..#.."]
CAN = ["......######..........",  # watering can, spout toward the frog
       ".....#......#.........",
       ".....#......#......###",
       ".....#......#.....###.",
       "..bbbbbbbbbbbbb..##...",
       ".bbbbbbbbbbbbbbb##....",
       ".bhbbbbbbbbbbbb#......",
       ".bhbbbbbbbbbbb#.......",
       ".bhbbbbbbbbbbbb.......",
       ".bbbbbbbbbbbbbb.......",
       ".bbbbbbbbbbbbbb.......",
       ".bbbbbbbbbbbbbb.......",
       "..bbbbbbbbbbbbb.......",
       "...bbbbbbbbbbb........"]

# ---------- the frog: hand-painted strips ----------
ART = os.path.join(os.path.dirname(os.path.abspath(__file__)), "art")
CELL = 64
OFF = (N - CELL) // 2  # the 64 cell sits centered on the 80 grid
# Cells of a full strip: the face, a half-lidded and a closed blink, a squash, a stretch.
FACE, HALF, CLOSED, SQUASH, STRETCH = range(5)
# Faces without a strip of their own borrow this cell of content.png.
BORROW = dict(cloudy=HALF, thirsty=HALF, sleeping=CLOSED, sleepy_love=CLOSED)

def _strip(name):
    path = os.path.join(ART, name + ".png")
    if not os.path.exists(path):
        return None
    im = np.array(Image.open(path).convert("RGBA"))
    return [im[:, i * CELL:(i + 1) * CELL] for i in range(im.shape[1] // CELL)]

def _cells(state):
    """Five cells for a face, whatever its strip has: missing ones repeat the face."""
    own = _strip(state)
    if own is None:
        idle = _strip("content")
        base = BORROW.get(state, FACE)
        if base != FACE:  # a borrowed sleepy or half-lidded face: no blink, no bounce
            return [idle[base]] * 5, False
        return idle, True
    if len(own) >= 5:
        return own[:5], True
    if len(own) >= 3:
        return own[:3] + [own[FACE]] * 2, True
    return [own[FACE]] * 5, False

def _recolor(cell, P):
    out = cell.copy()
    for src, dst in P.items():
        out[(cell[..., :3] == hexc(src)).all(-1) & (cell[..., 3] > 0), :3] = hexc(dst)
    return out

SENT = (1, 2, 3)  # "nothing drawn here" marker for the moving layers

def base(state):
    img = np.zeros((N, N, 3), np.uint8)
    img[:] = hexc(BG[state])
    return img

def _layer():
    lay = np.zeros((N, N, 3), np.uint8)
    lay[:] = SENT
    return lay

def _paste(img, lay, dx, dy):
    lay = np.roll(lay, (dy, dx), axis=(0, 1))
    m = (lay != SENT).any(-1)
    img[m] = lay[m]

def _frog_layer(cell):
    lay = _layer()
    a = cell[..., 3] > 0
    lay[OFF:OFF + CELL, OFF:OFF + CELL][a] = cell[..., :3][a]
    return lay

def scene(state, P, dx=0, dy=0, pdx=0, pdy=0, cell=FACE):
    if state in CARDS:  # pdy picks the sparkle phase
        return card(CARDS.index(state), pdy)
    img = base(state)
    asleep = state in ("sleeping", "sleepy_love")

    # --- backdrop ---
    if state == "celebrate":  # rainbow arc behind the frog
        cols = ["#FF6B6B", "#FFB347", "#FFE066", "#6BD66B", "#6EC6FF", "#B07CFF"]
        for i, col in enumerate(cols):
            r_out = 37 - i * 2.2
            ring = circ(40, 46, r_out) & ~circ(40, 46, r_out - 2.2) & (yy < 46)
            img[ring] = hexc(col)
    if asleep:
        for (x, y) in [(14, 16), (24, 8), (8, 34), (70, 38), (58, 30)]:
            stamp(img, x, y, STAR, {"#": C["star"]})
        stamp(img, 52, 6, MOON, {"#": C["star"]})
    if state == "soggy":  # puddle on the ground; the frog stands in it
        pud = ell(14, 64, 66, 75)
        img[pud] = hexc(C["blue"]); img[edge(pud)] = hexc(C["bluedark"])

    # --- the frog ---
    cells, _ = _cells(state)
    _paste(img, _frog_layer(_recolor(cells[cell], P)), dx, dy)

    # --- props in front ---
    pr = _layer()
    if state == "happy":
        stamp(pr, 7, 30, HEART, {"#": C["hot"]})
        stamp(pr, 65, 26, HEART, {"#": C["hot"]})
    elif state == "love":
        stamp(pr, 7, 20, HEART_L, {"#": C["hot"]})
        stamp(pr, 61, 14, HEART_L, {"#": C["pink"]})
        stamp(pr, 66, 30, HEART, {"#": C["hot"]})
        stamp(pr, 8, 36, HEART, {"#": C["pink"]})
    elif state == "thirsty":  # a watering can pouring on the frog: a thing she holds,
        # where a thought bubble is a comic convention a 4-year-old doesn't read yet
        stamp(pr, 6, 14, CAN, {"#": C["bluedark"], "b": C["blue"], "h": C["white"]})
        for x, y in [(28, 19), (26, 21), (29, 22), (27, 24)]:
            pr[y:y + 2, x] = hexc(C["blue"])
    elif state == "soggy":
        for (x, y) in [(12, 14), (60, 10), (8, 32), (68, 30)]:
            stamp(pr, x, y, DROP, {"#": C["blue"]})
    elif state == "sunny":
        for a in range(8):  # rays
            ang = a * np.pi / 4
            for r in (8, 9):
                pr[int(17 + r * np.sin(ang)), int(17 + r * np.cos(ang))] = hexc(C["orange"])
        pr[circ(17.5, 17.5, 5.5)] = hexc(C["orange"])
        pr[circ(17.5, 17.5, 4)] = hexc(C["yellow"])
    elif state == "cloudy":
        stamp(pr, 6, 16, CLOUD, {"#": C["cloud"]})
        stamp(pr, 58, 14, CLOUD, {"#": C["cloudsh"]})
    elif state == "sleeping":
        stamp(pr, 61, 22, Z_BIG, {"#": C["white"]})
        stamp(pr, 64, 13, Z_SM, {"#": C["white"]})
    elif state == "sleepy_love":
        stamp(pr, 63, 18, HEART, {"#": C["hot"]})
    elif state == "celebrate":
        conf = [(9, 26, "red"), (12, 46, "blue"), (68, 26, "green"), (66, 48, "purple"),
                (20, 12, "yellow"), (58, 10, "hot"), (6, 37, "orange"), (72, 38, "pink")]
        for x, y, c in conf:
            pr[y:y + 2, x:x + 2] = hexc(C[c])
        stamp(pr, 6, 56, SPARK, {"#": C["gold"]})
        stamp(pr, 69, 56, SPARK, {"#": C["gold"]})
    _paste(img, pr, pdx, pdy)

    if asleep:  # dim everything a touch
        img = (img.astype(float) * DIM).astype(np.uint8)
        img[:] = np.where((img == (np.array(hexc(BG["sleeping"])) * DIM).astype(np.uint8)).all(-1)[..., None],
                          hexc(BG["sleeping"]), img)
    return img

# ---------- animation ----------
# Each state loops through its frames: which cell of its strip, the frog offset (dx, dy),
# the prop offset (pdx, pdy), and how long the frame shows in ms. blink=True means the
# firmware drops in a quick blink (half, closed, half) at random moments, if the face's
# strip has blink cells.
def _f(ms, dx=0, dy=0, pdx=0, pdy=0, cell=FACE):
    return dict(ms=ms, dx=dx, dy=dy, pdx=pdx, pdy=pdy, cell=cell)

ANIM = {
    "content":   dict(blink=True, frames=[_f(2600), _f(120, cell=SQUASH),           # rest, then a hop
                                          _f(140, cell=STRETCH)]),
    "happy":     dict(blink=False, frames=[_f(200, dx=-1), _f(200, dy=-1, pdy=-1),     # wiggle
                                           _f(200, dx=1), _f(200, dy=-1, pdy=-1)]),
    "love":      dict(blink=True, frames=[_f(250), _f(250, dy=-1, pdy=-1),              # bob, hearts float
                                          _f(250, pdy=-2), _f(250, dy=-1, pdy=-1)]),
    "thirsty":   dict(blink=True, frames=[_f(1400), _f(1400, dy=1)]),                  # slow sigh
    "soggy":     dict(blink=False, frames=[_f(500), _f(500, pdy=1), _f(90, dx=-1, pdy=2),  # drip, shake off
                                           _f(90, dx=1, pdy=2), _f(90, dx=-1, pdy=3), _f(500, pdy=3)]),
    "sunny":     dict(blink=False, frames=[_f(500), _f(500, dx=1), _f(500), _f(500, dx=-1)]),  # sway
    "cloudy":    dict(blink=True, frames=[_f(1000), _f(1000, dy=-1, pdx=1),            # clouds drift
                                          _f(1000, pdx=2), _f(1000, dy=-1, pdx=1)]),
    "sleeping":  dict(blink=False, frames=[_f(2000), _f(2000, dy=-1, pdy=-1)]),        # slow breath, Zs rise
    "celebrate": dict(blink=False, frames=[_f(120), _f(110, cell=SQUASH, pdy=-1),      # bounce!
                                           _f(160, dy=-3, cell=STRETCH), _f(110, dy=-2, pdy=1)]),
    "sleepy_love": dict(blink=False, frames=[_f(500), _f(500, pdy=-1)]),
}

def frames(state, P):
    """The loop frames for a state, as (image, ms) pairs."""
    return [(scene(state, P, dx=f["dx"], dy=f["dy"], pdx=f["pdx"], pdy=f["pdy"], cell=f["cell"]), f["ms"])
            for f in ANIM[state]["frames"]]

def blink_frames(state, P):
    """The blink, in order (half, closed, half), or [] if this face doesn't blink."""
    if state in CARDS or not ANIM[state]["blink"] or not _cells(state)[1]:
        return []
    return [scene(state, P, cell=c) for c in (HALF, CLOSED, HALF)]

# ---------- plant growth stages (the surprise card) ----------
PLANT_BG = "#E9DDFF"
SOIL, SOILD, POT, POTD, LEAF, LEAFD, STEM = ("#7A5236", "#5C3C26", "#FF8DB9", "#D9689A",
                                             "#6BD66B", "#3FA64F", "#4FB564")
def plant(stage):
    n = 24
    im = np.zeros((n, n, 3), np.uint8); im[:] = hexc(PLANT_BG)
    pot = ["..############..", "..############..", "...##########...", "...##########...",
           "....########....", "....########...."]
    stamp(im, 4, 18, pot, {"#": POT})
    for x in range(6, 18): im[18, x] = hexc(POTD)
    for x in range(6, 18): im[17, x] = hexc(SOIL)
    for x in range(7, 17): im[16, x] = hexc(SOIL)
    if stage == 0:  # seed
        im[15, 11:13] = hexc(SOILD); im[14, 11:13] = hexc("#C8A26B")
    if stage >= 1:
        top = {1: 13, 2: 10, 3: 7, 4: 5}[stage]
        for y in range(top, 16): im[y, 12] = hexc(STEM)
    if stage == 1:
        stamp(im, 10, 11, ["##..", ".#.."], {"#": LEAF})
        stamp(im, 12, 11, ["..##", "..#."], {"#": LEAF})
    if stage >= 2:
        stamp(im, 7, 11, ["###..", ".####"], {"#": LEAF}); stamp(im, 13, 11, ["..###", "####."], {"#": LEAF})
    if stage >= 3:
        stamp(im, 8, 7, ["##..", ".###"], {"#": LEAFD}); stamp(im, 13, 7, ["..##", "###."], {"#": LEAFD})
    if stage == 3:
        stamp(im, 11, 4, [".#.", "###"], {"#": "#FFB3D6"})
    if stage == 4:
        flower = [".#.#.", "#####", "##o##", "#####", ".#.#."]
        stamp(im, 10, 1, flower, {"#": "#FF6FA8", "o": C["gold"]})
    return im

# ---------- the collection: a flower bed at the frog's feet ----------
# Every plant she grows to flower adds one flower here, for good. Drawn by the
# firmware on top of every frog face (not on the plant cards), dimmed at night.
FLOWER = [".p.p.",
          "ppopp",
          ".p.p.",
          "..s..",
          ".ls..",
          "..s.."]
BED_SLOTS = [(37, 68), (28, 66), (46, 66), (18, 62), (56, 62),   # centre out, then up the sides
             (10, 54), (64, 54), (6, 45), (69, 45)]
PETALS = ["#FF6FA8", "#FFE066", "#B07CFF", "#FFB347", "#6EC6FF",
          "#FF6B6B", "#FFFFFF", "#FF8DB9", "#9DE6D2"]
FLOWER_PARTS = dict(o=C["gold"], s="#4FB564", l="#6BD66B")
DIM = 0.78

def _inside(x, y):
    return (x + .5 - CX) ** 2 + (y + .5 - CY) ** 2 <= (CX - .5) ** 2
for _x, _y in BED_SLOTS:  # every flower pixel must land on the round screen
    for _j, _row in enumerate(FLOWER):
        for _i, _ch in enumerate(_row):
            assert _ch == "." or _inside(_x + _i, _y + _j), (_x, _y)

def flower_bed(img, n, dim=False):
    for i, (x, y) in enumerate(BED_SLOTS[:n]):
        cmap = dict(p=PETALS[i % len(PETALS)], **FLOWER_PARTS)
        if dim:
            cmap = {k: "#%02X%02X%02X" % tuple(int(c * DIM) for c in hexc(v)) for k, v in cmap.items()}
        stamp(img, x, y, FLOWER, cmap)
    return img

CARDS = ["card_seed", "card_sprout", "card_leaves", "card_bud", "card_flower"]

def card(stage, phase=0):
    """The surprise card: her plant, big, with twinkling sparkles. Blooming adds confetti."""
    bloom = stage == 4
    bg = hexc(BG["card_flower"] if bloom else BG["card_seed"])
    img = np.zeros((N, N, 3), np.uint8); img[:] = bg
    pl = plant(stage).repeat(3, 0).repeat(3, 1)  # 24x24 -> 72x72
    pl[(pl == hexc(PLANT_BG)).all(-1)] = bg
    img[2:74, 4:76] = pl
    spots = [[(10, 18), (64, 26), (14, 44), (60, 50)], [(18, 10), (66, 40), (7, 32), (54, 14)]]
    for (x, y) in spots[phase % 2]:
        stamp(img, x, y, SPARK, {"#": C["gold"]})
    if bloom:
        conf = [(12, 26), (64, 20), (20, 58), (62, 60), (28, 10), (52, 8), (6, 42), (72, 44)]
        names = ["red", "blue", "green", "purple", "yellow", "hot", "orange", "pink"]
        for (x, y), c in zip(conf, names):
            y2 = y + (1 if phase % 2 else 0)
            img[y2:y2 + 2, x:x + 2] = hexc(C[c])
    return img

def upscale(img, s):
    return Image.fromarray(img).resize((img.shape[1] * s, img.shape[0] * s), Image.NEAREST)

def round_mask(im):
    w, h = im.size
    m = Image.new("L", (w, h), 0)
    ImageDraw.Draw(m).ellipse((0, 0, w - 1, h - 1), fill=255)
    out = Image.new("RGB", (w, h), (0, 0, 0)); out.paste(im, (0, 0), m)
    return out

STATES = ["content", "happy", "love", "thirsty", "soggy", "sunny", "cloudy", "sleeping", "celebrate",
          "sleepy_love", "card_seed", "card_sprout", "card_leaves", "card_bud", "card_flower"]
for _c in CARDS:
    ANIM[_c] = dict(blink=False, frames=[_f(300), _f(300, pdy=1)])  # sparkles twinkle
LABEL = dict(content="content (idle)", happy="happy", love="love (button press)",
             thirsty="thirsty (soil dry)", soggy="soggy (too much water)", sunny="sunny (basking)",
             cloudy="cloudy / gray day", sleeping="sleeping (night)", celebrate="celebrate (just watered!)",
             sleepy_love="sleepy love (press at night)", card_seed="surprise: seed",
             card_sprout="surprise: sprout", card_leaves="surprise: leaves", card_bud="surprise: bud",
             card_flower="surprise: bloomed!")

if __name__ == "__main__":
    out = os.path.join(os.path.dirname(os.path.abspath(__file__)), "out")
    os.makedirs(out, exist_ok=True)
    for pname, P in PALETTES.items():
        os.makedirs(f"{out}/{pname}", exist_ok=True)
        for s in STATES:
            upscale(scene(s, P), SCALE).save(f"{out}/{pname}/{s}.png")
    # animated previews: each loop plays three times, with one blink in the middle
    for pname, P in PALETTES.items():
        os.makedirs(f"{out}/anim/{pname}", exist_ok=True)
        for s in STATES:
            fr = frames(s, P)
            seq = fr + fr + [(b, 50) for b in blink_frames(s, P)] + fr
            ims = [round_mask(upscale(im, SCALE)) for im, _ in seq]
            ims[0].save(f"{out}/anim/{pname}/{s}.gif", save_all=True, append_images=ims[1:],
                        duration=[ms for _, ms in seq], loop=0, disposal=1)
    os.makedirs(f"{out}/plant", exist_ok=True)
    for i, name in enumerate(["0_seed", "1_sprout", "2_leaves", "3_bud", "4_flower"]):
        upscale(plant(i), 5).save(f"{out}/plant/{name}.png")

    # contact sheet
    try:
        font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 15)
        tfont = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 22)
    except Exception:
        font = tfont = ImageFont.load_default()
    cell, pad, lab = 240, 24, 26
    cols = 3
    rows_per = (len(STATES) + cols - 1) // cols
    W = cols * (cell + pad) + pad
    blocks = []
    for pname, P in PALETTES.items():
        H = rows_per * (cell + pad + lab) + pad + 36
        sheet = Image.new("RGB", (W, H), (250, 247, 255)); d = ImageDraw.Draw(sheet)
        d.text((pad, 10), f"Palette: {pname}", fill=(58, 38, 86), font=tfont)
        for i, s in enumerate(STATES):
            r, c = divmod(i, cols)
            x = pad + c * (cell + pad); y = 46 + r * (cell + pad + lab)
            img = scene(s, P)
            if s not in CARDS:
                img = flower_bed(img, 3, dim=s in ("sleeping", "sleepy_love"))
            sheet.paste(round_mask(upscale(img, SCALE)), (x, y))
            d.text((x, y + cell + 4), LABEL[s], fill=(58, 38, 86), font=font)
        blocks.append(sheet)
    # plant strip
    ps = Image.new("RGB", (W, 200), (250, 247, 255)); d = ImageDraw.Draw(ps)
    d.text((pad, 10), "Plant growth stages", fill=(58, 38, 86), font=tfont)
    for i, name in enumerate(["seed", "sprout", "leaves", "bud", "flower"]):
        x = pad + i * 150
        ps.paste(upscale(plant(i), 5), (x, 44)); d.text((x, 44 + 124), name, fill=(58, 38, 86), font=font)
    blocks.append(ps)
    total = Image.new("RGB", (W, sum(b.height for b in blocks)), (250, 247, 255))
    y = 0
    for b in blocks: total.paste(b, (0, y)); y += b.height
    total.save(f"{out}/frog_preview.png")
    print("ok", total.size)
