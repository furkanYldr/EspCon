import tkinter as tk
from tkinter import colorchooser, messagebox
import re

# ==================== Renk Dönüşüm Fonksiyonları ====================
def rgb888_to_rgb565(r, g, b):
    """24-bit RGB rengini 16-bit RGB565 (ESP32 TFT formatı) hex koduna dönüştürür."""
    r5 = (r >> 3) & 0x1F
    g6 = (g >> 2) & 0x3F
    b5 = (b >> 3) & 0x1F
    val = (r5 << 11) | (g6 << 5) | b5
    return f"0x{val:04X}"

def rgb565_to_hex888(val_str):
    """16-bit RGB565 hex kodunu C++ formatından standart HTML hex rengine (#RRGGBB) dönüştürür."""
    try:
        val = int(val_str.strip(), 16) if isinstance(val_str, str) else val_str
        r = ((val >> 11) & 0x1F) << 3
        g = ((val >> 5) & 0x3F) << 2
        b = (val & 0x1F) << 3
        # Bit genişletme (siyah/beyaz doğruluğu için)
        r = r | (r >> 5)
        g = g | (g >> 6)
        b = b | (b >> 5)
        return f"#{r:02x}{g:02x}{b:02x}"
    except Exception:
        return "#000000"

# ==================== Ana Uygulama Sınıfı ====================
class SpriteEditor:
    def __init__(self, root):
        self.root = root
        self.root.title("GameHub RGB565 Sprite Editor")
        self.root.geometry("820x640")
        self.root.configure(bg="#1E1E1E")  # Premium Dark Mode

        # Grid Değişkenleri
        self.grid_size = 16  # Varsayılan 16x16
        self.cell_pixels = 24  # Her piksel hücresinin boyutu
        self.active_color = "#FF0000"  # Varsayılan aktif renk (Kırmızı)
        self.bg_color = "#000000"  # Varsayılan arka plan (Siyah)

        # Çizim Matrisi (Grid verisi)
        self.pixel_data = {}  # {(col, row): hex_color}
        self.init_pixel_data()

        # Hazır Renk Paleti (RGB565 retro renkleri)
        self.palette_colors = [
            "#000000", "#FFFFFF", "#FF0000", "#00FF00", 
            "#0000FF", "#FFFF00", "#FF00FF", "#00FFFF",
            "#FFA500", "#800080", "#808080", "#4B0082",
            "#A52A2A", "#008080", "#FFD700", "#ADFF2F"
        ]

        # UI Elemanlarını Oluştur
        self.setup_ui()
        self.draw_grid()

    def init_pixel_data(self):
        """Çizim matrisini arka plan rengiyle sıfırlar."""
        self.pixel_data.clear()
        for r in range(self.grid_size):
            for c in range(self.grid_size):
                self.pixel_data[(c, r)] = self.bg_color

    def setup_ui(self):
        # Üst Panel: Ayarlar
        top_frame = tk.Frame(self.root, bg="#2D2D2D", height=50)
        top_frame.pack(fill=tk.X, side=tk.TOP)

        tk.Label(top_frame, text="Grid Size:", fg="white", bg="#2D2D2D", font=("Arial", 10, "bold")).pack(side=tk.LEFT, padx=10)
        
        self.size_var = tk.StringVar(value="16x16")
        size_combo = tk.OptionMenu(top_frame, self.size_var, "8x8", "10x10", "12x12", "16x16", "24x24", "32x32", command=self.on_size_change)
        size_combo.config(bg="#3E3E3E", fg="white", activebackground="#4E4E4E", activeforeground="white", highlightthickness=0)
        size_combo.pack(side=tk.LEFT, padx=5)

        clear_btn = tk.Button(top_frame, text="Clear Grid", command=self.clear_grid, bg="#D9534F", fg="white", font=("Arial", 9, "bold"), relief=tk.FLAT)
        clear_btn.pack(side=tk.LEFT, padx=15)

        # Sol Panel: Çizim Alanı (Canvas)
        self.canvas_frame = tk.Frame(self.root, bg="#1E1E1E")
        self.canvas_frame.pack(side=tk.LEFT, padx=20, pady=20, fill=tk.BOTH, expand=True)

        self.canvas = tk.Canvas(self.canvas_frame, bg="#000000", highlightthickness=1, highlightbackground="#3E3E3E")
        self.canvas.pack(fill=tk.BOTH, expand=True)

        # Fare Sürükleme ve Tıklama Olayları
        self.canvas.bind("<Button-1>", self.on_canvas_click)
        self.canvas.bind("<B1-Motion>", self.on_canvas_drag)
        self.canvas.bind("<Button-3>", self.on_canvas_right_click)  # Sağ tık damlalık

        # Sağ Panel: Araçlar ve Renk Seçimi
        right_frame = tk.Frame(self.root, bg="#252526", width=340)
        right_frame.pack(side=tk.RIGHT, fill=tk.Y, padx=10, pady=10)
        right_frame.pack_propagate(False)

        # Renk Seçici Butonu
        tk.Label(right_frame, text="COLOR PALETTE", fg="white", bg="#252526", font=("Arial", 11, "bold")).pack(pady=10)
        
        self.color_preview = tk.Frame(right_frame, bg=self.active_color, height=35, width=120, highlightthickness=1, highlightbackground="white")
        self.color_preview.pack(pady=5)
        
        choose_color_btn = tk.Button(right_frame, text="Custom Color...", command=self.choose_custom_color, bg="#007ACC", fg="white", relief=tk.FLAT, font=("Arial", 10))
        choose_color_btn.pack(pady=5)

        # Hazır Renk Paleti Grid'i
        palette_frame = tk.Frame(right_frame, bg="#252526")
        palette_frame.pack(pady=10)

        for i, color in enumerate(self.palette_colors):
            row = i // 4
            col = i % 4
            btn = tk.Button(palette_frame, bg=color, width=4, height=1, relief=tk.FLAT, command=lambda c=color: self.set_active_color(c))
            btn.grid(row=row, column=col, padx=4, pady=4)

        # Bilgi Etiketi
        info_lbl = tk.Label(right_frame, text="Left Click = Draw  |  Right Click = Pipette Color", fg="#8C8C8C", bg="#252526", font=("Arial", 8))
        info_lbl.pack(pady=5)

        tk.Frame(right_frame, bg="#3E3E3E", height=1).pack(fill=tk.X, pady=10)

        # C++ KOD PANELİ
        tk.Label(right_frame, text="C++ ARRAY OUTPUT", fg="white", bg="#252526", font=("Arial", 10, "bold")).pack(anchor=tk.W, padx=15, pady=2)
        
        self.code_text = tk.Text(right_frame, height=12, width=38, bg="#1E1E1E", fg="#9CDCFE", font=("Consolas", 9), insertbackground="white")
        self.code_text.pack(padx=15, pady=5)

        # Butonlar
        btn_frame = tk.Frame(right_frame, bg="#252526")
        btn_frame.pack(fill=tk.X, padx=15, pady=5)

        gen_btn = tk.Button(btn_frame, text="Generate Code", command=self.generate_cpp_code, bg="#4B5F43", fg="white", relief=tk.FLAT, font=("Arial", 9, "bold"))
        gen_btn.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=2)

        copy_btn = tk.Button(btn_frame, text="Copy Code", command=self.copy_to_clipboard, bg="#007ACC", fg="white", relief=tk.FLAT, font=("Arial", 9, "bold"))
        copy_btn.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=2)

        import_btn = tk.Button(right_frame, text="Import Array Code", command=self.import_cpp_code, bg="#D9534F", fg="white", relief=tk.FLAT, font=("Arial", 9, "bold"))
        import_btn.pack(fill=tk.X, padx=17, pady=5)

    def draw_grid(self):
        """Canvas üzerinde çizim ızgarasını çizer."""
        self.canvas.delete("all")
        w = self.canvas.winfo_width()
        h = self.canvas.winfo_height()
        
        # Grid boyutuna göre hücre büyüklüğünü otomatik ayarla
        self.cell_pixels = min(w // self.grid_size, h // self.grid_size)
        if self.cell_pixels < 4: self.cell_pixels = 4

        x_offset = (w - (self.grid_size * self.cell_pixels)) // 2
        y_offset = (h - (self.grid_size * self.cell_pixels)) // 2

        self.canvas_x_offset = x_offset
        self.canvas_y_offset = y_offset

        # Hücreleri doldur
        for (col, row), color in self.pixel_data.items():
            x1 = x_offset + col * self.cell_pixels
            y1 = y_offset + row * self.cell_pixels
            x2 = x1 + self.cell_pixels
            y2 = y1 + self.cell_pixels
            self.canvas.create_rectangle(x1, y1, x2, y2, fill=color, outline="#333333", tags=f"pixel_{col}_{row}")

    def on_size_change(self, val):
        """Dropdown ile ızgara boyutu değiştirildiğinde tetiklenir."""
        new_size = int(val.split("x")[0])
        if new_size != self.grid_size:
            self.grid_size = new_size
            self.init_pixel_data()
            # Canvas'ın güncel boyut alabilmesi için ufak gecikmeli çizdiriyoruz
            self.root.update()
            self.draw_grid()

    def set_active_color(self, color):
        """Kullanıcının aktif kalem rengini ayarlar."""
        self.active_color = color
        self.color_preview.configure(bg=color)

    def choose_custom_color(self):
        """Renk seçici pencerisini açar."""
        color = colorchooser.askcolor(color=self.active_color, title="Choose Drawing Color")
        if color[1]:
            self.set_active_color(color[1])

    def paint_pixel(self, x, y):
        """Belirtilen ekran koordinatına denk gelen pikseli boyar."""
        # Canvas offsetlerini hesaba kat
        col = (x - self.canvas_x_offset) // self.cell_pixels
        row = (y - self.canvas_y_offset) // self.cell_pixels

        if 0 <= col < self.grid_size and 0 <= row < self.grid_size:
            self.pixel_data[(col, row)] = self.active_color
            x1 = self.canvas_x_offset + col * self.cell_pixels
            y1 = self.canvas_y_offset + row * self.cell_pixels
            x2 = x1 + self.cell_pixels
            y2 = y1 + self.cell_pixels
            
            # Canvas üzerindeki eski hücreyi silip yeniden boya
            self.canvas.delete(f"pixel_{col}_{row}")
            self.canvas.create_rectangle(x1, y1, x2, y2, fill=self.active_color, outline="#333333", tags=f"pixel_{col}_{row}")

    def pipette_pixel(self, x, y):
        """Tıklanan pikselin rengini damlalıkla kopyalar."""
        col = (x - self.canvas_x_offset) // self.cell_pixels
        row = (y - self.canvas_y_offset) // self.cell_pixels

        if 0 <= col < self.grid_size and 0 <= row < self.grid_size:
            color = self.pixel_data.get((col, row), "#000000")
            self.set_active_color(color)

    def on_canvas_click(self, event):
        self.paint_pixel(event.x, event.y)

    def on_canvas_drag(self, event):
        self.paint_pixel(event.x, event.y)

    def on_canvas_right_click(self, event):
        self.pipette_pixel(event.x, event.y)

    def clear_grid(self):
        """Tüm ızgarayı arka plan rengiyle temizler."""
        if messagebox.askyesno("Confirm Clear", "Clear drawing and reset grid?"):
            self.init_pixel_data()
            self.draw_grid()

    # ==================== C++ Array KOD ÜRETİCİ ====================
    def generate_cpp_code(self):
        """Matristeki pikselleri RGB565'e çevirip C++ dizisi formatında üretir."""
        lines = []
        name = f"custom_sprite_{self.grid_size}x{self.grid_size}"
        lines.append(f"// GameHub RGB565 Pixel Art Array")
        lines.append(f"const uint16_t {name}[{self.grid_size * self.grid_size}] PROGMEM = {{")
        
        array_values = []
        for r in range(self.grid_size):
            row_vals = []
            for c in range(self.grid_size):
                color = self.pixel_data.get((c, r), "#000000")
                # RGB HTML Hex'i RGB888'e çevir
                hr = int(color[1:3], 16)
                hg = int(color[3:5], 16)
                hb = int(color[5:7], 16)
                row_vals.append(rgb888_to_rgb565(hr, hg, hb))
            array_values.append("  " + ", ".join(row_vals))
            
        lines.append(",\n".join(array_values))
        lines.append("};")
        
        code = "\n".join(lines)
        self.code_text.delete("1.0", tk.END)
        self.code_text.insert(tk.END, code)

    def copy_to_clipboard(self):
        """Üretilen C++ kodunu panoya kopyalar."""
        code = self.code_text.get("1.0", tk.END).strip()
        if code:
            self.root.clipboard_clear()
            self.root.clipboard_append(code)
            messagebox.showinfo("Success", "C++ code copied to clipboard!")
        else:
            messagebox.showwarning("Warning", "Generate C++ code first!")

    # ==================== C++ Array KOD İTHALATÇI (IMPORT) ====================
    def import_cpp_code(self):
        """Metin kutusundaki C++ dizisini çözerek çizim alanına geri yükler."""
        raw_code = self.code_text.get("1.0", tk.END).strip()
        if not raw_code:
            messagebox.showwarning("Warning", "Paste C++ code array into the text box first!")
            return

        # 0xXXXX formatındaki hex değerlerini bul
        hex_values = re.findall(r"0x[0-9a-fA-F]{4}", raw_code)
        if not hex_values:
            messagebox.showerror("Error", "No valid RGB565 hex values (e.g. 0xF800) found in the text!")
            return

        # Uygun grid boyutunu tahmin et veya uyar
        length = len(hex_values)
        side = int(math.sqrt(length)) if 'math' in globals() else int(length ** 0.5)

        if side * side != length:
            messagebox.showerror("Error", f"Array size ({length}) is not a perfect square (e.g. 16x16=256)! Cannot load.")
            return

        if side not in [8, 10, 12, 16, 24, 32]:
            messagebox.showwarning("Warning", f"Loaded size is {side}x{side}. Standard combo grid sizes are 8, 10, 12, 16, 24, 32.")

        # Grid boyutunu ve dropdown'u güncelle
        self.grid_size = side
        self.size_var.set(f"{side}x{side}")
        self.init_pixel_data()

        # Hex değerlerini HTML renklerine çevirip matrise yaz
        for idx, hex_val in enumerate(hex_values):
            col = idx % side
            row = idx // side
            html_color = rgb565_to_hex888(hex_val)
            self.pixel_data[(col, row)] = html_color

        self.draw_grid()
        messagebox.showinfo("Success", f"Successfully imported {side}x{side} sprite!")

# ==================== Uygulama Başlatıcı ====================
if __name__ == "__main__":
    # Math modülü import'unu garantile
    import math
    root = tk.Tk()
    app = SpriteEditor(root)
    
    # Canvas'ın gerçek ekran boyutu alındıktan sonra ızgarayı çizmesi için bind yap
    root.bind("<Configure>", lambda e: app.draw_grid())
    
    root.mainloop()
