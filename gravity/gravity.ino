#include <TFT_eSPI.h>
#include <Wire.h>


TFT_eSPI tft = TFT_eSPI();           
TFT_eSprite img = TFT_eSprite(&tft); 

#define SaddamColor 0xEAC0

const int potPin = 1;


unsigned long previousMillis = 0;   
const unsigned long interval = 60;  
int ground = 160; 


int boxTop = 320;
int boxBottom = 0;
int boxLeft = 0;
int noxRight = 170 ;

               
int gravity = 10;                
float distance = 0;             
float speedY = -50; 
float speedX = 40;             
float x = 10;                     
float y = 150;
int potValue = 0;

bool hasTouchedGround = false; 

void setup() {
  tft.init();
  tft.setRotation(3);
  tft.setSwapBytes(true);

  tft.fillScreen(TFT_BLACK);
  img.createSprite(320, 170);
  img.setTextDatum(4);
  img.setTextColor(TFT_WHITE, TFT_BLACK);

  pinMode(15, OUTPUT); 
  digitalWrite(15, 1);

  img.pushSprite(0, 0);
}
void launchDraw() {
  img.fillSprite(TFT_BLACK);


  img.drawFastHLine(0, ground, 320, TFT_WHITE);

  
  if (y < ground) {
    img.drawPixel((int)x, (int)y, TFT_GREEN);
    hasTouchedGround = false; 
  } else {
    img.drawPixel((int)x, ground - 1, TFT_RED); 
    hasTouchedGround = true;                    
    speedY = 0;                                 
  }

 
  img.setCursor(5, 5); 
  img.print("X: ");
  img.print((int)x);
  img.setCursor(5, 15);
  img.print("Y: ");
  img.print((int)y);
  img.setCursor(5, 25);
  img.print("pot value :");
  img.print(potValue);
 
  if (hasTouchedGround) {
    img.setCursor(5, 35); 
    img.print("landed");
  }

 
  img.pushSprite(0, 0);
}

void loop() {

  potValue = analogRead(potPin);

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    if (y < ground) {
      x += 3;
      speedY += gravity * 0.1;  
      y += speedY * 0.1;
    }
    launchDraw(); 
  }
}
