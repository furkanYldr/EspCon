#ifndef SNAKE_H
#define SNAKE_H

#include <TFT_eSPI.h>
#include <Arduino.h>
#include "resource.h"
#include "tabs.h"
#include <vector>

using namespace std;


void snakeSetup();
void snakeUpdate();
void buttonControl();
void drawInit();
void drawSnake();
void adjustSpeedWithEncoder(); 
void moveSnake(int dx, int dy);
bool isSnakePart(int row, int col);
pair<int, int> generateFood();
bool foodEaten(int x , int y );
// Global değişkenler
extern TFT_eSPI tft;
extern TFT_eSprite img;

extern bool UP, DOWN, LEFT, RIGHT, SELECT;

extern uint8_t Area[areaRow][areaColmn]; ;


extern int timer, prevTimer;

#endif 