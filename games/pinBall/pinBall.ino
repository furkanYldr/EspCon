#include "pinball.h"
#include <TFT_eSPI.h>
#include <vector>
using namespace std;

void setup() {
  Serial.begin(9600);
  pinballSetup();   // tetrisSetup yerine bunu çağır
}

void loop() {
  pinballUpdate();  // tetrisUpdate yerine bunu çağır
}
