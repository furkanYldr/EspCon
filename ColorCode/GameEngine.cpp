#include "GameEngine.h"
#include "colorCode.h"
#include <TFT_eSPI.h>
#include <vector>
#include <algorithm>
#include <random>
#include <Arduino.h>

extern TFT_eSPI tft;
extern TFT_eSprite img;

using namespace std;

// Rastgelelik sağlayıcı
random_device rd;  // Donanım tabanlı rastgelelik
mt19937 g(rd());   // Mersenne Twister rastgele sayı üreteci

vector<sBlock> vecBlocks;
vector<sBlock> vecPlayer;
vector<sBlock> vecGameOrder;
vector<int> clearedLevel;



State currentState = IDLE;

bool isBlockSelected = false;
const int SCREEN_WIDTH = 320;
const int SCREEN_HEIGHT = 170;
int correct = 0;
int current = 0;

// id 1 = red ,id 2 = yellow ,id 3 = white ,id 4 = black ,id 5 = light blue ,id 6 = green
void addBlock(uint16_t col, int i) {
  sBlock b;
  sBlock p;
  p.color = EMPTY_SPACE;
  b.color = col;
  b.id = i;
  p.id = 0;
  b.order = 0;
  p.order = 0;
  vecBlocks.push_back(b);
  vecPlayer.push_back(p);
}

void playerBlocks(int blockNum) {
 
  int padding = (SCREEN_WIDTH-(38*blockNum + (blockNum - 1 )* 10 ))/2 ;
  int padBBlocks = (SCREEN_WIDTH - (padding * 2) - (38 * vecPlayer.size())) / (vecPlayer.size() - 1);

  int index = 0;
  for (const sBlock& block : vecPlayer) {
    img.fillRect(padding + ((padBBlocks + 38) * index), 60, 38, 38, block.color);
    index++;
  }
  
}



void drawBlock(int height) {
  int padding = 21;
  int padBBlocks = (SCREEN_WIDTH - (padding * 2) - (38 * vecBlocks.size())) / (vecBlocks.size() - 1);

  int index = 0;
  for (const sBlock& block : vecBlocks) {
    img.fillRect(padding + ((padBBlocks + 38) * index), height, 38, 38, block.color);
    index++;
  }
}

void selectionBlock(int index ,bool selected, bool drop) {
  int highlightPadding = 3;
  int padding = 21;
  int padBBlocks = (SCREEN_WIDTH - (padding * 2) - (38 * vecPlayer.size())) / (vecPlayer.size() - 1);
  static int block;
  switch (currentState) {
    case IDLE:
      img.fillRect((padding + ((padBBlocks + 38) * index)) - highlightPadding, 60 - highlightPadding, 38 + highlightPadding * 2, 38 + highlightPadding * 2, BUTTON_HIGHLIGHT);

      if (drop) {
        currentState = COLOR_MENU;
        block = index;
      }
      break;

    case COLOR_MENU:
      img.fillRect((padding + ((padBBlocks + 38) * block)) - highlightPadding, 60 - highlightPadding, 38 + highlightPadding * 2, 38 + highlightPadding * 2, BUTTON_HIGHLIGHT);
      img.fillRect((padding + ((padBBlocks + 38) * index)) - highlightPadding, 10 - highlightPadding, 38 + highlightPadding * 2, 38 + highlightPadding * 2, BUTTON_HIGHLIGHT);
      drawBlock(10);

      vecPlayer[block] = vecBlocks[index];

      if (selected) {
        currentState = SELECT_COLOR;

        for (sBlock& b : vecPlayer) {
          if (b.id == vecBlocks[index].id && &b != &vecPlayer[block]) {
            b.id = 0;
            b.order = 0;
            b.color = EMPTY_SPACE;
            vecPlayer[block] = vecBlocks[index];
            vecPlayer[block].order = block + 1;
          } else {
            vecPlayer[block] = vecBlocks[index];
            vecPlayer[block].order = block + 1;
          }
        }
         tempBlock= block ;
      }
     
      break;

    case SELECT_COLOR:
      currentState = IDLE;
      break;
  }
}

void correctOrderCount(bool button  , int index) {




  if (button) {

    correct = 0;

    for (sBlock player : vecPlayer) {
      for (sBlock gameOrder : vecGameOrder) {
        if (player.id != 0) {
          if (player.order == gameOrder.order && player.id == gameOrder.id) {
            correct++;
          }
        } else {
          img.setTextSize(1);
          img.setCursor(10, 75);
          img.fillRect(0, 70, 320, 30, TFT_RED);
          img.print("Please fill all the blocks before proceeding.");
        }
      }
    }
  }
  img.setTextSize(2);
  img.setCursor(20, 120);
  img.print("matches:");
  img.print(correct);

  if (correct == 6) {
          img.setTextSize(2);
          img.setCursor(10, 75);
          img.fillRect(0, 70, 320, 30, LIGHT_GREEN);
          img.print(index);
          img.print(". Level Clear ");
          clearedLevel.push_back(index);
  }
}
void shuffleORDER() {
  vecGameOrder = vecBlocks;
  mt19937 g(rd());

  // Vektörü karıştır
  shuffle(vecGameOrder.begin(), vecGameOrder.end(), g);

  // Sıraları güncelle
  int currOrder = 1;
  for (sBlock& block : vecGameOrder) {
    block.order = currOrder;
    currOrder++;
  }
  for (const sBlock& block : vecGameOrder) {
    Serial.print("Color: ");
    Serial.print(block.id);
    Serial.print("   ");
    Serial.println(block.order);
  }
}




void seriesMenu() {
 
  int levelNum = 1 ;

  for (int i = 0; i < 3; i++) {
    int paddingx = 21;
    int paddingy = 18;
    int padBBlocks = 10;

    for (int j = 0; j < 6 ; j++) {
      if( find(clearedLevel.begin(),clearedLevel.end(),levelNum) != clearedLevel.end() ){

img.fillRect(paddingx + ((padBBlocks + 38) * j), paddingy + ((padBBlocks + 38) * i ), 38, 38, LIGHT_BLUE);
      }else {
      img.fillRect(paddingx + ((padBBlocks + 38) * j), paddingy + ((padBBlocks + 38) * i ), 38, 38, YELLOW);
      
      }
      tft.setTextDatum(5);
      img.setCursor(paddingx + ((padBBlocks + 38) * j)+ 8 , paddingy + ((padBBlocks + 38) * i) + 13);
      img.print(levelNum);
      levelNum++;
    }
  }
}



void seriesMenuHighLight(int index) {
  int i = 0;
  int j = 0;

  int paddingx = 21;
  int paddingy = 18;
  int padBBlocks = 10;
  int highLightPadding = 3 ;

  if(index <= 5 && index >= 0) {
    i = index;
    j = 0;
    img.fillRect(paddingx + ((padBBlocks + 38) * i) -highLightPadding, paddingy + ((padBBlocks + 38) * j) -highLightPadding, 38 + 2*highLightPadding, 38+2*highLightPadding, PINK);
  } else if (index <= 11 && index > 5) {
    i = index - 6;
    j = 1;
 img.fillRect(paddingx + ((padBBlocks + 38) * i) -highLightPadding, paddingy + ((padBBlocks + 38) * j) -highLightPadding, 38 + 2*highLightPadding, 38+2*highLightPadding, PINK);
  } else if (index <= 17 && index > 11) {
    i = index - 12;
    j = 2;
   img.fillRect(paddingx + ((padBBlocks + 38) * i) -highLightPadding, paddingy + ((padBBlocks + 38) * j) -highLightPadding, 38 + 2*highLightPadding, 38+2*highLightPadding, PINK);
  }

}
void seriesLevel(int levelIndex ){
   vecGameOrder = vecBlocks;
 int i = 0 ;
for (int num : levels[levelIndex]) {
   vecGameOrder[i].order = num;
   i++;
        }

}
