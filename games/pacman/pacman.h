#ifndef PACMAN_H
#define PACMAN_H

#include <TFT_eSPI.h>
#include <Arduino.h>
#include "resource.h"
#include "ghosts.h"
#include "tabs.h"
#include <vector>
using namespace std;

// Animasyon ve yön enumları
enum animDirection {
  AUP,
  ADOWN,
  ALEFT,
  ARIGHT
};
enum pacmanDirection {
  PUP,
  PDOWN,
  PLEFT,
  PRIGHT,
  PEMPTY
};

// Pacman yapısı
struct STRUCTPACMAN {
  int px, py;
  int pvx, pvy;
  bool up, down, front, back, open;
  pacmanDirection wantedDirection;
  animDirection ANIM;
};

// Fonksiyon prototipleri
void gameManager();
void pacmanSetup();
void pacmanUpdate();
void addGhost(int col);
void drawInit();
void drawMaze();
void drawPacman(uint8_t matrix[10][10]);
void canMove();
void pacMOVEMENT();
void alignToGrid();
void collectFood(int row, int col);
void monitorMemory();
void healthTracker();
void gameSetup();
void setGameStart();
void ghostStateManager();
// Global değişkenler
extern TFT_eSPI tft;
extern TFT_eSprite img;
extern STRUCTPACMAN pacman;

extern bool UP, DOWN, LEFT, RIGHT, SELECT;

extern const int xPadding, yPadding;
extern int score;
extern int ghostAnim;

extern bool ghostCLYDE ;
extern bool ghostINKY;
extern  bool CATCH;
extern int timer, prevTimer;

#endif  // PACMAN_H