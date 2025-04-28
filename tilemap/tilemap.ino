#include <TFT_eSPI.h>
#include <RotaryEncoder.h>
#include <Arduino.h>
#include <vector>
#include "sprite.h"


#define PIN_IN1 47
#define PIN_IN2 48
#define rgh_btn 2
#define up_btn 1

RotaryEncoder encoder(PIN_IN1, PIN_IN2, RotaryEncoder::LatchMode::TWO03);
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite img = TFT_eSprite(&tft);
TFT_eSprite tileset = TFT_eSprite(&tft);  

float ballX = 480;
float ballY = 255;
float angle = 0;
int newEncoderPos = 0;
int prevEncoderPos = 0;
bool walk = false;
bool lastRgh = HIGH;

const int screenW = 320;
const int screenH = 170;
const int tileSize = 2*16;
const int mapCols = 24;
const int mapRows = 15;
const float speed = 3.0;

struct Bullet { float x, y, dx, dy; };
std::vector<Bullet> bullets;
unsigned long lastShotTime = 0;
const int shotCooldown = 200;

uint8_t tileMap[mapRows][mapCols] = { 
  {22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22},
  {22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22},
  {22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22},
  {22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22},
  {22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22},
  {22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22},
  {22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22},
  {22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22},
  {22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22},
  {22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22},
  {22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22},
  {22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22},
  {22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22},
  {22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22}
};

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(1);
  img.createSprite(screenW, screenH);
  tileset.createSprite(128, 128);  // <-- Yeni: tileset sprite oluştur
  pinMode(rgh_btn, INPUT_PULLUP);
  pinMode(up_btn, INPUT_PULLUP);
}

void updateInput() {
  encoder.tick();
  newEncoderPos = encoder.getPosition();

  if (newEncoderPos != prevEncoderPos) {
    int diff = newEncoderPos - prevEncoderPos;
    angle += diff * 45;
    if (angle < 0) angle += 360;
    if (angle >= 360) angle -= 360;
    prevEncoderPos = newEncoderPos;
  }

  bool currentRgh = digitalRead(rgh_btn);
  walk = (lastRgh == HIGH && currentRgh == LOW);
  lastRgh = currentRgh;

  if (!digitalRead(rgh_btn)) {
    float dx = cos(radians(angle)) * speed;
    float dy = sin(radians(angle)) * speed;

    ballX += dx;
    ballY += dy;

    if (ballX < 0) ballX = 0;
    if (ballX > tileSize * mapCols) ballX = tileSize * mapCols;
    if (ballY < 0) ballY = 0;
    if (ballY > tileSize * mapRows) ballY = tileSize * mapRows;
  }

  if (!digitalRead(up_btn) && millis() - lastShotTime > shotCooldown) {
    float rad = radians(angle);
    Bullet b;
    b.x = ballX;
    b.y = ballY;
    b.dx = cos(rad) * 5.0;
    b.dy = sin(rad) * 5.0;
    bullets.push_back(b);
    lastShotTime = millis();
  }

  for (int i = bullets.size() - 1; i >= 0; i--) {
    bullets[i].x += bullets[i].dx;
    bullets[i].y += bullets[i].dy;

    if (bullets[i].x < 0 || bullets[i].x > tileSize * mapCols ||
        bullets[i].y < 0 || bullets[i].y > tileSize * mapRows) {
      bullets.erase(bullets.begin() + i);
    }
  }
}

void drawTile(int tileNumber, int x, int y) {
  
  int tileIndex = tileNumber - 1;
  int sx = (tileIndex % 4) ;
  int sy = (tileIndex / 4) ;
  
  int startX = tileX * 16;
  int startY = tileY * 16;

  for (int ty = 0; ty < 16; ty++) {
  
      uint16_t color = tileset.readPixel(sx + tx, sy + ty);
      // Her pikseli 2x2 blok olarak çiz
      img.fillRect(x * 2, y * 2, 2, 2, color);
    
  }
}
void render() {
  img.fillSprite(TFT_BLACK);

  int startCol = max(0, (int)(ballX - screenW / 2) / tileSize);
  int endCol = min(mapCols, (int)(ballX + screenW / 2) / tileSize + 1);
  int startRow = max(0, (int)(ballY - screenH / 2) / tileSize);
  int endRow = min(mapRows, (int)(ballY + screenH / 2) / tileSize + 1);

  for (int row = startRow; row < endRow; row++) {
    for (int col = startCol; col < endCol; col++) {
      int tileType = tileMap[row][col];
      int drawX = (col * tileSize - ballX) + screenW / 2;
      int drawY = (row * tileSize - ballY) + screenH / 2;
      drawTile(tileType, drawX, drawY);  // <-- Yeni: görselden tile çiz
    }
  }

  img.fillCircle(screenW / 2, screenH / 2, 6, TFT_WHITE);

  float arrowLength = 15;
  float rad = radians(angle);
  int arrowX = screenW / 2 + cos(rad) * arrowLength;
  int arrowY = screenH / 2 + sin(rad) * arrowLength;
  img.drawLine(screenW / 2, screenH / 2, arrowX, arrowY, TFT_RED);
  img.fillCircle(arrowX, arrowY, 3, TFT_RED);

  for (Bullet& b : bullets) {
    int drawX = (b.x - ballX) + screenW / 2;
    int drawY = (b.y - ballY) + screenH / 2;
    img.fillCircle(drawX, drawY, 2, TFT_RED);
  }

  img.pushSprite(0, 0);
}

void loop() {
  updateInput();
  render();
}
