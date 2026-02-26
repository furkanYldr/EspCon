#ifndef MINESWEEPER_H
#define MINESWEEPER_H

#include <TFT_eSPI.h>
#include <Arduino.h>
#include "resource.h"

// --- Oyun Durumları ---
enum GameStates {
  start,      // Başlangıç ekranı
  game,       // Oyun oynanıyor
  gameOver,   // Kaybettin
  win         // Kazandın
};

// --- Global Değişkenler (Extern) ---
extern TFT_eSPI tft;
extern TFT_eSprite img;
extern int gameState;

// --- Fonksiyon Prototipleri ---
void mineSetup();
void mineUpdate();

#endif