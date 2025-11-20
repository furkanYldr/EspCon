#include "HWCDC.h"
#include "pinball.h"

#include "esp32-hal-gpio.h"
#include <cmath>
#include <vector>
using namespace std;

// ==================== Donanım Ayarları ====================
#define LEFT_FLIPPER_BUTTON   6    // Sol flipper butonu (örnek)
#define RIGHT_FLIPPER_BUTTON  7  // Sağ flipper butonu

// Global ekran ve sprite
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite img = TFT_eSprite(&tft);

// ==================== Veri Yapıları ====================

struct Ball {
  float x, y;
  float vx, vy;
  float r;
};

struct Flipper {
  float pivotX, pivotY;  // menteşe noktası
  float length;          // kanat uzunluğu
  float angle;           // rad cinsinden açı
  float minAngle;        // aşağıdaki (dinlenme) açı
  float maxAngle;        // yukarıdaki maksimum açı
  float speed;           // her frame döneceği miktar
  bool active;           // buton basılı mı
};

static Ball ball;
static Flipper leftFlipper;
static Flipper rightFlipper;

static int screenW = 0;
static int screenH = 0;

// Oyun alanı sınırları
static int playLeft   = 5;
static int playRight  = 165;
static int playTop    = 5;
static int playBottom = 315;

// Flipper arası alt açıklık (drain) için sınırlar
static float drainLeftX  = 0.0f; // sol pivot x
static float drainRightX = 0.0f; // sağ pivot x

// Fizik sabitleri
const float FRICTION   = 0.985f;   // sürtünme
const float BOUNCE     = -0.98f;   // duvar sekmesi
const float GRAVITY_Y  = 0.08f;    // eğimli masa graviti (aşağı ivme)

// FPS kontrolü
static unsigned long lastUpdateMs = 0;
const unsigned long FRAME_TIME_MS = 16; // ~60 FPS

// ==================== Yardımcı Fonksiyonlar ====================

static float deg(float d) {
  return d * (M_PI / 180.0f);
}

// Her seferinde topu yukarıdan rastgele doğur
static void resetBallFromTop() {
  ball.r  = 4;

  // play alanı içinde rastgele bir x
  int minX = playLeft  + ball.r + 2;
  int maxX = playRight - ball.r - 2;
  if (minX >= maxX) { // güvenlik
    minX = playLeft + 10;
    maxX = playRight - 10;
  }

  ball.x  = random(minX, maxX);
  ball.y  = playTop + ball.r + 2;

  // Hafif random yana hız + aşağı hız
  float randSide = (random(-100, 101)) / 100.0f; // -1.0 .. 1.0
  ball.vx = randSide * 1.5f;                     // ±1.5 px/frame
  ball.vy = 1.5f;                                // aşağı doğru başlangıç hızı
}

static void setupFlippers() {
  float w = (playRight - playLeft);
  float L = w * 0.25f;
  float y = playBottom - 25;

  // Sol flipper
  leftFlipper.pivotX = (playLeft + playRight) / 2 - L * 1.1f;
  leftFlipper.pivotY = y - 20;
  leftFlipper.length = L;
  // 0 rad: sağa, pozitif açı: aşağı sağ, negatif açı: yukarı sağ
  leftFlipper.minAngle = deg(20);    // aşağı (dinlenme)
  leftFlipper.maxAngle = deg(-30);   // yukarı (vururken)
  leftFlipper.angle    = leftFlipper.minAngle;
  leftFlipper.speed    = deg(8);
  leftFlipper.active   = false;

  // Sağ flipper
  rightFlipper.pivotX = (playLeft + playRight) / 2 + L * 1.1f;
  rightFlipper.pivotY = y - 20;
  rightFlipper.length = L;
  // Sağ flipper sola bakacak şekilde
  rightFlipper.minAngle = deg(160);  // aşağı
  rightFlipper.maxAngle = deg(210);  // yukarı
  rightFlipper.angle    = rightFlipper.minAngle;
  rightFlipper.speed    = deg(8);
  rightFlipper.active   = false;

  // Drain açıklığı pivotlar arasında
  drainLeftX  = leftFlipper.pivotX;
  drainRightX = rightFlipper.pivotX;
}

static void drawPlayfield() {
  img.fillSprite(0x026A);

  // Yan ve üst duvarlar
  img.drawLine(playLeft,  playTop,    playLeft,  playBottom, TFT_WHITE);
  img.drawLine(playRight, playTop,    playRight, playBottom, TFT_WHITE);
  img.drawLine(playLeft,  playTop,    playRight, playTop,    TFT_WHITE);

  // Alt duvar: sadece pivotların dışında
  img.drawLine(playLeft,             playBottom,
               (int)drainLeftX,      playBottom, TFT_WHITE);
  img.drawLine((int)drainRightX,     playBottom,
               playRight,            playBottom, TFT_WHITE);

  // Pivot yanlarındaki kısa dikey postlar
  int postHeight = 15;
  img.drawLine((int)drainLeftX,  playBottom,
               (int)drainLeftX,  playBottom - postHeight, TFT_WHITE);
  img.drawLine((int)drainRightX, playBottom,
               (int)drainRightX, playBottom - postHeight, TFT_WHITE);

  // Bumperlar
  int cx1 = playLeft   + (playRight - playLeft) / 3;
  int cx2 = playLeft   + 2 * (playRight - playLeft) / 3;
  int cy1 = playTop    + 40;
  int cy2 = playTop    + 80;
  int cy3 = playTop    + 120;

  img.fillCircle(cx1, cy1, 6, TFT_BLUE);
  img.fillCircle(cx2, cy1, 6, TFT_BLUE);
  img.fillCircle((playLeft + playRight) / 2, cy2, 7, TFT_RED);
  img.fillCircle(cx1, cy3, 5, TFT_GREEN);
  img.fillCircle(cx2, cy3, 5, TFT_GREEN);

  // Başlık
  img.setTextColor(TFT_WHITE, 0x026A);
  img.setTextSize(1);
  img.setCursor(10, 2);
  img.print("PINBALL");
}

// Flipper ucu
static void flipperEndpoint(const Flipper &f, float &x1, float &y1) {
  x1 = f.pivotX + cosf(f.angle) * f.length;
  y1 = f.pivotY + sinf(f.angle) * f.length;
}

// Flipper çizimi
static void drawFlipper(const Flipper &f, uint16_t color) {
  float x1, y1;
  flipperEndpoint(f, x1, y1);

  float dx = x1 - f.pivotX;
  float dy = y1 - f.pivotY;
  float len = sqrtf(dx * dx + dy * dy);
  if (len <= 0.0001f) return;

  dx /= len;
  dy /= len;

  float thickness = 5.0f;
  float nx = -dy;
  float ny = dx;

  float px0 = f.pivotX + nx * (thickness / 2);
  float py0 = f.pivotY + ny * (thickness / 2);
  float px1 = f.pivotX - nx * (thickness / 2);
  float py1 = f.pivotY - ny * (thickness / 2);

  float ex0 = x1 + nx * (thickness / 2);
  float ey0 = y1 + ny * (thickness / 2);
  float ex1 = x1 - nx * (thickness / 2);
  float ey1 = y1 - ny * (thickness / 2);

  img.fillTriangle((int)px0, (int)py0, (int)px1, (int)py1, (int)ex0, (int)ey0, color);
  img.fillTriangle((int)px1, (int)py1, (int)ex0, (int)ey0, (int)ex1, (int)ey1, color);

  img.drawLine((int)f.pivotX, (int)f.pivotY, (int)x1, (int)y1, TFT_BLACK);
}

static void drawBall() {
  img.fillCircle((int)ball.x, (int)ball.y, (int)ball.r, TFT_WHITE);
}

// Bumper çarpması
static bool hitCircle(int bx, int by, int cx, int cy, int r) {
  int dx = bx - cx;
  int dy = by - cy;
  return dx * dx + dy * dy <= r * r;
}

// Noktadan doğru parçasına uzaklık
static float distancePointToSegment(
  float px, float py,
  float x0, float y0,
  float x1, float y1,
  float &closestX, float &closestY
) {
  float dx = x1 - x0;
  float dy = y1 - y0;
  float len2 = dx * dx + dy * dy;

  if (len2 <= 0.00001f) {
    closestX = x0;
    closestY = y0;
    float ddx = px - x0;
    float ddy = py - y0;
    return sqrtf(ddx * ddx + ddy * ddy);
  }

  float t = ((px - x0) * dx + (py - y0) * dy) / len2;
  if (t < 0.0f) t = 0.0f;
  if (t > 1.0f) t = 1.0f;

  closestX = x0 + t * dx;
  closestY = y0 + t * dy;

  float ddx = px - closestX;
  float ddy = py - closestY;
  return sqrtf(ddx * ddx + ddy * ddy);
}

// Flipper ile çarpışma
static void handleFlipperCollision(const Flipper &f) {
  float fx1, fy1;
  flipperEndpoint(f, fx1, fy1);

  float closestX, closestY;
  float dist = distancePointToSegment(
    ball.x, ball.y,
    f.pivotX, f.pivotY,
    fx1, fy1,
    closestX, closestY
  );

  if (dist <= ball.r + 1.0f) {
    float nx = ball.x - closestX;
    float ny = ball.y - closestY;
    float len = sqrtf(nx * nx + ny * ny);
    if (len < 0.0001f) {
      float dx = fx1 - f.pivotX;
      float dy = fy1 - f.pivotY;
      len = sqrtf(dx * dx + dy * dy);
      if (len < 0.0001f) return;
      nx = -dy / len;
      ny = dx / len;
    } else {
      nx /= len;
      ny /= len;
    }

    float overlap = (ball.r + 1.0f) - dist;
    ball.x += nx * overlap;
    ball.y += ny * overlap;

    float dot = ball.vx * nx + ball.vy * ny;
    if (dot > 0) {
      ball.vx = ball.vx - 2.0f * dot * nx;
      ball.vy = ball.vy - 2.0f * dot * ny;
    }

    if (f.active) {
      ball.vx += nx * 1.5f;
      ball.vy += ny * 1.5f;
    }
  }
}

// ==================== Input & Fizik ====================

static void handleInput() {
  bool leftPressed  = (digitalRead(LEFT_FLIPPER_BUTTON)  == LOW);
  bool rightPressed = (digitalRead(RIGHT_FLIPPER_BUTTON) == LOW);

  // Flipperlar butona göre her zaman aktif
  leftFlipper.active  = leftPressed;
  rightFlipper.active = rightPressed;

  // SOL FLIPPER: minAngle = 20°, maxAngle = -30°
  if (leftFlipper.active) {
    leftFlipper.angle -= leftFlipper.speed;
    if (leftFlipper.angle < leftFlipper.maxAngle) {
      leftFlipper.angle = leftFlipper.maxAngle;
    }
  } else {
    leftFlipper.angle += leftFlipper.speed;
    if (leftFlipper.angle > leftFlipper.minAngle) {
      leftFlipper.angle = leftFlipper.minAngle;
    }
  }

  // SAĞ FLIPPER: minAngle = 160°, maxAngle = 210°
  if (rightFlipper.active) {
    rightFlipper.angle += rightFlipper.speed;
    if (rightFlipper.angle > rightFlipper.maxAngle) {
      rightFlipper.angle = rightFlipper.maxAngle;
    }
  } else {
    rightFlipper.angle -= rightFlipper.speed;
    if (rightFlipper.angle < rightFlipper.minAngle) {
      rightFlipper.angle = rightFlipper.minAngle;
    }
  }
}

static void updatePhysics() {
  // Eğimli masa graviti
  ball.vy += GRAVITY_Y;

  // Hareket
  ball.x += ball.vx;
  ball.y += ball.vy;

  // Sürtünme
  ball.vx *= FRICTION;
  ball.vy *= FRICTION;

  // Yan ve üst duvarlar
  if (ball.x - ball.r < playLeft + 1) {
    ball.x = playLeft + 1 + ball.r;
    ball.vx *= BOUNCE;
  }
  if (ball.x + ball.r > playRight - 1) {
    ball.x = playRight - 1 - ball.r;
    ball.vx *= BOUNCE;
  }
  if (ball.y - ball.r < playTop + 1) {
    ball.y = playTop + 1 + ball.r;
    ball.vy *= BOUNCE;
  }

  // Alt tarafta: sadece pivotların DIŞINDA duvar var
  if (ball.y + ball.r > playBottom - 1) {
    if (ball.x < drainLeftX || ball.x > drainRightX) {
      // Yan duvarlara çarpma
      ball.y = playBottom - 1 - ball.r;
      ball.vy *= BOUNCE;
    } else {
      // Flipperların arasından aşağı kaçtı -> yeni top yukarıdan
      resetBallFromTop();
      return;
    }
  }

  // Bumper çarpışmaları
  int bx = (int)ball.x;
  int by = (int)ball.y;

  int cx1 = playLeft   + (playRight - playLeft) / 3;
  int cx2 = playLeft   + 2 * (playRight - playLeft) / 3;
  int cy1 = playTop    + 40;
  int cy2 = playTop    + 80;
  int cy3 = playTop    + 120;

  bool bumperHit = false;

  if (hitCircle(bx, by, cx1, cy1, 10)) bumperHit = true;
  if (hitCircle(bx, by, cx2, cy1, 10)) bumperHit = true;
  if (hitCircle(bx, by, (playLeft + playRight) / 2, cy2, 12)) bumperHit = true;
  if (hitCircle(bx, by, cx1, cy3, 9)) bumperHit = true;
  if (hitCircle(bx, by, cx2, cy3, 9)) bumperHit = true;

  if (bumperHit) {
    ball.vy = -ball.vy * 1.1f;
    ball.vx = -ball.vx * 1.1f;
  }

  // Flipper çarpışmaları
  handleFlipperCollision(leftFlipper);
  handleFlipperCollision(rightFlipper);
}

// ==================== Dışarı Açık Fonksiyonlar ====================

void pinballSetup() {
  tft.init();
  tft.setRotation(4);
  img.setColorDepth(16);
  img.createSprite(170, 320);

  screenW = img.width();
  screenH = img.height();

  playLeft   = 5;
  playRight  = screenW - 5;
  playTop    = 5;
  playBottom = screenH - 5;

  pinMode(LEFT_FLIPPER_BUTTON, INPUT_PULLUP);
  pinMode(RIGHT_FLIPPER_BUTTON, INPUT_PULLUP);

  setupFlippers();
  resetBallFromTop();   // oyuna yukarıdan random top ile başla

  lastUpdateMs = millis();
}

void pinballUpdate() {
  unsigned long now = millis();
  if (now - lastUpdateMs < FRAME_TIME_MS) {
    return;
  }
  lastUpdateMs = now;

  drawPlayfield();
  handleInput();
  updatePhysics();

  drawFlipper(leftFlipper,  TFT_ORANGE);
  drawFlipper(rightFlipper, TFT_ORANGE);
  drawBall();

  img.pushSprite(0, 0);
}
