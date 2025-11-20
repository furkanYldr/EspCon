#ifndef PINBALL_H
#define PINBALL_H

#include <TFT_eSPI.h>
#include <Arduino.h>
#include "resource.h"
#include <vector>

void pinballSetup();
void pinballUpdate();

// Aynı ekran/sprite'ı kullanmak için:
extern TFT_eSPI tft;
extern TFT_eSprite img;

#endif
