// game_snake.h — GameHub Snake wrapper
#pragma once
#include <TFT_eSPI.h>
#include <cmath>
#include <cstring>
#include <vector>

namespace Snake {
using namespace std;

// ==================== Sabitler ====================
static const int CELL_SIZE  = 10;
static const int areaRow    = 20;
static const int areaColmn  = 17;

// ==================== Enum ====================
enum SnakeState { SS_start, SS_game, SS_gameOver, SS_pause };

// ==================== Değişkenler ====================
static SnakeState gameState = SS_start;
static unsigned long previousMillisMove = 0;
static int  sTimer = 0, prevTimer = 0;
static bool sUP=false, sDOWN=false, sLEFT=false, sRIGHT=false;
static bool sSELECT=false;
static bool sBtnPressed=false;

static uint8_t sArea[20][17];
static vector<pair<int,int>> sSnake;
static pair<int,int> sFood;

static int sScore = 0;

// ==================== Oyun Mantığı ====================
static bool isSnakePart(int row, int col) {
  for (const auto& seg : sSnake)
    if (seg.first==row && seg.second==col) return true;
  return false;
}

static pair<int,int> generateFood() {
  return { rand()%20, rand()%17 };
}

static bool foodEaten(int x, int y) {
  if (x==sFood.first && y==sFood.second) { sFood=generateFood(); sScore++; return true; }
  return false;
}

static void moveSnake(int dx, int dy) {
  int newX = sSnake.back().first  + dx;
  int newY = sSnake.back().second + dy;
  bool eaten = foodEaten(newX, newY);
  if (newX>19) newX=0; if (newX<0) newX=19;
  if (newY>16) newY=0; if (newY<0) newY=16;
  // Kendine çarpma
  for (const auto& seg : sSnake) {
    if (seg.first==newX && seg.second==newY) { gameState=SS_gameOver; return; }
  }
  sSnake.push_back({newX,newY});
  if (!eaten) sSnake.erase(sSnake.begin());
}

// ==================== Çizim ====================
static void drawSnakeGrid() {
  if (sUP)    moveSnake(-1, 0);
  if (sDOWN)  moveSnake( 1, 0);
  if (sLEFT)  moveSnake( 0,-1);
  if (sRIGHT) moveSnake( 0, 1);

  for (int i=0; i<areaRow; i++) {
    for (int j=0; j<areaColmn; j++) {
      int Y = i*CELL_SIZE+50;
      int X = j*CELL_SIZE+1;
      if (isSnakePart(i,j)) {
        img.fillRect(X+1,Y+1,CELL_SIZE-2,CELL_SIZE-2,TFT_DARKCYAN);
      } else if (i==sFood.first && j==sFood.second) {
        img.fillCircle(X+5,Y+5,4,TFT_YELLOW);
      }
    }
  }
}

static void sDrawInit() {
  img.fillSprite(0xA615);
  img.fillRect(0,0,172,50,0x218B);
  img.fillRect(0,250,172,70,0x218B);
  img.setCursor(41,10); img.setTextSize(3); img.setTextColor(TFT_YELLOW);
  img.print("SNAKE");
  img.setCursor(9,250); img.setTextSize(1); img.setTextColor(TFT_WHITE);
  img.print("Score:"); img.print(sScore);
  drawSnakeGrid();
  img.pushSprite(0,0);
}

static void sStartTAB() {
  img.fillSprite(TFT_BLACK);
  img.setCursor(25,20); img.setTextSize(3); img.setTextColor(TFT_YELLOW);
  img.print("SNAKE");
  img.setTextColor(TFT_WHITE);
  img.fillRoundRect(48,168,74,24,7,TFT_YELLOW);
  img.fillRoundRect(50,170,70,20,5,0x2C38);
  img.setCursor(67,175); img.setTextSize(1); img.print(" START ");
  img.pushSprite(0,0);
  if (digitalRead(select_btn)==LOW) { delay(150); gameState=SS_game; }
}

static void sGameOverTAB() {
  img.fillSprite(TFT_BLACK);
  img.setCursor(15,20); img.setTextSize(3); img.setTextColor(TFT_RED);
  img.print("GAMEOVER");
  img.setCursor(30,90); img.setTextSize(2); img.setTextColor(TFT_YELLOW);
  img.print("Score:"); img.print(sScore);
  img.fillRoundRect(48,188,74,24,7,TFT_YELLOW);
  img.fillRoundRect(50,190,70,20,5,0x2C38);
  img.setCursor(55,195); img.setTextSize(1); img.setTextColor(TFT_WHITE);
  img.print(" TRY AGAIN ");
  img.pushSprite(0,0);
  if (digitalRead(select_btn)==LOW) {
    delay(150);
    sSnake.clear(); sSnake.push_back({7,15});
    sFood=generateFood(); sScore=0;
    sUP=sDOWN=sLEFT=sRIGHT=false;
    gameState=SS_game;
  }
}

static void sButtonControl() {
  int btnUP    = digitalRead(up_btn);
  int btnDOWN  = digitalRead(dwn_btn);
  int btnRIGHT = digitalRead(rgh_btn);
  int btnLEFT  = digitalRead(lft_btn);
  int btnPAUSE = digitalRead(bck_btn);
  int btnSEL   = digitalRead(select_btn);

  if (!sBtnPressed) {
    if      (btnUP==LOW&&!sDOWN)   { sUP=true; sRIGHT=false; sLEFT=false; sBtnPressed=true; }
    else if (btnDOWN==LOW&&!sUP)   { sDOWN=true; sRIGHT=false; sLEFT=false; sBtnPressed=true; }
    else if (btnRIGHT==LOW&&!sLEFT){ sRIGHT=true; sDOWN=false; sUP=false; sBtnPressed=true; }
    else if (btnLEFT==LOW&&!sRIGHT){ sLEFT=true; sDOWN=false; sUP=false; sBtnPressed=true; }
    else if (btnPAUSE==LOW&&gameState==SS_game) { gameState=SS_pause; sBtnPressed=true; }
    else if (btnSEL==LOW&&gameState==SS_pause)  { gameState=SS_game; sBtnPressed=true; }
  }
  if (btnUP==HIGH&&btnDOWN==HIGH&&btnRIGHT==HIGH&&btnLEFT==HIGH&&btnPAUSE==HIGH&&btnSEL==HIGH)
    sBtnPressed=false;
}

// ==================== GameHub Arayüzü ====================
void snakeSetup() {
  tft.init();
  tft.setRotation(4);
  tft.fillScreen(TFT_BLACK);
  img.createSprite(172,320);
  memset(sArea,0,sizeof(sArea));
  sSnake.clear(); sSnake.push_back({7,15});
  sFood=generateFood(); sScore=0;
  sUP=sDOWN=sLEFT=sRIGHT=false; sBtnPressed=false;
  gameState=SS_start;
  img.pushSprite(0,0);
}

void snakeUpdate() {
  unsigned long now = millis();
  sButtonControl();
  if      (gameState==SS_start)    { sStartTAB(); }
  else if (gameState==SS_game) {
    if (now-previousMillisMove >= 10) {
      previousMillisMove=now;
      static unsigned long prevT=0;
      if (now-prevT>=200) { prevT=now; sDrawInit(); sTimer++; }
    }
  }
  else if (gameState==SS_gameOver) { sGameOverTAB(); }
  else if (gameState==SS_pause) {
    img.fillRect(0,95,170,32,TFT_BLACK);
    img.setCursor(35,100); img.setTextSize(3); img.setTextColor(TFT_RED);
    img.print("PAUSED");
    img.setCursor(20,140); img.setTextSize(1); img.setTextColor(TFT_WHITE);
    img.print("SELECT to continue");
    img.pushSprite(0,0);
  }
}

void setup()  { snakeSetup();  }
void update() { snakeUpdate(); }

} // namespace Snake
