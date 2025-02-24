
#ifndef TABS_H
#define TABS_H
#include "snake.h"

enum gameStateEnum{

 start,
 game,
 gameOver,
 pauseGame

};

extern gameStateEnum gameState;

void startTAB();
void gameOverTAB();




#endif 