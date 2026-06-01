// colorCode.cpp — Particle efektli, süslenmiş ColorCode
#include "esp32-hal-gpio.h"
#include <TFT_eSPI.h>
#include <RotaryEncoder.h>
#include "colorCode.h"
#include <Arduino.h>
#include "resource.h"
#include <vector>
#include "GameEngine.h"
using namespace std;

#define PIN_IN1 48
#define PIN_IN2 47
RotaryEncoder encoder(PIN_IN1, PIN_IN2, RotaryEncoder::LatchMode::TWO03);

#ifndef select_btn
#define up_btn     4
#define dwn_btn    3
#define lft_btn    5
#define rgh_btn    1
#define select_btn 7
#define bck_btn    6
#endif

TFT_eSPI    tft = TFT_eSPI();
TFT_eSprite img = TFT_eSprite(&tft);

// ==================== Particle (landscape 320x170) ====================
struct CPart { float x,y,vx,vy; uint8_t life,maxLife,sz; uint16_t col; };
static CPart cp[24];
static void cpClear(){ memset(cp,0,sizeof(cp)); }

static void cpSpawn(float ox,float oy,uint16_t col,float vxr=2.0f,float vyb=0.0f,int n=4){
  for(int k=0;k<n;k++) for(int i=0;i<24;i++){
    if(cp[i].life>0) continue;
    cp[i].x=ox+(float)(rand()%20-10)/10.0f;
    cp[i].y=oy+(float)(rand()%10-5)/10.0f;
    cp[i].vx=(float)(rand()%200-100)/100.0f*vxr;
    cp[i].vy=(float)(rand()%200-100)/100.0f*1.5f+vyb;
    cp[i].sz=(uint8_t)(rand()%2+1);
    cp[i].maxLife=(uint8_t)(rand()%20+15);
    cp[i].life=cp[i].maxLife;
    cp[i].col=col; break;
  }
}

static void cpBurst(float cx,float cy){
  static const uint16_t RCOLS[]={RED,YELLOW,WHITE,LIGHT_BLUE,GREEN,PINK,ORANGE};
  for(int k=0;k<12;k++) for(int i=0;i<24;i++){
    if(cp[i].life>0) continue;
    float ang=(float)(rand()%360)*3.14159f/180.0f;
    float spd=(float)(rand()%20+10)/10.0f;
    cp[i].x=cx; cp[i].y=cy;
    cp[i].vx=cosf(ang)*spd; cp[i].vy=sinf(ang)*spd;
    cp[i].sz=(uint8_t)(rand()%3+1);
    cp[i].maxLife=(uint8_t)(rand()%20+20);
    cp[i].life=cp[i].maxLife;
    cp[i].col=RCOLS[rand()%7]; break;
  }
}

static void cpUpdate(){
  for(int i=0;i<24;i++){
    if(!cp[i].life) continue;
    cp[i].x+=cp[i].vx; cp[i].y+=cp[i].vy;
    cp[i].vy+=0.06f; cp[i].life--;
    if(cp[i].x<0||cp[i].x>320||cp[i].y<0||cp[i].y>170) cp[i].life=0;
  }
}

static void cpDraw(){
  for(int i=0;i<24;i++){
    if(!cp[i].life) continue;
    float a=(float)cp[i].life/cp[i].maxLife;
    uint16_t c=cp[i].col;
    int r=(int)(((c>>11)&0x1F)*a), g2=(int)(((c>>5)&0x3F)*a), b=(int)((c&0x1F)*a);
    c=(uint16_t)((r<<11)|(g2<<5)|b);
    if(cp[i].sz<=1) img.drawPixel((int)cp[i].x,(int)cp[i].y,c);
    else            img.fillCircle((int)cp[i].x,(int)cp[i].y,cp[i].sz-1,c);
  }
}

// ==================== Gradient arka plan ====================
static void drawGrad(uint16_t top,uint16_t bot,int h=170){
  int tr=(top>>11)&0x1F,tg=(top>>5)&0x3F,tb=top&0x1F;
  int br=(bot>>11)&0x1F,bg=(bot>>5)&0x3F,bb=bot&0x1F;
  for(int y=0;y<h;y++){
    float t=(float)y/(h-1);
    int r=(int)(tr+(br-tr)*t),g=(int)(tg+(bg-tg)*t),b=(int)(tb+(bb-tb)*t);
    img.drawFastHLine(0,y,320,(uint16_t)((r<<11)|(g<<5)|b));
  }
}

// ==================== Oyun Değişkenleri ====================
int level=0, logicManager=0;
int newEncoderPos, prevEncoderPos=0;
int menuIndex=0, tab=0;
bool selectState,backState,dropDown,control;
int tempBlock=0;
bool endlessGameStart=false, isGameStarted=false;
bool lastSelectState=HIGH,lastBackState=HIGH,lastDropDown=HIGH,lastControl=HIGH;

static bool levelCleared=false;       // level complete flag — confetti tetikler
static bool wrongAnswer=false;        // yanlış yanıt — kırmızı patlama

// ==================== Menu ====================
void Menu(){
  drawGrad(0x0208, 0x1043);

  // Başlık
  img.setTextFont(4);
  img.setTextDatum(MC_DATUM);
  img.setTextColor(TFT_YELLOW);
  img.drawString("COLOR", 100, 28);
  img.setTextColor(TFT_WHITE);
  img.drawString("CODE",  220, 28);
  img.drawFastHLine(30,52,260,0xFFE0);
  img.setTextDatum(0);

  if(newEncoderPos!=prevEncoderPos){
    menuIndex=(prevEncoderPos<newEncoderPos)?1:2;
    prevEncoderPos=newEncoderPos;
  }
  if(menuIndex==0) menuIndex=1;

  // Endless butonu
  bool e_sel=(menuIndex==1);
  img.fillRoundRect(28,70,124,60,8,e_sel?YELLOW:0x18C3);
  if(e_sel) img.drawRoundRect(27,69,126,62,9,TFT_WHITE);
  img.setTextColor(e_sel?BLACK:WHITE);
  img.setTextFont(2); img.setCursor(42,86); img.print("ENDLESS");
  img.setTextFont(1); img.setTextColor(e_sel?0x18C3:0x8C71);
  img.setCursor(44,106); img.print("Free play");

  // Series butonu
  bool s_sel=(menuIndex==2);
  img.fillRoundRect(168,70,124,60,8,s_sel?LIGHT_BLUE:0x18C3);
  if(s_sel) img.drawRoundRect(167,69,126,62,9,TFT_WHITE);
  img.setTextColor(s_sel?BLACK:WHITE);
  img.setTextFont(2); img.setCursor(186,86); img.print("SERIES");
  img.setTextFont(1); img.setTextColor(s_sel?0x0208:0x8C71);
  img.setCursor(188,106); img.print("18 levels");

  // Alt bilgi
  img.setTextFont(1); img.setTextColor(0x39E7);
  img.setCursor(90,145); img.print("ENC=switch  SELECT=choose");

  // Ambient partiküller
  if(rand()%4==0) cpSpawn((float)(rand()%300+10),(float)(rand()%30+130),0x07FF,0.3f,-0.5f,1);
}

// ==================== Navigation ====================
void navigation(){
  if(selectState){
    if(menuIndex==1){ tab=3; }
    else if(menuIndex==2){ if(!isGameStarted) tab=1; }
  } else if(backState){
    if(tab==3)tab=0;
    else if(tab==0)tab=20;
    else tab--;
    if(isGameStarted) isGameStarted=false;
  }
  switch(tab){
    case 0: Menu(); break;
    case 1: Series(); break;
    case 2: levelScreen(level+1); break;
    case 3: break;
    default: break;
  }
}

// ==================== Level Ekranı ====================
void levelScreen(int index){
  drawGrad(0x0208,0x0841);

  // Level numarası
  img.setTextFont(4); img.setTextColor(TFT_YELLOW);
  img.setCursor(4,4); img.print("LVL ");
  img.setTextColor(TFT_WHITE); img.print(index);
  img.drawFastHLine(4,36,312,0x39E7);

  img.setTextDatum(0);
  img.setCursor(290,4); img.setTextFont(1); img.setTextColor(0x8C71); img.print("#");

  selectionBlock(tempBlock,selectState,dropDown);
  playerBlocks(6);

  if(newEncoderPos!=prevEncoderPos){
    if(prevEncoderPos<newEncoderPos){ if(tempBlock<=5&&tempBlock>0) tempBlock--; else tempBlock=5; }
    else                            { if(tempBlock<5&&tempBlock>=0) tempBlock++; else tempBlock=0; }
    prevEncoderPos=newEncoderPos;
  }
  correctOrderCount(control,index);
}

// ==================== Series Ekranı ====================
void Series(){
  drawGrad(0x0208,0x0841);

  img.setTextFont(2); img.setTextColor(TFT_YELLOW);
  img.setCursor(10,4); img.print("SERIES MODE");
  img.drawFastHLine(4,24,312,0x39E7);

  if(newEncoderPos!=prevEncoderPos){
    if(prevEncoderPos<newEncoderPos){ if(level<=17&&level>0) level--; else level=17; }
    else                            { if(level<17&&level>=0) level++; else level=0; }
    prevEncoderPos=newEncoderPos;
  }
  seriesMenuHighLight(level);
  seriesMenu();

  if(dropDown){
    if(!isGameStarted){
      seriesLevel(level+1);
      tab=2; isGameStarted=true;
    }
  }
}

// ==================== Update ====================
void update(){
  encoder.tick();
  newEncoderPos=encoder.getPosition();

  bool cSel=digitalRead(select_btn);
  bool cBck=digitalRead(bck_btn);
  bool cDrp=digitalRead(rgh_btn);
  bool cCtl=digitalRead(lft_btn);

  selectState=(lastSelectState==HIGH&&cSel==LOW);
  backState  =(lastBackState  ==HIGH&&cBck==LOW);
  dropDown   =(lastDropDown   ==HIGH&&cDrp==LOW);
  control    =(lastControl    ==HIGH&&cCtl==LOW);

  lastSelectState=cSel; lastBackState=cBck;
  lastDropDown=cDrp;    lastControl=cCtl;

  render();
}

// ==================== Render ====================
void render(){
  navigation();
  cpUpdate();
  cpDraw();
  img.pushSprite(0,0);
}