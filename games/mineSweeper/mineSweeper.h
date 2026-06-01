#ifndef MINESWEEPER_H
#define MINESWEEPER_H

#include <TFT_eSPI.h>
#include <Arduino.h>

extern TFT_eSPI    tft;
extern TFT_eSprite img;

enum MineState { MS_MAP_SELECT, MS_START, MS_GAME, MS_GAMEOVER, MS_WIN };
extern MineState gameState;

void mineSetup();
void mineUpdate();

#endif