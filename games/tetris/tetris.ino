#include "tetris.h"
#include <TFT_eSPI.h>
#include <vector>
using namespace std;



void setup() {
  Serial.begin(9600);
tetrisSetup();
}


void loop() {
  
tetrisUpdate();


}
