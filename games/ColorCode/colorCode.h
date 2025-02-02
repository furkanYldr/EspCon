#ifndef COLORCODE_H
#define COLORCODE_H

#include <TFT_eSPI.h>
#include <Arduino.h>
#include "resource.h"
#include <vector>
#include <RotaryEncoder.h>

using namespace std;
extern int tempBlock ;

void Csetup();
void update();
void render();
void Menu();
void navigation();
void levelScreen(int index);
void Series();

extern TFT_eSPI tft;
extern TFT_eSprite img;






#endif