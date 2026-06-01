// game_zelda.h — Advanced Zelda-style Open World Adventure game layout
#pragma once
#include <TFT_eSPI.h>
#include <vector>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include "game_zelda_sprites.h"

namespace Zelda {
using namespace std;

// ==================== Harita ve Ekran Sabitleri ====================
static const int TILE_SIZE  = 16;
static const int MAP_COLS   = 40;  // 640 piksel
static const int MAP_ROWS   = 20;  // 320 piksel
static const int VIEW_W     = 320;
static const int VIEW_H     = 156; // Kalan 14 piksel HUD için (320x170 toplam)

// Tile tipleri
#define T_GRASS 0
#define T_TREE  1
#define T_ROCK  2
#define T_WATER 3
#define T_CHEST 4
#define T_SAND  5

// Renkler
#define COLOR_GRASS     0x5E68
#define COLOR_TREE      0x1384
#define COLOR_ROCK      0x5AEB
#define COLOR_WATER     0x139F
#define COLOR_SAND      0xE64B
#define COLOR_HUD_BG    0x10A2
#define COLOR_HEART     0xF800
#define COLOR_COIN      0xFD60
#define COLOR_SHIELD    0x1C9F
#define COLOR_POTION    0xA3C0

// ==================== Yapılar ====================
struct ZPlayer {
  float x, y;
  float angle;          // Radyan cinsinden serbest yön açısı (0 - 2*PI)
  int health;           // Maks 3 kalp
  int rubies;
  bool isAttacking;
  int attackTimer;
  bool isHurt;
  int hurtTimer;
  bool shieldActive;    // Kalkan aktif mi (lft_btn)
  int dashTimer;        // Dash/Hızlanma aktiflik süresi (dwn_btn)
  int healPotions;      // İyileşme iksirleri (up_btn)
};

struct ZEnemy {
  float x, y;
  float angle;
  int health;
  bool active;
  int type;             // 0: Octorok (Rastgele devriye), 1: Gel (Oyuncu takipçisi)
  int moveTimer;
};

struct ZParticle {
  float x, y, vx, vy;
  uint8_t life;
  uint16_t col;
};

// ==================== Interrupt Sayaçları ====================
static volatile int zCounter = 0;
static volatile int zLastEncoded = 0;

static void IRAM_ATTR zReadEncoderISR() {
  int MSB = digitalRead(48); // CLK
  int LSB = digitalRead(47); // DT
  int encoded = (MSB << 1) | LSB;
  int sum = (zLastEncoded << 2) | encoded;
  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) zCounter++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) zCounter--;
  zLastEncoded = encoded;
}

// ==================== Global Değişkenler ====================
static ZPlayer player;
static vector<ZEnemy> enemies;
static ZParticle particles[24];
static int camX = 0, camY = 0;
static bool zBtnPressed_R = false;
static bool zBtnPressed_L = false;
static bool zBtnPressed_U = false;
static bool zBtnPressed_D = false;
static bool zBtnPressed_LF = false;
static bool zGameOver = false;
static bool zWin = false;

// Animasyon sayaçları
static uint8_t zWalkFrame    = 0;   // 0,1,2,3 -> yürüme kareleri
static uint8_t zAnimTimer    = 0;   // frame sayacı
static uint8_t zWaterFrame   = 0;   // su animasyonu
static uint8_t zWaterTimer   = 0;
static int8_t  zFaceDir      = 0;   // 0=aşağı 1=sağ 2=sol 3=yukarı

// Dünya haritası
static uint8_t worldMap[MAP_ROWS][MAP_COLS] = {
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,1,1,1,3,3,3,3,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,4,1},
  {1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,3,3,3,3,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,2,2,1,0,0,4,0,0,0,0,0,0,0,0,0,1,3,3,3,3,1,0,0,2,2,2,2,0,0,0,5,5,5,5,0,0,1},
  {1,0,0,2,2,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,3,3,1,1,0,0,2,2,2,2,0,0,0,5,5,5,5,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,5,5,5,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,1,0,0,0,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,0,0,1,1,1,1,1,1,1,1,2,2,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1},
  {1,3,3,1,0,0,1,3,3,3,3,3,3,1,0,0,1,3,3,3,3,3,3,3,3,3,1,0,0,1,3,3,3,3,3,3,3,3,3,1},
  {1,3,3,1,0,0,1,3,3,3,3,3,3,1,0,0,1,3,3,3,3,3,3,3,3,3,1,0,0,1,3,3,3,3,3,3,3,3,3,1},
  {1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1},
  {1,0,0,4,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,4,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1},
  {1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,0,0,1},
  {1,5,5,5,5,5,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,1,5,5,5,5,5,1,0,0,1},
  {1,5,5,5,5,5,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,1,5,5,5,5,5,1,0,0,1},
  {1,5,5,4,5,5,1,0,0,2,2,2,0,0,0,0,1,0,0,2,2,2,2,2,0,0,0,1,0,0,1,5,5,4,5,5,1,0,0,1},
  {1,5,5,5,5,5,1,0,0,2,2,2,0,0,0,0,1,0,0,2,2,2,2,2,0,0,0,0,0,0,1,5,5,5,5,5,1,0,0,1},
  {1,5,5,5,5,5,0,0,0,2,2,2,0,0,0,0,0,0,0,2,2,2,2,2,0,0,0,0,0,0,0,5,5,5,5,5,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

// ==================== Parçacık Sistemi ====================
static void zSpawnParticles(float x, float y, uint16_t col, int num = 5) {
  for (int k = 0; k < num; k++) {
    for (int i = 0; i < 24; i++) {
      if (particles[i].life > 0) continue;
      particles[i].x = x;
      particles[i].y = y;
      particles[i].vx = (float)(rand() % 200 - 100) / 100.0f * 1.6f;
      particles[i].vy = (float)(rand() % 200 - 100) / 100.0f * 1.6f - 0.4f;
      particles[i].life = rand() % 12 + 10;
      particles[i].col = col;
      break;
    }
  }
}

static void zUpdateParticles() {
  for (auto& p : particles) {
    if (p.life == 0) continue;
    p.x += p.vx;
    p.y += p.vy;
    p.vy += 0.04f; // Hafif yerçekimi
    p.life--;
  }
}

// ==================== Düşman Ekleme ====================
static void zAddEnemy(float x, float y, int type) {
  ZEnemy e = { x, y, 0.0f, 2, true, type, 0 };
  enemies.push_back(e);
}

// ==================== Reset ====================
static void zReset() {
  player.x = 32;
  player.y = 32;
  player.angle = 1.57f; // Aşağı doğru bakarak başlasın (PI/2)
  player.health = 3;
  player.rubies = 0;
  player.isAttacking = false;
  player.attackTimer = 0;
  player.isHurt = false;
  player.hurtTimer = 0;
  player.shieldActive = false;
  player.dashTimer = 0;
  player.healPotions = 2; // Başlangıçta 2 can potu

  enemies.clear();
  zAddEnemy(200, 50, 0);  // Octorok
  zAddEnemy(150, 160, 0); // Octorok
  zAddEnemy(450, 80, 1);  // Gel
  zAddEnemy(500, 200, 1); // Gel
  zAddEnemy(100, 240, 1); // Gel

  // Sandıkları sıfırla
  for(int r=0; r<MAP_ROWS; r++) {
    for(int c=0; c<MAP_COLS; c++) {
      if (worldMap[r][c] == 9) worldMap[r][c] = T_CHEST;
    }
  }

  memset(particles, 0, sizeof(particles));
  camX = 0; camY = 0;
  zGameOver = false;
  zWin = false;
  zCounter = 0;
}

// ==================== Çarpışma Tespiti (AABB) ====================
static bool zCheckCollision(float x, float y, float w, float h, int& hitTileX, int& hitTileY) {
  int startCol = (int)(x) / TILE_SIZE;
  int endCol   = (int)(x + w) / TILE_SIZE;
  int startRow = (int)(y) / TILE_SIZE;
  int endRow   = (int)(y + h) / TILE_SIZE;

  for (int r = startRow; r <= endRow; r++) {
    for (int c = startCol; c <= endCol; c++) {
      if (r < 0 || r >= MAP_ROWS || c < 0 || c >= MAP_COLS) return true;
      int t = worldMap[r][c];
      if (t == T_TREE || t == T_ROCK || t == T_WATER || t == T_CHEST) {
        hitTileX = c;
        hitTileY = r;
        return true;
      }
    }
  }
  return false;
}

// ==================== Görsel Çizimler (Sprite Tabanlı) ====================
static void drawTile(int tileX, int tileY, int type) {
  int x = tileX * TILE_SIZE - camX;
  int y = tileY * TILE_SIZE - camY;

  if (x + TILE_SIZE < 0 || x >= VIEW_W || y + TILE_SIZE < 0 || y >= VIEW_H) return;

  switch (type) {
    case T_GRASS: {

      bool alt = (tileX + tileY) % 4 == 0;
      if (alt)
        zFillTile(x, y, tile_grass2);  // Bush/ağaçlık varyant (üst üste çim)
      else
        zFillTile(x, y, tile_grass);
      break;
    }
    case T_TREE:
      zFillTile(x, y, tile_tree_tl);
      break;
    case T_ROCK:
      zFillTile(x, y, tile_rock);
      break;
    case T_WATER: {
      const uint16_t* wf = (zWaterFrame == 0) ? tile_water_a :
                           (zWaterFrame == 1) ? tile_water_b : tile_water_c;
      zFillTile(x, y, wf);
      break;
    }
    case T_CHEST:
      zFillTile(x, y, tile_grass);
      zDrawTile(x, y, spr_chest);
      break;
    case 9: // Açık sandık
      zFillTile(x, y, tile_grass);
      zDrawTile(x, y, spr_chest_open);
      break;
    case T_SAND:
      zFillTile(x, y, tile_dirt);  // Patika/toprak tile
      break;
    default:
      img.fillRect(x, y, 16, 16, COLOR_GRASS);
      break;
  }
}

static void drawPlayer() {
  int px = (int)player.x - camX - 3;  // Sprite 16px wide, merkezi tut
  int py = (int)player.y - camY - 16; // Sprite 32px tall, ayak altta

  if (player.isHurt && (millis() / 80) % 2 == 0) return;


  if (player.shieldActive) {
    img.drawCircle(px + 8, py + 20, 13, COLOR_SHIELD);
    img.drawCircle(px + 8, py + 20, 12, 0x039F);
  }

  const uint16_t* sprite = char_down_1;
  bool flipH = false;

  if (player.isAttacking) {
    if      (zFaceDir == 0) sprite = char_sword_down;
    else if (zFaceDir == 1) sprite = char_sword_side;
    else if (zFaceDir == 2) { sprite = char_sword_side; flipH = true; }
    else                     sprite = char_sword_up;
  } else {
    const uint16_t* frames[4][4] = {
      {char_down_1,  char_down_2,  char_down_3,  char_down_2},   // aşağı
      {char_side_1,  char_side_2,  char_side_3,  char_side_2},   // sağ
      {char_side_1,  char_side_2,  char_side_3,  char_side_2},   // sol (flip)
      {char_up_1,    char_up_2,    char_up_3,    char_up_2},     // yukarı
    };
    sprite = frames[zFaceDir][zWalkFrame];
    if (zFaceDir == 2) flipH = true;
  }

  zDrawChar(px, py, sprite, flipH);

}

static void drawEnemies() {
  for (const auto& e : enemies) {
    if (!e.active) continue;
    int ex = (int)e.x - camX;
    int ey = (int)e.y - camY;

    if (ex + 16 < 0 || ex >= VIEW_W || ey + 16 < 0 || ey >= VIEW_H) continue;

    if (e.type == 0) {
      img.fillEllipse(ex + 7, ey + 13, 6, 2, 0x18A3);
      img.fillCircle(ex + 7, ey + 7, 6, 0xC000);
      img.fillCircle(ex + 7, ey + 6, 5, 0xE800);
      img.fillRect(ex + 4, ey + 5, 2, 2, TFT_WHITE);
      img.fillRect(ex + 9, ey + 5, 2, 2, TFT_WHITE);
      img.drawPixel(ex + 4, ey + 5, 0x0000);
      img.drawPixel(ex + 9, ey + 5, 0x0000);
      img.drawFastVLine(ex + 3, ey + 11, 3, 0xC000);
      img.drawFastVLine(ex + 7, ey + 12, 3, 0xC000);
      img.drawFastVLine(ex + 11, ey + 11, 3, 0xC000);
      img.fillRect(ex + 1, ey - 4, 14, 3, 0x4208);
      img.fillRect(ex + 1, ey - 4, (e.health * 7), 3, 0xF800);
    } else {
      img.fillEllipse(ex + 7, ey + 13, 5, 2, 0x18A3);
      int bounce = (millis() / 200) % 2;
      img.fillRoundRect(ex + 2, ey + 3 + bounce, 10, 9 - bounce, 4, 0x03DF);
      img.fillRoundRect(ex + 3, ey + 4 + bounce, 8,  7 - bounce, 3, 0x1BFF);
      img.fillRect(ex + 3, ey + 5, 2, 2, TFT_WHITE);
      img.fillRect(ex + 8, ey + 5, 2, 2, TFT_WHITE);
      img.drawPixel(ex + 4, ey + 5, 0x0000);
      img.drawPixel(ex + 9, ey + 5, 0x0000);
      img.fillRect(ex + 1, ey - 4, 14, 3, 0x4208);
      img.fillRect(ex + 1, ey - 4, (e.health * 7), 3, 0x03DF);
    }
  }
}

static void drawHUD() {
  img.fillRect(0, 154, 320, 16, 0x0841);
  img.drawFastHLine(0, 154, 320, 0x39E7);

  int hx = 4;
  for (int i = 0; i < 3; i++) {
    const uint16_t* hspr = (i < player.health)    ? spr_heart_full  :
                           (i == player.health && player.health > 0 && player.health < 3) ? spr_heart_half :
                           spr_heart_empty;
    for (int y = 0; y < 11; y++) {
      for (int x = 0; x < 11; x++) {
        int sx = x * 16 / 11;
        int sy = y * 16 / 11;
        uint16_t c = pgm_read_word(&hspr[sy * 16 + sx]);
        if (c != ZELDA_TRANSPARENT)
          img.drawPixel(hx + x, 156 + y, c);
      }
    }
    hx += 13;
  }

  img.fillCircle(48, 161, 4, COLOR_COIN);
  img.fillCircle(47, 160, 2, 0xFFE0);
  img.setTextFont(1); img.setTextSize(1);
  img.setTextColor(COLOR_COIN);
  img.setCursor(55, 158);
  img.print(player.rubies);

  img.fillRoundRect(86, 157, 7, 9, 2, COLOR_POTION);
  img.fillRect(87, 155, 5, 3, 0xC618);
  img.setTextColor(0xFFFF);
  img.setCursor(96, 158);
  img.print(player.healPotions);

  img.setTextColor(0x8410);
  img.setCursor(112, 158);
  img.print("DSH:");
  // Dolum çubuğu (30 frame max)
  int dashPct = player.dashTimer > 0 ? (30 - player.dashTimer) * 20 / 30 : 20;
  img.fillRect(136, 158, 20, 6, 0x2945);
  img.fillRect(136, 158, dashPct, 6, player.dashTimer > 0 ? 0xFFE0 : 0x07E0);
  img.drawRect(136, 158, 20, 6, 0x8410);

  if (player.shieldActive) {
    img.fillRoundRect(162, 156, 28, 10, 2, 0x001F);
    img.setTextColor(TFT_WHITE);
    img.setCursor(165, 158);
    img.print("SHIELD");
  }

  int mx = 276, my = 155;
  img.fillRect(mx, my, 40, 14, 0x0841);
  img.drawRect(mx-1, my-1, 42, 16, 0x39E7);
  for (int r = 0; r < MAP_ROWS; r++) {
    int py2 = my + r * 14 / MAP_ROWS;
    for (int c = 0; c < MAP_COLS; c++) {
      int px2 = mx + c;
      if (px2 >= mx + 40) break;
      uint8_t t = worldMap[r][c];
      uint16_t mc = (t == T_WATER) ? 0x035F :
                    (t == T_TREE)  ? 0x0300 :
                    (t == T_ROCK)  ? 0x7BEF :
                    (t == T_CHEST) ? 0xFD60 :
                    (t == 9)       ? 0x8400 :
                    (t == T_SAND)  ? 0xC5A0 :
                                     0x2D45;  // cimen
      img.drawPixel(px2, py2, mc);
    }
  }
  int pmx = mx + (int)(player.x / TILE_SIZE);
  int pmy = my + (int)(player.y / TILE_SIZE) * 14 / MAP_ROWS;
  if (pmx >= mx && pmx < mx+40) img.fillRect(pmx-1, pmy, 3, 2, TFT_WHITE);
}

// ==================== Düşman Yapay Zekası (Pürüzsüz Açı Takibi) ====================
static void updateEnemies() {
  int hitX, hitY;
  for (auto& e : enemies) {
    if (!e.active) continue;

    e.moveTimer++;
    float dx = 0, dy = 0;
    float spd = (e.type == 0) ? 0.35f : 0.65f;

    if (e.type == 0) {
      if (e.moveTimer > 80) {
        e.angle = (float)(rand() % 360) * M_PI / 180.0f;
        e.moveTimer = 0;
      }
      dx = cosf(e.angle) * spd;
      dy = sinf(e.angle) * spd;
    } else {
      float diffX = player.x - e.x;
      float diffY = player.y - e.y;
      float dist = sqrtf(diffX*diffX + diffY*diffY);
      
      if (dist < 130 && dist > 8) {
        e.angle = atan2f(diffY, diffX);
        dx = cosf(e.angle) * spd;
        dy = sinf(e.angle) * spd;
      } else {
        dx = 0; dy = 0;
      }
    }


    if (!zCheckCollision(e.x + dx, e.y + dy, 11, 11, hitX, hitY)) {
      e.x += dx;
      e.y += dy;
    } else {
      if (e.type == 0) {
        e.angle = (float)(rand() % 360) * M_PI / 180.0f;
      }
    }

    // Oyuncu ile temas
    float distX = player.x - e.x;
    float distY = player.y - e.y;
    if (abs(distX) < 9 && abs(distY) < 9 && !player.isHurt) {
      if (player.shieldActive) {
        if (distX > 0) e.x -= 15; else e.x += 15;
        if (distY > 0) e.y -= 15; else e.y += 15;
        zSpawnParticles(e.x + 6, e.y + 6, COLOR_SHIELD, 3);
      } else {
        player.health--;
        player.isHurt = true;
        player.hurtTimer = 40;
        if (distX > 0) player.x += 14; else player.x -= 14;
        if (distY > 0) player.y += 14; else player.y -= 14;
        zSpawnParticles(player.x + 5, player.y + 5, COLOR_HEART, 4);

        if (player.health <= 0) zGameOver = true;
      }
    }
  }
}

// ==================== Oyuncu Saldırı Güncelleme ====================
static void updateAttack() {
  if (!player.isAttacking) return;

  player.attackTimer--;
  if (player.attackTimer <= 0) {
    player.isAttacking = false;
    return;
  }

  float sx = player.x + 6 + cosf(player.angle) * 12;
  float sy = player.y + 6 + sinf(player.angle) * 12;

  for (auto& e : enemies) {
    if (!e.active) continue;
    float dx = e.x + 6 - sx;
    float dy = e.y + 6 - sy;
    float dist = sqrtf(dx*dx + dy*dy);
    if (dist < 14) {
      e.health--;
      e.x += cosf(player.angle) * 15;
      e.y += sinf(player.angle) * 15;
      zSpawnParticles(e.x + 6, e.y + 6, TFT_WHITE, 5);

      if (e.health <= 0) {
        e.active = false;
        player.rubies += 5;
      }
    }
  }
}

// ==================== Animasyon Güncelleme ====================
static void updateAnimations(bool isMoving) {
  zWaterTimer++;
  if (zWaterTimer >= 20) { zWaterTimer = 0; zWaterFrame = (zWaterFrame + 1) % 3; }

  if (isMoving) {
    zAnimTimer++;
    if (zAnimTimer >= 8) { zAnimTimer = 0; zWalkFrame = (zWalkFrame + 1) % 4; }
  } else {
    zWalkFrame = 0;
    zAnimTimer = 0;
  }

  float a = player.angle;
  if      (a > 0.785f && a <= 2.356f)  zFaceDir = 0;  // Aşağı (PI/4 .. 3PI/4)
  else if (a > 2.356f && a <= 3.927f)  zFaceDir = 2;  // Sol (3PI/4 .. 5PI/4)
  else if (a > 3.927f && a <= 5.497f)  zFaceDir = 3;  // Yukarı (5PI/4 .. 7PI/4)
  else                                  zFaceDir = 1;  // Sağ (7PI/4 .. PI/4)
}

// ==================== Oyuncu Hareketi ve Açı Güncelleme ====================
static void updatePlayer() {
  if (player.isHurt) {
    player.hurtTimer--;
    if (player.hurtTimer <= 0) player.isHurt = false;
  }

  if (player.dashTimer > 0) player.dashTimer--;

  static int zPrevCounter = 0;
  int currentCounter = zCounter;
  int delta = currentCounter - zPrevCounter;
  if (delta != 0) {
    player.angle += delta * 0.18f;
    // Açı sınırla
    if (player.angle < 0) player.angle += 2.0f * M_PI;
    if (player.angle > 2.0f * M_PI) player.angle -= 2.0f * M_PI;
    zPrevCounter = currentCounter;
  }

  if (digitalRead(2) == LOW && !player.isAttacking && !zBtnPressed_R) {
    player.isAttacking = true;
    player.attackTimer = 10;
    zBtnPressed_R = true;
  }
  if (digitalRead(2) == HIGH) zBtnPressed_R = false;

  if (digitalRead(39) == LOW && !zBtnPressed_L) {
    zBtnPressed_L = true;
    float checkX = player.x + 6 + cosf(player.angle) * 12;
    float checkY = player.y + 6 + sinf(player.angle) * 12;
    int tc = (int)checkX / TILE_SIZE;
    int tr = (int)checkY / TILE_SIZE;
    if (tr >= 0 && tr < MAP_ROWS && tc >= 0 && tc < MAP_COLS) {
      if (worldMap[tr][tc] == T_CHEST) {
        worldMap[tr][tc] = 9; // Sandık açıldı
        player.rubies += 20;
        zSpawnParticles(tc * TILE_SIZE + 8, tr * TILE_SIZE + 8, COLOR_COIN, 8);
        bool anyChest = false;
        for(int y=0; y<MAP_ROWS; y++) {
          for(int x=0; x<MAP_COLS; x++) {
            if(worldMap[y][x] == T_CHEST) anyChest = true;
          }
        }
        if(!anyChest) zWin = true;
      }
    }
  }
  if (digitalRead(39) == HIGH) zBtnPressed_L = false;

  if (digitalRead(4) == LOW && !zBtnPressed_U) {
    zBtnPressed_U = true;
    if (player.healPotions > 0 && player.health < 3) {
      player.health = min(3, player.health + 1);
      player.healPotions--;
      zSpawnParticles(player.x + 6, player.y + 6, COLOR_POTION, 6);
    }
  }
  if (digitalRead(4) == HIGH) zBtnPressed_U = false;

  if (digitalRead(3) == LOW && !zBtnPressed_D && player.dashTimer == 0) {
    zBtnPressed_D = true;
    player.dashTimer = 30; // 30 frame hızlı hareket
    zSpawnParticles(player.x + 6, player.y + 6, TFT_YELLOW, 5);
  }
  if (digitalRead(3) == HIGH) zBtnPressed_D = false;

  player.shieldActive = (digitalRead(5) == LOW);

  float dx = 0, dy = 0;
  bool isMoving = false;
  if (digitalRead(1) == LOW && !player.isAttacking) {
    float spd = player.dashTimer > 0 ? 2.6f : 1.2f;
    if (player.shieldActive) spd *= 0.5f;
    int curTileX = (int)(player.x + 5) / TILE_SIZE;
    int curTileY = (int)(player.y + 5) / TILE_SIZE;
    if (curTileX >= 0 && curTileX < MAP_COLS && curTileY >= 0 && curTileY < MAP_ROWS)
      if (worldMap[curTileY][curTileX] == T_SAND) spd *= 0.6f;
    dx = cosf(player.angle) * spd;
    dy = sinf(player.angle) * spd;
    isMoving = true;
  }

  updateAnimations(isMoving);

  int hitX, hitY;
  if (dx != 0 || dy != 0) {
    if (!zCheckCollision(player.x + dx, player.y + dy, 10, 10, hitX, hitY)) {
      player.x += dx;
      player.y += dy;
    } else {
      if (!zCheckCollision(player.x + dx, player.y, 10, 10, hitX, hitY)) {
        player.x += dx;
      } else if (!zCheckCollision(player.x, player.y + dy, 10, 10, hitX, hitY)) {
        player.y += dy;
      }
    }
  }

  if (player.x < TILE_SIZE) player.x = TILE_SIZE;
  if (player.x > (MAP_COLS - 2) * TILE_SIZE) player.x = (MAP_COLS - 2) * TILE_SIZE;
  if (player.y < TILE_SIZE) player.y = TILE_SIZE;
  if (player.y > (MAP_ROWS - 2) * TILE_SIZE) player.y = (MAP_ROWS - 2) * TILE_SIZE;

  camX = (int)player.x - VIEW_W / 2;
  camY = (int)player.y - VIEW_H / 2;

  if (camX < 0) camX = 0;
  if (camX > (MAP_COLS * TILE_SIZE) - VIEW_W) camX = (MAP_COLS * TILE_SIZE) - VIEW_W;
  if (camY < 0) camY = 0;
  if (camY > (MAP_ROWS * TILE_SIZE) - VIEW_H) camY = (MAP_ROWS * TILE_SIZE) - VIEW_H;
}

// ==================== Render Ana Döngü ====================
static void zRender() {
  img.fillSprite(COLOR_GRASS);

  int startCol = camX / TILE_SIZE;
  int endCol   = (camX + VIEW_W) / TILE_SIZE + 1;
  int startRow = camY / TILE_SIZE;
  int endRow   = (camY + VIEW_H) / TILE_SIZE + 1;

  for (int r = startRow; r < endRow; r++) {
    for (int c = startCol; c < endCol; c++) {
      if (r >= 0 && r < MAP_ROWS && c >= 0 && c < MAP_COLS) {
        drawTile(c, r, worldMap[r][c]);
      }
    }
  }

  // Parçacıklar
  for (const auto& p : particles) {
    if (p.life == 0) continue;
    int px = (int)p.x - camX;
    int py = (int)p.y - camY;
    if (px >= 0 && px < VIEW_W && py >= 0 && py < VIEW_H) {
      img.drawPixel(px, py, p.col);
    }
  }

  drawEnemies();
  drawPlayer();
  drawHUD();

  if (zGameOver) {
    img.fillRect(40, 45, 240, 60, 0x6000);
    img.drawRect(40, 45, 240, 60, TFT_RED);
    img.setTextColor(TFT_RED); img.setTextFont(4); img.setTextDatum(MC_DATUM);
    img.drawString("GAME OVER", 160, 65);
    img.setTextColor(TFT_WHITE); img.setTextFont(1);
    img.drawString("R_SHOULDER to Restart", 160, 90);
  } else if (zWin) {
    img.fillRect(40, 45, 240, 60, 0x0320);
    img.drawRect(40, 45, 240, 60, TFT_GREEN);
    img.setTextColor(TFT_GREEN); img.setTextFont(4); img.setTextDatum(MC_DATUM);
    img.drawString("QUEST CLEAR!", 160, 65);
    img.setTextColor(TFT_WHITE); img.setTextFont(1);
    img.drawString("Opened all chests! R_SHOULDER to play again", 160, 90);
  }

  img.pushSprite(0, 0);
}

// ==================== GameHub Arayüzü ====================
void setup() {
  tft.init();
  tft.setRotation(1); // LANDSCAPE
  tft.fillScreen(TFT_BLACK);
  img.deleteSprite();
  img.createSprite(320, 170);
  img.setTextDatum(0);

  // ENCODER INTERRUPT AYARLARI (tıpkı convex_collision'daki gibi)
  pinMode(48, INPUT_PULLUP);
  pinMode(47, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(48), zReadEncoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(47), zReadEncoderISR, CHANGE);

  zReset();
}

void update() {
  if (zGameOver || zWin) {
    if (digitalRead(2) == LOW) { // R_SHOULDER to restart
      delay(200);
      zReset();
    }
    zRender();
    return;
  }

  updatePlayer();
  updateAttack();
  updateEnemies();
  zUpdateParticles();
  zRender();
}

} // namespace Zelda
