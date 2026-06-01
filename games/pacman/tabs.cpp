#include "tabs.h"
#include "pacman.h"
#include "resource.h"


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

  // Başlık — kırmızı
  img.setTextSize(3);
  img.setTextColor(TFT_RED);
  img.setCursor(8, 20);
  img.print("GAME OVER");

  // Skor
  img.setTextSize(2);
  img.setTextColor(TFT_YELLOW);
  img.setCursor(35, 70);
  img.print("Score");
  img.setCursor(55, 95);
  img.setTextColor(TFT_WHITE);
  img.print(score);

  // TRY AGAIN butonu
  img.fillRoundRect(38, 150, 96, 28, 7, TFT_RED);
  img.fillRoundRect(40, 152, 92, 24, 5, 0x6000);
  img.setCursor(44, 160);
  img.setTextSize(1);
  img.setTextColor(TFT_WHITE);
  img.print(" TRY AGAIN");

  img.pushSprite(0, 0);

  // SELECT ile yeniden başlat
  if (digitalRead(select_btn) == LOW) {
    delay(200);  // debounce
    // Tam sıfırlama
    score       = 0;
    health      = 3;
    timer       = 0;
    prevTimer   = 0;
    STATETimer  = 0;
    prevScore   = 0;
    countDown   = 4;
    frightenedCountDown = 0;
    ghostINKY   = false;
    ghostCLYDE  = false;
    resetCoinMatrix();
    setGameStart();
    gameState = game;
  }
}

void winTAB() {
  img.fillSprite(TFT_BLACK);

  // Başlık — sarı parlak
  img.setTextSize(2);
  img.setTextColor(TFT_YELLOW);
  img.setCursor(22, 20);
  img.print("YOU WIN!");

  // Skor
  img.setTextSize(1);
  img.setTextColor(TFT_WHITE);
  img.setCursor(35, 60);
  img.print("Final Score:");
  img.setTextSize(2);
  img.setTextColor(TFT_YELLOW);
  img.setCursor(45, 78);
  img.print(score);

  // Hayaletleri küçük ikon olarak sırala (frightened renginde)
  for (int i = 0; i < 4; i++) {
    for (int row = 0; row < 10; row++) {
      for (int col = 0; col < 10; col++) {
        if (ghost[row][col] == 1) {
          img.drawPixel(col + 20 + i * 35, row + 115, frightened);
        } else if (ghost[row][col] == 2) {
          img.drawPixel(col + 20 + i * 35, row + 115, TFT_WHITE);
        }
      }
    }
  }

  // PLAY AGAIN butonu
  img.fillRoundRect(38, 150, 96, 28, 7, TFT_YELLOW);
  img.fillRoundRect(40, 152, 92, 24, 5, 0x2C38);
  img.setCursor(40, 160);
  img.setTextSize(1);
  img.setTextColor(TFT_WHITE);
  img.print("  PLAY AGAIN");

  img.pushSprite(0, 0);

  // SELECT ile yeni oyun
  if (digitalRead(select_btn) == LOW) {
    delay(200);
    score       = 0;
    health      = 3;
    timer       = 0;
    prevTimer   = 0;
    STATETimer  = 0;
    prevScore   = 0;
    countDown   = 4;
    frightenedCountDown = 0;
    ghostINKY   = false;
    ghostCLYDE  = false;
    resetCoinMatrix();
    setGameStart();
    gameState = game;
  }
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

