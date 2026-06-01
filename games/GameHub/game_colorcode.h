// game_colorcode.h — ColorCode, namespace ColorCode içinde, tam inline
// Landscape: rotation=1, sprite 320x170
#pragma once
#include <TFT_eSPI.h>
#include <RotaryEncoder.h>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <cmath>

namespace ColorCode {
using namespace std;

// ==================== Renkler ====================
#define CC_YELLOW    0xF6AD
#define CC_RED       0xF840
#define CC_GREEN     0x0400
#define CC_ORANGE    0xFAC4
#define CC_PINK      0xEC1D
#define CC_BLACK     0x0000
#define CC_WHITE     0xFFFD
#define CC_LIGHT_BLUE 0x8E7D
#define CC_LIGHT_GREEN 0xA615
#define CC_BACKGROUND 0x0208
#define CC_EMPTY     0xA617
#define CC_HIGHLIGHT 0xDD1B

// Level verileri
static const int CC_LEVELS[18][6] = {
  {6,3,4,2,1,5},{3,6,4,1,5,2},{3,5,6,4,2,1},{3,6,4,1,2,5},{2,1,6,4,3,5},
  {4,5,2,6,1,3},{5,2,4,1,6,3},{5,2,1,4,3,6},{4,1,6,5,2,3},{2,4,5,6,1,3},
  {2,3,1,6,4,5},{6,1,3,4,2,5},{1,3,2,5,4,6},{3,4,5,6,2,1},{2,5,4,1,3,6},
  {1,3,4,5,2,6},{6,3,5,2,4,1},{1,4,3,6,2,5}
};

// ==================== Struct ====================
struct CCBlock { int id, order; uint16_t color; };

static vector<CCBlock> vecBlocks;
static vector<CCBlock> vecPlayer;
static vector<CCBlock> vecGameOrder;
static vector<int>     clearedLevel;

// ==================== Particle ====================
struct CPt { float x,y,vx,vy; uint8_t life,maxLife,sz; uint16_t col; };
static CPt cp[24];
static void cpClear(){ memset(cp,0,sizeof(cp)); }
static void cpSpawn(float ox,float oy,uint16_t col,float vxr=2.0f,float vyb=0.0f,int n=4){
  for(int k=0;k<n;k++) for(int i=0;i<24;i++){
    if(cp[i].life)continue;
    cp[i]={ox+(float)(rand()%20-10)/10.f, oy,
            (float)(rand()%200-100)/100.f*vxr,
            (float)(rand()%200-100)/100.f+vyb,
            0,0,(uint8_t)(rand()%2+1),col};
    cp[i].maxLife=(uint8_t)(rand()%20+15);
    cp[i].life=cp[i].maxLife; break;
  }
}
static void cpBurst(float cx,float cy){
  static const uint16_t RC[]={CC_RED,CC_YELLOW,CC_WHITE,CC_LIGHT_BLUE,CC_GREEN,CC_PINK,CC_ORANGE};
  for(int k=0;k<10;k++) for(int i=0;i<24;i++){
    if(cp[i].life)continue;
    float ang=(float)(rand()%360)*3.14159f/180.0f;
    float spd=(float)(rand()%20+8)/10.0f;
    cp[i]={cx,cy,cosf(ang)*spd,sinf(ang)*spd,0,0,(uint8_t)(rand()%3+1),RC[rand()%7]};
    cp[i].maxLife=(uint8_t)(rand()%25+20); cp[i].life=cp[i].maxLife; break;
  }
}
static void cpUpdate(){
  for(auto& p:cp){ if(!p.life)continue; p.x+=p.vx; p.y+=p.vy; p.vy+=0.05f; p.life--;
    if(p.x<0||p.x>320||p.y<0||p.y>170) p.life=0; }
}
static void cpDraw(){
  for(auto& p:cp){ if(!p.life)continue;
    float a=(float)p.life/p.maxLife; uint16_t c=p.col;
    int r=(int)(((c>>11)&0x1F)*a),g=(int)(((c>>5)&0x3F)*a),b=(int)((c&0x1F)*a);
    c=(uint16_t)((r<<11)|(g<<5)|b);
    if(p.sz<=1) img.drawPixel((int)p.x,(int)p.y,c);
    else         img.fillCircle((int)p.x,(int)p.y,p.sz-1,c);
  }
}

// ==================== Gradient ====================
static void ccGrad(uint16_t top,uint16_t bot,int h=170){
  int tr=(top>>11)&0x1F,tg=(top>>5)&0x3F,tb=top&0x1F;
  int br=(bot>>11)&0x1F,bg=(bot>>5)&0x3F,bb=bot&0x1F;
  for(int y=0;y<h;y++){
    float t=(float)y/(h-1);
    int r=(int)(tr+(br-tr)*t),g=(int)(tg+(bg-tg)*t),b=(int)(tb+(bb-tb)*t);
    img.drawFastHLine(0,y,320,(uint16_t)((r<<11)|(g<<5)|b));
  }
}

// ==================== Encoder (GameHub'ınkini kullanıyoruz) ====================
// GameHub.ino'da hubEncoder zaten var — biz kendi state'imizi tutuyoruz
static int ccEncPos=0, ccPrevEncPos=0;

// ==================== Oyun Değişkenleri ====================
static int ccLevel=0, ccTab=0, ccMenuIndex=1, ccTempBlock=0;
static bool ccSelect=false,ccBack=false,ccDrop=false,ccControl=false;
static bool ccIsGameStarted=false;
static int  ccCorrect=0;
static bool ccLastSel=HIGH,ccLastBck=HIGH,ccLastDrp=HIGH,ccLastCtl=HIGH;

// State machine
enum CCState { CC_IDLE, CC_COLOR_MENU, CC_SELECT_COLOR };
static CCState ccState=CC_IDLE;
static int ccChosenBlock=0;
static CCBlock ccOldBlock;

// ==================== Init ====================
static void ccAddBlock(uint16_t col,int id){
  CCBlock b={id,0,col}, p={0,0,CC_EMPTY};
  vecBlocks.push_back(b); vecPlayer.push_back(p);
}

static void ccResetPlayer() {
  for (auto& p : vecPlayer) {
    p = {0, 0, CC_EMPTY};
  }
}

static void ccShuffleOrder() {
  vecGameOrder = vecBlocks;
  for (int i = (int)vecGameOrder.size() - 1; i > 0; i--) {
    int j = rand() % (i + 1);
    swap(vecGameOrder[i], vecGameOrder[j]);
  }
  int currOrder = 1;
  for (auto& block : vecGameOrder) block.order = currOrder++;
}

// ==================== Çizim Yardımcıları ====================
static void ccDrawSourceBlocks(int y){
  int pad=21, padB=(320-(pad*2)-(38*6))/(6-1);
  for(int i=0;i<(int)vecBlocks.size();i++)
    img.fillRect(pad+(padB+38)*i, y, 38, 38, vecBlocks[i].color);
}
static void ccDrawPlayerBlocks(){
  int pad=21, padB=(320-(pad*2)-(38*6))/(6-1);
  for(int i=0;i<(int)vecPlayer.size();i++)
    img.fillRect(pad+(padB+38)*i, 60, 38, 38, vecPlayer[i].color);
}
static void ccHighlight(int idx,int y,uint16_t col){
  int pad=21, padB=(320-(pad*2)-(38*6))/(6-1), hp=3;
  img.fillRect(pad+(padB+38)*idx-hp, y-hp, 38+hp*2, 38+hp*2, col);
}

// ==================== Menu ====================
static void ccDrawMenu(){
  ccGrad(0x0208,0x1043);
  img.setTextFont(4); img.setTextDatum(MC_DATUM);
  img.setTextColor(TFT_YELLOW); img.drawString("COLOR",100,28);
  img.setTextColor(TFT_WHITE);  img.drawString("CODE", 220,28);
  img.drawFastHLine(30,52,260,0xFFE0); img.setTextDatum(0);
  // Ambient
  if(rand()%4==0) cpSpawn((float)(rand()%300+10),(float)(rand()%30+130),0x07FF,0.3f,-0.4f,1);
  bool e_sel=(ccMenuIndex==1), s_sel=(ccMenuIndex==2);
  img.fillRoundRect(28,70,124,60,8,e_sel?CC_YELLOW:0x18C3);
  if(e_sel) img.drawRoundRect(27,69,126,62,9,TFT_WHITE);
  img.setTextColor(e_sel?CC_BLACK:CC_WHITE); img.setTextFont(2);
  img.setCursor(42,86); img.print("ENDLESS");
  img.setTextFont(1); img.setTextColor(e_sel?0x18C3:0x8C71);
  img.setCursor(44,106); img.print("Free play");
  img.fillRoundRect(168,70,124,60,8,s_sel?CC_LIGHT_BLUE:0x18C3);
  if(s_sel) img.drawRoundRect(167,69,126,62,9,TFT_WHITE);
  img.setTextColor(s_sel?CC_BLACK:CC_WHITE); img.setTextFont(2);
  img.setCursor(186,86); img.print("SERIES");
  img.setTextFont(1); img.setTextColor(s_sel?0x0208:0x8C71);
  img.setCursor(188,106); img.print("18 levels");
  img.setTextFont(1); img.setTextColor(0x39E7);
  img.setCursor(80,148); img.print("ENC=switch  SELECT=choose");
}

// ==================== Series Menü ====================
static void ccDrawSeriesMenu(){
  int lv=1;
  for(int i=0;i<3;i++) for(int j=0;j<6;j++){
    bool cl=false;
    for(int x:clearedLevel) if(x==lv){cl=true;break;}
    img.fillRect(21+(48*j),18+(48*i),38,38,cl?0x07E0:CC_YELLOW);
    img.setTextFont(1); img.setCursor(21+(48*j)+8,18+(48*i)+13); img.print(lv++);
  }
}
static void ccHighlightSeries(int idx){
  int i=0,j=0,hp=3;
  if(idx<=5){i=idx;j=0;}
  else if(idx<=11){i=idx-6;j=1;}
  else{i=idx-12;j=2;}
  img.fillRect(21+(48*i)-hp,18+(48*j)-hp,38+hp*2,38+hp*2,CC_PINK);
}
static void ccSeriesLevel(int li){
  vecGameOrder=vecBlocks;
  if(li>=1&&li<=18) for(int i=0;i<6;i++) vecGameOrder[i].order=CC_LEVELS[li-1][i];
}

// ==================== Level Ekranı ====================
static void ccDrawLevelScreen(int idx){
  ccGrad(0x0208,0x0841);
  img.setTextFont(4); img.setTextColor(TFT_YELLOW);
  img.setCursor(4,4); img.print("LVL "); img.setTextColor(TFT_WHITE);
  if(ccMenuIndex==1){
    img.print("ENDLESS");
  } else {
    img.print(idx);
  }
  img.drawFastHLine(4,36,312,0x39E7);

  // Selection highlight
  int pad=21,padB=(320-(pad*2)-(38*6))/(6-1),hp=3;
  if(ccState==CC_IDLE){
    img.fillRect(pad+(padB+38)*ccTempBlock-hp,60-hp,38+hp*2,38+hp*2,CC_HIGHLIGHT);
  } else if(ccState==CC_COLOR_MENU){
    img.fillRect(pad+(padB+38)*ccChosenBlock-hp,60-hp,38+hp*2,38+hp*2,CC_HIGHLIGHT);
    img.fillRect(pad+(padB+38)*ccTempBlock-hp,10-hp,38+hp*2,38+hp*2,CC_HIGHLIGHT);
    ccDrawSourceBlocks(10);
    vecPlayer[ccChosenBlock]=vecBlocks[ccTempBlock];
  }

  ccDrawPlayerBlocks();

  // Matches display
  img.fillRect(0,120,320,24,0x18C3);
  img.setTextFont(1); img.setTextColor(0x8C71);
  img.setCursor(8,126); img.print("matches:");
  for(int d=0;d<6;d++){
    img.fillCircle(85+d*22,132,8,(d<ccCorrect)?0x07E0:0x39E7);
    img.drawCircle(85+d*22,132,8,TFT_WHITE);
  }
}

// ==================== SelectionBlock (state machine) ====================
static void ccSelectionBlock(bool drop){
  int pad=21,padB=(320-(pad*2)-(38*6))/(6-1);
  switch(ccState){
    case CC_IDLE:
      if(drop){
        ccState=CC_COLOR_MENU;
        ccChosenBlock=ccTempBlock;
        ccOldBlock=vecPlayer[ccChosenBlock];
      }
      break;
    case CC_COLOR_MENU:
      vecPlayer[ccChosenBlock]=vecBlocks[ccTempBlock];
      if(ccSelect){
        ccState=CC_SELECT_COLOR;
        for(auto& b:vecPlayer){
          if(b.id==vecBlocks[ccTempBlock].id && &b!=&vecPlayer[ccChosenBlock]){
            b={0,0,CC_EMPTY};
          }
        }
        vecPlayer[ccChosenBlock]=vecBlocks[ccTempBlock];
        vecPlayer[ccChosenBlock].order=ccChosenBlock+1;
        ccTempBlock=ccChosenBlock;
      }
      break;
    case CC_SELECT_COLOR:
      ccState=CC_IDLE; break;
  }
}

// ==================== CorrectOrderCount ====================
static void ccCheckOrder(bool btn,int idx){
  if(btn){
    bool allFilled=true;
    for(auto& p:vecPlayer) if(p.id==0){allFilled=false;break;}
    if(!allFilled){
      img.fillRect(0,70,320,28,0x6000); img.drawRect(0,70,320,28,TFT_RED);
      img.setTextFont(1); img.setTextColor(TFT_WHITE);
      img.setCursor(8,78); img.print("Fill all blocks first!");
      cpBurst(160,85); return;
    }
    ccCorrect=0;
    for(auto& pl:vecPlayer) for(auto& go:vecGameOrder)
      if(pl.order==go.order&&pl.id==go.id) ccCorrect++;
    if(ccCorrect<6) cpBurst(160,100);
  }
  if(ccCorrect==6){
    img.fillRect(0,70,320,44,0x0320); img.drawRect(0,70,320,44,0x07E0);
    img.setTextFont(2); img.setTextColor(0x07E0);
    img.setCursor(8,76);
    if(ccMenuIndex==1){
      img.print("Endless Level Clear!");
    } else {
      img.print(idx); img.print(". Level Clear!");
    }
    img.setTextFont(1); img.setTextColor(TFT_WHITE);
    img.setCursor(8,98);
    if(ccMenuIndex==1){
      img.print("Press BACK to play again");
    } else {
      img.print("Press BACK for next level");
    }
    cpBurst(80,50); cpBurst(240,50);
    bool already=false;
    for(int x:clearedLevel) if(x==idx){already=true;break;}
    if(!already) clearedLevel.push_back(idx);
  }
}

// ==================== Navigation ====================
static void ccNavigation(){
  if(ccSelect && ccTab == 0){
    if(ccMenuIndex==1){
      ccShuffleOrder();
      ccResetPlayer();
      ccTab=2;
      ccIsGameStarted=true;
      ccCorrect=0;
    }
    else if(ccMenuIndex==2){
      ccTab=1;
    }
  } else if(ccBack){
    if(ccState == CC_COLOR_MENU){
      vecPlayer[ccChosenBlock] = ccOldBlock;
      ccState = CC_IDLE;
    } else {
      if(ccTab==2){
        if(ccMenuIndex==1){ ccTab=0; }
        else { ccTab=1; }
        ccIsGameStarted=false;
      }
      else if(ccTab==1){
        ccTab=0;
      }
    }
  }
  switch(ccTab){
    case 0: ccDrawMenu(); break;
    case 1: { // Series list
      ccGrad(0x0208,0x0841);
      img.setTextFont(2); img.setTextColor(TFT_YELLOW); img.setCursor(10,4); img.print("SERIES MODE");
      img.drawFastHLine(4,24,312,0x39E7);
      ccHighlightSeries(ccLevel); ccDrawSeriesMenu();
      if(ccDrop&&!ccIsGameStarted){ 
        ccSeriesLevel(ccLevel+1); 
        ccResetPlayer();
        ccTab=2; 
        ccIsGameStarted=true; 
        ccCorrect=0; 
      }
      break;
    }
    case 2: { // Level play
      ccSelectionBlock(ccDrop);
      ccDrawLevelScreen(ccLevel+1);
      ccCheckOrder(ccControl,ccLevel+1);
      break;
    }
    default: break;
  }
}

// ==================== Encoder + Level Navigation ====================
static void ccUpdateEncoder(){
  static bool ccLastA=HIGH, ccLastB=HIGH;
  bool a=digitalRead(48), b=digitalRead(47);
  int delta = 0;
  if(ccLastA==HIGH&&a==LOW){ delta = (b==HIGH)?1:-1; ccEncPos += delta; }
  ccLastA=a; ccLastB=b;

  // UP/DOWN butonlarını kontrol et (UP = sol, DOWN = sağ)
  static bool ccLastUpBtn=HIGH, ccLastDwnBtn=HIGH;
  bool u = digitalRead(up_btn);
  bool d = digitalRead(dwn_btn);
  if(ccLastUpBtn==HIGH && u==LOW){ delta = 1; }
  else if(ccLastDwnBtn==HIGH && d==LOW){ delta = -1; }
  ccLastUpBtn=u; ccLastDwnBtn=d;

  if(delta != 0 || ccEncPos != ccPrevEncPos){
    if(delta == 0) delta = ccEncPos - ccPrevEncPos;
    if(ccTab==0){
      ccMenuIndex=(delta>0)?1:2;
    } else if(ccTab==1){
      if(delta>0){ if(ccLevel>0)ccLevel--; else ccLevel=17; }
      else        { if(ccLevel<17)ccLevel++; else ccLevel=0; }
    } else if(ccTab==2){
      if(ccState==CC_IDLE||ccState==CC_COLOR_MENU){
        if(delta>0){ if(ccTempBlock>0)ccTempBlock--; else ccTempBlock=5; }
        else        { if(ccTempBlock<5)ccTempBlock++; else ccTempBlock=0; }
      }
    }
    ccPrevEncPos=ccEncPos;
  }
}

// ==================== GameHub Arayüzü ====================
void setup(){
  tft.init(); tft.setRotation(1);  // LANDSCAPE
  tft.fillScreen(CC_BACKGROUND);
  img.deleteSprite(); img.createSprite(320,170);
  img.setTextDatum(0);
  vecBlocks.clear(); vecPlayer.clear(); vecGameOrder.clear();
  cpClear(); ccLevel=0; ccTab=0; ccMenuIndex=1; ccCorrect=0;
  ccIsGameStarted=false; ccState=CC_IDLE; ccTempBlock=0;
  ccAddBlock(CC_RED,1); ccAddBlock(CC_YELLOW,2); ccAddBlock(CC_WHITE,3);
  ccAddBlock(CC_BLACK,4); ccAddBlock(CC_LIGHT_BLUE,5); ccAddBlock(CC_GREEN,6);
  img.pushSprite(0,0);
}

void ccUpdate(){
  // Buton okuma
  bool cSel=digitalRead(select_btn), cBck=digitalRead(bck_btn);
  bool cDrp=digitalRead(rgh_btn),    cCtl=digitalRead(lft_btn);
  ccSelect =(ccLastSel==HIGH&&cSel==LOW);
  ccBack   =(ccLastBck==HIGH&&cBck==LOW);
  ccDrop   =(ccLastDrp==HIGH&&cDrp==LOW);
  ccControl=(ccLastCtl==HIGH&&cCtl==LOW);
  ccLastSel=cSel; ccLastBck=cBck; ccLastDrp=cDrp; ccLastCtl=cCtl;

  ccUpdateEncoder();
  img.fillSprite(CC_BACKGROUND);
  ccNavigation();
  cpUpdate(); cpDraw();
  img.pushSprite(0,0);
}

} // namespace ColorCode
