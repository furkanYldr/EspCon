// game_flappy.h — GameHub Flappy Bird wrapper
// namespace Flappy — tft/img global scope'tan otomatik bulunur
#pragma once
#include <TFT_eSPI.h>
#include <cstdlib>

namespace Flappy {

// ==================== Sprite Verileri ====================
static const int ROWS = 7;
static const int COLS = 15;

static int flapDown[7][15] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 4, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 4, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0 },
  { 0, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 2, 0, 0 },
  { 0, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 2, 2, 0 },
  { 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 2, 2, 2, 0 },
  { 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
static int flapUp[7][15] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0 },
  { 3, 3, 3, 3, 0, 0, 0, 0, 0, 1, 4, 0, 0, 0, 0 },
  { 0, 3, 3, 3, 3, 3, 0, 0, 0, 1, 4, 0, 0, 0, 0 },
  { 0, 0, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 2, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

// ==================== Oyun Değişkenleri ====================
static int   lastButtonState = HIGH;
static int   buttonState     = HIGH;
static int   anim            = 0;
static unsigned long previousMillis = 0;
static const unsigned long interval = 50;
static float velocity        = 0;
static float collumnSpeed    = 2;
static float fx              = 100;
static float fz              = 195;
static float fy              = 100;
static int   ground          = 200;
static int   gapPos1         = 70;
static int   gapPos2         = 100;
static int   gravity         = 55;
static int   ballX           = 55;
static int   currentCollumn  = 1;
static int   collumnGap      = 50;
static int   score           = 0;
static int   highScore       = 0;
static int   wight           = 20;
static int   logicManager    = 1;
static bool  flap            = true;
static int   lastLevel       = 1;
static bool  hasTouched      = false;
static int   cx1 = 187, cx2 = 170, cx3 = 203;
static int   randY = 0;

// ==================== Fonksiyonlar ====================
static void Score() {
  if (currentCollumn == 1 && ballX - 3 >= fx + wight) {
    score++;
    currentCollumn = 2;
  } else if (currentCollumn == 2 && ballX - 3 >= fz + wight) {
    score++;
    currentCollumn = 1;
  }
  if (score > highScore) highScore = score;
  if (score % 10 == 0 && score / 10 >= lastLevel) {
    collumnSpeed += 0.2f;
    lastLevel++;
  }
  img.setCursor(75, ground + 35);
  img.setTextSize(6);
  img.setTextDatum(4);
  img.setTextColor(TFT_WHITE, 0xDD4C);
  img.print(String(score));
  img.setCursor(80, ground + 20);
  img.setTextSize(1);
  img.setTextColor(TFT_WHITE, 0xDD4C);
  img.print("Best:");
  img.print(highScore);
}

static void GameOver(bool n) {
  if (n) {
    img.setCursor(20, 100);
    img.setTextColor(TFT_BLACK, TFT_RED);
    img.fillRect(0, 90, 172, 35, TFT_RED);
    img.setTextSize(2);
    img.print(" GAME OVER ");
    img.setCursor(30, 115);
    img.setTextSize(1);
    img.setTextColor(TFT_WHITE, TFT_RED);
    img.print("Press to restart");
  }
}

static void startGame() {
  img.setCursor(10, 72);
  img.setTextSize(2);
  img.setTextDatum(4);
  img.setTextColor(0xEC84);
  img.print(" FLAPPY BALL");
  img.setCursor(45, 95);
  img.setTextSize(1);
  img.setTextColor(TFT_WHITE);
  img.print("Press to start");
}

static void collomnGenerating() {
  img.fillRect((int)fx,      gapPos1 - 200, 5,  200, 0x2444);
  img.fillRect((int)fx + 5,  gapPos1 - 200, 10, 200, 0x8DA0);
  img.fillRect((int)fx + 15, gapPos1 - 200, 5,  200, 0xA627);
  img.fillRect((int)fx,      gapPos1 + 50,  5,  200, 0x2444);
  img.fillRect((int)fx + 5,  gapPos1 + 50,  10, 200, 0x8DA0);
  img.fillRect((int)fx + 15, gapPos1 + 50,  5,  200, 0xA627);

  img.fillRect((int)fz,      gapPos2 - 200, 5,  200, 0x2444);
  img.fillRect((int)fz + 5,  gapPos2 - 200, 10, 200, 0x8DA0);
  img.fillRect((int)fz + 15, gapPos2 - 200, 5,  200, 0xA627);
  img.fillRect((int)fz,      gapPos2 + 50,  5,  200, 0x2444);
  img.fillRect((int)fz + 5,  gapPos2 + 50,  10, 200, 0x8DA0);
  img.fillRect((int)fz + 15, gapPos2 + 50,  5,  200, 0xA627);

  if (fx + 20 < 0) { fx = 170; gapPos1 = 50 + std::rand() % 100; }
  if (fz + 20 < 0) { fz = 170; gapPos2 = 40 + std::rand() % 100; }
}

static void collisionDetections() {
  if (ballX + 5 >= fx && ballX - 5 <= fx + 20) {
    if ((int)fy - 5 < gapPos1 || (int)fy + 5 > gapPos1 + collumnGap) {
      hasTouched = true; logicManager = 3; return;
    }
  }
  if (ballX + 5 >= fz && ballX - 5 <= fz + 20) {
    if ((int)fy - 5 < gapPos2 || (int)fy + 5 > gapPos2 + collumnGap) {
      hasTouched = true; logicManager = 3; return;
    }
  }
}

static void drawFrame() {
  img.fillSprite(0x8E7D);
  // Bulutlar
  if (cx3 < 0) {
    cx1 = 197; cx2 = 180; cx3 = 213;
    randY = 50 - std::rand() % 150;
  } else { cx1--; cx2--; cx3--; }
  img.fillCircle(cx1, 85 + randY, 15, TFT_WHITE);
  img.fillCircle(cx2, 89 + randY, 11, TFT_WHITE);
  img.fillCircle(cx3, 93 + randY,  7, TFT_WHITE);

  collomnGenerating();
  img.fillRect(0, ground, 172, 20, TFT_DARKCYAN);
  img.fillRect(0, ground + 15, 172, 105, 0xDD4C);

  if (flap) { if (anim < 5) { anim++; } else { anim = 0; flap = !flap; } }

  if ((int)fy + 3 < ground) {
    img.fillCircle(ballX, (int)fy, 5, TFT_YELLOW);
    for (int row = 0; row < ROWS; row++) {
      for (int col = 0; col < COLS; col++) {
        int px = col + ballX - 6;
        int py = row + (int)fy - 3;
        int v  = flap ? flapDown[row][col] : flapUp[row][col];
        if      (v == 4) img.drawPixel(px, py, 0x0204);
        else if (v == 2) img.drawPixel(px, py, 0x9260);
        else if (v == 3) img.drawPixel(px, py, 0xFAC0);
        else if (v == 1) img.drawPixel(px, py, TFT_SILVER);
      }
    }
  } else {
    img.fillCircle(ballX, (int)fy, 5, TFT_RED);
    logicManager = 3; hasTouched = true;
  }

  if      (logicManager == 1) startGame();
  else if (logicManager == 3) GameOver(hasTouched);

  Score();
  img.pushSprite(0, 0);
}

static void resetGame() {
  logicManager = 2; velocity = 0;
  fx = 100; fz = 195; fy = 100;
  ground = 200; gapPos1 = 70; gapPos2 = 100;
  gravity = 55; ballX = 55; currentCollumn = 1;
  score = 0; hasTouched = false; collumnSpeed = 2;
  lastLevel = 1;
}

// ==================== GameHub Arayüzü ====================
void flappySetup() {
  tft.init();
  tft.setRotation(4);
  tft.setSwapBytes(true);
  tft.fillScreen(TFT_BLACK);
  img.createSprite(172, 320);
  img.setTextDatum(4);
  img.setTextColor(TFT_WHITE, TFT_BLACK);
  // Reset state
  logicManager = 1; velocity = 0;
  fx = 100; fz = 195; fy = 100;
  ground = 200; gapPos1 = 70; gapPos2 = 100;
  gravity = 55; ballX = 55; score = 0;
  highScore = 0; hasTouched = false;
  collumnSpeed = 2; lastLevel = 1;
  lastButtonState = HIGH;
}

void flappyUpdate() {
  // Sağ shoulder (up_btn  =2) veya up_btn ile zıpla
  buttonState = digitalRead(up_btn  );

  if (buttonState == LOW && lastButtonState == HIGH) {
    if      (logicManager == 1) logicManager = 2;
    else if (logicManager == 2) { velocity = -5; flap = true; }
    else if (logicManager == 3) resetGame();
  }
  lastButtonState = buttonState;

  unsigned long now = millis();
  if (now - previousMillis >= interval) {
    previousMillis = now;
    if (logicManager == 2) {
      if (!hasTouched) {
        fx -= collumnSpeed; fz -= collumnSpeed;
        velocity += gravity * 0.01f;
        fy += velocity;
      } else {
        fy = ground - 3;
      }
    }
    drawFrame();
  }
  collisionDetections();
}

void setup()  { flappySetup();  }
void update() { flappyUpdate(); }

} // namespace Flappy
