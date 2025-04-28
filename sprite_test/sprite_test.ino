#include <TFT_eSPI.h>
#include "sprite.h"
#include <RotaryEncoder.h>

#define PIN_IN1 47
#define PIN_IN2 48
#define rgh_btn 2 // Scroll üzerindeki buton

RotaryEncoder encoder(PIN_IN1, PIN_IN2, RotaryEncoder::LatchMode::TWO03);

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite img = TFT_eSprite(&tft);

const int screenW = 320;
const int screenH = 170;
const int tileSize = 2 * 16;
const int mapCols = 24;
const int mapRows = 15;

byte tileMap[mapRows][mapCols] = {
  {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8},
  {2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9},
  {3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10},
  {4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11},
  {5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12},
  {6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13},
  {7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14},
  {8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
  {9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0},
  {10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1},
  {11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2},
  {12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3},
  {13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4},
  {14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5}
};

uint16_t buffer[BERRY_GARDEN_WIDTH * BERRY_GARDEN_HEIGHT];

// Harita üzerinde top'un konumu (float ile pürüzsüz kaydırma yaparız)
float playerX = mapCols * tileSize / 2;
float playerY = mapRows * tileSize / 2;

// Yön açısı (derece cinsinden)
int directionAngle = 0;

void setup() {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  img.createSprite(screenW , screenH );

  drawMap();

  pinMode(rgh_btn, INPUT_PULLUP);
}

void loop() {
  encoder.tick(); // Rotary encoder'ı oku

  static int lastEncoderPos = 0;
  int newPos = encoder.getPosition();
  if (newPos != lastEncoderPos) {
    directionAngle = (directionAngle + (newPos - lastEncoderPos) * 15) % 360; // Her tık 15 derece döndür
    if (directionAngle < 0) directionAngle += 360;
    lastEncoderPos = newPos;
  }

  if (digitalRead(rgh_btn) == LOW) {
    // Butona basıldıysa, seçili yöne doğru hareket ettir
    movePlayer();
  }

   render(); // Haritayı yeniden çiz

  delay(20); // Küçük bir bekleme
}

void movePlayer() {
  // Hareketi X ve Y bileşenlerine ayır
  float radians = directionAngle * 3.1415926 / 180.0;
  float dx = cos(radians) * 4; // Hız
  float dy = sin(radians) * 4;
  playerX += dx;
  playerY += dy;

  // Harita dışına taşmayı engelle
  if (playerX < 0) playerX = 0;
  if (playerX > mapCols * tileSize - 1) playerX = mapCols * tileSize - 1;
  if (playerY < 0) playerY = 0;
  if (playerY > mapRows * tileSize - 1) playerY = mapRows * tileSize - 1;
}

void drawBerryTileByNumber(int tileNumber, int xPos, int yPos) {

  int tileIndex = tileNumber - 1;
  int tileX = tileIndex % 4;
  int tileY = tileIndex / 4;

  int startX = tileX * 16;
  int startY = tileY * 16;

  for (int y = 0; y < 16; y++) {
    for (int x = 0; x < 16; x++) {
      int i = (startY + y) * BERRY_GARDEN_WIDTH + (startX + x);
      if (i >= BERRY_GARDEN_WIDTH * BERRY_GARDEN_HEIGHT) continue;
      uint16_t rawColor = pgm_read_word(&Berry_Garden[i]);
      uint16_t color = (rawColor >> 8) | (rawColor << 8);

      img.fillRect(x * 2 +xPos, y * 2 + yPos , 2, 2, color);
    }
  }

  
}

void drawMap() {
  int camX = playerX - screenW / 2;
  int camY = playerY - screenH / 2;

  if (camX < 0) camX = 0;
  if (camY < 0) camY = 0;
  if (camX > mapCols * tileSize - screenW) camX = mapCols * tileSize - screenW;
  if (camY > mapRows * tileSize - screenH) camY = mapRows * tileSize - screenH;

  for (int row = 0; row < mapRows; row++) {
    for (int col = 0; col < mapCols; col++) {
      int tileNumber = tileMap[row][col];
      int tileWorldX = col * tileSize;
      int tileWorldY = row * tileSize;

      int tileScreenX = tileWorldX - camX;
      int tileScreenY = tileWorldY - camY;

      if (tileScreenX + tileSize < 0 || tileScreenX > screenW) continue;
      if (tileScreenY + tileSize < 0 || tileScreenY > screenH) continue;

      drawBerryTileByNumber(tileNumber, tileScreenX, tileScreenY);
    }
  }
}
void render(){

  img.fillScreen(TFT_BLACK);
  drawMap();
  drawPlayer(); 
  img.pushSprite(0,0);

}

void drawPlayer() {
  // Ortada sabit duran bir top ve yön oku
  int centerX = screenW / 2;
  int centerY = screenH / 2;

  img.fillCircle(centerX, centerY, 8, TFT_WHITE); // Top

  // Yön oku
  float radians = directionAngle * 3.1415926 / 180.0;
  int lineX = centerX + cos(radians) * 12;
  int lineY = centerY + sin(radians) * 12;
  img.drawLine(centerX, centerY, lineX, lineY, TFT_RED);
}
