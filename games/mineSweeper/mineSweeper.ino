#include "mineSweeper.h"

void setup() {
  Serial.begin(9600);
  mineSetup();
}

void loop() {
  mineUpdate();
}