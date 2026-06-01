
#ifndef TABS_H
#define TABS_H
#include "pacman.h"

enum gameStateEnum{

 start,
 game,
 gameOver,
 pauseGame,
 win

};

extern gameStateEnum gameState;

void startTAB();
void gameOverTAB();
void winTAB();
void startAnim();



#endif 