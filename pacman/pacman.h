#ifndef PACMAN_H
#define PACMAN_H


#include <TFT_eSPI.h>
#include <Arduino.h>
#include "resource.h"
#include <vector>
#include <RotaryEncoder.h>

using namespace std;

void pacmanSetup();
void pacmanUpdate();
//void pacmanRender();
void addGhost(int col);
void drawInit();
void drawMaze(int n, int m);
void drawPacman(int dx, int dy);
void drawGhost();
void canMove();
void pacMOVEMENT();


extern TFT_eSPI tft;
extern TFT_eSprite img;


struct structPac {
  int px;
  int py;
  bool up;
  bool down;
  bool front;
  bool back;
  bool open;
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