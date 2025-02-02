<<<<<<< HEAD
#ifndef TABS_H
#define TABS_H
#include "pacman.h"

enum gameStateEnum{

 start,
 game,
 gameOver,
 pauseGame

};

extern gameStateEnum gameState;

void startTAB();
void gameOverTAB();
void startAnim();




=======
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




>>>>>>> d3809c2514bf5a622f3d718869e316c3221eedf2
#endif 