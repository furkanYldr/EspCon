

#include "tabs.h"


gameStateEnum gameState = game;
bool R = true;


void startTAB() {
  if (gameState == start) {
    int xPadding = 30;
    img.fillSprite(TFT_BLACK);
    //drawMaze();
    img.setCursor(25, 20);
    img.setTextSize(3);
    img.setTextColor(TFT_YELLOW);
    img.print("SNAKE");
    img.setTextColor(TFT_WHITE);
    img.fillRoundRect(48, 168, 74, 24, 7, TFT_YELLOW);
    img.fillRoundRect(50, 170, 70, 20, 5, 0x2C38);
    img.setCursor(67, 175);
    img.setTextSize(1);
    img.print(" START ");


    img.pushSprite(0, 0);
  }
}

void gameOverTAB() {
img.fillSprite(TFT_BLACK);
    img.setCursor(15, 20);
    img.setTextSize(3);
    img.setTextColor(TFT_RED);
    img.print("GAMEOVER");
    img.setTextColor(TFT_WHITE);
    img.setCursor( 55, 140 );
    img.setTextSize(1);
    img.fillRoundRect(48, 188, 74, 24, 7, TFT_YELLOW);
    img.fillRoundRect(50, 190, 70, 20, 5, 0x2C38);
    img.setCursor(55, 195);
    img.setTextSize(1);
    img.print(" TRY AGAIN ");
    img.pushSprite(0,0);
    
}
