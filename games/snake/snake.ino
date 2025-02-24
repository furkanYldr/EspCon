#include <TFT_eSPI.h>
#include "snake.h"
#include <vector>
using namespace std;



void setup() {
  Serial.begin(9600);
  snakeSetup();
}

void loop() {
  
  
    snakeUpdate();





}
