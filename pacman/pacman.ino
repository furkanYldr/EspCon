#include <TFT_eSPI.h>
#include "pacman.h"
#include <vector>
using namespace std;






unsigned long previousMillisMove = 0;

const unsigned long movementInterval = 40;





void setup() {
  tft.init();
  tft.setRotation(2);
  pinMode(15, OUTPUT);
  digitalWrite(15, 1);

  pinMode(lft_btn, INPUT_PULLUP);
  pinMode(rgh_btn, INPUT_PULLUP);
  pinMode(up_btn, INPUT_PULLUP);
  pinMode(dwn_btn, INPUT_PULLUP);

  tft.fillScreen(0x0130);
  img.createSprite(172, 320);
  img.pushSprite(0, 0);
  addGhost(1);
  addGhost(2);
  addGhost(3);
  addGhost(4);
}

void loop() {
  unsigned long currentMillis = millis();





  if (currentMillis - previousMillisMove >= movementInterval) {
    previousMillisMove = currentMillis;
   pacmanUpdate();
  }
}