#include "HWCDC.h"
#include "tetris.h"


#include "esp32-hal-gpio.h"
#include <cmath>
#include "resource.h"
#include <TFT_eSPI.h>
#include <vector>
using namespace std;


struct sBlocks {
 int type;
 int xGrid;
 int yGrid;
 int rotation;
 bool motion ;
};

vector<sBlocks> vecBlocks ;

#define BUTTON_PIN 14
bool buttonState = HIGH;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite img = TFT_eSprite(&tft);



unsigned long previousMillis = 0;
const unsigned long interval = 500; // 500 ms'de bir blok düşsün

sBlocks holding = {0,0,0,0,false};

void tetrisSetup() {
  tft.init();
  tft.setRotation(4);
  img.createSprite(170, 320);


  img.fillSprite(0x026A);


  img.setTextColor(TFT_WHITE, 0x026A);

  // HOLD
  img.setCursor(140, 20);
  img.setTextSize(1);
  img.print("Hold");
  img.fillRect(140, 30, 20, 20, TFT_DARKGREY);
  img.drawRect(139, 29, 22, 22, TFT_GOLD);

  // UEUE

  img.setCursor(140, 60);
  img.print("Next");
  img.fillRect(140, 70, 20, 100, TFT_DARKCYAN);
  img.drawRect(139, 69, 22, 102, TFT_GOLD);
  // text
  img.setCursor(10, 2);
  img.setTextSize(2);
  img.print("LINES-000");
  img.setCursor(140, 180);
  img.setTextSize(1);
  img.println("Level");
  img.setCursor(140, 190);

  img.print("00");

  img.setCursor(140, 200);
  img.println("Score");
  img.setCursor(140, 210);
  img.print("00000");


  int blockW = 13;
  int blockH = 13;

  int startX = 3;
  int startY = 20;
  addBlocks();
    pinMode(BUTTON_PIN, INPUT_PULLUP); 

}
void tetrisUpdate() {
  int reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {
      holdBlock();
      }
    }
  }
  lastButtonState = reading;
   unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
  img.fillRect(3, 20, 130, 260, TFT_BLACK);
    moveBlock();
  spawnBlock();
  img.pushSprite(0, 0);

  }
}


void drawTetromino(int type, int rotation, int gridX, int gridY) {
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if (tetrominoShapes[type][rotation][y][x]) {
        drawBlock(gridX + x, gridY + y, blockColor[type]);
      }
    }
  }
}

void drawBlock(int gridX, int gridY, uint16_t color) {
  const int blockSize = 13;

  const int offsetX = 3;
  const int offsetY = 20;

  int pixelX = offsetX + gridX * blockSize;
  int pixelY = offsetY + gridY * blockSize;

  img.fillRect(pixelX, pixelY, blockSize, blockSize, color);

  img.drawRect(pixelX, pixelY, blockSize, blockSize, TFT_BLACK);
}

void spawnBlock(){


drawTetromino(vecBlocks[0].type,vecBlocks[0].rotation , vecBlocks[0].xGrid ,vecBlocks[0].yGrid);

}
void moveBlock(){
vecBlocks[0].yGrid += 1 ;
}

void rotateBlock(){

  vecBlocks[0].rotation = (vecBlocks[0].rotation + 1) % 4;


}

void holdBlock(){
holding.type = vecBlocks[0].type;
holding.motion = vecBlocks[0].motion;

vecBlocks.erase(vecBlocks.begin());
}
void addBlocks(){
sBlocks b ;
b.type =  random(0, 7);
b.motion = false;
b.rotation = 0;
b.yGrid = 0;
b.xGrid = 3;

vecBlocks.emplace_back(b);
}
