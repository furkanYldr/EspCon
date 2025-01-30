#ifndef TABS_H
#define TABS_H
#include "pacman.h"

enum gameStateEnum{

 start,
 game,
 gameOver,
 

};

extern gameStateEnum gameState;

void startTAB();
void gameOverTAB();
void startAnim();




#endif 