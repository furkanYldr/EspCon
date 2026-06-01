"""
FINAL Sprite Extractor — Doğru koordinatlarla tam dönüştürme
Overworld.png, character.png, objects.png -> game_zelda_sprites.h
"""
from PIL import Image
import os, sys

base = os.path.dirname(__file__)

def rgb888_to_rgb565(r, g, b):
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)

def extract_tile(img, tx, ty, tw=16, th=16):
    px, py = tx * tw, ty * th
    return img.crop((px, py, px + tw, py + th))

def tile_to_rgb565_cpp(tile, varname, tw=16, th=16, transparent=0x0821):
    """Tile -> C++ const uint16_t array (PROGMEM)"""
    lines = [f"static const uint16_t {varname}[{tw*th}] PROGMEM = {{"]
    for y in range(th):
        row = []
        for x in range(tw):
            r, g, b, a = tile.getpixel((x, y))
            if a < 64:
                row.append(f"0x{transparent:04X}")
            else:
                c = rgb888_to_rgb565(r, g, b)
                row.append(f"0x{c:04X}")
        comma = "," if y < th - 1 else ""
        lines.append("  " + ", ".join(row) + comma)
    lines.append("};")
    return "\n".join(lines)

def extract_char_16x24(img, col, row_start):
    """Karakterin 16x24 sprite'ini iki satir olarak birlestir"""
    top = extract_tile(img, col, row_start, 16, 16)
    bot = extract_tile(img, col, row_start + 1, 16, 16)
    combined = Image.new("RGBA", (16, 24))
    combined.paste(top, (0, 0))
    combined.paste(bot, (0, 16))
    return combined

def tile_24_to_rgb565_cpp(tile, varname, transparent=0x0821):
    """16x24 tile -> C++ array"""
    lines = [f"static const uint16_t {varname}[{16*24}] PROGMEM = {{"]
    for y in range(24):
        row = []
        for x in range(16):
            r, g, b, a = tile.getpixel((x, y))
            if a < 64:
                row.append(f"0x{transparent:04X}")
            else:
                c = rgb888_to_rgb565(r, g, b)
                row.append(f"0x{c:04X}")
        comma = "," if y < 23 else ""
        lines.append("  " + ", ".join(row) + comma)
    lines.append("};")
    return "\n".join(lines)

def save_preview(tile, name):
    big = tile.resize((tile.width * 4, tile.height * 4), Image.NEAREST)
    big.save(os.path.join(base, "preview2", f"{name}.png"))

os.makedirs(os.path.join(base, "preview2"), exist_ok=True)

print("Loading images...")
ow  = Image.open(os.path.join(base, "Overworld.png")).convert("RGBA")
obj = Image.open(os.path.join(base, "objects.png")).convert("RGBA")
ch  = Image.open(os.path.join(base, "character.png")).convert("RGBA")

cpp_blocks = []
cpp_blocks.append("""\
// ============================================================
// game_zelda_sprites.h  —  Otomatik uretildi
// ESP32 GameHub Zelda oyunu icin RGB565 sprite veri bankasi
// Transparent renk: 0x0821 (cok koyu lacivert = seffaf)
// KULLANIM: img.drawBitmap(x, y, tile_grass, 16, 16, 0xFFFF, 0x0821)
// VEYA: elle cizim icin for(int i=0;i<256;i++) pixel'i kontrol et
// ============================================================
#pragma once
#include <pgmspace.h>

// Transparan renk sabiti
#define ZELDA_TRANSPARENT 0x0821

// Tile/sprite boyutlari
static const int Z_TILE_W  = 16;
static const int Z_TILE_H  = 16;
static const int Z_CHAR_H  = 24;   // Karakter spritesi 16x24
""")

# ============================================================
# OVERWORLD TILE'LARI (16x16)
# Koordinatlar grid_preview/overworld_grid.png'den belirlendi
# ============================================================
cpp_blocks.append("// =========== HARITA TILE'LARI (16x16) ===========")

ow_tiles = {
    # name          tx  ty    aciklama
    "tile_grass":  (0,  0),   # Temel cimen
    "tile_grass2": (1,  0),   # Cimen varyant (golgeli)  
    "tile_water":  (0,  1),   # Su kare 1
    "tile_water2": (0,  2),   # Su kare 2 (animasyon)
    "tile_water3": (0,  3),   # Su kare 3
    "tile_tree_tl":(0,  6),   # Agac sol-ust
    "tile_tree_tr":(1,  6),   # Agac sag-ust
    "tile_tree_bl":(0,  7),   # Agac sol-alt
    "tile_tree_br":(1,  7),   # Agac sag-alt
    "tile_rock1":  (10, 8),   # Buyuk kaya sol
    "tile_rock2":  (11, 8),   # Buyuk kaya sag
    "tile_sand":   (15, 6),   # Cim/kumsal gecis
    "tile_sand2":  (15, 7),   # Kumsal
    "tile_path":   (0,  11),  # Toprak patika
    "tile_path2":  (1,  11),  # Patika varyant
}

for name, (tx, ty) in ow_tiles.items():
    t = extract_tile(ow, tx, ty, 16, 16)
    save_preview(t, name)
    cpp_blocks.append(tile_to_rgb565_cpp(t, name))
    cpp_blocks.append("")
    print(f"  OK overworld: {name} ({tx},{ty})")

# ============================================================
# OBJECTS: KALPLAR, SANDIK, ALEV, PARA
# Koordinatlar grid_preview/objects_grid.png'den
# ============================================================
cpp_blocks.append("// =========== NESNE SPRITE'LARI (16x16) ===========")

obj_tiles = {
    "spr_chest":       (0,  0),   # Kapali sandik
    "spr_chest2":      (1,  0),   # Kapali sandik 2
    "spr_chest_open":  (0,  1),   # Acik sandik (ust)
    "spr_heart_full":  (3,  0),   # Tam kalp
    "spr_heart_half":  (4,  0),   # Yari kalp
    "spr_heart_empty": (5,  0),   # Bos kalp
    "spr_coin1":       (0,  4),   # Altin para 1
    "spr_coin2":       (1,  4),   # Para 2
    "spr_fire1":       (4,  2),   # Alev kare 1
    "spr_fire2":       (5,  2),   # Alev kare 2
}

for name, (tx, ty) in obj_tiles.items():
    t = extract_tile(obj, tx, ty, 16, 16)
    save_preview(t, name)
    cpp_blocks.append(tile_to_rgb565_cpp(t, name))
    cpp_blocks.append("")
    print(f"  OK objects: {name} ({tx},{ty})")

# ============================================================
# KARAKTER SPRITE'LARI (16x24) — 4 yon x 3 kare animasyon
# Gozlemler (character_grid.png):
#   Row 0-1: ASAGI yuruyus (4 karakter tipi yan yana)
#   Row 2-3: YAN (sol/sag)
#   Row 4-5: YUKARI yuruyus  
# ============================================================
cpp_blocks.append("// =========== KARAKTER SPRITE'LARI (16x24) ===========")
cpp_blocks.append("// Kullanim: img.pushImage(x, y, 16, 24, char_down_1, ZELDA_TRANSPARENT)")
cpp_blocks.append("")

# Her yonde 3 kare: col 0,1,2 = bekleme, sol adim, sag adim
char_tiles = {
    # name              col row_start  aciklama
    "char_down_1":     (0,  0),   # Asagi bak, bekleme
    "char_down_2":     (1,  0),   # Asagi yuruyus kare 1
    "char_down_3":     (2,  0),   # Asagi yuruyus kare 2
    "char_side_1":     (0,  2),   # Yan bak (sag)
    "char_side_2":     (1,  2),   # Yan yuruyus 1
    "char_side_3":     (2,  2),   # Yan yuruyus 2
    "char_up_1":       (0,  4),   # Yukari bak
    "char_up_2":       (1,  4),   # Yukari yuruyus 1
    "char_up_3":       (2,  4),   # Yukari yuruyus 2
    # Kirpma animasyonlari (satir 6-7)
    "char_idle_1":     (0,  6),   # Bosluk/idle
    "char_idle_2":     (1,  6),   # Idle 2
    # Kilic vurusu (satir 8-9)
    "char_sword_down": (0,  8),   # Asagi vuruyor
    "char_sword_side": (0, 10),   # Yana vuruyor
    "char_sword_up":   (0, 12),   # Yukari vuruyor
}

for name, (col, row_start) in char_tiles.items():
    t = extract_char_16x24(ch, col, row_start * 2)
    save_preview(t, name)
    cpp_blocks.append(tile_24_to_rgb565_cpp(t, name))
    cpp_blocks.append("")
    print(f"  OK character: {name} ({col},{row_start})")

# ============================================================
# Yardimci drawSprite fonksiyonu
# ============================================================
cpp_blocks.append("""\
// =========== YARDIMCI FONKSIYONLAR ===========

// 16x16 tile ciz (transparan piksel atla)
static void zDrawTile(int sx, int sy, const uint16_t* data) {
  for (int y = 0; y < 16; y++) {
    for (int x = 0; x < 16; x++) {
      uint16_t c = pgm_read_word(&data[y * 16 + x]);
      if (c != ZELDA_TRANSPARENT)
        img.drawPixel(sx + x, sy + y, c);
    }
  }
}

// 16x24 karakter ciz (transparan piksel atla)
static void zDrawChar(int sx, int sy, const uint16_t* data) {
  for (int y = 0; y < 24; y++) {
    for (int x = 0; x < 16; x++) {
      uint16_t c = pgm_read_word(&data[y * 16 + x]);
      if (c != ZELDA_TRANSPARENT)
        img.drawPixel(sx + x, sy + y, c);
    }
  }
}

// Tile'i zemine doldurarak ciz (tam doldurma, transparan yok)
static void zFillTile(int sx, int sy, const uint16_t* data) {
  for (int y = 0; y < 16; y++) {
    for (int x = 0; x < 16; x++) {
      img.drawPixel(sx + x, sy + y, pgm_read_word(&data[y * 16 + x]));
    }
  }
}
""")

out_path = os.path.join(os.path.dirname(base), "GameHub", "game_zelda_sprites.h")
with open(out_path, "w", encoding="utf-8") as f:
    f.write("\n".join(cpp_blocks))

print(f"\nBasarili! {out_path}")
kb = os.path.getsize(out_path) / 1024
print(f"Dosya boyutu: {kb:.1f} KB")
