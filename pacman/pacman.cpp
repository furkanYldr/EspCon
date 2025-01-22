#include <cmath>

#include <TFT_eSPI.h>
#include "pacman.h"
#include "resource.h"





TFT_eSPI tft = TFT_eSPI();
TFT_eSprite img = TFT_eSprite(&tft);


int score = 0;
int x = 0;
int y = 0;
int pacmanAnim = 1;
int pacmanSpeed = 2;
int animCounter = 0;
const int xPadding = 26 ;
const int yPadding = -3 ;

vector<sGhost> vecGhosts;

structPac pacman = { 77, 159, 0, 0, false, false, true, true, true, PEMPTY, ARIGHT };

void addGhost(int col) {

  sGhost g;
  g.gx = 0;
  g.gy = 0;

  switch (col) {
    case 1:

      g.color = Pinky;
      break;
    case 2:

      g.color = Clyde;
      break;
    case 3:
      g.color = Blinky;
      break;
    case 4:
      g.color = Inky;
      break;
  }
  g.week = false;
  g.Alive = true;



  g.id = vecGhosts.size();
  vecGhosts.emplace_back(g);
}



void drawInit() {

  drawMaze(-2, 20);
  drawGhost();
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


  img.setCursor(20, 220);
  img.setTextSize(1);
  img.print("score:");
  img.print(score);




  img.pushSprite(0, 0);
}


void drawMaze(int n, int m) {

  ////MAZE DRAWWW
  for (int row = 0; row < MATRIX_ROWS; row++) {
    for (int col = 0; col < MATRIX_COLS; col++) {
      x = col * CELL_SIZE + n;
      y = row * CELL_SIZE + m;


      uint16_t color = (bmatrix[row][col] == 1) ? COLOR_1 : COLOR_0;


      img.fillRect(x, y, CELL_SIZE, CELL_SIZE, color);
    }
  }  /// PATH AND COIN DRAW

  for (int row = 0; row < coin_rows; row++) {
    for (int col = 0; col < coin_collums; col++) {

      x = col * CELL_SIZE + yPadding;
      y = row * CELL_SIZE + xPadding ;
      if (coin_matrix[row][col] == 1) {
        img.drawPixel(x, y, pacmanColor);

      } else if (coin_matrix[row][col] == 2) {

        img.fillCircle(x, y, 3, TFT_WHITE);
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




void drawGhost() {
  for (auto &ghostS : vecGhosts) {
    for (int row = 0; row < 10; row++) {
      for (int col = 0; col < 10; col++) {

        x = col + 65 + (ghostS.id * 15);
        y = row + 250;

        if (ghost[row][col] == 1) {

          img.drawPixel(x, y, ghostS.color);
        } else if (ghost[row][col] == 2) {

          img.drawPixel(x, y, TFT_WHITE);
        } else if (ghost[row][col] == 3) {

          img.drawPixel(x, y, TFT_BLACK);
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
  
  Serial.print("row: ");
  Serial.print(row);
  Serial.print(" ");
  Serial.print(centery);
  Serial.print(" ");
  Serial.print((row * CELL_SIZE) + xPadding);
  Serial.print(" ");
  Serial.print("collumn: ");
  Serial.print(col);
  Serial.print(" ");
  Serial.print(centerx);
  Serial.print(" ");
  Serial.println((col * CELL_SIZE) + yPadding);




  if ((centery - xPadding) % CELL_SIZE == 0) {

    if (row == 14 && pacman.px + 4  < 3) {
      pacman.px = 161;

    } else if (row == 14 && pacman.px +4 > 165) {

      pacman.px = -1 ;
    } 


    pacman.up = (row > 0 && coin_matrix[row - 1][col] != 0);

    pacman.down = (row < coin_rows && coin_matrix[row + 1][col] != 0);


    if (!pacman.up && pacman.pvy == -pacmanSpeed) pacman.pvy = 0;
    if (!pacman.down && pacman.pvy == pacmanSpeed) pacman.pvy = 0;
    collectFood(row, col);
  } else {
    pacman.up = false;
    pacman.down = false;
  }
  if ((centerx - yPadding) % CELL_SIZE == 0) {
    pacman.back = (col > 0 && coin_matrix[row][col - 1] != 0);

    pacman.front = (col < coin_collums && coin_matrix[row][col + 1] != 0);

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

  int DOWN = digitalRead(dwn_btn);
  int UP = digitalRead(up_btn);
  int RIGHT = digitalRead(rgh_btn);
  int LEFT = digitalRead(lft_btn);

  if (!btnPressed) {
    if (UP == LOW) {
      pacman.wantedDirection = PUP;
      btnPressed = true;
    } else if (DOWN == LOW) {
      pacman.wantedDirection = PDOWN;
      btnPressed = true;
    } else if (RIGHT == LOW) {
      pacman.wantedDirection = PRIGHT;
      btnPressed = true;
    } else if (LEFT == LOW) {
      pacman.wantedDirection = PLEFT;
      btnPressed = true;
    }
  }

  int centerx = pacman.px + 4;
  int centery = pacman.py + 5;

  int row = (centery - xPadding) / CELL_SIZE;
  int col = (centerx - yPadding) / CELL_SIZE;

  if ((centerx - yPadding) % CELL_SIZE == 0) {




    if (pacman.wantedDirection == PUP && pacman.up) {
      pacman.pvx = 0;
      pacman.pvy = -pacmanSpeed;
      //alignToGrid();
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


  // Buton bırakıldığında sıfırla
  if (UP == HIGH && DOWN == HIGH && RIGHT == HIGH && LEFT == HIGH) {
    btnPressed = false;
  }
}

void collectFood(int row, int col) {

  if (coin_matrix[row][col] == 1) {
    coin_matrix[row][col] = 3;
    score++;
  }
}







void pacmanSetup() {

  tft.init();
  tft.setRotation(2);
  pinMode(15, OUTPUT);
  digitalWrite(15, 1);

  pinMode(lft_btn, INPUT_PULLUP);
  pinMode(rgh_btn, INPUT_PULLUP);
  pinMode(up_btn, INPUT_PULLUP);
  pinMode(dwn_btn, INPUT_PULLUP);
  tft.fillScreen(0x0130);
  img.createSprite(172, 320);
  img.pushSprite(0, 0);
  addGhost(1);
  addGhost(2);
  addGhost(3);
  addGhost(4);
}
void pacmanUpdate() {

  animCounter += pacmanAnim;

  drawInit();
  if (animCounter == 4 || animCounter > 4) {

    pacman.open = !pacman.open;
    animCounter = 0;
  }
}
