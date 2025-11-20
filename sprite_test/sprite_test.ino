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

uint8_t tileMap[mapRows][mapCols] = { 
  {18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18},
  {18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18},
  {18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18},
  {18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18},
  {18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18},
  {18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18},
  {18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18},
  {18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18},
  {18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18},
  {18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18},
  {18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18},
  {18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18}
};


uint16_t buffer[BERRY_GARDEN_WIDTH * BERRY_GARDEN_HEIGHT];

float playerX = mapCols * tileSize / 2;
float playerY = mapRows * tileSize / 2;

int directionAngle = 0;

int lastEncoderPos = 0;

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  img.createSprite(screenW , screenH );

  drawMap();

  pinMode(rgh_btn, INPUT_PULLUP);
}
void loop() {
  encoder.tick();
  int newPos = encoder.getPosition();
  if (newPos != lastEncoderPos) {
    int diff = newPos - lastEncoderPos;

    directionAngle += diff * 45; // 1 tıkta 45 derece
    directionAngle = (directionAngle + 360) % 360; // 0-359 arasında tut

    lastEncoderPos = newPos;
  }

  if (digitalRead(rgh_btn) == LOW) {
    movePlayer();
  }

  render();
}

float speed = 5 ;

void movePlayer() {
  float radians = directionAngle * 3.1415926 / 180.0;
  float dx = cos(radians) * speed; 
  float dy = sin(radians) * speed;
  playerX += dx;
  playerY += dy;

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
  if (camY > mapRows * tileSize - screenH) camY = mapCols * tileSize - screenH;

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
  int centerX = screenW / 2;
  int centerY = screenH / 2;

  img.fillCircle(centerX, centerY, 8, TFT_WHITE); // Top

  float radians = directionAngle * 3.1415926 / 180.0;
  int lineX = centerX + cos(radians) * 16;
  int lineY = centerY + sin(radians) * 16;
  img.drawLine(centerX, centerY, lineX, lineY, TFT_RED);
  img.fillCircle(lineX,lineY,2,TFT_RED);
}
