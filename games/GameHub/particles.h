// particles.h — GameHub ortak particle sistemi
// climate_demo'dan uyarlanmıştır — her oyuna include edilebilir
#pragma once
#include <TFT_eSPI.h>
#include <cmath>
#include <cstring>

// ==================== Tip Tanımları ====================
#define PT_NONE       0
#define PT_SPARKLE    1   // Satır/level clear spark
#define PT_DUST       2   // Düşüş tozu
#define PT_RAIN       3   // Aşağıdan yukarı yağmur (game over)
#define PT_BURST      4   // Merkezden patlama
#define PT_AMBIENT    5   // Yavaş akan ortam partikülleri
#define PT_CONFETTI   6   // Gökkuşağı konfeti

#define MAX_PT 28

struct Particle {
  float    x, y;
  float    vx, vy;
  uint8_t  life, maxLife;
  uint8_t  sz;
  uint16_t color;
  uint8_t  type;
};

// Namespace ile sarılı — her oyunun kendi örneği olsun
struct ParticleSystem {
  Particle p[MAX_PT];

  void clear() { memset(p, 0, sizeof(p)); }

  // Fade-out alpha blending (RGB565)
  static uint16_t fadeColor(uint16_t c, float alpha) {
    if (alpha >= 1.0f) return c;
    int r = (int)(((c >> 11) & 0x1F) * alpha);
    int g = (int)(((c >>  5) & 0x3F) * alpha);
    int b = (int)( (c        & 0x1F) * alpha);
    return (uint16_t)((r << 11) | (g << 5) | b);
  }

  // ==================== Spawn ====================
  void spawn(uint8_t type, float ox, float oy,
             uint16_t col, uint8_t minSz=1, uint8_t maxSz=3,
             float vxRange=2.0f, float vyRange=2.0f, float vyBias=0.0f) {
    for (int i = 0; i < MAX_PT; i++) {
      if (p[i].life > 0) continue;
      p[i].type    = type;
      p[i].x       = ox + (float)(random(-30, 30)) / 10.0f;
      p[i].y       = oy;
      p[i].vx      = (float)(random(-100, 101)) / 100.0f * vxRange;
      p[i].vy      = (float)(random(-100, 101)) / 100.0f * vyRange + vyBias;
      p[i].sz      = (uint8_t)random(minSz, maxSz + 1);
      p[i].maxLife = (uint8_t)random(18, 55);
      p[i].life    = p[i].maxLife;
      p[i].color   = col;
      return;
    }
  }

  // Satır temizleme — yatay patlama
  void spawnLineClear(int y, uint16_t col, int screenW = 172) {
    for (int n = 0; n < 8; n++) {
      float ox = (float)random(4, screenW - 4);
      spawn(PT_SPARKLE, ox, (float)y, col, 1, 3, 2.5f, 2.0f, -0.8f);
    }
  }

  // Blok düşüş tozu — tabanda küçük toz
  void spawnDust(int x, int y, uint16_t col) {
    for (int n = 0; n < 4; n++)
      spawn(PT_DUST, (float)x, (float)y, col, 1, 2, 1.2f, 0.5f, -0.3f);
  }

  // Game-over yağmur — yukarıdan aşağı
  void spawnRain(int screenW = 172) {
    for (int n = 0; n < 3; n++) {
      int slot = -1;
      for (int i = 0; i < MAX_PT; i++) { if (!p[i].life) { slot = i; break; } }
      if (slot < 0) return;
      p[slot].type    = PT_RAIN;
      p[slot].x       = (float)random(2, screenW - 2);
      p[slot].y       = 0.0f;
      p[slot].vx      = (float)(random(-20, 21)) / 20.0f;
      p[slot].vy      = (float)random(15, 35) / 10.0f;
      p[slot].sz      = (uint8_t)random(1, 3);
      p[slot].maxLife = (uint8_t)random(30, 70);
      p[slot].life    = p[slot].maxLife;
      p[slot].color   = 0xF800;  // kırmızı yağmur
    }
  }

  // Level complete / Tetris! — merkez patlaması
  void spawnBurst(float cx, float cy, uint16_t col, int count = 12) {
    for (int n = 0; n < count; n++) {
      int slot = -1;
      for (int i = 0; i < MAX_PT; i++) { if (!p[i].life) { slot = i; break; } }
      if (slot < 0) return;
      float ang = (float)random(0, 360) * (3.14159f / 180.0f);
      float spd = (float)random(12, 30) / 10.0f;
      p[slot].type    = PT_BURST;
      p[slot].x       = cx;
      p[slot].y       = cy;
      p[slot].vx      = cosf(ang) * spd;
      p[slot].vy      = sinf(ang) * spd;
      p[slot].sz      = (uint8_t)random(1, 4);
      p[slot].maxLife = (uint8_t)random(20, 50);
      p[slot].life    = p[slot].maxLife;
      p[slot].color   = col;
    }
  }

  // Ambient yavaş akan partiküller
  void tickAmbient(uint16_t col, int screenW = 172, int areaTop = 0, int areaBot = 320) {
    // Her frame 1 yeni parçacık oluşturma şansı
    if (random(0, 3) == 0) {
      for (int i = 0; i < MAX_PT; i++) {
        if (p[i].life > 0) continue;
        p[i].type    = PT_AMBIENT;
        p[i].x       = (float)random(2, screenW - 2);
        p[i].y       = (float)areaBot;
        p[i].vx      = (float)(random(-8, 9)) / 10.0f;
        p[i].vy      = -(float)random(3, 10) / 10.0f;
        p[i].sz      = 1;
        p[i].maxLife = (uint8_t)random(40, 100);
        p[i].life    = p[i].maxLife;
        p[i].color   = col;
        break;
      }
    }
  }

  // Konfeti — level complete için gökkuşağı
  void spawnConfetti(int screenW = 172) {
    static const uint16_t COLS[] = {
      0xFFE0, 0xF800, 0x07E0, 0x001F, 0xF81F, 0x07FF, 0xFC00
    };
    for (int n = 0; n < 6; n++) {
      int slot = -1;
      for (int i = 0; i < MAX_PT; i++) { if (!p[i].life) { slot = i; break; } }
      if (slot < 0) return;
      p[slot].type    = PT_CONFETTI;
      p[slot].x       = (float)random(4, screenW - 4);
      p[slot].y       = 0.0f;
      p[slot].vx      = (float)(random(-15, 16)) / 10.0f;
      p[slot].vy      = (float)random(8, 22) / 10.0f;
      p[slot].sz      = (uint8_t)random(2, 4);
      p[slot].maxLife = (uint8_t)random(30, 60);
      p[slot].life    = p[slot].maxLife;
      p[slot].color   = COLS[random(0, 7)];
    }
  }

  // ==================== Update ====================
  void update(int screenW = 172, int screenH = 320) {
    for (int i = 0; i < MAX_PT; i++) {
      if (!p[i].life) continue;
      p[i].x   += p[i].vx;
      p[i].y   += p[i].vy;
      if (p[i].type == PT_SPARKLE || p[i].type == PT_DUST)
        p[i].vy += 0.08f;  // hafif yerçekimi
      p[i].life--;
      if (p[i].x < 0 || p[i].x > screenW || p[i].y < -10 || p[i].y > screenH + 10)
        p[i].life = 0;
    }
  }

  // ==================== Draw ====================
  void draw(TFT_eSprite& spr) {
    for (int i = 0; i < MAX_PT; i++) {
      if (!p[i].life) continue;
      float alpha = (float)p[i].life / (float)p[i].maxLife;
      uint16_t c  = fadeColor(p[i].color, alpha);
      int px = (int)p[i].x, py = (int)p[i].y;
      if (p[i].sz <= 1) spr.drawPixel(px, py, c);
      else              spr.fillCircle(px, py, (int)p[i].sz - 1, c);
    }
  }

  bool anyAlive() const {
    for (int i = 0; i < MAX_PT; i++) if (p[i].life) return true;
    return false;
  }
};
