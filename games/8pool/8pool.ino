// ============================================================
//  8 BALL POOL  —  ESP32 / TFT_eSPI
//  Encoder (47/48) → nişan açısı (smooth döndürme)
//  jumpButton  (2)  → basılı tut = güç yükle, bırak = vur
//  snapButton  (6)  → her basışta +15° sabit adım
// ============================================================

#include <TFT_eSPI.h>
#include <Wire.h>
#include <vector>
#include <cmath>
#include <RotaryEncoder.h>
using namespace std;

// ==================== Donanım ====================
#define jumpButton   2    // güç / vur  (sağ shoulder)
#define snapButton   39   // 15° adım   (sol shoulder)
#define PIN_IN1      48
#define PIN_IN2      47

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite img = TFT_eSprite(&tft);

RotaryEncoder encoder(PIN_IN1, PIN_IN2, RotaryEncoder::LatchMode::TWO03);

// ==================== Ekran / Masa ====================
const int SCREEN_W = 172;
const int SCREEN_H = 320;
const int TABLE_L  = 10;
const int TABLE_R  = 162;
const int TABLE_T  = 10;
const int TABLE_B  = 310;
const int POCKET_R = 7;

// Masa renkleri
#define COL_TABLE    0x0340   // koyu yeşil yüzey
#define COL_FELT     0x0540   // hafif açık yeşil iç
#define COL_RAIL     0x1A00   // koyu kahve kenar
#define COL_RAIL2    0xA540   // açık ahşap şerit
#define COL_CUE      0xFDA0   // kue rengi (bal rengi)
#define COL_POCKET   0x0000   // cep siyah
#define COL_POWER_BG 0x18C3
#define COL_DOTLINE  0x8C71   // nişan çizgisi

// ==================== Top Tanımları ====================
// Gerçek 8-ball pool renkleri (RGB565)
#define BALL_1_YEL   0xFFE0   // 1 — Sarı (solid)
#define BALL_2_BLU   0x001F   // 2 — Mavi (solid)
#define BALL_3_RED   0xF800   // 3 — Kırmızı (solid)
#define BALL_4_PUR   0x780F   // 4 — Mor (solid)
#define BALL_5_ORA   0xFC60   // 5 — Turuncu (solid)
#define BALL_6_GRN   0x03E0   // 6 — Yeşil (solid)
#define BALL_7_MAR   0x8000   // 7 — Bordo (solid)
#define BALL_8_BLK   0x0000   // 8 — Siyah (solid)
// 9-15 aynı renkler ama STRIPE (beyaz zemin, ortada renkli daire)
#define BALL_9_YEL   0xFFE0
#define BALL_10_BLU  0x001F
#define BALL_11_RED  0xF800
#define BALL_12_PUR  0x780F
#define BALL_13_ORA  0xFC60
#define BALL_14_GRN  0x03E0
#define BALL_15_MAR  0x8000

// ==================== Veri Yapıları ====================
struct sBall {
  float    px, py;
  float    ox, oy;
  float    vx, vy;
  float    ax, ay;
  float    radius;
  uint16_t bColor;      // ana renk (solid) veya iç daire rengi (stripe)
  float    mass;
  int      id;
  bool     pocketed;
  bool     isCue;
  bool     isStripe;    // true = çizgili top (beyaz zemin + renkli iç daire)
};

struct sLineSegment {
  float sx, sy, ex, ey;
  float radius;
};

struct sPocket {
  float px, py, pradius;
};

vector<sBall>                   vecBalls;
vector<sLineSegment>            vecLines;
vector<sPocket>                 vecPocket;
vector<sBall*>                  vecFakeBalls;
vector<pair<sBall*, sBall*>>    vecCollidingPairs;

// ==================== Oyun Durumu ====================
enum GameState { AIMING, ROLLING, GAME_OVER };
GameState gameState = AIMING;

// Encoder / açı
int   prevEncoderPos = 0;
int   newEncoderPos  = 0;
float aimAngle       = (float)M_PI;   // başlangıç: sola bak

// Güç (basılı tut)
bool          btnHeld      = false;
unsigned long btnHoldStart = 0;
float         shotPower    = 0.0f;    // 0.0 – 1.0
const float   MAX_POWER    = 1300.0f;
const float   HOLD_TIME    = 1800.0f; // ms

// Skor / zamanlama
int           score        = 0;
unsigned long prevMillis   = 0;
const unsigned long FRAME  = 30;

// Buton önceki durumları
int  lastJumpState = HIGH;
int  lastSnapState = HIGH;

// ==================== Yardımcılar ====================
static bool allStopped() {
  for (const auto& b : vecBalls) {
    if (b.pocketed) continue;
    if (b.vx * b.vx + b.vy * b.vy > 0.08f) return false;
  }
  return true;
}

static bool allColorPocketed() {
  for (const auto& b : vecBalls) {
    if (!b.isCue && !b.pocketed) return false;
  }
  return true;
}

static sBall* getCueBall() {
  for (auto& b : vecBalls) {
    if (b.isCue && !b.pocketed) return &b;
  }
  return nullptr;
}

// ==================== Kurulum ====================
void addBall(float x, float y, float r,
             uint16_t color, bool cue = false, bool stripe = false) {
  sBall b;
  b.px = x;  b.py = y;
  b.vx = 0;  b.vy = 0;
  b.ax = 0;  b.ay = 0;
  b.radius   = r;
  b.mass     = r * 10.0f;
  b.bColor   = color;
  b.id       = vecBalls.size();
  b.pocketed = false;
  b.isCue    = cue;
  b.isStripe = stripe;
  vecBalls.push_back(b);
}

void addLine(float x1, float y1, float x2, float y2) {
  sLineSegment l;
  l.sx = x1; l.sy = y1;
  l.ex = x2; l.ey = y2;
  l.radius = 0;
  vecLines.push_back(l);
}

void addPocket(float x, float y) {
  sPocket p;
  p.px = x; p.py = y;
  p.pradius = (float)POCKET_R;
  vecPocket.push_back(p);
}

void resetTable() {
  vecBalls.clear();
  vecLines.clear();
  vecPocket.clear();
  score     = 0;
  gameState = AIMING;
  aimAngle  = (float)M_PI;   // sola

  // -------- Beyaz top (cue) — alt merkez --------
  addBall(86, 238, 5, TFT_WHITE, true, false);

  // -------- Üçgen dizilim (resmi 8-ball rack) --------
  // Apex (row 0) cue topa en yakın, base (row 4) en uzak
  // row 0 → y yüksek (152), row 4 → y küçük (112)
  const float bx  = 86.0f;
  const float by  = 152.0f;   // apex y
  const float rs  = 11.6f;    // top merkez arası mesafe

  // Resmi dizilim (satır × sütun):
  //  Row 0:  [1]
  //  Row 1:  [7] [12]
  //  Row 2:  [11][8] [6]    ← 8-ball ortada
  //  Row 3:  [4] [3] [9][14]
  //  Row 4:  [15][2] [13][5][10]  ← köşeler: 1 solid (2) + 1 stripe (15/10)

  // struct: { color, isStripe }
  struct BallDef { uint16_t color; bool stripe; };
  const BallDef rack[15] = {
    { BALL_1_YEL,  false },   // row0, col0  — 1
    { BALL_7_MAR,  false },   // row1, col0  — 7
    { BALL_12_PUR, true  },   // row1, col1  — 12
    { BALL_11_RED, true  },   // row2, col0  — 11
    { BALL_8_BLK,  false },   // row2, col1  — 8 (ortada!)
    { BALL_6_GRN,  false },   // row2, col2  — 6
    { BALL_4_PUR,  false },   // row3, col0  — 4
    { BALL_3_RED,  false },   // row3, col1  — 3
    { BALL_9_YEL,  true  },   // row3, col2  — 9
    { BALL_14_GRN, true  },   // row3, col3  — 14
    { BALL_15_MAR, true  },   // row4, col0  — 15 (sol köşe: stripe)
    { BALL_2_BLU,  false },   // row4, col1  — 2
    { BALL_13_ORA, true  },   // row4, col2  — 13
    { BALL_5_ORA,  false },   // row4, col3  — 5
    { BALL_10_BLU, true  },   // row4, col4  — 10 (sağ köşe: stripe)
  };

  int k = 0;
  for (int row = 0; row < 5; row++) {
    for (int col = 0; col <= row; col++) {
      // Apex en altta (by), base yukarıda (y azalır)
      float x = bx + (col - row * 0.5f) * rs;
      float y = by - row * rs * 0.866f;
      addBall(x, y, 5, rack[k].color, false, rack[k].stripe);
      k++;
    }
  }

  // -------- Masa duvarları (cep aralarında açık) --------
  int mid = (TABLE_T + TABLE_B) / 2;
  // Sol duvar
  addLine(TABLE_L, TABLE_T + POCKET_R + 2, TABLE_L, mid - POCKET_R - 2);
  addLine(TABLE_L, mid + POCKET_R + 2,     TABLE_L, TABLE_B - POCKET_R - 2);
  // Sağ duvar
  addLine(TABLE_R, TABLE_T + POCKET_R + 2, TABLE_R, mid - POCKET_R - 2);
  addLine(TABLE_R, mid + POCKET_R + 2,     TABLE_R, TABLE_B - POCKET_R - 2);
  // Üst duvar
  addLine(TABLE_L + POCKET_R + 2, TABLE_T, TABLE_R - POCKET_R - 2, TABLE_T);
  // Alt duvar
  addLine(TABLE_L + POCKET_R + 2, TABLE_B, TABLE_R - POCKET_R - 2, TABLE_B);

  // -------- 6 Cep --------
  addPocket(TABLE_L, TABLE_T);
  addPocket(TABLE_R, TABLE_T);
  addPocket(TABLE_L, mid);
  addPocket(TABLE_R, mid);
  addPocket(TABLE_L, TABLE_B);
  addPocket(TABLE_R, TABLE_B);
}

// ==================== Fizik (Korundu) ====================
const float FRICTION = 0.985f;
const float fStable  = 0.5f;

void collisionDetect() {
  auto doCircleOverlap = [](float x1,float y1,float r1,
                             float x2,float y2,float r2) {
    float dx=x1-x2, dy=y1-y2;
    return dx*dx+dy*dy <= (r1+r2)*(r1+r2);
  };

  // Fizik güncelle
  for (auto& ball : vecBalls) {
    if (ball.pocketed) continue;
    ball.ox = ball.px; ball.oy = ball.py;
    ball.ax = -ball.vx * 0.6f;
    ball.ay = -ball.vy * 0.6f;
    ball.vx += ball.ax * 0.01f;
    ball.vy += ball.ay * 0.01f;
    ball.px += ball.vx * 0.01f;
    ball.py += ball.vy * 0.01f;
    // Belirli bir momentumun altındaysa direkt sıfırla (titreme önleme)
    if (fabs(ball.vx) < 1.8f) ball.vx = 0.0f;
    if (fabs(ball.vy) < 1.8f) ball.vy = 0.0f;
  }

  // Duvar çarpışmaları
  for (auto& ball : vecBalls) {
    if (ball.pocketed) continue;
    for (auto& edge : vecLines) {
      float lx1 = edge.ex-edge.sx, ly1 = edge.ey-edge.sy;
      float lx2 = ball.px-edge.sx, ly2 = ball.py-edge.sy;
      float elen = lx1*lx1 + ly1*ly1;
      float t = max(0.0f, min(elen, lx1*lx2+ly1*ly2)) / elen;
      float cpx = edge.sx + t*lx1;
      float cpy = edge.sy + t*ly1;
      float dx = ball.px-cpx, dy = ball.py-cpy;
      float dist = sqrtf(dx*dx+dy*dy);
      if (dist <= ball.radius + edge.radius) {
        sBall* fb = new sBall();
        fb->radius=edge.radius; fb->mass=ball.mass*0.8f;
        fb->px=cpx; fb->py=cpy;
        fb->vx=-ball.vx; fb->vy=-ball.vy;
        vecFakeBalls.push_back(fb);
        vecCollidingPairs.push_back({&ball, fb});
        float ov = 1.0f*(dist-ball.radius-fb->radius);
        ball.px -= ov*(ball.px-fb->px)/dist;
        ball.py -= ov*(ball.py-fb->py)/dist;
      }
    }

    // Top–top çarpışmaları
    for (auto& target : vecBalls) {
      if (ball.id==target.id || target.pocketed) continue;
      if (doCircleOverlap(ball.px,ball.py,ball.radius,
                          target.px,target.py,target.radius)) {
        vecCollidingPairs.push_back({&ball,&target});
        float dx=ball.px-target.px, dy=ball.py-target.py;
        float dist=sqrtf(dx*dx+dy*dy);
        if (dist<0.001f) dist=0.001f;
        float ov=(dist-ball.radius-target.radius)*0.5f;
        ball.px   -= ov*dx/dist; ball.py   -= ov*dy/dist;
        target.px += ov*dx/dist; target.py += ov*dy/dist;
      }
    }
  }

  // Momentum çözümü
  for (auto c : vecCollidingPairs) {
    sBall* b1=c.first; sBall* b2=c.second;
    float dist=sqrtf(pow(b1->px-b2->px,2)+pow(b1->py-b2->py,2));
    if (dist<0.001f) dist=0.001f;
    float nx=(b2->px-b1->px)/dist, ny=(b2->py-b1->py)/dist;
    float tx=-ny, ty=nx;
    float dpTan1=b1->vx*tx+b1->vy*ty, dpTan2=b2->vx*tx+b2->vy*ty;
    float dpN1  =b1->vx*nx+b1->vy*ny, dpN2  =b2->vx*nx+b2->vy*ny;
    float m1=(dpN1*(b1->mass-b2->mass)+2.0f*b2->mass*dpN2)/(b1->mass+b2->mass);
    float m2=(dpN2*(b2->mass-b1->mass)+2.0f*b1->mass*dpN1)/(b1->mass+b2->mass);
    b1->vx=tx*dpTan1+nx*m1; b1->vy=ty*dpTan1+ny*m1;
    b2->vx=tx*dpTan2+nx*m2; b2->vy=ty*dpTan2+ny*m2;
  }

  vecCollidingPairs.clear();
  for (auto f : vecFakeBalls) delete f;
  vecFakeBalls.clear();
}

// ==================== Cep Kontrolü ====================
void checkPockets() {
  for (auto& ball : vecBalls) {
    if (ball.pocketed) continue;
    for (const auto& p : vecPocket) {
      float dx=ball.px-p.px, dy=ball.py-p.py;
      float threshold = p.pradius + ball.radius - 2;
      if (dx*dx+dy*dy <= threshold*threshold) {
        if (ball.isCue) {
          // Beyaz cebe girdiyse geri getir — ceza yok ama pozisyon sıfırla
          ball.px=86; ball.py=238;
          ball.vx=0;  ball.vy=0;
        } else {
          ball.pocketed = true;
          ball.vx=0;   ball.vy=0;
          score += 10;
        }
        break;
      }
    }
  }
}

// ==================== TOP ÇİZİMİ ====================
void drawOneBall(const sBall& b) {
  int cx = (int)b.px;
  int cy = (int)b.py;
  int r  = (int)b.radius;

  if (b.isCue) {
    // Beyaz top: tam beyaz + çerçeve + parlama
    img.fillCircle(cx, cy, r, TFT_WHITE);
    img.drawCircle(cx, cy, r, 0xC618);
    img.drawPixel(cx-1, cy-1, TFT_WHITE);
    return;
  }

  if (!b.isStripe) {
    // === SOLID TOP: tam dolu renk ===
    img.fillCircle(cx, cy, r, b.bColor);
    // Parlama (sol üst köşe açık piksel)
    if (r >= 4) {
      img.fillCircle(cx-1, cy-1, 1, 0xFFFF);
    }
    // İnce koyu kenar
    img.drawCircle(cx, cy, r, (uint16_t)(b.bColor >> 1) & 0x7BEF);
  } else {
    // === STRIPE TOP: beyaz zemin + ortada renkli daire ===
    img.fillCircle(cx, cy, r, TFT_WHITE);
    // Ortada renkli daire (yarıçap = topun ~%55'i)
    int sr = max(2, (int)(r * 0.55f));
    img.fillCircle(cx, cy, sr, b.bColor);
    // Parlama
    img.fillCircle(cx-1, cy-1, 1, TFT_WHITE);
    // Dış çerçeve
    img.drawCircle(cx, cy, r, 0xC618);
  }
}

// ==================== MASA ÇİZİMİ ====================
void drawTable() {
  // Arka plan (kenar ray)
  img.fillSprite(COL_RAIL);

  // Ray ahşap şeridi
  img.fillRect(TABLE_L-3, TABLE_T-3,
               TABLE_R-TABLE_L+6, TABLE_B-TABLE_T+6, COL_RAIL2);

  // Masa yüzeyi (keçe)
  img.fillRect(TABLE_L, TABLE_T,
               TABLE_R-TABLE_L, TABLE_B-TABLE_T, COL_TABLE);

  // Hafif iç çizgi (keçe sınırı)
  img.drawRect(TABLE_L, TABLE_T,
               TABLE_R-TABLE_L, TABLE_B-TABLE_T, COL_FELT);

  // Orta nokta işareti (beyaz spot)
  int mid = (TABLE_T + TABLE_B) / 2;
  img.fillCircle(86, mid, 2, 0x3186);
  // Dördüncü çizgi noktası (head spot)
  img.fillCircle(86, TABLE_T + (TABLE_B-TABLE_T)/4, 2, 0x3186);

  // 6 Cep
  for (const auto& p : vecPocket) {
    int px = (int)p.px, py = (int)p.py;
    img.fillCircle(px, py, (int)p.pradius+3, 0x1082);
    img.fillCircle(px, py, (int)p.pradius+1, 0x0861);
    img.fillCircle(px, py, (int)p.pradius,   COL_POCKET);
  }
}

// Nişan çizgisi
void drawAimLine(const sBall& cue) {
  float dx = cosf(aimAngle);
  float dy = sinf(aimAngle);
  for (int step = 9; step < 85; step += 9) {
    float lx = cue.px + dx * step;
    float ly = cue.py + dy * step;
    if (lx > TABLE_L+1 && lx < TABLE_R-1 &&
        ly > TABLE_T+1 && ly < TABLE_B-1) {
      img.fillCircle((int)lx, (int)ly, 1, COL_DOTLINE);
    }
  }
  // İlk nokta biraz daha parlak
  img.fillCircle((int)(cue.px+dx*9), (int)(cue.py+dy*9), 1, TFT_WHITE);
}

// Kue çubuğu
void drawCue(const sBall& cue, float power) {
  float dx = cosf(aimAngle);
  float dy = sinf(aimAngle);
  float pull = 6.0f + power * 20.0f;
  float sx = cue.px - dx * (cue.radius + pull);
  float sy = cue.py - dy * (cue.radius + pull);
  float ex = cue.px - dx * (cue.radius + pull + 50.0f);
  float ey = cue.py - dy * (cue.radius + pull + 50.0f);
  // Kue gövdesi (3 paralel çizgi)
  for (int t = -2; t <= 2; t++) {
    float ox = -dy * t, oy = dx * t;
    img.drawLine((int)(sx+ox),(int)(sy+oy),(int)(ex+ox),(int)(ey+oy), COL_CUE);
  }
  // Kue ucu (beyaz)
  img.fillCircle((int)sx,(int)sy, 2, TFT_WHITE);
  // Kue sonu (koyu)
  img.fillCircle((int)ex,(int)ey, 3, 0x6B4D);
}

// Güç barı (sol kenar)
void drawPowerBar(float power) {
  int bx=2, by=255, bw=6, bh=45;
  img.fillRect(bx,by,bw,bh, COL_POWER_BG);
  int filled = (int)(power * bh);
  if (filled > 0) {
    uint16_t col = (power < 0.4f) ? 0x07E0 :
                   (power < 0.75f ? 0xFFE0 : 0xF800);
    img.fillRect(bx, by+bh-filled, bw, filled, col);
  }
  img.drawRect(bx,by,bw,bh, TFT_WHITE);
  img.setTextColor(TFT_WHITE);
  img.setTextSize(1);
  img.setCursor(1, 248);
  img.print("P");
}

// UI — üst bar
void drawUI() {
  img.fillRect(0, 0, SCREEN_W, 9, 0x1082);
  img.setTextColor(TFT_YELLOW);
  img.setTextSize(1);
  img.setCursor(3, 1);
  img.print("8POOL");
  img.setTextColor(TFT_WHITE);
  img.setCursor(55, 1);
  img.print("Skor:");
  img.print(score);
  // Kalan top
  int rem=0;
  for (const auto& b : vecBalls)
    if (!b.isCue && !b.pocketed) rem++;
  img.setTextColor(TFT_CYAN);
  img.setCursor(130, 1);
  img.print("x"); img.print(rem);
}

// Game over ekranı
void drawGameOver() {
  img.fillSprite(COL_RAIL);
  img.fillRect(10,30,152,260,0x0861);
  img.drawRect(10,30,152,260, COL_RAIL2);

  img.setTextColor(TFT_YELLOW);
  img.setTextSize(2);
  img.setCursor(22, 55);
  img.print("YOU WIN!");

  img.setTextColor(TFT_WHITE);
  img.setTextSize(1);
  img.setCursor(42, 100);
  img.print("Final Score:");
  img.setTextColor(TFT_YELLOW);
  img.setTextSize(3);
  img.setCursor(52, 118);
  img.print(score);

  // Küçük top ikonları (sıra sıra)
  uint16_t topColors[] = {
    BALL_1_YEL, BALL_2_BLU, BALL_3_RED, BALL_4_PUR,
    BALL_5_ORA, BALL_6_GRN, BALL_7_MAR, BALL_8_BLK
  };
  for (int i = 0; i < 8; i++) {
    int tx = 24 + i*18;
    int ty = 175;
    img.fillCircle(tx, ty, 6, topColors[i]);
    img.drawCircle(tx, ty, 6, 0x4208);
  }

  // Buton
  img.fillRoundRect(36, 210, 100, 28, 8, TFT_YELLOW);
  img.fillRoundRect(38, 212, 96,  24, 6, 0x0861);
  img.setCursor(42, 220);
  img.setTextSize(1);
  img.setTextColor(TFT_WHITE);
  img.print("  PLAY AGAIN");
  img.pushSprite(0, 0);
}

// ==================== SETUP ====================
void setup() {
  pinMode(15, OUTPUT);
  digitalWrite(15, 1);

  Serial.begin(115200);
  pinMode(jumpButton, INPUT_PULLUP);
  pinMode(snapButton, INPUT_PULLUP);
  pinMode(PIN_IN1,    INPUT_PULLUP);
  pinMode(PIN_IN2,    INPUT_PULLUP);

  tft.init();
  tft.setRotation(4);
  tft.setSwapBytes(true);
  tft.fillScreen(TFT_BLACK);

  img.createSprite(SCREEN_W, SCREEN_H);
  img.setTextDatum(0);

  resetTable();
  img.pushSprite(0, 0);
}

// ==================== LOOP ====================
void loop() {
  unsigned long now = millis();
  if (now - prevMillis < FRAME) return;
  prevMillis = now;

  // ---- GAME OVER ----
  if (gameState == GAME_OVER) {
    drawGameOver();
    int j = digitalRead(jumpButton);
    if (j == LOW && lastJumpState == HIGH) {
      resetTable();
    }
    lastJumpState = j;
    return;
  }

  // ---- Encoder ----
  encoder.tick();
  newEncoderPos = encoder.getPosition();
  if (newEncoderPos != prevEncoderPos) {
    int delta = newEncoderPos - prevEncoderPos;
    aimAngle += delta * (3.0f * (float)M_PI / 180.0f);
    while (aimAngle >  2.0f*(float)M_PI) aimAngle -= 2.0f*(float)M_PI;
    while (aimAngle <  0.0f)             aimAngle += 2.0f*(float)M_PI;
    prevEncoderPos = newEncoderPos;
  }

  // ---- Snap butonu (15° sabit adım) ----
  int snapNow = digitalRead(snapButton);
  if (snapNow == LOW && lastSnapState == HIGH && gameState == AIMING) {
    aimAngle += 15.0f * (float)M_PI / 180.0f;
    if (aimAngle > 2.0f*(float)M_PI) aimAngle -= 2.0f*(float)M_PI;
  }
  lastSnapState = snapNow;

  // ---- Jump butonu (güç / vur) ----
  int jumpNow = digitalRead(jumpButton);
  bool justPressed  = (jumpNow == LOW  && lastJumpState == HIGH);
  bool justReleased = (jumpNow == HIGH && lastJumpState == LOW);
  lastJumpState = jumpNow;

  // ==================== STATE ====================
  if (gameState == AIMING) {
    if (justPressed) {
      btnHeld      = true;
      btnHoldStart = now;
      shotPower    = 0.0f;
    }
    if (btnHeld && jumpNow == LOW) {
      shotPower = min((now - btnHoldStart) / HOLD_TIME, 1.0f);
    }
    if (justReleased && btnHeld) {
      btnHeld = false;
      sBall* cue = getCueBall();
      if (cue && shotPower > 0.02f) {
        cue->vx = cosf(aimAngle) * shotPower * MAX_POWER;
        cue->vy = sinf(aimAngle) * shotPower * MAX_POWER;
        gameState = ROLLING;
      }
      shotPower = 0.0f;
    }
  }

  if (gameState == ROLLING) {
    collisionDetect();
    checkPockets();
    if (allStopped()) {
      if (allColorPocketed()) {
        gameState = GAME_OVER;
      } else {
        gameState = AIMING;
      }
    }
  }

  // ==================== ÇİZİM ====================
  drawTable();

  // Topları çiz (cebe düşenler hariç)
  for (const auto& b : vecBalls) {
    if (!b.pocketed) drawOneBall(b);
  }

  drawUI();

  if (gameState == AIMING) {
    sBall* cue = getCueBall();
    if (cue) {
      drawAimLine(*cue);
      drawCue(*cue, shotPower);
    }
    drawPowerBar(shotPower);
  }

  if (gameState == ROLLING) {
    img.setTextColor(TFT_CYAN);
    img.setTextSize(1);
    img.setCursor(62, 305);
    img.print("rolling..");
  }

  img.pushSprite(0, 0);
}
