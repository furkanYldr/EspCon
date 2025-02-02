
#ifndef GAMEENGINE_H
#define GAMEENGINE_H
#include "colorCode.h"
#include <vector>
#include <Arduino.h>
using namespace std;



enum State {
  IDLE,        
  COLOR_MENU, 
  SELECT_COLOR
  } ;

void addBlock(uint16_t col ,int i );
void drawBlock(int height);
void shuffleORDER();
void playerBlocks();
void selectionBlock(int index, bool selected ,bool drop );
void correctOrderCount(bool button ,int index );
void seriesMenu();
void seriesMenuHighLight(int index);
void seriesLevel(int levelIndex );
void playerBlocks(int blockNum);

struct sBlock {
    uint16_t color;
    int id;  
    int order;
  
};


extern vector<sBlock> vecBlocks;


#endif 