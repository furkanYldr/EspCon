#include <TFT_eSPI.h>
#include <Wire.h>

TFT_eSPI tft = TFT_eSPI();          
TFT_eSprite img = TFT_eSprite(&tft); 

#define SaddamColor 0xEAC0

const int potPin = 1;

unsigned long previousMillis = 0;  
const unsigned long interval = 60; 
int ground = 160; 

int boxTop = 0;
int boxBottom = 320;
int boxLeft = 0;
int boxRight = 170;

int gravity = 10;                 
float speedY = -50; 
float speedX = 40;               
float x = 10;                     
float y = 150;


bool hasTouchedGround = false;  

void setup() {
  tft.init();
  tft.setRotation(2);
  tft.setSwapBytes(true);

  tft.fillScreen(TFT_BLACK);
  img.createSprite(170,320);
  img.setTextDatum(4);
  img.setTextColor(TFT_WHITE, TFT_BLACK);

  pinMode(15, OUTPUT); 
  digitalWrite(15, 1);

  img.pushSprite(0, 0);
}

void collision() {
  img.fillSprite(TFT_BLACK);

  if (y >= boxBottom || y <= boxTop) {

    speedY = -speedY;
 
  } else {
 
  }

  if (x >= boxRight || x <= boxLeft) {
    speedX = -speedX; 
  }

  img.drawPixel((int)x, (int)y, TFT_GREEN);

  img.setCursor(5, 5);  
  img.print("X: ");
  img.print((int)x);
  img.setCursor(5, 15);
  img.print("Y: ");
  img.print((int)y);


  img.pushSprite(0, 0);
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

   
    y += speedY * 0.1;
    x += speedX * 0.1;

    collision();
  }
}
