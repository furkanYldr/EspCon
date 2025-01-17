#include "HWCDC.h"
#include "esp32-hal-gpio.h"
#include <TFT_eSPI.h>
#include <RotaryEncoder.h>
#include "colorCode.h"
#include <Arduino.h>
#include "resource.h"
#include <vector>

#include "GameEngine.h"
using namespace std;

#define PIN_IN1 43
#define PIN_IN2 44

RotaryEncoder encoder(PIN_IN1, PIN_IN2, RotaryEncoder::LatchMode::TWO03);

#define up_btn 11
#define dwn_btn 3
#define lft_btn 2
#define rgh_btn 13
#define selectBtn 10
#define backBtn 12

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite img = TFT_eSprite(&tft);

//int cellSize = 20 ;
int level = 0;
int logicManager = 0;
int newEncoderPos;
int prevEncoderPos = 0;
int menuIndex = 0;
int tab = 0;
bool selectState;
bool backState;
bool dropDown;
bool control;
int tempBlock = 0;
bool endlessGameStart = false;
bool isGameStarted = false;

bool lastSelectState = HIGH;
bool lastBackState = HIGH;
bool lastDropDown = HIGH;
bool lastControl = HIGH;


void Csetup() {
  tft.init();
  tft.setRotation(3);
  img.createSprite(320, 170);
  pinMode(lft_btn, INPUT_PULLUP);
  pinMode(rgh_btn, INPUT_PULLUP);
  pinMode(up_btn, INPUT_PULLUP);
  pinMode(dwn_btn, INPUT_PULLUP);
  pinMode(selectBtn, INPUT_PULLUP);
  pinMode(backBtn, INPUT_PULLUP);


  // id 1 = red ,id 2 = yellow ,id 3 = white ,id 4 = black ,id 5 = light blue ,id 6 = green
  addBlock(RED, 1);
  addBlock(YELLOW, 2);
  addBlock(WHITE, 3);
  addBlock(BLACK, 4);
  addBlock(LIGHT_BLUE, 5);
  addBlock(GREEN, 6);



  for (const sBlock& block : vecBlocks) {
    Serial.print("Color: ");
    Serial.print(block.id);
    Serial.print("   ");
    Serial.print(block.color);
    Serial.print("   ");
    Serial.println(block.order);
  }



  img.pushSprite(0, 0);
}
void Menu() {
  img.fillSprite(BACKGROUND);
  img.setCursor(75, 20);
  img.setTextSize(3);
  img.setTextColor(WHITE);
  img.print("COLOR-CODE");

  if (newEncoderPos != prevEncoderPos) {
    if (prevEncoderPos < newEncoderPos) {
      menuIndex = 1;
    } else {
      menuIndex = 2;
    }

    prevEncoderPos = newEncoderPos;
  }
  if (menuIndex == 1) {
    img.fillRoundRect(38, 93, 104, 49, 10, YELLOW);
  } else if (menuIndex == 2) {
    img.fillRoundRect(178, 93, 104, 49, 10, YELLOW);
  } else {
    menuIndex = 1;
  }

  img.fillRoundRect(40, 95, 100, 45, 8, LIGHT_BLUE);
  img.fillRoundRect(180, 95, 100, 45, 8, LIGHT_BLUE);
  img.setTextSize(2);

  img.setCursor(50, 110);
  img.print("Endless");
  img.setCursor(190, 110);
  img.print("Series");
}
void navigation() {

  if (selectState) {
    if (menuIndex == 1) {

      tab = 1;
      Serial.print(tab);

    } else if (menuIndex == 2) {
      if (!isGameStarted) {

        tab = 2;
      }
    }

  } else if (backState) {
    tab--;
    if (isGameStarted) {

      isGameStarted = false;
    }
  }





  switch (tab) {
    case 0:
      Menu();
      break;
    case 1:

      break;
    case 2:
      Series();
      break;
    case 3:
      levelScreen(level + 1);
      break;
    default:

      break;
  }
}

void levelScreen(int index) {

  img.fillSprite(BACKGROUND);
  img.setTextDatum(3);
  img.setCursor(290, 150);
  img.print(index);
  selectionBlock(tempBlock, selectState, dropDown);
  playerBlocks(6);


  if (newEncoderPos != prevEncoderPos) {
    if (prevEncoderPos < newEncoderPos) {
      if (tempBlock <= 5 && tempBlock > 0) {
        tempBlock--;
      } else {

        tempBlock = 5;
      }


    } else if (prevEncoderPos > newEncoderPos) {

      if (tempBlock < 5 && tempBlock >= 0) {
        tempBlock++;

      } else {
        tempBlock = 0;
      }
    }

    prevEncoderPos = newEncoderPos;
  }
 
  correctOrderCount(control, index);
}





void Series() {





  img.fillSprite(BACKGROUND);

  if (newEncoderPos != prevEncoderPos) {
    if (prevEncoderPos < newEncoderPos) {
      if (level <= 17 && level > 0) {
        level--;
      } else {

        level = 17;
      }


    } else if (prevEncoderPos > newEncoderPos) {

      if (level < 17 && level >= 0) {
        level++;

      } else {
        level = 0;
      }
    }


    prevEncoderPos = newEncoderPos;
  }
  seriesMenuHighLight(level);
  seriesMenu();


  if (dropDown) {
    if (!isGameStarted) {

      seriesLevel(level + 1);

      tab = 3;
      isGameStarted = true;
    }
  }
}




void update() {
  encoder.tick();
  newEncoderPos = encoder.getPosition();

  // Butonların mevcut durumlarını oku
  bool currentSelectState = digitalRead(selectBtn);
  bool currentBackState = digitalRead(backBtn);
  bool currentDropDown = digitalRead(rgh_btn);
  bool currentControl = digitalRead(lft_btn);

  // selectBtn için durum değişikliği algılama
  if (lastSelectState == HIGH && currentSelectState == LOW) {
    selectState = true;  // İşlem yapılabilir
  } else {
    selectState = false;  // İşlem yapılmaz
  }

  // backBtn için durum değişikliği algılama
  if (lastBackState == HIGH && currentBackState == LOW) {
    backState = true;
  } else {
    backState = false;
  }

  // rgh_btn için durum değişikliği algılama
  if (lastDropDown == HIGH && currentDropDown == LOW) {
    dropDown = true;
  } else {
    dropDown = false;
  }

  // lft_btn için durum değişikliği algılama
  if (lastControl == HIGH && currentControl == LOW) {
    control = true;
  } else {
    control = false;
  }

  // Son durumları güncelle
  lastSelectState = currentSelectState;
  lastBackState = currentBackState;
  lastDropDown = currentDropDown;
  lastControl = currentControl;

  render();
}






void render() {

  navigation();
  img.pushSprite(0, 0);
}