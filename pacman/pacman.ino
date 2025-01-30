#include <TFT_eSPI.h>
#include "pacman.h"
#include <vector>
using namespace std;



void setup() {
  Serial.begin(9600);
  pacmanSetup();
}

void loop() {
  
  
    pacmanUpdate();





}
