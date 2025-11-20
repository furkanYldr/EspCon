#ifndef TETRIS_H
#define TETRIS_H

#include <TFT_eSPI.h>
#include <Arduino.h>
#include "resource.h"

#include <vector>










void tetrisSetup();
void tetrisUpdate();

void drawTetromino(int type, int rotation, int gridX, int gridY);
void drawBlock(int gridX, int gridY, uint16_t color);


void spawnBlock();
void moveBlock();
void rotateBlock();
void holdBlock();
void addBlocks();






extern TFT_eSPI tft;
extern TFT_eSprite img;

#endif