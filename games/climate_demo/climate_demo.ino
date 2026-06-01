/*
 * climate_demo.ino
 * LilyGO T-Display S3 — Akilli Klima Demo Ekrani
 * Cozunurluk: 170 x 320 px (ST7789, Paralel 8-bit)
 *
 * IR Protokolleri:
 *   Ozel (Ana ESP32): NEC adres=0xAC55, komut=1-6  → profil ID
 *   TV Remote direkt: bilinen NEC komut byte'lari  → profil
 *
 * Profil / TV Butonu Eslesmesi:
 *   cmd=1 / 0x5F → COOL   22C | Fan:AUTO    | Salm:ACIK
 *   cmd=2 / 0x4E → SLEEP  26C | Fan:SESSIZ  | Salm:DIKEY  | 8h
 *   cmd=3 / 0x53 → ECO    28C | Fan:DUSUK   | Salm:KAPALI
 *   cmd=4 / 0x50 → HEAT   20C | Fan:AUTO    | Salm:ACIK
 *   cmd=5 / 0x12 → TURBO  18C | Fan:MAKSIMUM| Salm:ACIK
 *   cmd=6 / 0x4F → AUTO   24C | Fan:AUTO    | Salm:AUTO
 *   0x4C → DRY  | 0x0E → FAN  | 0x4B → COOL20 | 0x48 → HEAT22
 *   0x0A → NIGHT | 0x06 → ECO+ | 0x44 → KAPALI | 0x47 → DEHUM
 */

// ================================================================
// TFT_eSPI INLINE SETUP — LilyGO T-Display S3 (Setup206)
// ================================================================
#define USER_SETUP_LOADED
#define USER_SETUP_ID 206

#define ST7789_DRIVER
#define INIT_SEQUENCE_3
#define CGRAM_OFFSET
#define TFT_RGB_ORDER    TFT_RGB
#define TFT_INVERSION_ON
#define TFT_PARALLEL_8_BIT

#define TFT_WIDTH  170
#define TFT_HEIGHT 320

#define TFT_CS   6
#define TFT_DC   7
#define TFT_RST  5
#define TFT_WR   8
#define TFT_RD   9

#define TFT_D0  39
#define TFT_D1  40
#define TFT_D2  41
#define TFT_D3  42
#define TFT_D4  45
#define TFT_D5  46
#define TFT_D6  47
#define TFT_D7  48

#define TFT_BL  38
#define TFT_BACKLIGHT_ON HIGH

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT

// ================================================================
// IR ALICI AYARLARI
// ================================================================
#define NO_LED_FEEDBACK_CODE
#define RECORD_GAP_MICROS 12000
#define IR_RECEIVE_PIN    1

// ================================================================
// KUTUPHANELER
// ================================================================
#include <TFT_eSPI.h>
#include <IRremote.hpp>

// ================================================================
// PROTOKOL TANIMLARI
// ================================================================
#define CUSTOM_PROTO_ADDR  ((uint16_t)0xAC55)

// TV kumanda NEC komut byte'lari — hex.h'daki siralama ile eslesiyor
static const uint8_t TV_CMDS[] = {
  0x5F, 0x4E, 0x53, 0x50, 0x12, 0x4F,  // P1-P6 (main profiles)
  0x4C, 0x0E, 0x4B, 0x48, 0x0A, 0x06,  // DRY, FAN, COOL20, HEAT22, NIGHT, ECO+
  0x44, 0x47                             // OFF, DEHUM
};
static const uint8_t TV_CMD_COUNT = (uint8_t)(sizeof(TV_CMDS));

// ================================================================
// ANIMASYON TIPLERI
// ================================================================
#define ANIM_COOL   0   // Mavi parcacilar asagi
#define ANIM_HEAT   1   // Kirmizi parcacilar yukari
#define ANIM_SLEEP  2   // Yavash yuzen yildizlar
#define ANIM_FAN    3   // Merkezden disha isinan parcacilar
#define ANIM_ECO    4   // Yesil dalga parcacilar
#define ANIM_TURBO  5   // Hizli yogun parcacilar
#define ANIM_AUTO   6   // Yoerunge hareketi
#define ANIM_DRY    7   // Statik (parcacik yok)
#define ANIM_OFF    8   // Animasyon yok

// ================================================================
// RENK MAKROSU — RGB888 → RGB565
// ================================================================
#define C565(r,g,b) ((uint16_t)( \
  (((uint8_t)(r) & 0xF8u) << 8) | \
  (((uint8_t)(g) & 0xFCu) << 3) | \
  ((uint8_t)(b) >> 3)            \
))

// --- Cool / Mavi ---
#define COL_COOL_TOP   C565(0x01,0x18,0x6E)
#define COL_COOL_BOT   C565(0x00,0x4E,0xC4)
#define COL_COOL_ACC   C565(0x00,0xCC,0xFF)
#define COL_COOL_TXT   C565(0xFF,0xFF,0xFF)

// --- Heat / Turuncu-Kirmizi ---
#define COL_HEAT_TOP   C565(0x6E,0x01,0x01)
#define COL_HEAT_BOT   C565(0xC4,0x30,0x00)
#define COL_HEAT_ACC   C565(0xFF,0x90,0x00)
#define COL_HEAT_TXT   C565(0xFF,0xFF,0xFF)

// --- Sleep / Gece Mavisi ---
#define COL_SLEEP_TOP  C565(0x02,0x02,0x28)
#define COL_SLEEP_BOT  C565(0x08,0x10,0x60)
#define COL_SLEEP_ACC  C565(0x40,0x60,0xFF)
#define COL_SLEEP_TXT  C565(0xCC,0xCC,0xFF)

// --- Eco / Yesil ---
#define COL_ECO_TOP    C565(0x00,0x28,0x08)
#define COL_ECO_BOT    C565(0x00,0x80,0x28)
#define COL_ECO_ACC    C565(0x00,0xFF,0x80)
#define COL_ECO_TXT    C565(0xFF,0xFF,0xFF)

// --- Turbo / Parlak Kirmizi ---
#define COL_TURBO_TOP  C565(0x50,0x00,0x00)
#define COL_TURBO_BOT  C565(0xCC,0x00,0x00)
#define COL_TURBO_ACC  C565(0xFF,0x40,0x40)
#define COL_TURBO_TXT  C565(0xFF,0xFF,0xFF)

// --- Auto / Mor ---
#define COL_AUTO_TOP   C565(0x28,0x00,0x50)
#define COL_AUTO_BOT   C565(0x60,0x00,0xA0)
#define COL_AUTO_ACC   C565(0xCC,0x60,0xFF)
#define COL_AUTO_TXT   C565(0xFF,0xFF,0xFF)

// --- Dry / Kehribar ---
#define COL_DRY_TOP    C565(0x40,0x20,0x00)
#define COL_DRY_BOT    C565(0x90,0x60,0x00)
#define COL_DRY_ACC    C565(0xFF,0xCC,0x00)
#define COL_DRY_TXT    C565(0xFF,0xFF,0xFF)

// --- Fan / Celik Mavi ---
#define COL_FAN_TOP    C565(0x00,0x20,0x40)
#define COL_FAN_BOT    C565(0x10,0x50,0x80)
#define COL_FAN_ACC    C565(0x60,0xC0,0xFF)
#define COL_FAN_TXT    C565(0xFF,0xFF,0xFF)

// --- Off / Koyu Gri ---
#define COL_OFF_TOP    C565(0x10,0x10,0x10)
#define COL_OFF_BOT    C565(0x25,0x25,0x25)
#define COL_OFF_ACC    C565(0x60,0x60,0x60)
#define COL_OFF_TXT    C565(0x80,0x80,0x80)

// ================================================================
// PROFIL YAPISI
// ================================================================
struct ACProfile {
  const char* name;      // Mod adi
  const char* subtitle;  // Alt baslik
  int8_t      temp;      // Sicaklik C (-1 = yok)
  const char* fan;       // Fan hizi
  const char* swing;     // Salim
  int8_t      timer;     // Saat (-1 = yok)
  uint16_t    bgTop;
  uint16_t    bgBot;
  uint16_t    accent;
  uint16_t    textColor;
  uint8_t     animType;
};

// ================================================================
// PROFIL TABLOSU
// [0-5]  = Ozel protokol profilleri (custom cmd 1-6 ile eslesiyor)
// [6-13] = TV remote ek buton profilleri
// ================================================================
static const ACProfile PROFILES[] = {
  // [0] P1 — COOL 22 (custom:1 / tv:0x5F)
  {"COOL",  "Sogutma Modu",   22,"AUTO",    "ACIK",   -1,COL_COOL_TOP, COL_COOL_BOT, COL_COOL_ACC, COL_COOL_TXT, ANIM_COOL },
  // [1] P2 — SLEEP 26 (custom:2 / tv:0x4E)
  {"SLEEP", "Uyku Modu",      26,"SESSIZ",  "DIKEY",   8,COL_SLEEP_TOP,COL_SLEEP_BOT,COL_SLEEP_ACC,COL_SLEEP_TXT,ANIM_SLEEP},
  // [2] P3 — ECO 28 (custom:3 / tv:0x53)
  {"ECO",   "Tasarruf Modu",  28,"DUSUK",   "KAPALI", -1,COL_ECO_TOP,  COL_ECO_BOT,  COL_ECO_ACC,  COL_ECO_TXT,  ANIM_ECO  },
  // [3] P4 — HEAT 20 (custom:4 / tv:0x50)
  {"HEAT",  "Isitma Modu",    20,"AUTO",    "ACIK",   -1,COL_HEAT_TOP, COL_HEAT_BOT, COL_HEAT_ACC, COL_HEAT_TXT, ANIM_HEAT },
  // [4] P5 — TURBO 18 (custom:5 / tv:0x12)
  {"TURBO", "Hizli Sogutma",  18,"MAKSIMUM","ACIK",   -1,COL_TURBO_TOP,COL_TURBO_BOT,COL_TURBO_ACC,COL_TURBO_TXT,ANIM_TURBO},
  // [5] P6 — AUTO 24 (custom:6 / tv:0x4F)
  {"AUTO",  "Otomatik Mod",   24,"AUTO",    "AUTO",   -1,COL_AUTO_TOP, COL_AUTO_BOT, COL_AUTO_ACC, COL_AUTO_TXT, ANIM_AUTO },
  // [6] DRY (tv:0x4C)
  {"DRY",   "Nem Alma",       22,"AUTO",    "KAPALI", -1,COL_DRY_TOP,  COL_DRY_BOT,  COL_DRY_ACC,  COL_DRY_TXT,  ANIM_DRY  },
  // [7] FAN (tv:0x0E)
  {"FAN",   "Sadece Fan",     -1,"YUKSEK",  "ACIK",   -1,COL_FAN_TOP,  COL_FAN_BOT,  COL_FAN_ACC,  COL_FAN_TXT,  ANIM_FAN  },
  // [8] COOL 20 (tv:0x4B)
  {"COOL",  "Guclu Sogutma",  20,"ORTA",    "ACIK",   -1,COL_COOL_TOP, COL_COOL_BOT, COL_COOL_ACC, COL_COOL_TXT, ANIM_COOL },
  // [9] HEAT 22 (tv:0x48)
  {"HEAT",  "Hafif Isitma",   22,"ORTA",    "ACIK",   -1,COL_HEAT_TOP, COL_HEAT_BOT, COL_HEAT_ACC, COL_HEAT_TXT, ANIM_HEAT },
  // [10] NIGHT (tv:0x0A)
  {"NIGHT", "Gece Modu",      28,"MINIMUM", "DIKEY",   6,COL_SLEEP_TOP,COL_SLEEP_BOT,COL_SLEEP_ACC,COL_SLEEP_TXT,ANIM_SLEEP},
  // [11] ECO+ (tv:0x06)
  {"ECO+",  "Yogun Tasarruf", 27,"DUSUK",   "KAPALI", -1,COL_ECO_TOP,  COL_ECO_BOT,  COL_ECO_ACC,  COL_ECO_TXT,  ANIM_ECO  },
  // [12] OFF (tv:0x44)
  {"OFF",   "Sistem Kapali",  -1,"---",     "---",    -1,COL_OFF_TOP,  COL_OFF_BOT,  COL_OFF_ACC,  COL_OFF_TXT,  ANIM_OFF  },
  // [13] DEHUM (tv:0x47)
  {"DEHUM", "Nem Giderici",   22,"AUTO",    "KAPALI", -1,COL_DRY_TOP,  COL_DRY_BOT,  COL_DRY_ACC,  COL_DRY_TXT,  ANIM_DRY  },
};
#define PROFILES_COUNT ((int)(sizeof(PROFILES)/sizeof(PROFILES[0])))

// ================================================================
// PARTIKUL SISTEMI
// ================================================================
#define MAX_PARTICLES  30
#define ANIM_Y_TOP     70
#define ANIM_Y_BOT    188
#define ANIM_CX        85
#define ANIM_CY       129

struct Particle {
  float    x, y;
  float    vx, vy;
  uint8_t  life, maxLife;
  uint8_t  sz;
  uint16_t color;
};

static Particle ptcl[MAX_PARTICLES];

void clearParticles() { memset(ptcl, 0, sizeof(ptcl)); }

void spawnParticle(uint8_t anim, uint16_t accent) {
  for (int i = 0; i < MAX_PARTICLES; i++) {
    if (ptcl[i].life > 0) continue;
    ptcl[i].color   = accent;
    ptcl[i].sz      = (uint8_t)random(1, 3);
    ptcl[i].maxLife = (uint8_t)random(25, 70);
    ptcl[i].life    = ptcl[i].maxLife;
    float ang, r;
    switch (anim) {
      case ANIM_COOL:
        ptcl[i].x  = (float)random(5,165); ptcl[i].y = (float)ANIM_Y_TOP;
        ptcl[i].vx = (float)(random(0,20)-10)/10.0f;
        ptcl[i].vy = (float)random(6,22)/10.0f;
        break;
      case ANIM_TURBO:
        ptcl[i].x  = (float)random(5,165); ptcl[i].y = (float)ANIM_Y_TOP;
        ptcl[i].vx = (float)(random(0,40)-20)/10.0f;
        ptcl[i].vy = (float)random(15,35)/10.0f;
        ptcl[i].sz = (uint8_t)random(1,4);
        ptcl[i].maxLife = (uint8_t)random(12,35);
        ptcl[i].life    = ptcl[i].maxLife;
        break;
      case ANIM_HEAT:
        ptcl[i].x  = (float)random(5,165); ptcl[i].y = (float)ANIM_Y_BOT;
        ptcl[i].vx = (float)(random(0,20)-10)/10.0f;
        ptcl[i].vy = -(float)random(6,22)/10.0f;
        break;
      case ANIM_SLEEP:
        ptcl[i].x  = (float)random(5,165);
        ptcl[i].y  = (float)random(ANIM_Y_TOP, ANIM_Y_BOT);
        ptcl[i].vx = (float)(random(0,8)-4)/10.0f;
        ptcl[i].vy = (float)(random(0,8)-4)/10.0f;
        ptcl[i].sz = 1;
        ptcl[i].maxLife = (uint8_t)random(50,120);
        ptcl[i].life    = ptcl[i].maxLife;
        break;
      case ANIM_ECO:
        ptcl[i].x  = 2.0f;
        ptcl[i].y  = (float)random(ANIM_Y_TOP+5, ANIM_Y_BOT-5);
        ptcl[i].vx = (float)random(6,16)/10.0f;
        ptcl[i].vy = sinf(ptcl[i].y * 0.15f) * 0.5f;
        ptcl[i].sz = 2;
        break;
      case ANIM_FAN:
        ang = (float)random(0,360)*(3.14159f/180.0f);
        ptcl[i].x  = (float)ANIM_CX; ptcl[i].y = (float)ANIM_CY;
        ptcl[i].vx = cosf(ang)*(float)random(12,26)/10.0f;
        ptcl[i].vy = sinf(ang)*(float)random(12,26)/10.0f;
        break;
      case ANIM_AUTO:
        ang = (float)random(0,360)*(3.14159f/180.0f);
        r   = (float)random(22,58);
        ptcl[i].x  = ANIM_CX + cosf(ang)*r;
        ptcl[i].y  = ANIM_CY + sinf(ang)*r;
        ptcl[i].vx = -sinf(ang)*1.5f;
        ptcl[i].vy =  cosf(ang)*1.5f;
        break;
      default:
        ptcl[i].life = 0;
        return;
    }
    return;
  }
}

void updateParticles(uint8_t anim, uint16_t accent) {
  if (anim == ANIM_DRY || anim == ANIM_OFF) return;
  int sc = (anim == ANIM_TURBO) ? 3 : 1;
  for (int s = 0; s < sc; s++) spawnParticle(anim, accent);
  for (int i = 0; i < MAX_PARTICLES; i++) {
    if (!ptcl[i].life) continue;
    ptcl[i].x += ptcl[i].vx;
    ptcl[i].y += ptcl[i].vy;
    ptcl[i].life--;
    if (ptcl[i].x < 0 || ptcl[i].x >= TFT_WIDTH ||
        ptcl[i].y < ANIM_Y_TOP-10 || ptcl[i].y > ANIM_Y_BOT+10)
      ptcl[i].life = 0;
  }
}

// ================================================================
// TFT GLOBAL NESNELER
// ================================================================
TFT_eSPI    tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);

// ================================================================
// UYGULAMA DURUMU
// ================================================================
int      currentProfile = 12;  // Baslangiçta OFF ekraniyla basla (PROFILES[12] = OFF)
uint32_t blinkTimer     = 0;
bool     blinkState     = false;
uint32_t customProtoProtectedUntil = 0;

// ================================================================
// GRADIENT CIZIMI
// ================================================================
void drawGradient(uint16_t topCol, uint16_t botCol) {
  int tr=(topCol>>11)&0x1F, tg=(topCol>>5)&0x3F, tb=topCol&0x1F;
  int br=(botCol >>11)&0x1F, bg=(botCol >>5)&0x3F, bb=botCol &0x1F;
  for (int y=0; y<TFT_HEIGHT; y++) {
    float t=(float)y/(float)(TFT_HEIGHT-1);
    int r=(int)(tr+(br-tr)*t); int g=(int)(tg+(bg-tg)*t); int b=(int)(tb+(bb-tb)*t);
    if(r>31)r=31; if(g>63)g=63; if(b>31)b=31;
    if(r<0)r=0;   if(g<0)g=0;   if(b<0)b=0;
    spr.drawFastHLine(0,y,TFT_WIDTH,(uint16_t)((r<<11)|(g<<5)|b));
  }
}

// ================================================================
// PARTIKUL CIZIMI
// ================================================================
void drawParticles() {
  for (int i=0; i<MAX_PARTICLES; i++) {
    if (!ptcl[i].life) continue;
    float alpha=(float)ptcl[i].life/(float)ptcl[i].maxLife;
    uint16_t c=ptcl[i].color;
    if (alpha < 0.55f) {
      float s=alpha/0.55f;
      int r=(int)(((c>>11)&0x1F)*s);
      int g=(int)(((c>>5)&0x3F)*s);
      int b=(int)((c&0x1F)*s);
      c=(uint16_t)((r<<11)|(g<<5)|b);
    }
    int px=(int)ptcl[i].x, py=(int)ptcl[i].y;
    if (ptcl[i].sz<=1) spr.drawPixel(px,py,c);
    else spr.fillCircle(px,py,(int)ptcl[i].sz,c);
  }
}

// ================================================================
// PROFIL EKRANI CIZIMI
// ================================================================
void drawProfileScreen(int idx) {
  const ACProfile& p = PROFILES[idx];

  // 1. Gradient arka plan
  drawGradient(p.bgTop, p.bgBot);

  // 2. Ust renkli serit
  spr.fillRect(0, 0, TFT_WIDTH, 5, p.accent);

  // 3. Mod adi (font4, 26px, ortali)
  spr.setTextDatum(MC_DATUM);
  spr.setTextFont(4);
  spr.setTextColor(p.textColor);
  spr.drawString(p.name, TFT_WIDTH/2, 28);

  // 4. Alt baslik (font2, 16px, vurgu rengi)
  spr.setTextFont(2);
  spr.setTextColor(p.accent);
  spr.drawString(p.subtitle, TFT_WIDTH/2, 50);

  // 5. Ust cizgi
  spr.drawFastHLine(8, 63, TFT_WIDTH-16, p.accent);

  // 6. Partikul animasyonu
  drawParticles();

  // 7. Animasyon bolgesinde buyuk sembol (soluk)
  {
    uint16_t r5=((p.accent>>11)&0x1F)*3/10;
    uint16_t g6=((p.accent>>5) &0x3F)*3/10;
    uint16_t b5= (p.accent     &0x1F)*3/10;
    uint16_t dimCol=(uint16_t)((r5<<11)|(g6<<5)|b5);
    const char* sym="*";
    switch(p.animType){
      case ANIM_COOL:  sym="*"; break;
      case ANIM_HEAT:  sym="^"; break;
      case ANIM_SLEEP: sym="z"; break;
      case ANIM_FAN:   sym="@"; break;
      case ANIM_ECO:   sym="~"; break;
      case ANIM_TURBO: sym="!"; break;
      case ANIM_AUTO:  sym="o"; break;
      case ANIM_DRY:   sym="."; break;
      case ANIM_OFF:   sym="-"; break;
    }
    spr.setTextSize(3);
    spr.setTextFont(2);
    spr.setTextDatum(MC_DATUM);
    spr.setTextColor(dimCol);
    spr.drawString(sym, ANIM_CX, ANIM_CY);
    spr.setTextSize(1);
  }

  // 8. Alt cizgi
  spr.drawFastHLine(8, ANIM_Y_BOT+4, TFT_WIDTH-16, p.accent);

  // 9. Sicaklik — font7 (7-segment 48px) + derece + C
  spr.setTextColor(p.textColor);
  if (p.temp >= 0) {
    char numStr[4];
    snprintf(numStr, sizeof(numStr), "%d", (int)p.temp);
    spr.setTextFont(7);
    spr.setTextDatum(MR_DATUM);
    spr.drawString(numStr, TFT_WIDTH/2+8, 218);
    // Derece sembolu: kucuk daire + C
    spr.drawCircle(TFT_WIDTH/2+14, 204, 4, p.accent);
    spr.setTextFont(4);
    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(p.accent);
    spr.drawString("C", TFT_WIDTH/2+21, 202);
  } else {
    spr.setTextFont(4);
    spr.setTextDatum(MC_DATUM);
    spr.setTextColor(p.accent);
    spr.drawString("-- C", TFT_WIDTH/2, 215);
  }

  // Ince ayirici
  spr.drawFastHLine(8, 242, TFT_WIDTH-16, C565(0x28,0x28,0x28));

  // 10. Fan hizi
  spr.setTextFont(2);
  spr.setTextDatum(ML_DATUM);
  spr.setTextColor(p.textColor);
  char fanStr[28];
  snprintf(fanStr, sizeof(fanStr), "Fan:   %s", p.fan);
  spr.drawString(fanStr, 12, 252);

  // 11. Salim
  char swStr[28];
  snprintf(swStr, sizeof(swStr), "Salim: %s", p.swing);
  spr.drawString(swStr, 12, 270);

  // 12. Timer
  if (p.timer > 0) {
    char tmStr[24];
    snprintf(tmStr, sizeof(tmStr), "Timer: %dh", (int)p.timer);
    spr.setTextColor(p.accent);
    spr.drawString(tmStr, 12, 288);
  } else {
    spr.setTextColor(C565(0x35,0x35,0x35));
    spr.drawString("Timer: --", 12, 288);
  }

  // 13. Profil gostergesi (alt bar)
  spr.fillRect(0, 304, TFT_WIDTH, 16, C565(0x04,0x04,0x04));
  if (idx < 6) {
    // 6 nokta: aktif = accent, pasif = gri
    for (int d=0; d<6; d++) {
      uint16_t dc = (d==idx) ? p.accent : C565(0x28,0x28,0x28);
      spr.fillCircle(22+d*20, 312, 5, dc);
    }
    char pidStr[4];
    snprintf(pidStr, sizeof(pidStr), "P%d", idx+1);
    spr.setTextFont(2);
    spr.setTextColor(p.textColor);
    spr.setTextDatum(MR_DATUM);
    spr.drawString(pidStr, TFT_WIDTH-6, 312);
  } else {
    // TV remote direkt — "TV" yazisi
    spr.setTextFont(2);
    spr.setTextColor(p.accent);
    spr.setTextDatum(MC_DATUM);
    spr.drawString("TV REMOTE", TFT_WIDTH/2, 312);
  }

  // 14. Alt renkli serit
  spr.fillRect(0, TFT_HEIGHT-4, TFT_WIDTH, 4, p.accent);

  spr.pushSprite(0, 0);
}

// ================================================================
// SINYAL YOK EKRANI
// ================================================================
void drawNoSignalScreen() {
  drawGradient(C565(0x04,0x04,0x14), C565(0x08,0x08,0x28));

  // Ust serit
  spr.fillRect(0, 0, TFT_WIDTH, 4, C565(0x20,0x40,0xFF));

  // Baslik
  spr.setTextDatum(MC_DATUM);
  spr.setTextFont(4);
  spr.setTextColor(C565(0xFF,0xFF,0xFF));
  spr.drawString("AC KONTROL", TFT_WIDTH/2, 90);

  // Alt baslik
  spr.setTextFont(2);
  spr.setTextColor(C565(0x60,0x80,0xFF));
  spr.drawString("Demo Ekrani", TFT_WIDTH/2, 116);

  // Cizgi
  spr.drawFastHLine(28, 134, TFT_WIDTH-56, C565(0x20,0x40,0xFF));

  // Yanip sonen bekleme yazisi
  if (millis()-blinkTimer > 700) { blinkTimer=millis(); blinkState=!blinkState; }
  if (blinkState) {
    spr.setTextFont(2);
    spr.setTextColor(C565(0x40,0x60,0xCC));
    spr.drawString("Sinyal bekleniyor...", TFT_WIDTH/2, 165);
  }

  // Teknik bilgi
  spr.setTextFont(2);
  spr.setTextColor(C565(0x20,0x30,0x55));
  char buf[36];
  snprintf(buf, sizeof(buf), "NEC 0xAC55 | GPIO %d", (int)IR_RECEIVE_PIN);
  spr.drawString(buf, TFT_WIDTH/2, 228);

  // TV remote bilgi
  spr.setTextColor(C565(0x18,0x28,0x48));
  spr.drawString("TV remote direkt aktif", TFT_WIDTH/2, 248);

  // Alt serit
  spr.fillRect(0, TFT_HEIGHT-4, TFT_WIDTH, 4, C565(0x20,0x40,0xFF));

  spr.pushSprite(0, 0);
}

// ================================================================
// IR ARAMA FONKSIYONLARI
// ================================================================
int getProfileByCustomCmd(uint8_t cmd) {
  // Ana cihaz artik ham TV cmd byte'ini gonderiyor (sabit ID degil)
  // Direkt TV_CMDS tablosunda ara — TV remote ile ayni mantik
  return getProfileByTVCmd(cmd);
}

int getProfileByTVCmd(uint8_t cmd) {
  for (int i=0; i<TV_CMD_COUNT; i++)
    if (TV_CMDS[i] == cmd) return i;
  return -1;
}

// ================================================================
// IR ISLEYICI
// ================================================================
void handleIR() {
  if (!IrReceiver.decode()) return;

  uint16_t addr = IrReceiver.decodedIRData.address;
  uint8_t  cmd  = (uint8_t)(IrReceiver.decodedIRData.command & 0xFF);

  Serial.printf("IR: addr=0x%04X cmd=0x%02X\n", addr, cmd);

  int newProfile = -2;

  if (addr == CUSTOM_PROTO_ADDR) {
    // Ana ESP32'den gelen ozel protokol
    // 1000ms boyunca TV remote sinyallerini reddet
    customProtoProtectedUntil = millis() + 1000;

    if (cmd == 0x00) {
      currentProfile = 12;  // OFF profili (PROFILES[12])
      clearParticles();
      Serial.println("Custom: OFF sinyali -> OFF ekrani");
      IrReceiver.resume();
      return;
    }
    newProfile = getProfileByCustomCmd(cmd);
    Serial.printf("Custom protokol: profil=%d\n", newProfile);

  } else {
    // TV remote direkt
    // Custom proto korumasi aktifse bu sinyali yok say
    if ((int32_t)(millis() - customProtoProtectedUntil) < 0) {
      Serial.println("TV remote yok sayildi (custom proto korumasi aktif)");
      IrReceiver.resume();
      return;
    }
    newProfile = getProfileByTVCmd(cmd);
    Serial.printf("TV remote direkt: profil=%d (addr=0x%04X)\n", newProfile, addr);
  }

  if (newProfile >= 0 && newProfile < PROFILES_COUNT) {
    if (newProfile != currentProfile) {
      currentProfile = newProfile;
      clearParticles();
      Serial.printf("Profil aktif: %d (%s)\n", currentProfile, PROFILES[currentProfile].name);
    }
  } else if (newProfile == -2) {
    Serial.println("Bilinmeyen IR sinyali, yok sayildi.");
  }

  IrReceiver.resume();
}

// ================================================================
// SETUP
// ================================================================
void setup() {
  Serial.begin(115200);
  delay(400);

  Serial.println("\n===================================");
  Serial.println(" LilyGO T-Display S3 — AC Demo");
  Serial.printf(" IR Pin    : GPIO %d\n", (int)IR_RECEIVE_PIN);
  Serial.printf(" Proto Addr: 0x%04X\n", (int)CUSTOM_PROTO_ADDR);
  Serial.printf(" Profil    : %d adet\n", PROFILES_COUNT);
  Serial.println("===================================");

  // Arka isik
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);

  // TFT baslatma
  tft.init();
  tft.setRotation(0);   // Portrait: 170x320
  tft.fillScreen(0x0000);

  // Sprite (tam ekran double-buffer, ~106KB RAM)
  if (!spr.createSprite(TFT_WIDTH, TFT_HEIGHT)) {
    Serial.println("HATA: Sprite olusturulamadi! RAM yetersiz.");
    while(1) delay(500);
  }
  spr.setSwapBytes(true);

  // IR alici
  IrReceiver.begin(IR_RECEIVE_PIN, DISABLE_LED_FEEDBACK);
  Serial.println("IR alici hazir.");
  Serial.println("===================================\n");
}

// ================================================================
// LOOP
// ================================================================
void loop() {
  // IR her iterasyonda kontrol (non-blocking)
  handleIR();

  // ~30 FPS render
  static uint32_t lastRender = 0;
  uint32_t now = millis();
  if (now - lastRender < 33) return;
  lastRender = now;

  if (currentProfile >= 0 && currentProfile < PROFILES_COUNT) {
    updateParticles(PROFILES[currentProfile].animType, PROFILES[currentProfile].accent);
    drawProfileScreen(currentProfile);
  } else {
    drawNoSignalScreen();
  }
}