#include "esp32-hal-gpio.h"
#include <cmath>
#include <TFT_eSPI.h>
#include "snake.h"
#include "resource.h"
#include <cstring>








unsigned long previousMillisMove = 0;
const unsigned long movementInterval = 800;
int timer = 0;
int prevTimer = 0;
int prevScore = 0;
int countDown = 4;

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite img = TFT_eSprite(&tft);



bool UP = false;
bool DOWN = false;
bool LEFT = false;
bool RIGHT = false;
bool SELECT = false;
bool CATCH = false;

uint8_t Area[areaRow][areaColmn];
vector<std::pair<int, int>> snake;

void drawInit() {
  img.fillSprite(0xA615);  //0x9634 ;
  img.fillRect(0, 0, 172, 50, 0x218B);
  img.fillRect(0, 250, 172, 70, 0x218B);
  img.setCursor(41, 10);
  img.setTextSize(3);

  drawSnake();
  img.setTextColor(TFT_YELLOW);
  img.print("SNAKE");

  img.setCursor(9, 250);
  img.setTextSize(1);
  img.setTextColor(TFT_WHITE);

  img.print("score:");


  img.pushSprite(0, 0);
}


void drawSnake() {
  pair<int, int > food = {10,10};
  if (UP) {

    moveSnake(-1, 0);
  }
  if (DOWN) {

    moveSnake(1, 0);
  }
  if (LEFT) {

    moveSnake(0, -1);
  }
  if (RIGHT) {

    moveSnake(0, 1);
  }

  for (int i = 0; i < areaRow; i++) {
    for (int j = 0; j < areaColmn; j++) {
      int Y = i * CELL_SIZE + 50;
      int X = j * CELL_SIZE + 1;

      if (isSnakePart(i, j)) {
        img.fillRect(X + 1, Y + 1, CELL_SIZE - 2, CELL_SIZE - 2, TFT_DARKCYAN);
      } else if (i == food.first && j == food.second) {

        img.fillCircle(X + 5, Y + 5, 4, TFT_YELLOW);
      }
    }
  }
}
void moveSnake(int dx, int dy) {
  int newX = snake.back().first + dx;
  int newY = snake.back().second + dy;

  if (newX > 19) newX = 0;
  if (newX < 0) newX = 19;
  if (newY > 16) newY = 0;
  if (newY < 0) newY = 16;

  snake.push_back({ newX, newY });
  snake.erase(snake.begin());
}

pair<int, int> generateFood() {
  int foodX = rand() % 20;
  int foodY = rand() % 17;
  return { foodX, foodY };
}

void snakeSetup() {


  tft.init();
  tft.setRotation(4);
  pinMode(bck_btn, INPUT_PULLUP);
  pinMode(lft_btn, INPUT_PULLUP);
  pinMode(rgh_btn, INPUT_PULLUP);
  pinMode(up_btn, INPUT_PULLUP);
  pinMode(dwn_btn, INPUT_PULLUP);
  tft.fillScreen(0x0130);
  img.createSprite(172, 320);
  memset(Area, 0, sizeof(Area));
  snake.push_back({ 7, 15 });
  snake.push_back({ 8, 15 });
  snake.push_back({ 9, 15 });
  snake.push_back({ 10, 15 });
  snake.push_back({ 11, 15 });

  img.pushSprite(0, 0);
}


void snakeUpdate() {

  unsigned long currentMillis = millis();

  buttonControl();

  if (gameState == start) {
    startTAB();
  } else if (gameState == game) {

    if (currentMillis - previousMillisMove >= 10) {
      previousMillisMove = currentMillis;

      static unsigned long previousMillisTime = 0;

      if (currentMillis - previousMillisTime >= 500) {
        previousMillisTime = currentMillis;
        prevTimer = timer;
        drawInit();
        timer++;
      }
    }

  } else if (gameState == gameOver) {

    gameOverTAB();
  }
}


bool btnPressed = false;

void buttonControl() {

  int btnDOWN = digitalRead(dwn_btn);
  int btnUP = digitalRead(up_btn);
  int btnRIGHT = digitalRead(rgh_btn);
  int btnLEFT = digitalRead(lft_btn);
  int btnPAUSE = digitalRead(bck_btn);
  int btnSELECT = digitalRead(select_btn);

  if (!btnPressed) {
    if (btnUP == LOW && !DOWN) {  // Ters hareketi engelle
      UP = true;
      RIGHT = false;
      LEFT = false;
      btnPressed = true;
    } else if (btnDOWN == LOW && !UP) {
      DOWN = true;
      RIGHT = false;
      LEFT = false;
      btnPressed = true;
    } else if (btnRIGHT == LOW && !LEFT) {
      RIGHT = true;
      DOWN = false;
      UP = false;
      btnPressed = true;
    } else if (btnLEFT == LOW && !RIGHT) {
      LEFT = true;
      DOWN = false;
      UP = false;
      btnPressed = true;
    } else if (btnPAUSE == LOW) {
      if (gameState == game) {
        gameState = pauseGame;
        countDown = 4;
      }
      btnPressed = true;
    } else if (btnSELECT == LOW) {
      if (gameState == pauseGame) {
        gameState = game;
      } else {
        SELECT = true;
      }
      btnPressed = true;
    }
  }
  if (btnUP == HIGH && btnDOWN == HIGH && btnRIGHT == HIGH && btnLEFT == HIGH && btnPAUSE == HIGH && btnSELECT == HIGH) {
    btnPressed = false;
  }
}
bool isSnakePart(int row, int col) {
  for (const auto& segment : snake) {
    if (segment.first == row && segment.second == col) {
      return true;
    }
  }
  return false;
}
