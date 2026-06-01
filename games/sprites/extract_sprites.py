"""
Sprite Extractor — ESP32 GameHub için RGB565 dönüştürücü
Overworld.png ve character.png'den tile'ları çeker ve C++ array'e çevirir.
"""

from PIL import Image
import math, os

def rgb888_to_rgb565(r, g, b):
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)

def image_info(path):
    img = Image.open(path).convert("RGBA")
    print(f"  {os.path.basename(path)}: {img.width}x{img.height} px")
    return img

def extract_tile(img, tx, ty, tw=16, th=16, transparent_color=None):
    """Sprite sheet'den tx,ty indeksli 16x16 tile'ı çek"""
    px = tx * tw
    py = ty * th
    tile = img.crop((px, py, px + tw, py + th))
    return tile

def tile_to_rgb565_array(tile, varname, tw=16, th=16, transparent=0x0000):
    """Tile'ı RGB565 C++ const array'e çevir"""
    pixels = []
    for y in range(th):
        row = []
        for x in range(tw):
            r, g, b, a = tile.getpixel((x, y))
            if a < 128:  # Saydam piksel
                row.append(f"0x{transparent:04X}")
            else:
                c = rgb888_to_rgb565(r, g, b)
                row.append(f"0x{c:04X}")
        pixels.append(", ".join(row))
    
    lines = [f"static const uint16_t {varname}[{tw*th}] PROGMEM = {{"]
    for i, row in enumerate(pixels):
        comma = "," if i < th - 1 else ""
        lines.append(f"  {row}{comma}")
    lines.append("};")
    return "\n".join(lines)

def save_tile_preview(tile, path):
    """Tile'ı büyütülmüş şekilde kaydet (görsel kontrol için)"""
    big = tile.resize((tile.width * 4, tile.height * 4), Image.NEAREST)
    big.save(path)

# ============================================================
print("=== Sprite Analizi ===")
base = os.path.dirname(__file__)

overworld = image_info(os.path.join(base, "Overworld.png"))
character = image_info(os.path.join(base, "character.png"))
objects_img = image_info(os.path.join(base, "objects.png"))

ow_tiles_x = overworld.width // 16
ow_tiles_y = overworld.height // 16
print(f"  Overworld tile grid: {ow_tiles_x} x {ow_tiles_y} tiles (16x16 varsayımıyla)")

ch_tiles_x = character.width // 16
ch_tiles_y = character.height // 16
print(f"  Character grid (16x16): {ch_tiles_x} x {ch_tiles_y}")

# Önce her şeyi preview olarak kaydet
os.makedirs(os.path.join(base, "preview"), exist_ok=True)

# Overworld'den seçilecek tile'lar — koordinatlar (tx, ty)
# Bunlar visual incelemeyle belirlendi:
TILE_DEFS = {
    # Name              tx  ty   açıklama
    "tile_grass":      (0,  0),  # Sol üst köşe çimen
    "tile_grass2":     (1,  0),  # Çimen varyantı
    "tile_water":      (0,  1),  # Su
    "tile_water2":     (1,  1),  # Su animasyonu kare 2
    "tile_rock":       (2,  3),  # Kaya/taş
    "tile_tree_tl":    (0,  4),  # Ağaç sol üst
    "tile_tree_tr":    (1,  4),  # Ağaç sağ üst
    "tile_tree_bl":    (0,  5),  # Ağaç sol alt
    "tile_tree_br":    (1,  5),  # Ağaç sağ alt
    "tile_sand":       (4,  6),  # Kum/toprak
    "tile_path":       (5,  6),  # Patika
}

# Character sprites — 16x16 tile varsayımıyla
# Karakter genelde 16x24 veya 16x16 olabilir — kontrol ediyoruz
CHAR_DEFS = {
    "char_down_1":    (0,  0),
    "char_down_2":    (1,  0),
    "char_down_3":    (2,  0),
    "char_left_1":    (0,  1),
    "char_left_2":    (1,  1),
    "char_left_3":    (2,  1),
    "char_right_1":   (0,  2),
    "char_right_2":   (1,  2),
    "char_right_3":   (2,  2),
    "char_up_1":      (0,  3),
    "char_up_2":      (1,  3),
    "char_up_3":      (2,  3),
}

# Objects — kalpler, sandık vs
OBJ_DEFS = {
    "obj_heart_full":   (1,  0),
    "obj_heart_half":   (2,  0),
    "obj_heart_empty":  (3,  0),
    "obj_chest":        (4,  0),
    "obj_chest_open":   (5,  0),
    "obj_coin":         (0,  1),
}

print("\n=== Tile'ları çekiyor ve preview kaydediyor ===")

all_cpp = []
all_cpp.append("// ================================================")
all_cpp.append("// game_zelda_sprites.h — Otomatik üretildi")
all_cpp.append("// Overworld.png + character.png + objects.png'den")
all_cpp.append("// ================================================")
all_cpp.append("#pragma once")
all_cpp.append("#include <pgmspace.h>")
all_cpp.append("")
all_cpp.append("// --- DÜNYA TILE'LARI (16x16) ---")

for name, (tx, ty) in TILE_DEFS.items():
    try:
        tile = extract_tile(overworld, tx, ty, 16, 16)
        save_tile_preview(tile, os.path.join(base, "preview", f"{name}.png"))
        cpp = tile_to_rgb565_array(tile, name)
        all_cpp.append(cpp)
        all_cpp.append("")
        print(f"  OK: {name} ({tx},{ty})")
    except Exception as ex:
        print(f"  HATA: {name} — {ex}")

all_cpp.append("// --- KARAKTER SPRITE'LARI (16x16) ---")
for name, (tx, ty) in CHAR_DEFS.items():
    try:
        tile = extract_tile(character, tx, ty, 16, 16)
        save_tile_preview(tile, os.path.join(base, "preview", f"{name}.png"))
        cpp = tile_to_rgb565_array(tile, name)
        all_cpp.append(cpp)
        all_cpp.append("")
        print(f"  OK: {name} ({tx},{ty})")
    except Exception as ex:
        print(f"  HATA: {name} — {ex}")

all_cpp.append("// --- NESNE SPRITE'LARI (16x16) ---")
for name, (tx, ty) in OBJ_DEFS.items():
    try:
        tile = extract_tile(objects_img, tx, ty, 16, 16)
        save_tile_preview(tile, os.path.join(base, "preview", f"{name}.png"))
        cpp = tile_to_rgb565_array(tile, name)
        all_cpp.append(cpp)
        all_cpp.append("")
        print(f"  OK: {name} ({tx},{ty})")
    except Exception as ex:
        print(f"  HATA: {name} — {ex}")

# C++ header dosyasını yaz
out_path = os.path.join(os.path.dirname(base), "GameHub", "game_zelda_sprites.h")
with open(out_path, "w", encoding="utf-8") as f:
    f.write("\n".join(all_cpp))

print(f"\n✓ Tamamlandı! Çıktı: {out_path}")
print(f"✓ Tile preview'ları: {os.path.join(base, 'preview')}")
print("\nŞimdi preview klasöründeki resimleri kontrol et,")
print("yanlış tile'lar varsa koordinatları düzeltiriz.")
