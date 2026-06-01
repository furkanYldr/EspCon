"""
Overworld.png'yi 16x16 grid olarak analiz et — hangi tile nerede?
Her tile'ı preview klasörüne row_col.png olarak kaydet
"""
from PIL import Image, ImageDraw, ImageFont
import os

base = os.path.dirname(__file__)
overworld = Image.open(os.path.join(base, "Overworld.png")).convert("RGBA")
objects_img = Image.open(os.path.join(base, "objects.png")).convert("RGBA")
character = Image.open(os.path.join(base, "character.png")).convert("RGBA")

os.makedirs(os.path.join(base, "grid_preview"), exist_ok=True)

def make_grid_preview(img, prefix, tw=16, th=16, max_cols=20, max_rows=10):
    cols = min(img.width // tw, max_cols)
    rows = min(img.height // th, max_rows)
    
    # Annotated grid image
    cell_size = 64  # her tile 64x64 olsun
    grid_w = cols * (cell_size + 1)
    grid_h = rows * (cell_size + 1)
    grid = Image.new("RGBA", (grid_w, grid_h), (40, 40, 40, 255))
    
    for r in range(rows):
        for c in range(cols):
            px = c * tw
            py = r * th
            tile = img.crop((px, py, px+tw, py+th))
            tile_big = tile.resize((cell_size, cell_size), Image.NEAREST)
            gx = c * (cell_size + 1)
            gy = r * (cell_size + 1)
            grid.paste(tile_big, (gx, gy))
    
    # Koordinat grid çizgisi
    draw = ImageDraw.Draw(grid)
    for r in range(rows):
        for c in range(cols):
            gx = c * (cell_size + 1)
            gy = r * (cell_size + 1)
            draw.rectangle([gx, gy, gx+cell_size, gy+cell_size], outline=(255,255,0,180))
            draw.text((gx+2, gy+2), f"{c},{r}", fill=(255,255,0,220))
    
    out = os.path.join(base, "grid_preview", f"{prefix}_grid.png")
    grid.save(out)
    print(f"Grid saved: {out} ({cols}x{rows} tiles)")
    return cols, rows

print("=== Grid Preview Olusturuluyor ===")
make_grid_preview(overworld, "overworld", 16, 16, max_cols=20, max_rows=12)
make_grid_preview(objects_img, "objects", 16, 16, max_cols=20, max_rows=8)
make_grid_preview(character, "character", 16, 16, max_cols=17, max_rows=16)
print("Tamamlandi! grid_preview klasorune bak.")
