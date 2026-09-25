"""Pixel-art frog faces for a 240x240 round display.
Drawn on a 60x60 grid, scaled 4x (nearest neighbor)."""
import numpy as np
from PIL import Image, ImageDraw, ImageFont
import os

N = 60
SCALE = 4
yy, xx = np.mgrid[0:N, 0:N]
CX = CY = 30

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

PALETTES = {
    "mint": dict(body="#72D27F", shade="#4DAE63", belly="#EAF8D8", line="#26383A",
                 cheek="#FF8DB9", lid="#5CC06F", dry="#B5C97A", dryshade="#95A85E"),
    "lilac": dict(body="#C7A2FF", shade="#A07BE6", belly="#FCEBFF", line="#3A2656",
                  cheek="#FF6FA8", lid="#B28FF2", dry="#CDBBD6", dryshade="#AE9BB8"),
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

HEART = [".##.##.", "#######", "#######", ".#####.", "..###..", "...#..."]
HEART_S = [".#.#.", "#####", ".###.", "..#.."]
DROP = ["..#..", "..#..", ".###.", "#####", "#####", ".###."]
SUN = ["#..#..#", ".#####.", "#######", "#######", "#######", ".#####.", "#..#..#"]
CLOUD = ["...###....", ".######...", "#########.", "##########", ".########."]
MOON = [".###.", "##...", "##...", "##...", ".###."]
STAR = [".#.", "###", ".#."]
Z_BIG = ["#####", "...#.", "..#..", ".#...", "#####"]
Z_SM = ["###", "..#", ".#.", "#..", "###"]
CROWN = ["#..#..#", "##.#.##", "#######", "#######"]
SPARK = ["..#..", "..#..", "##.##", "..#..", "..#.."]

# ---------- frog geometry ----------
HEAD = ell(15, 20, 45, 43)
BUMP_L, BUMP_R = circ(22, 22, 6.2), circ(38, 22, 6.2)
BODY = ell(19, 36, 41, 53)
FOOT_L, FOOT_R = ell(13, 48, 25, 55), ell(35, 48, 47, 55)
BELLY = ell(23, 41, 37, 52)
SIL = HEAD | BUMP_L | BUMP_R | BODY | FOOT_L | FOOT_R
EYES = [(22, 22), (38, 22)]

def draw_frog(img, P, dry=False, droop=0):
    body, shade = (P["dry"], P["dryshade"]) if dry else (P["body"], P["shade"])
    sil = SIL
    if droop:  # shift everything down a bit (tired/droopy)
        sil = np.roll(SIL, droop, axis=0)
    img[sil] = hexc(body)
    # shading on lower body + feet
    low = sil & (yy >= 46 + droop)
    img[low] = hexc(shade)
    img[np.roll(BELLY, droop, 0)] = hexc(P["belly"])
    for f in (FOOT_L, FOOT_R):
        f2 = np.roll(f, droop, 0)
        img[f2] = hexc(shade)
        e = edge(f2) & (yy < 52 + droop)
        img[e] = hexc(P["line"])
    img[edge(sil)] = hexc(P["line"])
    return droop

# ---------- eyes ----------
def eye_open(img, P, cx, cy, sparkle=True, look=(0, 1), pr=3.0):
    img[circ(cx, cy, 5)] = hexc(C["white"])
    img[edge(circ(cx, cy, 5))] = hexc(P["line"])
    img[circ(cx + look[0], cy + look[1], pr)] = hexc(P["line"])
    if sparkle:
        px, py = cx + look[0], cy + look[1]
        for (dx, dy) in [(-2, -2), (-1, -2), (-2, -1), (-1, -1)]:
            img[py + dy, px + dx] = hexc(C["white"])
        img[py + 1, px + 1] = hexc(C["white"])

def eye_closed_happy(img, P, cx, cy):  # ^
    for dx, dy in [(-3, 1), (-2, 0), (-1, -1), (0, -1), (1, -1), (2, 0), (3, 1)]:
        img[cy + dy, cx + dx] = hexc(P["line"])
        img[cy + dy + 1, cx + dx] = hexc(P["line"]) if abs(dx) < 3 and dy < 0 else img[cy + dy + 1, cx + dx]

def eye_sleep(img, P, cx, cy):  # u-shaped lash line
    for dx, dy in [(-3, -1), (-2, 0), (-1, 1), (0, 1), (1, 1), (2, 0), (3, -1)]:
        img[cy + dy, cx + dx] = hexc(P["line"])

def eye_half(img, P, cx, cy, dry=False):
    eye_open(img, P, cx, cy, sparkle=False, look=(0, 2), pr=2.5)
    lid = circ(cx, cy, 5) & (yy <= cy)
    img[lid] = hexc(P["dryshade"] if dry else P["lid"])
    img[edge(circ(cx, cy, 5))] = hexc(P["line"])
    for x in range(cx - 4, cx + 5):
        img[cy, x] = hexc(P["line"])

def eye_heart(img, P, cx, cy):
    img[circ(cx, cy, 5)] = hexc(C["white"])
    img[edge(circ(cx, cy, 5))] = hexc(P["line"])
    stamp(img, cx - 3, cy - 2, HEART, {"#": C["hot"]})
    img[cy - 1, cx - 2] = hexc(C["white"])

def eye_squeeze(img, P, cx, cy, left):  # > <
    pts = [(-2, -2), (-1, -1), (0, 0), (-1, 1), (-2, 2)]
    for dx, dy in pts:
        dx = dx if left else -dx
        img[cy + dy, cx + dx + (1 if left else -1)] = hexc(P["line"])
        img[cy + dy, cx + dx + (2 if left else -2)] = hexc(P["line"])

def eye_star(img, P, cx, cy):
    img[circ(cx, cy, 5)] = hexc(C["white"])
    img[edge(circ(cx, cy, 5))] = hexc(P["line"])
    stamp(img, cx - 2, cy - 2, SPARK, {"#": C["gold"]})
    img[cy, cx] = hexc(C["golddark"])

def sunglasses(img, P):
    for (cx, cy) in EYES:
        img[ell(cx - 5, cy - 3, cx + 6, cy + 5)] = hexc(C["glass"])
        img[edge(ell(cx - 5, cy - 3, cx + 6, cy + 5))] = hexc(C["hot"])
        img[cy - 1, cx - 2] = hexc(C["white"]); img[cy - 1, cx - 1] = hexc(C["white"])
    for x in range(27, 34):
        img[21, x] = hexc(C["hot"])

# ---------- mouths ----------
MY = 35
def mouth_smile(img, P, y=MY):
    for dx, dy in [(-3, 0), (-2, 1), (-1, 1), (0, 1), (1, 1), (2, 1), (3, 0)]:
        img[y + dy, 30 + dx] = hexc(P["line"])

def mouth_big(img, P, y=MY):
    m = ell(25, y - 1, 36, y + 6) & (yy >= y)
    img[m] = hexc(C["mouth"])
    img[ell(27, y + 2, 34, y + 7) & m] = hexc(C["tongue"])
    img[edge(m)] = hexc(P["line"])

def mouth_small(img, P, y=MY):
    for dx in (-1, 0, 1):
        img[y + 1, 30 + dx] = hexc(P["line"])

def mouth_o(img, P, y=MY):
    m = circ(30.5, y + 1.5, 1.8)
    img[m] = hexc(C["mouth"]); img[edge(m)] = hexc(P["line"])

def mouth_tongue(img, P, y=MY):
    for dx in range(-3, 4):
        img[y + 1, 30 + dx] = hexc(P["line"])
    t = ell(29, y + 1, 34, y + 6) & (yy > y + 1)
    img[t] = hexc(C["tongue"]); img[edge(t) & (yy > y + 1)] = hexc(C["mouth"])

def mouth_wavy(img, P, y=MY):
    for i, dx in enumerate(range(-4, 5)):
        img[y + (i % 2), 30 + dx] = hexc(P["line"])

def cheeks(img, P, y=33, color=None):
    col = hexc(color or P["cheek"])
    for x0 in (17, 40):
        img[y:y + 2, x0:x0 + 4] = col

def crown(img):
    stamp(img, 27, 13, CROWN, {"#": C["gold"]})
    img[15, 30] = hexc(C["gem"])
    img[14, 30] = hexc(C["gem"])
    for x in range(27, 34):
        img[16, x] = hexc(C["golddark"])

def eye_blink(img, P, cx, cy, dry=False):  # closed for a blink: lid down, one lash line
    img[circ(cx, cy, 5)] = hexc(P["dryshade"] if dry else P["lid"])
    img[edge(circ(cx, cy, 5))] = hexc(P["line"])
    for x in range(cx - 4, cx + 5):
        img[cy + 1, x] = hexc(P["line"])

# ---------- scenes ----------
# A scene is three layers: the backdrop (never moves), the frog, and the props in
# front (hearts, drops, clouds, Zs...). Animation frames nudge the frog by (dx, dy)
# and the props by (pdx, pdy). blink=True closes the frog's eyes.
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

def scene(state, P, dx=0, dy=0, pdx=0, pdy=0, blink=False):
    if state in CARDS:  # pdy picks the sparkle phase
        return card(CARDS.index(state), pdy)
    img = base(state)
    asleep = state in ("sleeping", "sleepy_love")

    # --- backdrop ---
    if state == "celebrate":  # rainbow arc behind the frog
        cols = ["#FF6B6B", "#FFB347", "#FFE066", "#6BD66B", "#6EC6FF", "#B07CFF"]
        for i, col in enumerate(cols):
            r_out = 27 - i * 1.6
            ring = circ(30, 34, r_out) & ~circ(30, 34, r_out - 1.6) & (yy < 34)
            img[ring] = hexc(col)
    if asleep:
        for (x, y) in [(12, 12), (33, 6), (8, 26), (52, 28), (19, 6)]:
            stamp(img, x, y, STAR, {"#": C["star"]})
        stamp(img, 40, 4, MOON, {"#": C["star"]})
    if state == "soggy":  # puddle on the ground; the frog stands in it
        pud = ell(8, 53, 52, 58)
        img[pud] = hexc(C["blue"]); img[edge(pud)] = hexc(C["bluedark"])

    # --- the frog ---
    fr = _layer()
    dry = state == "thirsty"
    d = 2 if state == "thirsty" else 0
    draw_frog(fr, P, dry=dry, droop=d)
    crown(fr) if d == 0 else stamp(fr, 27, 13 + d, CROWN, {"#": C["gold"]})
    if d:
        fr[15 + d, 30] = hexc(C["gem"])
    E = [(x, y + d) for x, y in EYES]

    if state == "content":
        for x, y in E: eye_open(fr, P, x, y)
        mouth_smile(fr, P); cheeks(fr, P)
    elif state == "happy":
        for x, y in E: eye_closed_happy(fr, P, x, y)
        mouth_big(fr, P); cheeks(fr, P)
    elif state == "love":
        for x, y in E: eye_heart(fr, P, x, y)
        mouth_smile(fr, P); cheeks(fr, P, color=C["hot"])
    elif state == "thirsty":
        for x, y in E: eye_half(fr, P, x, y, dry=True)
        mouth_tongue(fr, P, y=MY + d)
    elif state == "soggy":
        for i, (x, y) in enumerate(E): eye_squeeze(fr, P, x, y, left=(i == 0))
        mouth_wavy(fr, P); cheeks(fr, P)
    elif state == "sunny":
        sunglasses(fr, P)
        mouth_smile(fr, P); cheeks(fr, P)
    elif state == "cloudy":
        for x, y in E: eye_half(fr, P, x, y)
        mouth_small(fr, P); cheeks(fr, P)
    elif state == "sleeping":
        for x, y in E: eye_sleep(fr, P, x, y)
        mouth_small(fr, P); cheeks(fr, P)
    elif state == "sleepy_love":  # woken by a press: one eye peeks open
        eye_sleep(fr, P, *E[0])
        eye_open(fr, P, *E[1], look=(-1, 1), pr=2.5)
        mouth_smile(fr, P); cheeks(fr, P, color=C["hot"])
    elif state == "celebrate":
        for x, y in E: eye_star(fr, P, x, y)
        mouth_big(fr, P); cheeks(fr, P)
    if blink:
        for x, y in E: eye_blink(fr, P, x, y, dry=dry)
    _paste(img, fr, dx, dy)

    # --- props in front ---
    pr = _layer()
    if state == "happy":
        stamp(pr, 7, 22, HEART_S, {"#": C["hot"]})
        stamp(pr, 48, 18, HEART_S, {"#": C["hot"]})
    elif state == "love":
        stamp(pr, 6, 16, HEART, {"#": C["hot"]})
        stamp(pr, 47, 11, HEART, {"#": C["pink"]})
        stamp(pr, 50, 26, HEART_S, {"#": C["hot"]})
        stamp(pr, 10, 30, HEART_S, {"#": C["pink"]})
    elif state == "thirsty":  # thought bubble with a water drop
        pr[circ(48, 12, 6)] = hexc(C["white"]); pr[edge(circ(48, 12, 6))] = hexc(C["cloudsh"])
        pr[circ(46.5, 21.5, 1.4)] = hexc(C["white"])
        stamp(pr, 46, 9, DROP, {"#": C["blue"]})
        pr[12, 47] = hexc(C["white"])
    elif state == "soggy":
        for (x, y) in [(10, 8), (46, 6), (15, 20), (49, 22)]:
            stamp(pr, x, y, DROP, {"#": C["blue"]})
    elif state == "sunny":
        stamp(pr, 8, 10, SUN, {"#": C["orange"]})
        pr[circ(11.5, 13.5, 2.3)] = hexc(C["yellow"])
    elif state == "cloudy":
        stamp(pr, 5, 12, CLOUD, {"#": C["cloud"]})
        stamp(pr, 44, 8, CLOUD, {"#": C["cloudsh"]})
    elif state == "sleeping":
        stamp(pr, 45, 17, Z_BIG, {"#": C["white"]})
        stamp(pr, 50, 9, Z_SM, {"#": C["white"]})
    elif state == "sleepy_love":
        stamp(pr, 46, 12, HEART_S, {"#": C["hot"]})
    elif state == "celebrate":
        conf = [(8, 20, "red"), (11, 35, "blue"), (50, 20, "green"), (48, 38, "purple"),
                (16, 9, "yellow"), (44, 8, "hot"), (6, 28, "orange"), (53, 30, "pink")]
        for x, y, c in conf:
            pr[y:y + 2, x:x + 2] = hexc(C[c])
        stamp(pr, 5, 42, SPARK, {"#": C["gold"]})
        stamp(pr, 50, 43, SPARK, {"#": C["gold"]})
    _paste(img, pr, pdx, pdy)

    if asleep:  # dim everything a touch
        img = (img.astype(float) * 0.78).astype(np.uint8)
        img[:] = np.where((img == (np.array(hexc(BG["sleeping"])) * .78).astype(np.uint8)).all(-1)[..., None],
                          hexc(BG["sleeping"]), img)
    return img

# ---------- animation ----------
# Each state loops through its frames: frog offset (dx, dy), prop offset (pdx, pdy),
# and how long the frame shows in ms. blink=True means the firmware drops in a quick
# blink at random moments (only for states whose eyes are open).
def _f(ms, dx=0, dy=0, pdx=0, pdy=0):
    return dict(ms=ms, dx=dx, dy=dy, pdx=pdx, pdy=pdy)

ANIM = {
    "content":   dict(blink=True, frames=[_f(900), _f(900, dy=-1)]),                   # breathe
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
    "celebrate": dict(blink=False, frames=[_f(120), _f(110, dy=-2, pdy=-1),            # bounce!
                                           _f(160, dy=-3), _f(110, dy=-2, pdy=1)]),
    "sleepy_love": dict(blink=False, frames=[_f(500), _f(500, pdy=-1)]),
}
for _c in ["card_seed", "card_sprout", "card_leaves", "card_bud", "card_flower"]:
    ANIM[_c] = dict(blink=False, frames=[_f(300), _f(300, pdy=1)])  # sparkles twinkle

def frames(state, P):
    """The loop frames for a state, as (image, ms) pairs."""
    return [(scene(state, P, dx=f["dx"], dy=f["dy"], pdx=f["pdx"], pdy=f["pdy"]), f["ms"])
            for f in ANIM[state]["frames"]]

def blink_frame(state, P):
    return scene(state, P, blink=True) if ANIM[state]["blink"] else None

# ---------- plant growth stages (for the side of the screen) ----------
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
BED_SLOTS = [(27, 50), (20, 49), (34, 49), (13, 46), (41, 46),   # centre out, then up the sides
             (8, 40), (46, 40), (5, 33), (50, 33)]
PETALS = ["#FF6FA8", "#FFE066", "#B07CFF", "#FFB347", "#6EC6FF",
          "#FF6B6B", "#FFFFFF", "#FF8DB9", "#9DE6D2"]
FLOWER_PARTS = dict(o=C["gold"], s="#4FB564", l="#6BD66B")
DIM = 0.78

def _inside(x, y):
    return (x + .5 - 30) ** 2 + (y + .5 - 30) ** 2 <= 29.5 ** 2
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
    pl = plant(stage).repeat(2, 0).repeat(2, 1)  # 24x24 -> 48x48
    pl[(pl == hexc(PLANT_BG)).all(-1)] = bg
    img[4:52, 6:54] = pl
    spots = [[(8, 14), (47, 20), (12, 34), (44, 40)], [(14, 8), (48, 30), (6, 24), (40, 12)]]
    for (x, y) in spots[phase % 2]:
        stamp(img, x, y, SPARK, {"#": C["gold"]})
    if bloom:
        conf = [(10, 20, "red"), (48, 16, "blue"), (16, 44, "green"), (46, 46, "purple"),
                (22, 8, "yellow"), (38, 6, "hot"), (5, 32, "orange"), (53, 34, "pink")]
        for x, y, c in conf:
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
            seq = fr + fr
            b = blink_frame(s, P)
            if b is not None:
                seq.append((b, 150))
            seq += fr
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
            sheet.paste(round_mask(upscale(scene(s, P), SCALE)), (x, y))
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
