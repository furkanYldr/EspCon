"""
KESIN FINAL — Dogru piksel koordinatlariyla tam donusum
Tum sprite'lar 16x16 veya 16x24 olarak ESP32 icin hazirlanir
"""
from PIL import Image
import os

base = os.path.dirname(__file__)
ow  = Image.open(os.path.join(base, "Overworld.png")).convert("RGBA")
obj = Image.open(os.path.join(base, "objects.png")).convert("RGBA")
ch  = Image.open(os.path.join(base, "character.png")).convert("RGBA")

def rgb565(r, g, b): return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
TRANS = 0x0821

def crop(img, x, y, w, h):    return img.crop((x, y, x+w, y+h))
def ow_tile(tx, ty):           return crop(ow, tx*16, ty*16, 16, 16)
def obj_tile(tx, ty, tw=16, th=16): return crop(obj, tx, ty, tw, th)

def to_cpp(tile, name):
    w, h = tile.size
    lines = [f"static const uint16_t {name}[{w*h}] PROGMEM = {{"]
    for y in range(h):
        row = []
        for x in range(w):
            r, g, b, a = tile.getpixel((x, y))
            c = TRANS if a < 64 else rgb565(r, g, b)
            row.append(f"0x{c:04X}")
        comma = "," if y < h-1 else ""
        lines.append("  " + ", ".join(row) + comma)
    lines.append("};")
    return "\n".join(lines)

def ch_sprite(tx, ty):
    """Karakterin 16x32 spritesi (2 satir = 32px yukseklik)"""
    return crop(ch, tx*16, ty*16, 16, 32)

def save_prev(tile, name):
    p = os.path.join(base, "preview_final")
    os.makedirs(p, exist_ok=True)
    tile.resize((tile.width*4, tile.height*4), Image.NEAREST).save(os.path.join(p, f"{name}.png"))

blocks = []
blocks.append("""\
// ============================================================
// game_zelda_sprites.h — Otomatik uretildi (FINAL)
// Tum sprite'lar RGB565 formatinda, PROGMEM'de
// Transparan: 0x0821 | Tile boyutu: 16x16 | Karakter: 16x32
// ============================================================
#pragma once
#include <pgmspace.h>
#include <TFT_eSPI.h>

#define ZELDA_TRANSPARENT 0x0821
#define Z_TILE  16
#define Z_CHAR_W 16
#define Z_CHAR_H 32
""")

# ============================================================
# OVERWORLD TILE'LARI — Kesin koordinatlar (grid_preview incelendi)
# ============================================================
blocks.append("// ===== DUNYA TILE'LARI (16x16) =====\n")

# Temel zemin tile'lari
tiles_ow = [
    ("tile_grass",   0,  0),   # Cimen (temel)
    ("tile_grass2",  1,  0),   # Cimen karanlik
    ("tile_water_a", 0,  1),   # Su animasyon A
    ("tile_water_b", 0,  2),   # Su animasyon B
    ("tile_water_c", 0,  3),   # Su animasyon C
    ("tile_tree_tl", 0,  6),   # Agac sol-ust
    ("tile_tree_tr", 1,  6),   # Agac sag-ust
    ("tile_tree_bl", 0,  7),   # Agac sol-alt
    ("tile_tree_br", 1,  7),   # Agac sag-alt
    ("tile_sand",    15, 6),   # Kumlu zemin
    ("tile_sand2",   15, 7),   # Kum 2
    ("tile_dirt",    0,  11),  # Toprak patika
    ("tile_dirt2",   1,  11),  # Toprak 2
    ("tile_grass_l", 0,  4),   # Cimen-su sol kenar
    ("tile_grass_r", 1,  4),   # Cimen-su sag kenar
]

for name, tx, ty in tiles_ow:
    t = ow_tile(tx, ty)
    save_prev(t, name)
    blocks.append(to_cpp(t, name))
    blocks.append("")
    print(f"  ow: {name}")

# ============================================================
# KAYALAR — Objects.png'deki buyuk kayalar (32x24 -> 16x16'ya olceklendir)
# Veya overworld'deki kaya tile
# ============================================================
# Overworld'de gri kaya tile var mi? Row 5, col 5 bolge
# grid_preview'da (5,2) gri tash gorunuyor
blocks.append("// ===== KAYA TILE'LARI =====\n")
rock_tiles = [
    ("tile_rock",   5,  2),   # Kaya/tas tile
    ("tile_rock2",  6,  2),   # Kaya 2
    ("tile_wall",   4,  2),   # Duvar taslari
]
for name, tx, ty in rock_tiles:
    t = ow_tile(tx, ty)
    save_prev(t, name)
    blocks.append(to_cpp(t, name))
    blocks.append("")
    print(f"  rock: {name}")

# ============================================================
# OBJECTS.PNG — Piksel bazli crop
# Inceleme sonucu:
#   Row 0 (y=0): Sandik(x=0,w=32), Yaprak(x=32), Kalpler(x=64..144, ~16px aralik)
#                Sandik2(x=144), Pot(x=192..208)
#   Row 2 (y=48): Kucuk kalplar (kirmizi), Alevler
#   Row 3 (y=64): Paralar
# ============================================================
blocks.append("// ===== NESNE SPRITE'LARI (16x16) =====\n")

# Objects'te kalplar 14x14 gibi, 16 aralikla
# Gorsel: col4=tam kalp, col5=yari kalp, col6=1/4 kalp, col7=bos
obj_sprites = [
    # name                x    y   w   h   aciklama
    ("spr_chest",         0,   0,  16, 16),  # Kapali sandik
    ("spr_chest_open",    0,  16,  16, 16),  # Acik sandik
    ("spr_heart_full",   64,   0,  16, 16),  # Tam kalp
    ("spr_heart_half",   80,   0,  16, 16),  # Yari kalp
    ("spr_heart_empty",  96,   0,  16, 16),  # Bos kalp
    ("spr_coin",          0,  64,  16, 16),  # Altin para
    ("spr_coin2",        16,  64,  16, 16),  # Para 2
    ("spr_fire_a",       64,  48,  16, 16),  # Alev A
    ("spr_fire_b",       80,  48,  16, 16),  # Alev B
]

for name, x, y, w, h in obj_sprites:
    t = obj_tile(x, y, w, h)
    # ESP32 icin 16x16'ya kes/olcekle
    if t.size != (16, 16):
        t = t.resize((16, 16), Image.NEAREST)
    save_prev(t, name)
    blocks.append(to_cpp(t, name))
    blocks.append("")
    print(f"  obj: {name}")

# ============================================================
# KARAKTER SPRITE'LARI — character.png
# Grid analizi: 16x16 grid, her karakter 2 satir yuksekliginde
# Row 0-1: Asagi yuruyus (col 0,1,2,3 = bos, adim1, adim2, idle)  
# Row 2-3: Yana bakis
# Row 4-5: Yukari
# Row 6-7: Idle/durma pozisyonu
# Row 8-9: Kilic asagi
# Row 10-11: Kilic yana
# Row 12-13: Kilic yukari
# Row 14-15: Kilic arka (yukari saldiri)
# ============================================================
blocks.append("// ===== KARAKTER SPRITE'LARI (16x32) =====\n")
blocks.append("// Her sprite 16x32 piksel (2x16px satir birlestirme)")
blocks.append("")

char_sprites = [
    # name             col  row  aciklama
    ("char_down_1",     0,   0),  # Asagi, duruyor
    ("char_down_2",     1,   0),  # Asagi, sol adim
    ("char_down_3",     2,   0),  # Asagi, sag adim
    ("char_down_4",     3,   0),  # Asagi, sol adim 2
    ("char_side_1",     0,   2),  # Yana, duruyor (sag)
    ("char_side_2",     1,   2),  # Yana, adim1
    ("char_side_3",     2,   2),  # Yana, adim2
    ("char_side_4",     3,   2),  # Yana, adim3
    ("char_up_1",       0,   4),  # Yukari, duruyor
    ("char_up_2",       1,   4),  # Yukari, adim1
    ("char_up_3",       2,   4),  # Yukari, adim2
    ("char_up_4",       3,   4),  # Yukari, adim3
    ("char_sword_down", 0,   8),  # Kilic asagi
    ("char_sword_side", 0,  10),  # Kilic yana
    ("char_sword_up",   0,  12),  # Kilic yukari
]

for name, col, row in char_sprites:
    t = ch_sprite(col, row)
    save_prev(t, name)
    blocks.append(to_cpp(t, name))
    blocks.append("")
    print(f"  char: {name}")

# ============================================================
# YARDIMCI FONKSIYONLAR (extern img varsayimi)
# ============================================================
blocks.append("""\
// ===== YARDIMCI CIZIM FONKSIYONLARI =====

extern TFT_eSprite img;

// 16x16 tile ciz (transparent piksel atla)
inline void zDrawTile(int sx, int sy, const uint16_t* data) {
  for (int y = 0; y < 16; y++) {
    for (int x = 0; x < 16; x++) {
      uint16_t c = pgm_read_word(&data[y * 16 + x]);
      if (c != ZELDA_TRANSPARENT)
        img.drawPixel(sx + x, sy + y, c);
    }
  }
}

// 16x16 tile tam doldurarak ciz (zemin tile icin)
inline void zFillTile(int sx, int sy, const uint16_t* data) {
  for (int y = 0; y < 16; y++)
    for (int x = 0; x < 16; x++)
      img.drawPixel(sx + x, sy + y, pgm_read_word(&data[y * 16 + x]));
}

// 16x32 karakter sprite ciz (transparent atla, yatay flip destegi)
inline void zDrawChar(int sx, int sy, const uint16_t* data, bool flipH = false) {
  for (int y = 0; y < 32; y++) {
    for (int x = 0; x < 16; x++) {
      uint16_t c = pgm_read_word(&data[y * 16 + (flipH ? 15-x : x)]);
      if (c != ZELDA_TRANSPARENT)
        img.drawPixel(sx + x, sy + y, c);
    }
  }
}
""")

out = os.path.join(os.path.dirname(base), "GameHub", "game_zelda_sprites.h")
with open(out, "w", encoding="utf-8") as f:
    f.write("\n".join(blocks))

kb = os.path.getsize(out) / 1024
print(f"\nKaydedildi: {out}")
print(f"Boyut: {kb:.1f} KB")
