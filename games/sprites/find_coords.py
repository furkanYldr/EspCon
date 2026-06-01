"""
Koordinat dogrulama ve duzeltme scripti
Objects grid'e gore kalpler row 0'da col 3,4,5,6,7 olarak gorunuyor
Kaya tiles overworld'un farkli bir yerinde
"""
from PIL import Image
import os

base = os.path.dirname(__file__)
obj = Image.open(os.path.join(base, "objects.png")).convert("RGBA")
ow  = Image.open(os.path.join(base, "Overworld.png")).convert("RGBA")

os.makedirs(os.path.join(base, "preview2"), exist_ok=True)

def save_preview(img, name):
    big = img.resize((img.width*4, img.height*4), Image.NEAREST)
    big.save(os.path.join(base, "preview2", f"chk_{name}.png"))

def extract(img, tx, ty, tw=16, th=16):
    return img.crop((tx*tw, ty*th, (tx+1)*tw, (ty+1)*th))

# Objects - kalpleri tara (row 0, col 0-12)
print("Objects row 0:")
for c in range(13):
    t = extract(obj, c, 0)
    save_preview(t, f"obj_r0_c{c}")
    print(f"  col {c}: saved")

# Objects row 2 - alevler
print("Objects row 2:")
for c in range(13):
    t = extract(obj, c, 2)
    save_preview(t, f"obj_r2_c{c}")

# Overworld - kayalar icin row 8-9
print("Overworld row 8-9:")
for r in [8, 9]:
    for c in range(16):
        t = extract(ow, c, r)
        save_preview(t, f"ow_r{r}_c{c}")

print("Tamamlandi!")
