<<<<<<< HEAD

#include "tabs.h"


gameStateEnum gameState = start;
bool R = true;


void startTAB() {
  if (gameState == start) {

    int xPadding = 30;


    img.fillSprite(TFT_BLACK);
    //drawMaze();
    img.setCursor(25, 20);
    img.setTextSize(3);
    img.setTextColor(TFT_YELLOW);
    img.print("PAC-MAN");
    img.setTextColor(TFT_WHITE);
    img.fillRoundRect(48, 168, 74, 24, 7, TFT_YELLOW);
    img.fillRoundRect(50, 170, 70, 20, 5, 0x2C38);
    img.setCursor(67, 175);
    img.setTextSize(1);
    img.print(" START ");

    startAnim();

    img.pushSprite(0, 0);
    if (SELECT) {
      gameState = game;
      SELECT = false;
    }
  }
}

void gameOverTAB() {
img.fillSprite(TFT_BLACK);
    //drawMaze();
    img.setCursor(15, 20);
    img.setTextSize(3);
    img.setTextColor(TFT_RED);
    img.print("GAMEOVER");
    img.setTextColor(TFT_WHITE);
    img.setCursor( 55, 140 );
    img.setTextSize(1);
    img.print(score);
    img.fillRoundRect(48, 188, 74, 24, 7, TFT_YELLOW);
    img.fillRoundRect(50, 190, 70, 20, 5, 0x2C38);
    img.setCursor(55, 195);
    img.setTextSize(1);
    img.print(" TRY AGAIN ");
    img.pushSprite(0,0);
    
}

void startAnim() {

  static int x = 0;
  static int y = 0;
  static int v = 2;

  static int px = 0;
  px += v;


  for (int row = 0; row < 10; row++) {
    for (int col = 0; col < 10; col++) {

      if (x > 300) {
        R = false;
        v = -2;

      } else if (x < -130) {
        R = true;
        v = 2;
      }

      x = col + px;
      y = row + 100;
      if (pacman.open) {

        if (R) {

          if (pacmanFront[row][col] == 1) {
            img.drawPixel(x, y, pacmanColor);
          } else {
            img.drawPixel(x, y, COLOR_0);
          }
        } else {
          if (pacmanBack[row][col] == 1) {
            img.drawPixel(x, y, pacmanColor);
          } else {
            img.drawPixel(x, y, COLOR_0);
          }
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


  uint8_t matrix[10][10];  // Geçici matris

  switch (ghostAnim) {
    case 0:
      memcpy(matrix, ghost, sizeof(matrix));
      break;
    case 1:
      memcpy(matrix, ghost1, sizeof(matrix));
      break;
    case 2:
      memcpy(matrix, ghost2, sizeof(matrix));
      break;
    case 3:
      memcpy(matrix, ghost3, sizeof(matrix));
      break;
  }

  // Ghost'un hareketini temizle ve güncelle
  img.fillRect(x, y + 1, 10, 9, COLOR_0);


  // Ghost'u çizer
  for (int row = 0; row < 10; row++) {
    for (int col = 0; col < 10; col++) {
      int gx = col + x;
      y = row + 100;

      if (matrix[row][col] == 1) {
        if (v == 2) {
          img.drawPixel(gx - 40 - 20, y, BlinkyColor);
          img.drawPixel(gx - 55 - 20, y, PinkyColor);
          img.drawPixel(gx - 70 - 20, y, InkyColor);
          img.drawPixel(gx - 85 - 20, y, ClydeColor);
        } else  {
          img.drawPixel(gx - 40 - 20, y, frightened);
          img.drawPixel(gx - 55 - 20, y, frightened);
          img.drawPixel(gx - 70 - 20, y, frightened);
          img.drawPixel(gx - 85 - 20, y, frightened);
        }
      } else if (matrix[row][col] == 2) {
        img.drawPixel(gx - 40 - 20, y, TFT_WHITE);

        img.drawPixel(gx - 55 - 20, y, TFT_WHITE);

        img.drawPixel(gx - 70 - 20, y, TFT_WHITE);
        img.drawPixel(gx - 85 - 20, y, TFT_WHITE);
      } else if (matrix[row][col] == 3) {
        img.drawPixel(gx - 40 - 20, y, TFT_BLACK);
        img.drawPixel(gx - 55 - 20, y, TFT_BLACK);
        img.drawPixel(gx - 70 - 20, y, TFT_BLACK);
        img.drawPixel(gx - 85 - 20, y, TFT_BLACK);
      }
    }
  }
}


=======

#include "tabs.h"


gameStateEnum gameState = start;
bool R = true;


void startTAB() {
  if (gameState == start) {

    int xPadding = 30;


    img.fillSprite(TFT_BLACK);
    //drawMaze();
    img.setCursor(25, 20);
    img.setTextSize(3);
    img.setTextColor(TFT_YELLOW);
    img.print("PAC-MAN");
    img.setTextColor(TFT_WHITE);
    img.fillRoundRect(48, 168, 74, 24, 7, TFT_YELLOW);
    img.fillRoundRect(50, 170, 70, 20, 5, 0x2C38);
    img.setCursor(67, 175);
    img.setTextSize(1);
    img.print(" START ");

    startAnim();

    img.pushSprite(0, 0);
    if (SELECT) {
      gameState = game;
      SELECT = false;
    }
  }
}

void gameOverTAB() {
img.fillSprite(TFT_BLACK);
    //drawMaze();
    img.setCursor(15, 20);
    img.setTextSize(3);
    img.setTextColor(TFT_RED);
    img.print("GAMEOVER");
    img.pushSprite(0,0);
    
}













void startAnim() {




  static int x = 0;
  static int y = 0;
  static int v = 2;

  static int px = 0;
  px += v;


  for (int row = 0; row < 10; row++) {
    for (int col = 0; col < 10; col++) {

      if (x > 300) {
        R = false;
        v = -2;

      } else if (x < -130) {
        R = true;
        v = 2;
      }

      x = col + px;
      y = row + 100;
      if (pacman.open) {

        if (R) {

          if (pacmanFront[row][col] == 1) {
            img.drawPixel(x, y, pacmanColor);
          } else {
            img.drawPixel(x, y, COLOR_0);
          }
        } else {
          if (pacmanBack[row][col] == 1) {
            img.drawPixel(x, y, pacmanColor);
          } else {
            img.drawPixel(x, y, COLOR_0);
          }
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


  uint8_t matrix[10][10];  // Geçici matris

  switch (ghostAnim) {
    case 0:
      memcpy(matrix, ghost, sizeof(matrix));
      break;
    case 1:
      memcpy(matrix, ghost1, sizeof(matrix));
      break;
    case 2:
      memcpy(matrix, ghost2, sizeof(matrix));
      break;
    case 3:
      memcpy(matrix, ghost3, sizeof(matrix));
      break;
  }

  // Ghost'un hareketini temizle ve güncelle
  img.fillRect(x, y + 1, 10, 9, COLOR_0);


  // Ghost'u çizer
  for (int row = 0; row < 10; row++) {
    for (int col = 0; col < 10; col++) {
      int gx = col + x;
      y = row + 100;

      if (matrix[row][col] == 1) {
        if (v == 2) {
          img.drawPixel(gx - 40 - 20, y, BlinkyColor);
          img.drawPixel(gx - 55 - 20, y, PinkyColor);
          img.drawPixel(gx - 70 - 20, y, InkyColor);
          img.drawPixel(gx - 85 - 20, y, ClydeColor);
        } else  {
          img.drawPixel(gx - 40 - 20, y, frightened);
          img.drawPixel(gx - 55 - 20, y, frightened);
          img.drawPixel(gx - 70 - 20, y, frightened);
          img.drawPixel(gx - 85 - 20, y, frightened);
        }
      } else if (matrix[row][col] == 2) {
        img.drawPixel(gx - 40 - 20, y, TFT_WHITE);

        img.drawPixel(gx - 55 - 20, y, TFT_WHITE);

        img.drawPixel(gx - 70 - 20, y, TFT_WHITE);
        img.drawPixel(gx - 85 - 20, y, TFT_WHITE);
      } else if (matrix[row][col] == 3) {
        img.drawPixel(gx - 40 - 20, y, TFT_BLACK);
        img.drawPixel(gx - 55 - 20, y, TFT_BLACK);
        img.drawPixel(gx - 70 - 20, y, TFT_BLACK);
        img.drawPixel(gx - 85 - 20, y, TFT_BLACK);
      }
    }
  }
}


>>>>>>> d3809c2514bf5a622f3d718869e316c3221eedf2
