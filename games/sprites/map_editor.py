"""
Zelda Map Editor v3 — ESP32 GameHub
Düzeltmeler:
  - Tile seçiminde kendiliğinden değişme BUG düzeltildi (event propagation bloğu)
  - Sprite kesici → ayrı büyük Toplevel pencere (scroll+zoom+snap)
  - Sabit snap grid (16x16, 32x32, özel) 
  - Animasyon kareleri düzenleme (her kare için sprite seç)
  - Alpha compositing (şeffaf PNG, siyah yok)
  - Ayarlanabilir harita boyutu

pip install pillow
"""

import tkinter as tk
from tkinter import ttk, filedialog, messagebox
import json, os, time, threading, re, copy
from PIL import Image, ImageTk, ImageDraw

BASE_DIR    = os.path.dirname(os.path.abspath(__file__))
GAMEHUB_DIR = os.path.join(os.path.dirname(BASE_DIR), "GameHub")
SAVE_FILE   = os.path.join(BASE_DIR, "map_editor_save.json")
TILE_W = TILE_H = 16

DEFAULT_TILE_DEFS = [
    {"id":0, "name":"Çimen",      "code":"T_GRASS",   "sheet":"Overworld.png","sx":0,"sy":0,"sw":16,"sh":16,"base":-1,"anim":[]},
    {"id":1, "name":"Ağaç",       "code":"T_TREE",    "sheet":"Overworld.png","sx":0,"sy":96,"sw":16,"sh":16,"base":0,"anim":[]},
    {"id":2, "name":"Kaya",       "code":"T_ROCK",    "sheet":"Overworld.png","sx":80,"sy":128,"sw":16,"sh":16,"base":0,"anim":[]},
    {"id":3, "name":"Su",         "code":"T_WATER",   "sheet":"Overworld.png","sx":0,"sy":16,"sw":16,"sh":16,"base":-1,"anim":[
        {"sheet":"Overworld.png","sx":0,"sy":16,"sw":16,"sh":16},
        {"sheet":"Overworld.png","sx":0,"sy":32,"sw":16,"sh":16},
        {"sheet":"Overworld.png","sx":0,"sy":48,"sw":16,"sh":16},
    ]},
    {"id":4, "name":"Sandık",     "code":"T_CHEST",   "sheet":"objects.png","sx":0,"sy":0,"sw":16,"sh":16,"base":0,"anim":[]},
    {"id":5, "name":"Kum/Patika", "code":"T_SAND",    "sheet":"Overworld.png","sx":0,"sy":176,"sw":16,"sh":16,"base":-1,"anim":[]},
    {"id":9, "name":"Açık Sandık","code":"OPEN_CHEST","sheet":"objects.png","sx":0,"sy":16,"sw":16,"sh":16,"base":0,"anim":[]},
]

DEFAULT_MAP_COLS = 40
DEFAULT_MAP_ROWS = 20
DEFAULT_MAP = [
    [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1],
    [1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,1,1,1,3,3,3,3,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,4,1],
    [1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,3,3,3,3,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1],
    [1,0,0,2,2,1,0,0,4,0,0,0,0,0,0,0,0,0,1,3,3,3,3,1,0,0,2,2,2,2,0,0,0,5,5,5,5,0,0,1],
    [1,0,0,2,2,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,3,3,1,1,0,0,2,2,2,2,0,0,0,5,5,5,5,0,0,1],
    [1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,5,5,5,0,0,1],
    [1,0,0,0,0,0,0,0,0,0,1,0,0,0,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1],
    [1,1,1,1,0,0,1,1,1,1,1,1,1,1,2,2,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1],
    [1,3,3,1,0,0,1,3,3,3,3,3,3,1,0,0,1,3,3,3,3,3,3,3,3,3,1,0,0,1,3,3,3,3,3,3,3,3,3,1],
    [1,3,3,1,0,0,1,3,3,3,3,3,3,1,0,0,1,3,3,3,3,3,3,3,3,3,1,0,0,1,3,3,3,3,3,3,3,3,3,1],
    [1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1],
    [1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1],
    [1,0,0,4,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,4,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1],
    [1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,0,0,1],
    [1,5,5,5,5,5,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,1,5,5,5,5,5,1,0,0,1],
    [1,5,5,5,5,5,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,1,5,5,5,5,5,1,0,0,1],
    [1,5,5,4,5,5,1,0,0,2,2,2,0,0,0,0,1,0,0,2,2,2,2,2,0,0,0,1,0,0,1,5,5,4,5,5,1,0,0,1],
    [1,5,5,5,5,5,1,0,0,2,2,2,0,0,0,0,1,0,0,2,2,2,2,2,0,0,0,0,0,0,1,5,5,5,5,5,1,0,0,1],
    [1,5,5,5,5,5,0,0,0,2,2,2,0,0,0,0,0,0,0,2,2,2,2,2,0,0,0,0,0,0,0,5,5,5,5,5,0,0,0,1],
    [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1],
]


# ─────────────────────────────────────────────────────────
class SpriteCache:
    def __init__(self):
        self._raw   = {}
        self._comp  = {}
        for name in ["Overworld.png","objects.png","character.png"]:
            p = os.path.join(BASE_DIR, name)
            if os.path.exists(p):
                self._raw[name] = Image.open(p).convert("RGBA")

    def raw_sheet(self, name):
        return self._raw.get(name)

    def crop(self, sheet, sx, sy, sw, sh):
        src = self._raw.get(sheet)
        if src is None:
            return Image.new("RGBA",(sw,sh),(80,80,80,180))
        img = src.crop((sx,sy,sx+sw,sy+sh)).copy()
        # tam boyut garantisi (sheet dışı koordinat koruması)
        if img.size != (sw, sh):
            canvas = Image.new("RGBA",(sw,sh),(0,0,0,0))
            canvas.paste(img,(0,0))
            return canvas
        return img

    def composite(self, base_raw, top_raw, scale):
        key = (id(base_raw), id(top_raw), scale)
        if key in self._comp:
            return self._comp[key]
        sw, sh = top_raw.size
        tw, th = sw*scale, sh*scale
        canvas = Image.new("RGBA",(tw,th),(0,0,0,0))
        if base_raw is not None:
            canvas.alpha_composite(base_raw.resize((tw,th),Image.NEAREST))
        canvas.alpha_composite(top_raw.resize((tw,th),Image.NEAREST))
        result = canvas.convert("RGBA")
        self._comp[key] = result
        return result

    def clear(self):
        self._comp.clear()


# ─────────────────────────────────────────────────────────
class MapEditor(tk.Tk):

    def __init__(self):
        super().__init__()
        self.title("🗺️ Zelda Map Editor v3")
        self.geometry("1300x780")
        self.configure(bg="#0d0d1a")

        self.cache     = SpriteCache()
        self.tile_defs = copy.deepcopy(DEFAULT_TILE_DEFS)
        self.map_cols  = DEFAULT_MAP_COLS
        self.map_rows  = DEFAULT_MAP_ROWS
        self.map_data  = [row[:] for row in DEFAULT_MAP]
        self.sel_tile  = 0
        self.zoom      = 2
        self.anim_frame= 0
        self._drawing  = False
        self._erasing  = False
        self._tk_imgs  = {}
        self._pal_imgs = {}
        self._block_paint = False   # ← Bug fix: palette tıklaması haritayı boyamasın

        self._build_ui()
        self._refresh_palette()
        self._render_all()
        self._start_anim()
        self.bind_all("<Key>", self._on_key)
        self.protocol("WM_DELETE_WINDOW", self._quit)
        self._auto_load()

    # ══════════════════════════════════════════════════════
    #  UI İnşa
    # ══════════════════════════════════════════════════════
    def _build_ui(self):
        # ── Toolbar
        bar = tk.Frame(self, bg="#16213e", pady=5)
        bar.pack(fill=tk.X)
        tk.Label(bar, text="🗺️ ZELDA MAP EDITOR v3",
                 fg="#e94560", bg="#16213e",
                 font=("Segoe UI",12,"bold")).pack(side=tk.LEFT, padx=10)

        B = dict(bg="#0f3460",fg="white",relief=tk.FLAT,
                 font=("Segoe UI",9,"bold"),padx=9,pady=3,
                 cursor="hand2",activebackground="#e94560",
                 activeforeground="white",bd=0)
        for txt,cmd in [
            ("💾 Kaydet",self._save),("📂 Yükle",self._load),
            ("🔄 Sıfırla",self._reset),("📋 C++ Kodu",self._show_cpp),
            ("💾 .h Yaz",self._write_h),
            ("✂️ Sprite Kes",self._open_cutter),
        ]:
            tk.Button(bar,text=txt,command=cmd,**B).pack(side=tk.LEFT,padx=3)

        # Zoom
        tk.Label(bar,text=" Zoom:",fg="#aaa",bg="#16213e",font=("Segoe UI",9)).pack(side=tk.LEFT,padx=(10,2))
        self._zoom_v = tk.IntVar(value=self.zoom)
        tk.Spinbox(bar,from_=1,to=6,textvariable=self._zoom_v,width=2,
                   command=self._on_zoom,bg="#0f3460",fg="white",
                   buttonbackground="#0f3460",insertbackground="white",
                   font=("Segoe UI",9)).pack(side=tk.LEFT)

        # Boyut
        for lbl,var_name,default,rng in [("  Cols","_cols_v",self.map_cols,(8,200)),
                                          ("  Rows","_rows_v",self.map_rows,(4,100))]:
            tk.Label(bar,text=lbl,fg="#aaa",bg="#16213e",font=("Segoe UI",9)).pack(side=tk.LEFT,padx=(6,2))
            v = tk.IntVar(value=default)
            setattr(self,var_name,v)
            tk.Spinbox(bar,from_=rng[0],to=rng[1],textvariable=v,width=4,
                       command=self._resize_map,bg="#0f3460",fg="white",
                       buttonbackground="#0f3460",insertbackground="white",
                       font=("Segoe UI",9)).pack(side=tk.LEFT)

        self._info = tk.Label(bar,text="",fg="#53d8fb",bg="#16213e",font=("Segoe UI",9))
        self._info.pack(side=tk.RIGHT,padx=10)

        # ── Sol: Palette
        left = tk.Frame(self, bg="#0d0d1a", width=175)
        left.pack(side=tk.LEFT, fill=tk.Y)
        left.pack_propagate(False)
        self._build_palette(left)

        # ── Orta: Harita
        mid = tk.Frame(self, bg="#0d0d1a")
        mid.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        hbar = tk.Scrollbar(mid, orient=tk.HORIZONTAL)
        vbar = tk.Scrollbar(mid, orient=tk.VERTICAL)
        hbar.pack(side=tk.BOTTOM, fill=tk.X)
        vbar.pack(side=tk.RIGHT, fill=tk.Y)
        self._canvas = tk.Canvas(mid, bg="#0d0d1a", cursor="crosshair",
                                 highlightthickness=0,
                                 xscrollcommand=hbar.set, yscrollcommand=vbar.set)
        self._canvas.pack(fill=tk.BOTH, expand=True)
        hbar.config(command=self._canvas.xview)
        vbar.config(command=self._canvas.yview)

        # Mouse bind'ları (harita canvas'ında)
        self._canvas.bind("<ButtonPress-1>",   self._md)
        self._canvas.bind("<B1-Motion>",       self._mm)
        self._canvas.bind("<ButtonRelease-1>", self._mu)
        self._canvas.bind("<ButtonPress-3>",   self._rd)
        self._canvas.bind("<B3-Motion>",       self._rm)
        self._canvas.bind("<ButtonRelease-3>", lambda e: setattr(self,"_erasing",False))
        self._canvas.bind("<Motion>",          self._hover)
        self._canvas.bind("<Double-Button-1>", self._dbl)

        # ── Sağ: Araçlar
        right = tk.Frame(self, bg="#16213e", width=180)
        right.pack(side=tk.RIGHT, fill=tk.Y)
        right.pack_propagate(False)
        self._build_right(right)

    # ──────────────────────────────────────────────
    def _build_palette(self, parent):
        tk.Label(parent,text="🎨 Tile Paleti",fg="#e94560",bg="#16213e",
                 font=("Segoe UI",10,"bold")).pack(pady=(6,2))
        tk.Label(parent,text="Sol tık: seç   Çift tık: düzenle/animasyon",
                 fg="#555",bg="#16213e",font=("Segoe UI",7),wraplength=168).pack()

        pc = tk.Canvas(parent, bg="#16213e", highlightthickness=0)
        sb = tk.Scrollbar(parent, orient=tk.VERTICAL, command=pc.yview)
        sb.pack(side=tk.RIGHT, fill=tk.Y)
        pc.pack(fill=tk.BOTH, expand=True)
        pc.configure(yscrollcommand=sb.set)

        self._pal_inner = tk.Frame(pc, bg="#16213e")
        self._pal_win   = pc.create_window((0,0), window=self._pal_inner, anchor="nw")
        self._pal_inner.bind("<Configure>",
            lambda e: pc.configure(scrollregion=pc.bbox("all")))
        self._pal_pc = pc
        self._pal_btns = {}

    def _refresh_palette(self):
        for w in self._pal_inner.winfo_children():
            w.destroy()
        self._pal_btns.clear()
        self._pal_imgs.clear()

        for td in self.tile_defs:
            tid = td["id"]
            img   = self._composite(tid, scale=3)
            tk_im = ImageTk.PhotoImage(img)
            self._pal_imgs[tid] = tk_im

            sel = (tid == self.sel_tile)
            bg  = "#e94560" if sel else "#0f3460"

            # Her satır bir Frame — tıklamayı doğru yakala
            row = tk.Frame(self._pal_inner, bg=bg, pady=2, padx=3,
                           cursor="hand2", relief=tk.FLAT)
            row.pack(fill=tk.X, padx=3, pady=2)

            ico = tk.Label(row, image=tk_im, bg=bg)
            ico.pack(side=tk.LEFT)

            has_anim = bool(td.get("anim"))
            name_text = f"{td['name']}" + (" 🌊" if has_anim else "")
            lbl = tk.Label(row, text=f"{name_text}\n({td['code']})",
                           bg=bg, fg="white", font=("Segoe UI",7), justify=tk.LEFT)
            lbl.pack(side=tk.LEFT, padx=3)

            # ── ÖNEMLI BUG FIX:
            # return "break" ile event bubble'ı durduruyor, haritaya ulaşmıyor
            def _click(e, t=tid):
                self._sel(t)
                return "break"
            def _dbl_click(e, t=tid):
                self._edit_tile_def(t)
                return "break"

            for w in (row, ico, lbl):
                w.bind("<ButtonPress-1>",   _click)
                w.bind("<Double-Button-1>", _dbl_click)

            self._pal_btns[tid] = row

    def _sel(self, tid):
        self.sel_tile = tid
        # Sadece rengi güncelle (tam refresh yerine — daha hızlı)
        for t, frame in self._pal_btns.items():
            c = "#e94560" if t == tid else "#0f3460"
            frame.configure(bg=c)
            for ch in frame.winfo_children():
                ch.configure(bg=c)
        td = self._td(tid)
        if td:
            self._sel_lbl.configure(text=f"Seçili:\n{td['name']}\n({td['code']})")

    # ──────────────────────────────────────────────
    def _build_right(self, parent):
        B = dict(bg="#0f3460",fg="white",relief=tk.FLAT,
                 font=("Segoe UI",8,"bold"),padx=6,pady=3,
                 cursor="hand2",activebackground="#e94560",activeforeground="white")

        tk.Label(parent,text="🔧 Araçlar",fg="#e94560",bg="#16213e",
                 font=("Segoe UI",10,"bold")).pack(pady=(8,4))

        td0 = self.tile_defs[0]
        self._sel_lbl = tk.Label(parent,
                                  text=f"Seçili:\n{td0['name']}\n({td0['code']})",
                                  fg="#53d8fb",bg="#16213e",
                                  font=("Segoe UI",8,"bold"),justify=tk.CENTER)
        self._sel_lbl.pack(pady=4)

        tk.Frame(parent,height=1,bg="#0f3460").pack(fill=tk.X,padx=8,pady=4)
        tk.Button(parent,text="🪣 Tümünü Doldur",command=self._fill_all,**B).pack(fill=tk.X,padx=8,pady=2)
        tk.Button(parent,text="🧹 Temizle (Çimen)",command=self._clear_all,**B).pack(fill=tk.X,padx=8,pady=2)
        tk.Button(parent,text="🗑️ Tile Def Sil",command=self._del_tile_def,**B).pack(fill=tk.X,padx=8,pady=2)

        tk.Frame(parent,height=1,bg="#0f3460").pack(fill=tk.X,padx=8,pady=6)

        # Animasyon önizleme
        tk.Label(parent,text="🌊 Animasyon Önizleme",fg="#aaa",bg="#16213e",
                 font=("Segoe UI",8)).pack()
        self._anim_cv = tk.Canvas(parent,width=162,height=58,
                                   bg="#0d0d1a",highlightthickness=1,
                                   highlightbackground="#0f3460")
        self._anim_cv.pack(padx=6,pady=4)
        self._anim_imgs = []
        for i in range(3):
            src = self.cache.crop("Overworld.png",0,16+i*16,16,16)
            ti  = ImageTk.PhotoImage(src.resize((50,50),Image.NEAREST))
            self._anim_imgs.append(ti)
            self._anim_cv.create_image(4+i*54,4,anchor=tk.NW,image=ti,tags=f"af{i}")

        tk.Label(parent,text="Hız (ms):",fg="#aaa",bg="#16213e",font=("Segoe UI",7)).pack()
        self._anim_spd = tk.IntVar(value=400)
        tk.Scale(parent,from_=80,to=1200,orient=tk.HORIZONTAL,
                 variable=self._anim_spd,resolution=40,
                 bg="#0f3460",fg="white",troughcolor="#0d0d1a",
                 highlightthickness=0,font=("Segoe UI",7),length=160).pack(padx=6)
        self._anim_on = tk.BooleanVar(value=True)
        tk.Checkbutton(parent,text="Animasyon Aktif",variable=self._anim_on,
                       bg="#16213e",fg="white",selectcolor="#0f3460",
                       font=("Segoe UI",8)).pack()

        tk.Frame(parent,height=1,bg="#0f3460").pack(fill=tk.X,padx=8,pady=6)
        tk.Label(parent,text="📊 İstatistik",fg="#aaa",bg="#16213e",
                 font=("Segoe UI",8)).pack()
        self._stats = tk.Text(parent,height=9,bg="#0d0d1a",fg="#53d8fb",
                              font=("Courier New",7),relief=tk.FLAT,state=tk.DISABLED)
        self._stats.pack(padx=6,fill=tk.X)

        tk.Label(parent,
                 text="\n⌨️ [1-9]:tile seç\n🖱️ Sol:boyar\n🖱️ Sağ:çimen\n⌨️ Çift tık:düzenle",
                 fg="#555",bg="#16213e",font=("Segoe UI",7),justify=tk.LEFT).pack(padx=6,pady=4)

    # ══════════════════════════════════════════════════════
    #  Sprite Kesici — Ayrı Büyük Pencere
    # ══════════════════════════════════════════════════════
    def _open_cutter(self):
        """Sprite sheet kesici — büyük ayrı pencere."""
        win = tk.Toplevel(self)
        win.title("✂️ Sprite Sheet Kesici")
        win.geometry("900x680")
        win.configure(bg="#0d0d1a")

        # ── Üst kontrol satırı
        ctrl = tk.Frame(win, bg="#16213e", pady=5)
        ctrl.pack(fill=tk.X)

        tk.Label(ctrl,text="Sheet:",fg="#aaa",bg="#16213e",
                 font=("Segoe UI",9)).pack(side=tk.LEFT,padx=8)
        sheet_v = tk.StringVar(value="Overworld.png")
        sheet_cb = ttk.Combobox(ctrl,textvariable=sheet_v,
                                values=["Overworld.png","objects.png","character.png"],
                                width=14,font=("Segoe UI",9))
        sheet_cb.pack(side=tk.LEFT,padx=4)

        tk.Label(ctrl,text="Zoom:",fg="#aaa",bg="#16213e",
                 font=("Segoe UI",9)).pack(side=tk.LEFT,padx=(12,2))
        zoom_v = tk.IntVar(value=3)
        tk.Spinbox(ctrl,from_=1,to=8,textvariable=zoom_v,width=2,
                   bg="#0f3460",fg="white",buttonbackground="#0f3460",
                   insertbackground="white",font=("Segoe UI",9)).pack(side=tk.LEFT)

        # Snap modu
        tk.Label(ctrl,text="  Snap:",fg="#aaa",bg="#16213e",
                 font=("Segoe UI",9)).pack(side=tk.LEFT,padx=(12,2))
        snap_v = tk.StringVar(value="Serbest")
        snap_opts = ["Serbest","8×8","16×16","32×32","özel..."]
        snap_cb = ttk.Combobox(ctrl,textvariable=snap_v,values=snap_opts,
                               width=10,font=("Segoe UI",9))
        snap_cb.pack(side=tk.LEFT,padx=4)
        snap_custom = [16,16]   # Özel snap değerleri

        info_lbl = tk.Label(ctrl,text="Seçim yok",fg="#53d8fb",bg="#16213e",
                            font=("Courier New",9))
        info_lbl.pack(side=tk.LEFT,padx=16)

        B = dict(bg="#0f3460",fg="white",relief=tk.FLAT,
                 font=("Segoe UI",9,"bold"),padx=10,pady=3,
                 cursor="hand2",activebackground="#e94560",activeforeground="white")
        add_btn = tk.Button(ctrl,text="➕ Palette Ekle",**B)
        add_btn.pack(side=tk.RIGHT,padx=10)
        reload_btn = tk.Button(ctrl,text="🔄 Yükle",**B)
        reload_btn.pack(side=tk.RIGHT,padx=4)

        # ── Sheet canvas (scroll destekli)
        cv_frame = tk.Frame(win,bg="#0d0d1a")
        cv_frame.pack(fill=tk.BOTH,expand=True,padx=4,pady=4)
        hbar = tk.Scrollbar(cv_frame,orient=tk.HORIZONTAL)
        vbar = tk.Scrollbar(cv_frame,orient=tk.VERTICAL)
        hbar.pack(side=tk.BOTTOM,fill=tk.X)
        vbar.pack(side=tk.RIGHT,fill=tk.Y)
        cv = tk.Canvas(cv_frame,bg="#111",cursor="crosshair",
                       highlightthickness=2,highlightbackground="#0f3460",
                       xscrollcommand=hbar.set,yscrollcommand=vbar.set)
        cv.pack(fill=tk.BOTH,expand=True)
        hbar.config(command=cv.xview)
        vbar.config(command=cv.yview)

        # ── Alt önizleme şeridi
        bot = tk.Frame(win,bg="#16213e",height=80)
        bot.pack(fill=tk.X)
        bot.pack_propagate(False)
        tk.Label(bot,text="Seçim Önizleme →",fg="#aaa",bg="#16213e",
                 font=("Segoe UI",8)).pack(side=tk.LEFT,padx=8)
        prev_cv = tk.Canvas(bot,width=72,height=72,bg="#222",
                            highlightthickness=1,highlightbackground="#0f3460")
        prev_cv.pack(side=tk.LEFT,pady=4)
        self._cut_prev_tk = [None]
        self._cut_sel_data = [None]   # (sheet,sx,sy,sw,sh)

        # ── Durum
        sheet_tk  = [None]
        drag_start= [None]
        drag_rect = [None]
        sel_coords= [None]   # canvas piksel (cx0,cy0,cx1,cy1)

        def snap_val(cx, cy):
            """Snap moduna göre canvas koordinatını hizala."""
            sv = snap_v.get()
            z  = zoom_v.get()
            if sv == "Serbest":
                return cx, cy
            if sv == "özel...":
                gx, gy = snap_custom
            else:
                parts = sv.split("×")
                gx = gy = int(parts[0])
            # piksel hizala
            px, py = cx//z, cy//z
            px = round(px/gx)*gx
            py = round(py/gy)*gy
            return px*z, py*z

        def load_sheet():
            sh   = sheet_v.get()
            z    = zoom_v.get()
            src  = self.cache.raw_sheet(sh)
            if src is None:
                info_lbl.configure(text=f"{sh} bulunamadı!")
                return
            w, h = src.width*z, src.height*z
            scaled = src.resize((w,h),Image.NEAREST)

            # Snap grid çiz
            with_grid = scaled.copy()
            draw = ImageDraw.Draw(with_grid)
            sv = snap_v.get()
            if sv != "Serbest":
                if sv == "özel...":
                    gx,gy = snap_custom
                else:
                    parts = sv.split("×")
                    gx = gy = int(parts[0])
                for gxi in range(0, src.width+1, gx):
                    draw.line([(gxi*z,0),(gxi*z,h-1)], fill=(80,80,80,180), width=1)
                for gyi in range(0, src.height+1, gy):
                    draw.line([(0,gyi*z),(w-1,gyi*z)], fill=(80,80,80,180), width=1)

            sheet_tk[0] = ImageTk.PhotoImage(with_grid)
            cv.configure(scrollregion=(0,0,w,h))
            cv.delete("all")
            cv.create_image(0,0,anchor=tk.NW,image=sheet_tk[0])
            sel_coords[0] = None

        def update_preview():
            d = self._cut_sel_data[0]
            if d is None: return
            sh,sx,sy,sw,sh2 = d
            img = self.cache.crop(sh,sx,sy,sw,sh2)
            s = max(1, 64//max(sw,sh2))
            pv = img.resize((sw*s,sh2*s),Image.NEAREST)
            self._cut_prev_tk[0] = ImageTk.PhotoImage(pv)
            prev_cv.delete("all")
            prev_cv.create_image(4,4,anchor=tk.NW,image=self._cut_prev_tk[0])

        def on_press(e):
            x0 = cv.canvasx(e.x); y0 = cv.canvasy(e.y)
            x0,y0 = snap_val(int(x0),int(y0))
            drag_start[0] = (x0,y0)
            cv.delete("sel_r","sel_info")

        def on_drag(e):
            if drag_start[0] is None: return
            x0,y0 = drag_start[0]
            x1 = cv.canvasx(e.x); y1 = cv.canvasy(e.y)
            x1,y1 = snap_val(int(x1),int(y1))
            cv.delete("sel_r","sel_info")
            cv.create_rectangle(x0,y0,x1,y1,outline="#e94560",width=2,tags="sel_r")
            # Kesik çizgi iç
            cv.create_rectangle(x0+2,y0+2,x1-2,y1-2,outline="#ff8080",width=1,
                                 dash=(4,3),tags="sel_r")
            z = zoom_v.get()
            sx0,sy0 = min(x0,x1)//z, min(y0,y1)//z
            sw2 = max(1,abs(x1-x0)//z)
            sh2 = max(1,abs(y1-y0)//z)
            sel_coords[0] = (min(x0,x1),min(y0,y1),max(x0,x1),max(y0,y1))
            cv.create_text(min(x0,x1)+4, min(y0,y1)-10,
                           text=f"x={sx0} y={sy0} {sw2}×{sh2}",
                           fill="#ffe", anchor="sw", font=("Courier New",8), tags="sel_info")
            # Anlık önizleme güncelle
            sh = sheet_v.get()
            self._cut_sel_data[0] = (sh,sx0,sy0,sw2,sh2)
            update_preview()
            info_lbl.configure(text=f"x={sx0} y={sy0}  {sw2}×{sh2} px")

        def on_release(e):
            drag_start[0] = None

        cv.bind("<ButtonPress-1>",   on_press)
        cv.bind("<B1-Motion>",       on_drag)
        cv.bind("<ButtonRelease-1>", on_release)

        def on_reload():
            # Özel snap dialog
            if snap_v.get() == "özel...":
                gx = simpledialog_int(win,"Snap X (piksel):",snap_custom[0])
                gy = simpledialog_int(win,"Snap Y (piksel):",snap_custom[1])
                if gx and gy:
                    snap_custom[0],snap_custom[1] = gx,gy
            load_sheet()

        def simpledialog_int(parent,prompt,default):
            try:
                from tkinter import simpledialog
                return simpledialog.askinteger("Snap",prompt,initialvalue=default,parent=parent)
            except Exception:
                return default

        reload_btn.configure(command=on_reload)
        sheet_cb.bind("<<ComboboxSelected>>", lambda e: load_sheet())
        snap_cb.bind( "<<ComboboxSelected>>", lambda e: load_sheet())

        # ── Palette'e ekle dialog
        def add_to_palette():
            d = self._cut_sel_data[0]
            if d is None:
                messagebox.showwarning("Uyarı","Önce bir alan seçin!",parent=win)
                return
            sh,sx,sy,sw2,sh2 = d
            self._add_tile_dialog(win,sh,sx,sy,sw2,sh2)

        add_btn.configure(command=add_to_palette)
        load_sheet()

    def _add_tile_dialog(self, parent, sheet, sx, sy, sw, sh):
        """Yeni tile ekle dialog."""
        dlg = tk.Toplevel(parent)
        dlg.title("Tile Ekle")
        dlg.configure(bg="#0d0d1a")
        dlg.resizable(False,False)

        # Önizleme
        prev = self.cache.crop(sheet,sx,sy,sw,sh)
        ps   = max(1, 80//max(sw,sh))
        pt   = prev.resize((sw*ps,sh*ps),Image.NEAREST)
        prev_tk = ImageTk.PhotoImage(pt)
        tk.Label(dlg,image=prev_tk,bg="#0d0d1a").grid(row=0,column=0,columnspan=2,pady=8)
        dlg._ptk = prev_tk

        def lbl(txt,r):
            tk.Label(dlg,text=txt,fg="#aaa",bg="#0d0d1a",
                     font=("Segoe UI",9)).grid(row=r,column=0,sticky="e",padx=8,pady=3)
        def ent(default,r,w=18):
            e = tk.Entry(dlg,bg="#16213e",fg="white",insertbackground="white",
                         font=("Segoe UI",9),width=w)
            e.insert(0,str(default))
            e.grid(row=r,column=1,padx=8,pady=3)
            return e

        lbl("Ad:",1);   name_e = ent(f"Tile_{len(self.tile_defs)}",1)
        lbl("Kod:",2);  code_e = ent(f"T_CUSTOM{len(self.tile_defs)}",2)
        lbl("ID:",3)
        used   = {t["id"] for t in self.tile_defs}
        nid    = max(used)+1 if used else 10
        id_e   = ent(nid,3,6)

        lbl("Zemin tile:",4)
        bases  = ["-1 (Şeffaf)"] + [f"{t['id']} ({t['name']})" for t in self.tile_defs]
        base_v = tk.StringVar(value="0 (Çimen)" if any(t["id"]==0 for t in self.tile_defs) else bases[0])
        ttk.Combobox(dlg,textvariable=base_v,values=bases,
                     width=16,font=("Segoe UI",9)).grid(row=4,column=1,padx=8,pady=3)

        lbl("Solid (engel):",5)
        sol_v = tk.BooleanVar(value=True)
        tk.Checkbutton(dlg,variable=sol_v,bg="#0d0d1a",
                       selectcolor="#0f3460").grid(row=5,column=1,sticky="w",padx=8)

        def ok():
            try: tid = int(id_e.get())
            except ValueError:
                messagebox.showerror("Hata","Geçersiz ID",parent=dlg); return
            if any(t["id"]==tid for t in self.tile_defs):
                messagebox.showerror("Hata",f"ID {tid} kullanımda",parent=dlg); return
            base_id = int(base_v.get().split()[0])
            new_td = {"id":tid,"name":name_e.get(),"code":code_e.get(),
                      "sheet":sheet,"sx":sx,"sy":sy,"sw":sw,"sh":sh,
                      "base":base_id,"anim":[],"solid":sol_v.get()}
            self.tile_defs.append(new_td)
            self.tile_defs.sort(key=lambda x:x["id"])
            self.cache.clear()
            self._refresh_palette()
            dlg.destroy()
            messagebox.showinfo("Eklendi",f"'{name_e.get()}' (ID={tid}) eklendi!")

        tk.Button(dlg,text="✅ Ekle",command=ok,
                  bg="#e94560",fg="white",relief=tk.FLAT,
                  font=("Segoe UI",10,"bold"),padx=12,pady=5).grid(
                      row=6,column=0,columnspan=2,pady=10)
        dlg.grab_set()

    # ══════════════════════════════════════════════════════
    #  Tile Tanım Editörü (animasyon + piksel düzenleme)
    # ══════════════════════════════════════════════════════
    def _edit_tile_def(self, tid):
        td = self._td(tid)
        if td is None: return
        win = tk.Toplevel(self)
        win.title(f"Tile Düzenle — {td['name']} (ID={tid})")
        win.geometry("820x580")
        win.configure(bg="#0d0d1a")

        nb = ttk.Notebook(win)
        nb.pack(fill=tk.BOTH,expand=True,padx=6,pady=6)

        # ── Sekme 1: Animasyon Kareleri
        anim_tab = tk.Frame(nb,bg="#16213e")
        nb.add(anim_tab,text="🌊 Animasyon Kareleri")
        self._build_anim_editor(anim_tab, td)

        # ── Sekme 2: Piksel Editörü
        pix_tab = tk.Frame(nb,bg="#16213e")
        nb.add(pix_tab,text="🖌️ Piksel Editörü")
        self._build_pixel_editor(pix_tab, td)

    # ── Animasyon Editörü
    def _build_anim_editor(self, parent, td):
        tk.Label(parent,
                 text=f"Animasyon kareleri (her kare bir tile). Şu an {len(td['anim'])} kare.",
                 fg="#53d8fb",bg="#16213e",font=("Segoe UI",9)).pack(pady=8)

        # Kare listesi
        list_frame = tk.Frame(parent,bg="#16213e")
        list_frame.pack(fill=tk.BOTH,expand=True,padx=10)

        # Önizleme satırı
        prev_row = tk.Frame(parent,bg="#16213e")
        prev_row.pack(fill=tk.X,padx=10,pady=4)

        frame_imgs = [None]*10   # GC
        frame_cvs  = []

        def refresh_frames():
            for w in list_frame.winfo_children():
                w.destroy()
            frame_cvs.clear()

            for i, af in enumerate(td["anim"]):
                row = tk.Frame(list_frame,bg="#0f3460",pady=3,padx=4)
                row.pack(fill=tk.X,pady=2)

                # Önizleme
                img  = self.cache.crop(af["sheet"],af["sx"],af.get("sy",0),
                                        af.get("sw",td["sw"]),af.get("sh",td["sh"]))
                ti   = ImageTk.PhotoImage(img.resize((32,32),Image.NEAREST))
                frame_imgs[i%10] = ti
                tk.Label(row,image=ti,bg="#0f3460").pack(side=tk.LEFT,padx=4)

                tk.Label(row,text=f"Kare {i+1}:  {af['sheet']}  "
                                   f"x={af['sx']} y={af.get('sy',0)} "
                                   f"{af.get('sw',td['sw'])}×{af.get('sh',td['sh'])}",
                         fg="white",bg="#0f3460",font=("Courier New",8)).pack(side=tk.LEFT)

                # Sil
                def del_frame(idx=i):
                    td["anim"].pop(idx)
                    refresh_frames()
                tk.Button(row,text="🗑️",command=del_frame,
                          bg="#e94560",fg="white",relief=tk.FLAT,
                          font=("Segoe UI",8),padx=4).pack(side=tk.RIGHT,padx=4)

                # Yukarı/Aşağı
                def move_up(idx=i):
                    if idx>0:
                        td["anim"][idx],td["anim"][idx-1] = td["anim"][idx-1],td["anim"][idx]
                        refresh_frames()
                def move_dn(idx=i):
                    if idx<len(td["anim"])-1:
                        td["anim"][idx],td["anim"][idx+1] = td["anim"][idx+1],td["anim"][idx]
                        refresh_frames()
                tk.Button(row,text="⬆",command=move_up,bg="#0f3460",fg="white",
                          relief=tk.FLAT,font=("Segoe UI",8),padx=3).pack(side=tk.RIGHT,padx=2)
                tk.Button(row,text="⬇",command=move_dn,bg="#0f3460",fg="white",
                          relief=tk.FLAT,font=("Segoe UI",8),padx=3).pack(side=tk.RIGHT)

            # + Kare Ekle butonu
            add_f = tk.Frame(list_frame,bg="#16213e",pady=4)
            add_f.pack(fill=tk.X)

            B = dict(bg="#0f3460",fg="white",relief=tk.FLAT,
                     font=("Segoe UI",8,"bold"),padx=8,pady=3,
                     cursor="hand2",activebackground="#e94560",activeforeground="white")

            def add_from_sheet():
                """Sheet'ten kare seç."""
                sel_win = tk.Toplevel(parent)
                sel_win.title("Kare için Sprite Seç")
                sel_win.geometry("750x520")
                sel_win.configure(bg="#0d0d1a")

                ctrl2 = tk.Frame(sel_win,bg="#16213e",pady=4)
                ctrl2.pack(fill=tk.X)
                sv2 = tk.StringVar(value=td["sheet"])
                ttk.Combobox(ctrl2,textvariable=sv2,
                             values=["Overworld.png","objects.png","character.png"],
                             width=14).pack(side=tk.LEFT,padx=8)
                zv2 = tk.IntVar(value=3)
                tk.Spinbox(ctrl2,from_=1,to=6,textvariable=zv2,width=2,
                           bg="#0f3460",fg="white",buttonbackground="#0f3460",
                           insertbackground="white").pack(side=tk.LEFT,padx=4)
                snap2 = tk.StringVar(value="16×16")
                ttk.Combobox(ctrl2,textvariable=snap2,
                             values=["Serbest","8×8","16×16","32×32"],
                             width=8).pack(side=tk.LEFT,padx=4)
                info2 = tk.Label(ctrl2,text="",fg="#53d8fb",bg="#16213e",
                                 font=("Courier New",8))
                info2.pack(side=tk.LEFT,padx=10)

                cf2 = tk.Frame(sel_win,bg="#0d0d1a")
                cf2.pack(fill=tk.BOTH,expand=True,padx=4,pady=4)
                h2 = tk.Scrollbar(cf2,orient=tk.HORIZONTAL)
                v2 = tk.Scrollbar(cf2,orient=tk.VERTICAL)
                h2.pack(side=tk.BOTTOM,fill=tk.X)
                v2.pack(side=tk.RIGHT,fill=tk.Y)
                cv2 = tk.Canvas(cf2,bg="#111",cursor="crosshair",
                                highlightthickness=1,highlightbackground="#0f3460",
                                xscrollcommand=h2.set,yscrollcommand=v2.set)
                cv2.pack(fill=tk.BOTH,expand=True)
                h2.config(command=cv2.xview)
                v2.config(command=cv2.yview)

                sheet_tk2 = [None]
                drag2 = [None]
                sel2  = [None]   # (sx,sy,sw,sh)

                def load2():
                    sh2 = sv2.get(); z2 = zv2.get()
                    src = self.cache.raw_sheet(sh2)
                    if not src: return
                    w2,h2v = src.width*z2,src.height*z2
                    sc2 = src.resize((w2,h2v),Image.NEAREST).copy()
                    draw = ImageDraw.Draw(sc2)
                    sn = snap2.get()
                    if sn!="Serbest":
                        g = int(sn.split("×")[0])
                        for gx in range(0,src.width+1,g):
                            draw.line([(gx*z2,0),(gx*z2,h2v-1)],fill=(80,80,80,200),width=1)
                        for gy2 in range(0,src.height+1,g):
                            draw.line([(0,gy2*z2),(w2-1,gy2*z2)],fill=(80,80,80,200),width=1)
                    sheet_tk2[0] = ImageTk.PhotoImage(sc2)
                    cv2.configure(scrollregion=(0,0,w2,h2v))
                    cv2.delete("all")
                    cv2.create_image(0,0,anchor=tk.NW,image=sheet_tk2[0])

                def snap2_fn(cx,cy):
                    z2 = zv2.get(); sn = snap2.get()
                    if sn=="Serbest": return cx,cy
                    g = int(sn.split("×")[0])
                    px,py = cx//z2, cy//z2
                    return round(px/g)*g*z2, round(py/g)*g*z2

                def p2(e):
                    x,y = cv2.canvasx(e.x),cv2.canvasy(e.y)
                    drag2[0] = snap2_fn(int(x),int(y))
                def d2(e):
                    if not drag2[0]: return
                    x0,y0 = drag2[0]
                    x1,y1 = snap2_fn(int(cv2.canvasx(e.x)),int(cv2.canvasy(e.y)))
                    cv2.delete("s2"); cv2.delete("i2")
                    cv2.create_rectangle(x0,y0,x1,y1,outline="#e94560",width=2,tags="s2")
                    z2 = zv2.get()
                    sx2 = min(x0,x1)//z2; sy2 = min(y0,y1)//z2
                    sw2 = max(1,abs(x1-x0)//z2); sh2 = max(1,abs(y1-y0)//z2)
                    sel2[0] = (sx2,sy2,sw2,sh2)
                    info2.configure(text=f"x={sx2} y={sy2} {sw2}×{sh2}")
                    cv2.create_text(min(x0,x1)+3,min(y0,y1)-8,
                                    text=f"{sx2},{sy2} {sw2}×{sh2}",
                                    fill="#ffe",anchor="sw",font=("Courier New",8),tags="i2")

                cv2.bind("<ButtonPress-1>", p2)
                cv2.bind("<B1-Motion>",     d2)
                sv2.trace_add("write",lambda *a: load2())
                zv2.trace_add("write",lambda *a: load2())
                snap2.trace_add("write",lambda *a: load2())

                def confirm():
                    if not sel2[0]:
                        messagebox.showwarning("Uyarı","Alan seçin!",parent=sel_win); return
                    sx2,sy2,sw2,sh2 = sel2[0]
                    td["anim"].append({"sheet":sv2.get(),"sx":sx2,"sy":sy2,
                                       "sw":sw2,"sh":sh2})
                    sel_win.destroy()
                    refresh_frames()
                    # Palette'i güncelle
                    self.cache.clear()
                    self._refresh_palette()
                    self._update_anim_tiles()

                tk.Button(sel_win,text="✅ Kare Olarak Ekle",command=confirm,
                          bg="#e94560",fg="white",relief=tk.FLAT,
                          font=("Segoe UI",10,"bold"),padx=12,pady=6).pack(pady=8)
                load2()
                sel_win.grab_set()

            def add_existing():
                """Mevcut tile def'ten kare ekle."""
                dlg2 = tk.Toplevel(parent)
                dlg2.title("Mevcut Tile'dan Kare Ekle")
                dlg2.configure(bg="#0d0d1a")
                dlg2.resizable(False,False)

                imgs2 = {}
                for idx,t in enumerate(self.tile_defs):
                    img2 = self._composite(t["id"],scale=3)
                    tk2  = ImageTk.PhotoImage(img2)
                    imgs2[t["id"]] = tk2
                    row2 = tk.Frame(dlg2,bg="#0f3460",pady=2,padx=4,cursor="hand2")
                    row2.pack(fill=tk.X,padx=6,pady=2)
                    tk.Label(row2,image=tk2,bg="#0f3460").pack(side=tk.LEFT)
                    tk.Label(row2,text=f"{t['name']} ({t['code']})",
                             bg="#0f3460",fg="white",font=("Segoe UI",8)).pack(side=tk.LEFT,padx=6)
                    def pick(t2=t):
                        td["anim"].append({"sheet":t2["sheet"],"sx":t2["sx"],
                                           "sy":t2["sy"],"sw":t2["sw"],"sh":t2["sh"]})
                        dlg2.destroy()
                        refresh_frames()
                        self.cache.clear()
                        self._refresh_palette()
                        self._update_anim_tiles()
                    for w2 in (row2,):
                        w2.bind("<ButtonPress-1>", lambda e,f=pick: (f(), None))
                    row2._img = tk2

                dlg2.grab_set()

            tk.Button(add_f,text="➕ Sheet'ten Kare Seç",command=add_from_sheet,**B).pack(side=tk.LEFT,padx=4)
            tk.Button(add_f,text="➕ Mevcut Tile'dan",command=add_existing,**B).pack(side=tk.LEFT,padx=4)

            def clear_anim():
                if messagebox.askyesno("Emin mi?","Tüm animasyon kareleri silinecek.",parent=parent):
                    td["anim"].clear()
                    refresh_frames()
                    self.cache.clear(); self._refresh_palette(); self._update_anim_tiles()
            tk.Button(add_f,text="🗑️ Tümünü Sil",command=clear_anim,**B).pack(side=tk.LEFT,padx=4)

        refresh_frames()

    # ── Piksel Editörü
    def _build_pixel_editor(self, parent, td):
        PSIZE = 16
        sw, sh = td["sw"], td["sh"]
        raw  = self.cache.crop(td["sheet"],td["sx"],td["sy"],sw,sh)
        work = [raw.copy()]   # mutable wrapper

        c_w, c_h = sw*PSIZE, sh*PSIZE

        left = tk.Frame(parent,bg="#16213e")
        left.pack(side=tk.LEFT,fill=tk.Y,padx=8,pady=8)

        pe = tk.Canvas(left,width=c_w,height=c_h,bg="#111",
                       cursor="crosshair",highlightthickness=2,
                       highlightbackground="#e94560")
        pe.pack()

        right2 = tk.Frame(parent,bg="#16213e")
        right2.pack(side=tk.LEFT,fill=tk.Y,padx=8,pady=8)

        cur_color = ["#3DE800"]
        cur_alpha = [255]
        eraser    = [False]

        pe_tk = [None]
        def redraw():
            bg_base = Image.new("RGBA",(sw,sh),(40,40,40,255))
            bg_base.alpha_composite(work[0])
            big = bg_base.resize((c_w,c_h),Image.NEAREST)
            pe_tk[0] = ImageTk.PhotoImage(big)
            pe.delete("all")
            pe.create_image(0,0,anchor=tk.NW,image=pe_tk[0])
            for gx in range(0,c_w+1,PSIZE):
                pe.create_line(gx,0,gx,c_h,fill="#333",width=1)
            for gy in range(0,c_h+1,PSIZE):
                pe.create_line(0,gy,c_w,gy,fill="#333",width=1)

        def paint(e):
            px,py = e.x//PSIZE, e.y//PSIZE
            if 0<=px<sw and 0<=py<sh:
                if eraser[0]:
                    work[0].putpixel((px,py),(0,0,0,0))
                else:
                    hx = cur_color[0].lstrip("#")
                    r2,g2,b2 = int(hx[0:2],16),int(hx[2:4],16),int(hx[4:6],16)
                    work[0].putpixel((px,py),(r2,g2,b2,cur_alpha[0]))
                redraw()

        def pick(e):
            px,py = e.x//PSIZE, e.y//PSIZE
            if 0<=px<sw and 0<=py<sh:
                r2,g2,b2,a2 = work[0].getpixel((px,py))
                cur_color[0] = f"#{r2:02X}{g2:02X}{b2:02X}"
                cur_alpha[0] = a2
                alpha_v.set(a2)
                col_preview.configure(bg=cur_color[0])

        pe.bind("<ButtonPress-1>", paint)
        pe.bind("<B1-Motion>",     paint)
        pe.bind("<ButtonPress-3>", pick)

        # Renk göstergesi
        col_preview = tk.Label(right2,bg=cur_color[0],width=8,height=2,
                               relief=tk.RAISED)
        col_preview.pack(pady=6)

        def open_color():
            from tkinter.colorchooser import askcolor
            res = askcolor(color=cur_color[0],title="Renk Seç",parent=parent)
            if res[1]:
                cur_color[0] = res[1]
                col_preview.configure(bg=cur_color[0])
        tk.Button(right2,text="🎨 Renk Seç",command=open_color,
                  bg="#0f3460",fg="white",relief=tk.FLAT,
                  font=("Segoe UI",8),padx=6,pady=3).pack(fill=tk.X,pady=2)

        # Hızlı renkler
        qc_frame = tk.Frame(right2,bg="#16213e")
        qc_frame.pack()
        for hc in ["#000000","#FFFFFF","#FF0000","#00C800","#0040FF",
                   "#FFFF00","#FF8000","#804000","#A0A0A0","#3DE800"]:
            b = tk.Label(qc_frame,bg=hc,width=2,height=1,cursor="hand2")
            b.pack(side=tk.LEFT,padx=1,pady=2)
            b.bind("<ButtonPress-1>",
                   lambda e,c=hc: (cur_color.__setitem__(0,c), col_preview.configure(bg=c)))

        tk.Label(right2,text="Alpha:",fg="#aaa",bg="#16213e",font=("Segoe UI",8)).pack(pady=(8,0))
        alpha_v = tk.IntVar(value=255)
        def on_alpha(*_): cur_alpha[0] = alpha_v.get()
        tk.Scale(right2,from_=0,to=255,orient=tk.HORIZONTAL,variable=alpha_v,
                 resolution=1,bg="#0f3460",fg="white",troughcolor="#0d0d1a",
                 highlightthickness=0,font=("Segoe UI",7),length=130,
                 command=on_alpha).pack()

        def toggle_eraser():
            eraser[0] = not eraser[0]
            er_btn.configure(bg="#e94560" if eraser[0] else "#0f3460",
                             text=("✏️ Boya" if eraser[0] else "🧹 Silgi (Saydam)"))
        er_btn = tk.Button(right2,text="🧹 Silgi (Saydam)",command=toggle_eraser,
                           bg="#0f3460",fg="white",relief=tk.FLAT,
                           font=("Segoe UI",8),padx=6,pady=3)
        er_btn.pack(fill=tk.X,pady=4)

        def save_apply():
            self.cache._raw.setdefault(td["sheet"],
                Image.new("RGBA",(64,64),(0,0,0,0)))
            sheet_img = self.cache._raw[td["sheet"]]
            sheet_img.paste(work[0],(td["sx"],td["sy"]))
            self.cache.clear()
            self._refresh_palette()
            self._render_all()
            messagebox.showinfo("Uygulandı",
                "Editörde uygulandı. PNG'ye kaydetmek için '💾 PNG' butonuna bas.",
                parent=parent)

        def save_png():
            path = os.path.join(BASE_DIR,td["sheet"])
            if self.cache._raw.get(td["sheet"]):
                self.cache._raw[td["sheet"]].save(path)
                messagebox.showinfo("Kaydedildi",f"{td['sheet']} güncellendi!",parent=parent)

        tk.Button(right2,text="✅ Uygula",command=save_apply,
                  bg="#e94560",fg="white",relief=tk.FLAT,
                  font=("Segoe UI",9,"bold"),padx=10,pady=4).pack(fill=tk.X,pady=3)
        tk.Button(right2,text="💾 PNG Kaydet",command=save_png,
                  bg="#0f3460",fg="white",relief=tk.FLAT,
                  font=("Segoe UI",9,"bold"),padx=10,pady=4).pack(fill=tk.X,pady=2)

        redraw()

    # ══════════════════════════════════════════════════════
    #  Render
    # ══════════════════════════════════════════════════════
    def _composite(self, tid, scale=1, anim_frame=0):
        td = self._td(tid)
        if td is None:
            return Image.new("RGBA",(TILE_W*scale,TILE_H*scale),(80,80,80,255))
        anim = td.get("anim",[])
        if anim and anim_frame > 0:
            af = anim[anim_frame % len(anim)]
            top = self.cache.crop(af["sheet"],af["sx"],af.get("sy",td["sy"]),
                                  af.get("sw",td["sw"]),af.get("sh",td["sh"]))
        else:
            top = self.cache.crop(td["sheet"],td["sx"],td["sy"],td["sw"],td["sh"])
        base_id = td.get("base",-1)
        base_raw = None
        if base_id >= 0:
            bt = self._td(base_id)
            if bt:
                base_raw = self.cache.crop(bt["sheet"],bt["sx"],bt["sy"],bt["sw"],bt["sh"])
        return self.cache.composite(base_raw, top, scale)

    def _render_all(self):
        z = self.zoom
        tw, th = TILE_W*z, TILE_H*z
        self._canvas.configure(scrollregion=(0,0,self.map_cols*tw,self.map_rows*th))
        self._canvas.delete("all")
        self._tk_imgs.clear()
        for r in range(self.map_rows):
            for c in range(self.map_cols):
                self._draw_cell(c,r)
        for c in range(self.map_cols+1):
            self._canvas.create_line(c*tw,0,c*tw,self.map_rows*th,fill="#1a1a2e",width=1,tags="g")
        for r in range(self.map_rows+1):
            self._canvas.create_line(0,r*th,self.map_cols*tw,r*th,fill="#1a1a2e",width=1,tags="g")
        self._upd_stats()

    def _draw_cell(self, col, row, af=None):
        z  = self.zoom
        tw, th = TILE_W*z, TILE_H*z
        tid   = self.map_data[row][col]
        frame = af if af is not None else self.anim_frame
        img   = self._composite(tid,scale=z,anim_frame=frame)
        ti    = ImageTk.PhotoImage(img)
        self._tk_imgs[(col,row)] = ti
        tag = f"c{col}_{row}"
        self._canvas.delete(tag)
        self._canvas.create_image(col*tw,row*th,anchor=tk.NW,image=ti,tags=tag)

    def _update_anim_tiles(self):
        for r in range(self.map_rows):
            for c in range(self.map_cols):
                td = self._td(self.map_data[r][c])
                if td and td.get("anim"):
                    self._draw_cell(c,r,af=self.anim_frame)

    # ══════════════════════════════════════════════════════
    #  Mouse
    # ══════════════════════════════════════════════════════
    def _to_tile(self, e):
        z = self.zoom
        cx = self._canvas.canvasx(e.x)
        cy = self._canvas.canvasy(e.y)
        c,r = int(cx/(TILE_W*z)), int(cy/(TILE_H*z))
        if 0<=c<self.map_cols and 0<=r<self.map_rows:
            return c,r
        return None,None

    # BUG FIX: _block_paint ile palette tıklaması haritaya ulaşmıyor
    def _md(self, e):
        if self._block_paint: return
        self._drawing = True
        self._paint(*self._to_tile(e))
    def _mm(self, e):
        if self._drawing: self._paint(*self._to_tile(e))
    def _mu(self, e): self._drawing = False
    def _rd(self, e):
        self._erasing = True; self._erase(*self._to_tile(e))
    def _rm(self, e):
        if self._erasing: self._erase(*self._to_tile(e))

    def _paint(self, col, row):
        if col is None: return
        if self.map_data[row][col] == self.sel_tile: return
        self.map_data[row][col] = self.sel_tile
        self._draw_cell(col,row)
        self._upd_stats()

    def _erase(self, col, row):
        if col is None: return
        if self.map_data[row][col] == 0: return
        self.map_data[row][col] = 0
        self._draw_cell(col,row)
        self._upd_stats()

    def _hover(self, e):
        c,r = self._to_tile(e)
        if c is not None:
            tid = self.map_data[r][c]
            td  = self._td(tid) or {}
            self._info.configure(
                text=f"col:{c:3d} row:{r:2d}  {td.get('name','?')} ({td.get('code','?')}={tid})")

    def _dbl(self, e):
        c,r = self._to_tile(e)
        if c is not None:
            self._edit_tile_def(self.map_data[r][c])

    def _on_key(self, e):
        # Palette aktifken klavye tile seçimi
        km = {'1':0,'2':1,'3':2,'4':3,'5':4,'6':5,'9':9}
        if e.char in km:
            tid = km[e.char]
            if any(t["id"]==tid for t in self.tile_defs):
                self._sel(tid)

    # ══════════════════════════════════════════════════════
    #  Araçlar
    # ══════════════════════════════════════════════════════
    def _fill_all(self):
        td = self._td(self.sel_tile)
        name = td["name"] if td else str(self.sel_tile)
        if not messagebox.askyesno("Emin mi?",f"Tümü '{name}' ile dolacak."): return
        for r in range(self.map_rows):
            for c in range(self.map_cols):
                self.map_data[r][c] = self.sel_tile
        self._render_all()

    def _clear_all(self):
        if not messagebox.askyesno("Emin mi?","Tümü çimene dönecek."): return
        for r in range(self.map_rows):
            for c in range(self.map_cols):
                self.map_data[r][c] = 0
        self._render_all()

    def _del_tile_def(self):
        if self.sel_tile==0:
            messagebox.showwarning("Uyarı","Çimen tile silinemez!"); return
        td = self._td(self.sel_tile)
        if not td: return
        if not messagebox.askyesno("Sil?",f"'{td['name']}' silinecek. Haritada çimene dönüşür."): return
        self.tile_defs = [t for t in self.tile_defs if t["id"]!=self.sel_tile]
        for r in range(self.map_rows):
            for c in range(self.map_cols):
                if self.map_data[r][c]==self.sel_tile:
                    self.map_data[r][c]=0
        self.sel_tile = 0
        self.cache.clear()
        self._refresh_palette()
        self._render_all()

    def _resize_map(self):
        nc = max(8,self._cols_v.get())
        nr = max(4,self._rows_v.get())
        new = []
        for r in range(nr):
            row = (self.map_data[r][:nc] if r<self.map_rows else [])
            while len(row)<nc: row.append(0)
            new.append(row)
        self.map_cols,self.map_rows = nc,nr
        self.map_data = new
        self._cols_v.set(nc); self._rows_v.set(nr)
        self._render_all()

    def _on_zoom(self):
        self.zoom = max(1,min(6,self._zoom_v.get()))
        self.cache.clear()
        self._render_all()

    def _td(self, tid):
        for t in self.tile_defs:
            if t["id"]==tid: return t
        return None

    def _upd_stats(self):
        counts = {}
        for r in range(self.map_rows):
            for c in range(self.map_cols):
                t = self.map_data[r][c]
                counts[t] = counts.get(t,0)+1
        lines = [f"{'Tile':<11} {'Adet':>4}", "─"*17]
        for tid,cnt in sorted(counts.items(),key=lambda x:-x[1]):
            td = self._td(tid)
            n  = td["name"] if td else f"?({tid})"
            lines.append(f"{n[:11]:<11} {cnt:>4}")
        self._stats.configure(state=tk.NORMAL)
        self._stats.delete("1.0",tk.END)
        self._stats.insert(tk.END,"\n".join(lines))
        self._stats.configure(state=tk.DISABLED)

    # ══════════════════════════════════════════════════════
    #  Animasyon Thread
    # ══════════════════════════════════════════════════════
    def _start_anim(self):
        def loop():
            while True:
                try:
                    if self._anim_on.get():
                        self.anim_frame = (self.anim_frame+1)%3
                        self._update_anim_tiles()
                        self._anim_cv.delete("hl")
                        hx = 4+self.anim_frame*54
                        self._anim_cv.create_rectangle(hx-2,2,hx+52,56,
                                                        outline="#e94560",width=2,tags="hl")
                    time.sleep(self._anim_spd.get()/1000)
                except Exception:
                    break
        threading.Thread(target=loop,daemon=True).start()

    # ══════════════════════════════════════════════════════
    #  C++ & Dosya
    # ══════════════════════════════════════════════════════
    def _gen_cpp(self):
        lines = [f"// worldMap — Zelda Map Editor v3",
                 f"// {self.map_cols}x{self.map_rows}",
                 f"static uint8_t worldMap[MAP_ROWS][MAP_COLS] = {{"]
        for ri,row in enumerate(self.map_data):
            comma = "," if ri<self.map_rows-1 else ""
            lines.append(f"  {{{','.join(str(v) for v in row)}}}{comma}")
        lines.append("};")
        return "\n".join(lines)

    def _show_cpp(self):
        cpp = self._gen_cpp()
        win = tk.Toplevel(self)
        win.title("📋 C++ Kodu")
        win.configure(bg="#0d0d1a")
        win.geometry("800x520")
        txt = tk.Text(win,bg="#16213e",fg="#e8e8e8",
                      font=("Courier New",9),relief=tk.FLAT,insertbackground="white")
        txt.pack(fill=tk.BOTH,expand=True,padx=8,pady=8)
        txt.insert(tk.END,cpp)
        txt.configure(state=tk.DISABLED)
        def cp():
            self.clipboard_clear(); self.clipboard_append(cpp)
            messagebox.showinfo("Kopyalandı","Panoya kopyalandı!")
        tk.Button(win,text="📋 Panoya Kopyala",command=cp,
                  bg="#e94560",fg="white",relief=tk.FLAT,
                  font=("Segoe UI",10,"bold"),padx=16,pady=6).pack(pady=8)

    def _write_h(self):
        cpp = self._gen_cpp()
        h_path = os.path.join(GAMEHUB_DIR,"game_zelda.h")
        if not os.path.exists(h_path):
            h_path = filedialog.askopenfilename(
                filetypes=[("Header","*.h"),("All","*.*")],title="game_zelda.h seç")
        if not h_path or not os.path.exists(h_path):
            messagebox.showerror("Hata","game_zelda.h bulunamadı!"); return
        with open(h_path,"r",encoding="utf-8") as f: content=f.read()
        pat = r'static uint8_t worldMap\[MAP_ROWS\]\[MAP_COLS\] = \{[\s\S]*?\};'
        new,n = re.subn(pat,cpp,content)
        if n==0:
            messagebox.showerror("Hata","worldMap[] bulunamadı — manuel yapıştır."); return
        with open(h_path,"w",encoding="utf-8") as f: f.write(new)
        messagebox.showinfo("Güncellendi!",h_path)

    def _save(self):
        p = filedialog.asksaveasfilename(
            defaultextension=".json",
            filetypes=[("JSON","*.json"),("All","*.*")],
            initialdir=BASE_DIR,title="Haritayı Kaydet")
        if not p: return
        self._save_to(p)
        messagebox.showinfo("Kaydedildi",p)

    def _save_to(self, path):
        data = {"map":self.map_data,"cols":self.map_cols,"rows":self.map_rows,
                "tile_defs":[t for t in self.tile_defs if t["id"]>=10]}
        with open(path,"w",encoding="utf-8") as f:
            json.dump(data,f,indent=2,ensure_ascii=False)

    def _load(self):
        p = filedialog.askopenfilename(
            filetypes=[("JSON","*.json"),("All","*.*")],
            initialdir=BASE_DIR,title="Harita Yükle")
        if not p: return
        self._load_from(p)

    def _load_from(self, path):
        with open(path,encoding="utf-8") as f: data=json.load(f)
        self.map_data = data["map"]
        self.map_cols = data.get("cols",DEFAULT_MAP_COLS)
        self.map_rows = data.get("rows",DEFAULT_MAP_ROWS)
        self._cols_v.set(self.map_cols); self._rows_v.set(self.map_rows)
        for ct in data.get("tile_defs",[]):
            if not any(t["id"]==ct["id"] for t in self.tile_defs):
                self.tile_defs.append(ct)
        self.tile_defs.sort(key=lambda x:x["id"])
        self.cache.clear(); self._refresh_palette(); self._render_all()

    def _reset(self):
        if not messagebox.askyesno("Sıfırla","Varsayılan haritaya dön?"): return
        self.map_data  = [row[:] for row in DEFAULT_MAP]
        self.map_cols  = DEFAULT_MAP_COLS
        self.map_rows  = DEFAULT_MAP_ROWS
        self.tile_defs = copy.deepcopy(DEFAULT_TILE_DEFS)
        self._cols_v.set(self.map_cols); self._rows_v.set(self.map_rows)
        self.cache.clear(); self._refresh_palette(); self._render_all()

    def _auto_load(self):
        if os.path.exists(SAVE_FILE):
            try:
                if messagebox.askyesno("Otomatik Kayıt","Önceki çalışma bulundu. Yüklensin mi?"):
                    self._load_from(SAVE_FILE)
            except Exception as ex:
                print("Otomatik yükleme hatası:",ex)

    def _quit(self):
        try: self._save_to(SAVE_FILE)
        except Exception: pass
        self.destroy()


if __name__ == "__main__":
    MapEditor().mainloop()
