#include <TFT_eSPI.h>
#include <Wire.h>

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite img = TFT_eSprite(&tft);

#define jumpButton 1

int lastButtonState = HIGH;
int buttonState = HIGH;

unsigned long previousMillis = 0;
const unsigned long interval = 50;
float velocity = 0;
float collumnSpeed = 2;
float x = 100;
float z = 195;
float y = 100;
int ground = 200;
int gapPos1 = 70;
int gapPos2 = 100;
int gravity = 55;
int ballX = 55;
int currentCollumn = 1;
int collumnGap = 50;
int score = 0;
int highScore = 0;
int wight = 20;
int logicManager = 1;

bool hasTouched = false;

void setup() {

  pinMode(15, OUTPUT);
  digitalWrite(15, 1);

  Serial.begin(9600);
  pinMode(jumpButton, INPUT_PULLUP);
  tft.init();
  tft.setRotation(2);
  tft.setSwapBytes(true);

  tft.fillScreen(TFT_BLACK);
  img.createSprite(172, 320);
  img.setTextDatum(4);
  img.setTextColor(TFT_WHITE, TFT_BLACK);


  img.pushSprite(0, 0);
}

void drawInit() {
  img.fillSprite(0x8E7D);
  collomnGenerating();
  img.fillRect(0, ground, 170, 20, TFT_DARKCYAN);
  img.fillRect(0, ground + 15, 170, 105, 0xDD4C);

  if (y + 3 < ground) {
    img.fillCircle(ballX, y, 5, 0xFFF9);

  } else {
    img.fillCircle(ballX, y, 5, TFT_RED);
    logicManager = 3;
    hasTouched = true;
  }
  if (logicManager == 1) {
    startGame();
  } else if (logicManager == 3) {
    GameOver(hasTouched);
  } else {
  }

  Score();

  img.pushSprite(0, 0);
}

void collomnGenerating() {


  img.fillRect(x, 0, wight, 200, 0x3D31);
  img.fillRect(x, gapPos1, wight, collumnGap, 0x8E7D);

  img.fillRect(z, 0, wight, 200, 0x3D31);
  img.fillRect(z, gapPos2, wight, collumnGap, 0x8E7D);

  if (x + 20 < 0) {
    x = 170;
    gapPos1 = 50 + std::rand() % 100;
  }
  if (z + 20 < 0) {
    z = 170;
    gapPos2 = 40 + std::rand() % 100;
  }
}

void GameOver(boolean n) {
  if (n) {
    img.setCursor(20, 100);
    img.setTextColor(TFT_BLACK, TFT_RED);
    img.fillRect(0, 97, 170, 20, TFT_RED);
    img.setTextSize(2);
    img.print(" GAME OVER ");

    img.fillRect(60, 200, 70, 20, 0xFE80);
    img.setCursor(20, 160);
    img.setTextSize(2);
    img.print(" TRY AGAIN ");
  }
}

void collisionDetections() {


  if (ballX + 5 >= x && ballX - 5 <= x + 20) {
    if (y - 5 < gapPos1 || y + 5 > gapPos1 + collumnGap) {
      hasTouched = true;
      logicManager = 3;
      return;
    }
  }

  if (ballX + 5 >= z && ballX - 5 <= z + 20) {
    if (y - 5 < gapPos2 || y + 5 > gapPos2 + collumnGap) {
      hasTouched = true;
      logicManager = 3;
      return;
    }
  }
}
void restart() {


  logicManager = 2;

  x = 100;
  z = 195;
  y = 100;
  ground = 200;
  gapPos1 = 70;
  gapPos2 = 100;
  gravity = 55;
  ballX = 55;
  currentCollumn = 1;
  score = 0;
  hasTouched = false;
}
void Score() {

  if (currentCollumn == 1 && ballX - 3 >= x + wight) {
    score += 1;
    currentCollumn = 2;

  } else if (currentCollumn == 2 && ballX - 3 >= z + wight) {
    score += 1;
    currentCollumn = 1;
  }
  if (score > highScore) {

    highScore = score;
  }

  img.setCursor(8, ground + 40);
  img.setTextSize(6);
  img.setTextDatum(4);
  img.setTextColor(TFT_WHITE, 0xDD4C);
  img.print(String(score));
  img.setCursor(80, ground + 20);
  img.setTextSize(1);
  img.print("Best score:");
  img.print(highScore);
}

void startGame() {

  img.setCursor(8, 70);
  img.setTextSize(2);
  img.setTextDatum(4);
  img.setTextColor(0xEC84);  //0xEC84
  img.print(String(" FLAPPY BALL"));
  img.setCursor(125, 95);
  img.setTextSize(1);
  img.print("By FYldrr");
}

void loop() {

  buttonState = digitalRead(jumpButton);




  if (buttonState == LOW && lastButtonState == HIGH) {  // Buton durumunu kontrol et
    if (logicManager == 1) {
      logicManager = 2;
    } else if (logicManager == 2) {
      velocity = -5;
    } else if (logicManager == 3) {
      restart();
    }
  }

  lastButtonState = buttonState;

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    if (logicManager == 2) {
      if (hasTouched == false) {
        x -= collumnSpeed;
        z -= collumnSpeed;
        velocity += gravity * 0.01;
        y += velocity;
      } else {
        velocity = 0;
        y = ground - 3;
      }
    }
    drawInit();
    Serial.println(logicManager);
  }




  collisionDetections();
}
