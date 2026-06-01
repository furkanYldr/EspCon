"""
Objects.png - sprite'lari tam piksel konumlarıyla bul
Gorsel inceleme: kalp 16x16, sandik 32x32, vs
"""
from PIL import Image
import os

base = os.path.dirname(__file__)
obj = Image.open(os.path.join(base, "objects.png")).convert("RGBA")
ow  = Image.open(os.path.join(base, "Overworld.png")).convert("RGBA")

os.makedirs(os.path.join(base, "preview3"), exist_ok=True)

def save(img, name, scale=4):
    big = img.resize((img.width*scale, img.height*scale), Image.NEAREST)
    big.save(os.path.join(base, "preview3", f"{name}.png"))

def crop(img, x, y, w, h):
    return img.crop((x, y, x+w, y+h))

def rgb888_to_rgb565(r, g, b):
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)

def to_cpp(tile, name, transparent=0x0821):
    w, h = tile.size
    lines = [f"static const uint16_t {name}[{w*h}] PROGMEM = {{"]
    for y in range(h):
        row = []
        for x in range(w):
            r, g, b, a = tile.getpixel((x, y))
            if a < 64:
                row.append(f"0x{transparent:04X}")
            else:
                c = rgb888_to_rgb565(r, g, b)
                row.append(f"0x{c:04X}")
        comma = "," if y < h-1 else ""
        lines.append("  " + ", ".join(row) + comma)
    lines.append("};")
    return "\n".join(lines)

cpp = []
cpp.append("// game_zelda_sprites2.h — Duzeltilmis koordinatlarla")
cpp.append("#pragma once")
cpp.append("#include <pgmspace.h>")
cpp.append("#define ZELDA_TRANSPARENT 0x0821")
cpp.append("")

# =============================================
# OBJECTS.PNG — Piksel bazinda crop
# Kalp (16x14): x=48, y=0 (full)
# Kalp yarim: x=64, y=0
# Kalp bos: x=80, y=0
# Sandik (16x16): x=0, y=0
# =============================================
# Tam inceleme icin objects.png'nin ilk 100x60 bolgesini kaydet
save(crop(obj, 0, 0, 200, 60), "obj_top_strip", scale=3)
save(crop(obj, 0, 0, 528, 80), "obj_full_top", scale=2)

# Overworld - kayalari bul
# 640x576 -> 40x36 tiles
# Sol bolgedeki kayalar (gri tas): overworld grid'den teyit
# Gri donuk renkler arasiyor
# Satir 5 civarinda duvar/kaya benzeri seyler var
for r in [4, 5, 6]:
    strip = crop(ow, 0, r*16, 640, 16)
    save(strip, f"ow_row{r}_strip", scale=2)

# Overworld column 3-6 satirlari
save(crop(ow, 3*16, 2*16, 16*6, 16*5), "ow_rocks_area", scale=3)

print("Tamamlandi! preview3 klasorune bak.")
print(f"Objects boyutu: {obj.size}")

# Objects grid (16x16 yerine 18x18 veya baska bir grid mi?)
# Deneme: 16x16 grid ile col 4, row 0 = kalp tam mı?
t = crop(obj, 4*16, 0, 16, 16)
save(t, "test_heart_16x16_c4r0")
# 18x18 grid?
t2 = crop(obj, 4*18, 0, 18, 18)
save(t2, "test_heart_18x18_c4r0")
