
#include <TFT_eSPI.h>
#include "pacman.h"
#include "resource.h"





TFT_eSPI tft = TFT_eSPI();
TFT_eSprite img = TFT_eSprite(&tft);



int x = 0;
int y = 0;
int pacmanSpeed = 1;
int animCounter = 0;

vector<sGhost> vecGhosts;

structPac pacman = { 81, 159, false, false, true, true, true };

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
  drawPacman(0, 0);
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

      x = col * CELL_SIZE + 9;
      y = row * CELL_SIZE + 32;
      if (coin_matrix[row][col] == 1) {
        img.drawPixel(x, y, pacmanColor);

      } else if (coin_matrix[row][col] == 2) {

        img.fillCircle(x, y, 3, TFT_WHITE);
      }
    }
  }
}



void drawPacman(int dx, int dy) {
  canMove();
  img.fillRect(pacman.px, pacman.py + 1, 10, 9, COLOR_0);
  pacman.px += dx;
  pacman.py += dy;
  for (int row = 0; row < 10; row++) {
    for (int col = 0; col < 10; col++) {

      x = col + pacman.px;
      y = row + pacman.py;
      if (pacman.open) {
        if (pacman1[row][col] == 1) {
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

  int centerx = pacman.px + 5;
  int centery = pacman.py + 6;

  int row = (centery - 30) / CELL_SIZE;
  int col = (centerx - 6) / CELL_SIZE;

  pacman.up = (row > 0 && coin_matrix[row - 1][col] != 0);
  pacman.down = (row < coin_rows - 1 && coin_matrix[row + 1][col] != 0);
  pacman.front = (col < coin_collums - 1 && coin_matrix[row][col + 1] != 0);
  pacman.back = (col > 0 && coin_matrix[row][col - 1] != 0);
  pacMOVEMENT();
}
void pacMOVEMENT() {

  int DOWN = digitalRead(dwn_btn);
  int UP = digitalRead(up_btn);
  int RIGHT = digitalRead(rgh_btn);
  int LEFT = digitalRead(lft_btn);

  if (UP == LOW) {
    if (pacman.up) {

      pacman.px += 0;
      pacman.py += -2;
    }
  }
  if (DOWN == LOW) {
    if (pacman.down) {

      pacman.px += 0;
      pacman.py += 2;
    }
  }

  if (RIGHT == LOW) {
    if (pacman.front) {

      pacman.px += 2;
      pacman.py += 0;
    }
  }

  if (LEFT == LOW) {
    if (pacman.back) {

      pacman.px += -2;
      pacman.py += 0;
    }
  }
}

void pacmanSetup(){



}
void pacmanUpdate(){

  animCounter += pacmanSpeed;

    drawInit();
    if (animCounter == 4 || animCounter > 4) {

      pacman.open = !pacman.open;
      animCounter = 0;
    }

}




































