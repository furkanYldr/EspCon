
#include <vector>
#include "colorCode.h"


using namespace std;




// back 12 pause 1  



unsigned long previousMillisMove = 0;

const unsigned long movementInterval = 20;


void setup() {
  Serial.begin(115200);

  pinMode(15, OUTPUT);
  digitalWrite(15, 1);

 

 Csetup();
 

}


void loop() {
  unsigned long currentMillis = millis();
  

  



  if (currentMillis - previousMillisMove >= movementInterval) {
  previousMillisMove = currentMillis;
 update();
  }
}