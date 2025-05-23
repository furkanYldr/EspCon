#include "esp32-hal-gpio.h"
#include <cmath>

#include <TFT_eSPI.h>
#include "pacman.h"
#include "resource.h"


unsigned long previousMillisMove = 0;
const unsigned long movementInterval = 10;

unsigned long core0Time = 0;
unsigned long core1Time = 0;
int timer = 0;
int prevTimer = 0;
int STATETimer = 0;
int prevScore = 0;
int countDown = 4;
int frightenedCountDown = 0;

int health = 3;

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite img = TFT_eSprite(&tft);


int score = 0;
int x = 0;
int y = 0;
int pacmanAnim = 1;
int pacmanSpeed = 2;
int animCounter = 0;
int c = 0;
const int xPadding = 76;
const int yPadding = -3;
int ghostAnim = 0;

bool UP = false;
bool DOWN = false;
bool LEFT = false;
bool RIGHT = false;
bool SELECT = false;
bool CATCH = false;

bool ghostINKY = false;
bool ghostCLYDE = false;
STRUCTPACMAN pacman = { 77, 209, 0, 0, false, false, true, true, true, PEMPTY, ARIGHT };


void drawInit() {
  img.fillSprite(TFT_BLACK);
  img.setCursor(25, 20);
  img.setTextSize(3);
  img.setTextColor(TFT_YELLOW);
  img.print("PAC-MAN");
  drawMaze();
  drawBlinky();
  switch (pacman.ANIM) {
    case ALEFT:
      drawPacman(pacmanBack);
      break;
    case ARIGHT:
      drawPacman(pacmanFront);
      break;
    case AUP:
      drawPacman(pacmanUp);
      break;
    case ADOWN:
      drawPacman(pacmanDown);
      break;
  }
  img.setCursor(110, 266);
  img.setTextSize(1);
  img.setTextColor(TFT_WHITE);

  img.print("score:");
  img.println(score);
  img.setCursor(150, 55);
  img.print(timer);

  healthTracker();
  pathFinder(pacman.px, pacman.py);
  img.pushSprite(0, 0);
}

void gameSetup() {

  if (CATCH) {
    Serial.println("çalışıyor catch ");
    if (health >= 1) {
      health--;
      CATCH = false;
      setGameStart();
    } else {
      gameState = gameOver;
    }
  }
}
void ghostStateManager() {

  for (STRUCTGHOST& ghost : vecGHOST) {
    
    if (ghost.STATE != FRIGHTENED && ghost.STATE !=EATEN) {
      Serial.println(".alışıyorum ghostmanager");
      if (STATETimer < 7 || (STATETimer < 20 && STATETimer > 14)) {

        ghost.STATE = SCATTER;
      } else {
        ghost.STATE = CHASE;
      }
    }
     else if (ghost.STATE == FRIGHTENED && ghost.STATE !=EATEN ) {
      if (frightenedCountDown > 0) {
        if (timer > prevTimer) {
          frightenedCountDown--;
        }
      } else if (frightenedCountDown == 0) {
        ghost.STATE = SCATTER;
        if(strike > 0 ){
          strike = 0 ;
        }
      }
    }

  }
}
void setGameStart() {

  // PACMAN RESETS
  pacman.wantedDirection = PEMPTY;
  pacman.ANIM = ARIGHT;
  pacman.px = 77;
  pacman.py = 209;
  pacman.pvy = 0;
  pacman.pvx = 0;

  vecGHOST[0].px = 77;  // Blinky
  vecGHOST[0].py = 137;
  vecGHOST[1].px = 80;  // Inky
  vecGHOST[1].py = 155;
  vecGHOST[2].px = 70;  // Clyde
  vecGHOST[2].py = 155;
  vecGHOST[3].px = 90;  // Clyde
  vecGHOST[3].py = 155;

  pinkyTimer = 0;
  STATETimer = 0;

  for (STRUCTGHOST& ghost : vecGHOST) {

    ghost.STATE = SCATTER;
    ghost.wantedDirection = GEMPTY;
    ghost.pvx = 0;
    ghost.pvy = 0;
  }

  c = 0;
}



void drawMaze() {

  int n = -2;
  int m = 70;
  ////MAZE DRAWWW
  for (int row = 0; row < MATRIX_ROWS; row++) {
    for (int col = 0; col < MATRIX_COLS; col++) {
      x = col * CELL_SIZE + n;
      y = row * CELL_SIZE + m;


      uint16_t color = (bmatrix[row][col] == 1) ? COLOR_1 : COLOR_0;


      img.fillRect(x, y, CELL_SIZE, CELL_SIZE, color);
    }
  }
  /// PATH AND COIN DRAW

  for (int row = 0; row < coin_rows; row++) {
    for (int col = 0; col < coin_collums; col++) {

      x = col * CELL_SIZE + yPadding;
      y = row * CELL_SIZE + xPadding;
      if (coin_matrix[row][col] == 1) {
        img.drawPixel(x, y, pacmanColor);

      } else if (coin_matrix[row][col] == 2) {

        img.fillCircle(x, y, 3, TFT_WHITE);
      }
    }
  }
}

void drawFruit() {


  for (int row = 0; row < 10; row++) {
    for (int col = 0; col < 11; col++) {

      x = col + 75;
      y = row + 275;

      switch (cherry[row][col]) {
        case 0:

          break;
        case 1:
          img.drawPixel(x, y, 0x9226);
          break;
        case 2:
          img.drawPixel(x, y, 0x4DC3);
          break;
        case 3:
          img.drawPixel(x, y, 0xF840);
          break;
        case 4:
          img.drawPixel(x, y, 0xC006);
          break;

        case 5:
          img.drawPixel(x, y, 0xE947);
          break;
      }
    }
  }
}

void drawPacman(uint8_t matrix[10][10]) {
  canMove();

  img.fillRect(pacman.px, pacman.py + 1, 10, 9, COLOR_0);
  pacman.px += pacman.pvx;
  pacman.py += pacman.pvy;
  for (int row = 0; row < 10; row++) {
    for (int col = 0; col < 10; col++) {

      x = col + pacman.px;
      y = row + pacman.py;
      if (pacman.open) {
        if (matrix[row][col] == 1) {
          img.drawPixel(x, y, pacmanColor);
        } else {
          img.drawPixel(x, y, COLOR_0);
        }
      } else {
        if (pacman2[row][col] == 1) {
          img.drawPixel(x, y, pacmanColor);
        } else {
          img.drawPixel(x, y, COLOR_0);
        }
      }
    }
  }
}

void canMove() {

  int centerx = pacman.px + 4;
  int centery = pacman.py + 5;

  int row = (centery - xPadding) / CELL_SIZE;
  int col = (centerx - yPadding) / CELL_SIZE;

  if ((centery - xPadding) % CELL_SIZE == 0) {

    if (row == 14 && pacman.px + 4 < 3) {
      pacman.px = 161;

    } else if (row == 14 && pacman.px + 4 > 165) {

      pacman.px = -1;
    }

    pacman.up = (row > 0 && coin_matrix[row - 1][col] != 0 && coin_matrix[row - 1][col] != 7);
    pacman.down = (row < coin_rows - 1 && coin_matrix[row + 1][col] != 0 && coin_matrix[row + 1][col] != 7);



    if (!pacman.up && pacman.pvy == -pacmanSpeed) pacman.pvy = 0;
    if (!pacman.down && pacman.pvy == pacmanSpeed) pacman.pvy = 0;
    collectFood(row, col);
  } else {
    pacman.up = false;
    pacman.down = false;
  }
  if ((centerx - yPadding) % CELL_SIZE == 0) {
    pacman.back = (col > 0 && coin_matrix[row][col - 1] != 0 && coin_matrix[row][col - 1] != 7);
    pacman.front = (col < coin_collums - 1 && coin_matrix[row][col + 1] != 0 && coin_matrix[row][col + 1] != 7);

    if (!pacman.front && pacman.pvx == pacmanSpeed) pacman.pvx = 0;
    if (!pacman.back && pacman.pvx == -pacmanSpeed) pacman.pvx = 0;
    collectFood(row, col);
  } else {

    pacman.back = false;
    pacman.front = false;
  }

  pacMOVEMENT();
}

bool btnPressed = false;

void pacMOVEMENT() {


  if (UP) {
    pacman.wantedDirection = PUP;
    UP = false;
  } else if (DOWN) {
    pacman.wantedDirection = PDOWN;
    DOWN = false;
  } else if (RIGHT) {
    pacman.wantedDirection = PRIGHT;
    RIGHT = false;
  } else if (LEFT) {
    pacman.wantedDirection = PLEFT;
    LEFT = false;
  }


  int centerx = pacman.px + 4;
  int centery = pacman.py + 5;

  int row = (centery - xPadding) / CELL_SIZE;
  int col = (centerx - yPadding) / CELL_SIZE;

  if ((centerx - yPadding) % CELL_SIZE == 0) {




    if (pacman.wantedDirection == PUP && pacman.up) {
      pacman.pvx = 0;
      pacman.pvy = -pacmanSpeed;
      pacman.wantedDirection = PEMPTY;
      pacman.ANIM = AUP;

      btnPressed = true;
    } else if (pacman.wantedDirection == PDOWN && pacman.down) {
      pacman.pvx = 0;
      pacman.pvy = pacmanSpeed;
      //alignToGrid();
      pacman.wantedDirection = PEMPTY;
      pacman.ANIM = ADOWN;

      btnPressed = true;
    }
  }
  if ((centery - xPadding) % CELL_SIZE == 0) {
    if (pacman.wantedDirection == PRIGHT && pacman.front) {
      pacman.pvx = pacmanSpeed;
      pacman.pvy = 0;
      //alignToGrid();
      pacman.wantedDirection = PEMPTY;
      pacman.ANIM = ARIGHT;
      btnPressed = true;
    } else if (pacman.wantedDirection == PLEFT && pacman.back) {
      pacman.pvx = -pacmanSpeed;
      pacman.ANIM = ALEFT;
      pacman.pvy = 0;
      // alignToGrid();
      pacman.wantedDirection = PEMPTY;
      btnPressed = true;
    }
  }
}

void buttonControl() {

  int btnDOWN = digitalRead(dwn_btn);
  int btnUP = digitalRead(up_btn);
  int btnRIGHT = digitalRead(rgh_btn);
  int btnLEFT = digitalRead(lft_btn);
  int btnPAUSE = digitalRead(bck_btn);
  int btnSELECT = digitalRead(select_btn);

  if (!btnPressed) {
    if (btnUP == LOW) {
      UP = true;
      btnPressed = true;
    } else if (btnDOWN == LOW) {
      DOWN = true;
      btnPressed = true;
    } else if (btnRIGHT == LOW) {
      RIGHT = true;
      btnPressed = true;
    } else if (btnLEFT == LOW) {
      LEFT = true;
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
  // Buton bırakıldığında sıfırla
  if (btnUP == HIGH && btnDOWN == HIGH && btnRIGHT == HIGH && btnLEFT == HIGH && btnPAUSE == HIGH && btnSELECT == HIGH) {
    btnPressed = false;
  }
}




void collectFood(int row, int col) {
  if (coin_matrix[row][col] == 1) { // Normal coin toplama
    coin_matrix[row][col] = 3;  // Coin toplandı olarak işaretle
    score++;
    if (score > prevScore) {
      if (c < 40) {
        c++;
      }
      if (c == 20) {
        ghostINKY = true;
        c++;
      }
      if (c == 40) {
        ghostCLYDE = true;
        c++;
      }
    }
    prevScore = score;
  } 
  else if (coin_matrix[row][col] == 2) { // Power-up toplama
    if (coin_matrix[row][col] != 3) {  
    
      coin_matrix[row][col] = 3; 
      Serial.print("freak fruit eaten row = ");
      Serial.println(coin_matrix[row][col]);
      score += 20;
      getFreak();
      frightenedCountDown = 25;
      goBack = true;
    }
  }
}



void pacmanSetup() {

  tft.init();
  tft.setRotation(4);
  pinMode(15, OUTPUT);
  digitalWrite(15, 1);

  pinMode(bck_btn, INPUT_PULLUP);
  pinMode(lft_btn, INPUT_PULLUP);
  pinMode(rgh_btn, INPUT_PULLUP);
  pinMode(up_btn, INPUT_PULLUP);
  pinMode(dwn_btn, INPUT_PULLUP);
  tft.fillScreen(0x0130);
  img.createSprite(172, 320);
  img.pushSprite(0, 0);
}

void healthTracker() {
  for (int i = 0; i < health && i < 3; i++) {
    for (int row = 0; row < 10; row++) {
      for (int col = 0; col < 10; col++) {

        x = col + 10;
        y = row + 265;

        if (pacmanFront[row][col] == 1) {

          img.drawPixel(x + (i * 15), y, pacmanColor);
        }
      }
    }
  }
}


void pacmanUpdate() {

  unsigned long currentMillis = millis();

  buttonControl();

  if (gameState == start) {
    if (currentMillis - previousMillisMove >= movementInterval) {
      previousMillisMove = currentMillis;
      animCounter += pacmanAnim;
      if (animCounter == 4 || animCounter > 4) {
        pacman.open = !pacman.open;
        animCounter = 0;
        if (ghostAnim < 3) {
          ghostAnim++;
        } else {
          ghostAnim = 0;
        }
      }
    }
    startTAB();
  } else if (gameState == game) {


    if (countDown == 0) {

      if (currentMillis - previousMillisMove >= movementInterval) {
        previousMillisMove = currentMillis;

        getAlive();
        eaten();
        catchPacMan();
        drawInit();
        gameSetup();

        animCounter += pacmanAnim;

        if (animCounter == 4 || animCounter > 4) {

          pacman.open = !pacman.open;
          animCounter = 0;
          if (ghostAnim < 3) {
            ghostAnim++;
          } else {
            ghostAnim = 0;
          }
        }
      }

      static unsigned long previousMillisTime = 0;

      if (currentMillis - previousMillisTime >= 1000) {
        previousMillisTime = currentMillis;
        prevTimer = timer;
        timer++;
        STATETimer++;
        ghostStateManager();
        spawnGhost();
      }

    } else if (countDown >= 0) {
      static unsigned long previousMillisCountdown = 0;
      if (currentMillis - previousMillisCountdown >= 1000) {
        previousMillisCountdown = currentMillis;
        countDown--;
        drawMaze();
        drawBlinky();
        img.setCursor(77, 150);
        img.setTextSize(3);
        img.setTextColor(TFT_RED);
        img.print(countDown);
        img.pushSprite(0, 0);
      }
    }



  } else if (gameState == pauseGame) {


    img.setCursor(35, 100);
    img.setTextSize(3);
    img.fillRect(0, 95, 170, 32, TFT_BLACK);
    img.setTextColor(TFT_RED);
    img.print("PAUSED");
    img.setTextColor(TFT_WHITE);
    img.fillRoundRect(48, 168, 74, 24, 7, TFT_YELLOW);
    img.fillRoundRect(50, 170, 70, 20, 5, 0x2C38);
    img.setCursor(62, 175);
    img.setTextSize(1);
    img.print("CONTINUE");
    img.pushSprite(0, 0);
  } else if (gameState == gameOver) {
    gameOverTAB();
  }
}

