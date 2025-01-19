#include <TFT_eSPI.h>
#include "pacman.h"
#include <vector>
using namespace std;






unsigned long previousMillisMove = 0;

const unsigned long movementInterval = 40;





void setup() {
pacmanSetup();
}

void loop() {
  unsigned long currentMillis = millis();





  if (currentMillis - previousMillisMove >= movementInterval) {
    previousMillisMove = currentMillis;
   pacmanUpdate();
  }
}