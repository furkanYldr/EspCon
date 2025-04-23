#include <TFT_eSPI.h>

#define BUTTON1_PIN 7// BACK
#define BUTTON2_PIN 6// L
#define BUTTON3_PIN 5// UP
#define BUTTON4_PIN 4// D
#define BUTTON5_PIN 3// SELECT
#define BUTTON6_PIN 2// R



void setup() {
  Serial.begin(115200);

pinMode(BUTTON1_PIN, INPUT_PULLUP);
pinMode(BUTTON2_PIN, INPUT_PULLUP);
pinMode(BUTTON3_PIN, INPUT_PULLUP);
pinMode(BUTTON4_PIN, INPUT_PULLUP);
pinMode(BUTTON5_PIN, INPUT_PULLUP);
pinMode(BUTTON6_PIN, INPUT_PULLUP);


}

void loop() {
  Serial.print("BACK: "); Serial.println(!digitalRead(BUTTON1_PIN));
  Serial.print("LEFT: "); Serial.println(!digitalRead(BUTTON2_PIN));
  Serial.print("UP: "); Serial.println(!digitalRead(BUTTON3_PIN));
  Serial.print("DOWN: "); Serial.println(!digitalRead(BUTTON4_PIN));
  Serial.print("SELECT: "); Serial.println(!digitalRead(BUTTON5_PIN));
  Serial.print("RIGHT: "); Serial.println(!digitalRead(BUTTON6_PIN));

  delay(500); // 500ms bekle ki okumalar görülebilsin
}

