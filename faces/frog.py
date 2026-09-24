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
          celebrate="#FFE3F4")

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

# ---------- scenes ----------
def base(state):
    img = np.zeros((N, N, 3), np.uint8)
    img[:] = hexc(BG[state])
    return img

def scene(state, P):
    img = base(state)
    if state == "celebrate":  # rainbow arc behind the frog
        cols = ["#FF6B6B", "#FFB347", "#FFE066", "#6BD66B", "#6EC6FF", "#B07CFF"]
        for i, col in enumerate(cols):
            r_out = 27 - i * 1.6
            ring = circ(30, 34, r_out) & ~circ(30, 34, r_out - 1.6) & (yy < 34)
            img[ring] = hexc(col)
    if state == "sleeping":
        for (x, y) in [(12, 12), (33, 6), (8, 26), (52, 28), (19, 6)]:
            stamp(img, x, y, STAR, {"#": C["star"]})
        stamp(img, 40, 4, MOON, {"#": C["star"]})
    dry = state == "thirsty"
    droop = 2 if state in ("thirsty",) else 0
    draw_frog(img, P, dry=dry, droop=droop)
    d = droop
    crown(img) if d == 0 else stamp(img, 27, 13 + d, CROWN, {"#": C["gold"]})
    if d:
        img[15 + d, 30] = hexc(C["gem"])
    E = [(x, y + d) for x, y in EYES]

    if state == "content":
        for x, y in E: eye_open(img, P, x, y)
        mouth_smile(img, P); cheeks(img, P)
    elif state == "happy":
        for x, y in E: eye_closed_happy(img, P, x, y)
        mouth_big(img, P); cheeks(img, P)
        stamp(img, 7, 22, HEART_S, {"#": C["hot"]})
        stamp(img, 48, 18, HEART_S, {"#": C["hot"]})
    elif state == "love":
        for x, y in E: eye_heart(img, P, x, y)
        mouth_smile(img, P); cheeks(img, P, color=C["hot"])
        stamp(img, 6, 16, HEART, {"#": C["hot"]})
        stamp(img, 47, 11, HEART, {"#": C["pink"]})
        stamp(img, 50, 26, HEART_S, {"#": C["hot"]})
        stamp(img, 10, 30, HEART_S, {"#": C["pink"]})
    elif state == "thirsty":
        for x, y in E: eye_half(img, P, x, y, dry=True)
        mouth_tongue(img, P, y=MY + d)
        # thought bubble with a water drop
        img[circ(48, 12, 6)] = hexc(C["white"]); img[edge(circ(48, 12, 6))] = hexc(C["cloudsh"])
        img[circ(46.5, 21.5, 1.4)] = hexc(C["white"])
        stamp(img, 46, 9, DROP, {"#": C["blue"]})
        img[12, 47] = hexc(C["white"])
    elif state == "soggy":
        for i, (x, y) in enumerate(E): eye_squeeze(img, P, x, y, left=(i == 0))
        mouth_wavy(img, P); cheeks(img, P)
        for (x, y) in [(10, 8), (46, 6), (15, 20), (49, 22)]:
            stamp(img, x, y, DROP, {"#": C["blue"]})
        pud = ell(8, 53, 52, 58) & ~SIL
        img[pud] = hexc(C["blue"]); img[edge(pud) & ~SIL] = hexc(C["bluedark"])
    elif state == "sunny":
        sunglasses(img, P)
        mouth_smile(img, P); cheeks(img, P)
        stamp(img, 8, 10, SUN, {"#": C["orange"]})
        img[circ(11.5, 13.5, 2.3)] = hexc(C["yellow"])
    elif state == "cloudy":
        for x, y in E: eye_half(img, P, x, y)
        mouth_small(img, P); cheeks(img, P)
        stamp(img, 5, 12, CLOUD, {"#": C["cloud"]})
        stamp(img, 44, 8, CLOUD, {"#": C["cloudsh"]})
    elif state == "sleeping":
        for x, y in E: eye_sleep(img, P, x, y)
        mouth_small(img, P); cheeks(img, P)
        stamp(img, 45, 17, Z_BIG, {"#": C["white"]})
        stamp(img, 50, 9, Z_SM, {"#": C["white"]})
    elif state == "celebrate":
        for x, y in E: eye_star(img, P, x, y)
        mouth_big(img, P); cheeks(img, P)
        conf = [(8, 20, "red"), (11, 35, "blue"), (50, 20, "green"), (48, 38, "purple"),
                (16, 9, "yellow"), (44, 8, "hot"), (6, 28, "orange"), (53, 30, "pink")]
        for x, y, c in conf:
            img[y:y + 2, x:x + 2] = hexc(C[c])
        stamp(img, 5, 42, SPARK, {"#": C["gold"]})
        stamp(img, 50, 43, SPARK, {"#": C["gold"]})
    if state == "sleeping":  # dim everything a touch
        img = (img.astype(float) * 0.78).astype(np.uint8)
        img[:] = np.where((img == (np.array(hexc(BG["sleeping"])) * .78).astype(np.uint8)).all(-1)[..., None],
                          hexc(BG["sleeping"]), img)
    return img

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

def upscale(img, s):
    return Image.fromarray(img).resize((img.shape[1] * s, img.shape[0] * s), Image.NEAREST)

def round_mask(im):
    w, h = im.size
    m = Image.new("L", (w, h), 0)
    ImageDraw.Draw(m).ellipse((0, 0, w - 1, h - 1), fill=255)
    out = Image.new("RGB", (w, h), (0, 0, 0)); out.paste(im, (0, 0), m)
    return out

STATES = ["content", "happy", "love", "thirsty", "soggy", "sunny", "cloudy", "sleeping", "celebrate"]
LABEL = dict(content="content (idle)", happy="happy", love="love (button press)",
             thirsty="thirsty (soil dry)", soggy="soggy (too much water)", sunny="sunny (basking)",
             cloudy="cloudy / gray day", sleeping="sleeping (night)", celebrate="celebrate (just watered!)")

if __name__ == "__main__":
    out = os.path.join(os.path.dirname(os.path.abspath(__file__)), "out")
    os.makedirs(out, exist_ok=True)
    for pname, P in PALETTES.items():
        os.makedirs(f"{out}/{pname}", exist_ok=True)
        for s in STATES:
            upscale(scene(s, P), SCALE).save(f"{out}/{pname}/{s}.png")
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
