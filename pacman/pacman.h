#ifndef PACMAN_H
#define PACMAN_H


#include <TFT_eSPI.h>
#include <Arduino.h>
#include "resource.h"
#include "ghosts.h"
#include <vector>
#include <RotaryEncoder.h>

using namespace std;

void pacmanSetup();
void pacmanUpdate();
//void pacmanRender();
void addGhost(int col);
void drawInit();
void drawMaze(int n, int m);
void drawPacman(uint8_t matrix[10][10]);
void drawGhost();
void canMove();
void pacMOVEMENT();
void alignToGrid();
void collectFood(int row , int col);

extern TFT_eSPI tft;
extern TFT_eSprite img;
enum animDirection { AUP,
                     ADOWN,
                     ALEFT,
                     ARIGHT };
enum Direction { PUP,
                 PDOWN,
                 PLEFT,
                 PRIGHT,
                 PEMPTY };
struct structPac {
  int px;
  int py;
  int pvx;
  int pvy;
  bool up;
  bool down;
  bool front;
  bool back;
  bool open;
  Direction wantedDirection;
  animDirection ANIM;
};


struct sGhost {

  int gx;
  int gy;
  int id;
  int color;
  bool Alive;
  bool week;
};





#endif