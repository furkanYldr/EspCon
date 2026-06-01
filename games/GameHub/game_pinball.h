// game_pinball.h — GameHub Pinball wrapper
#pragma once
#include <TFT_eSPI.h>
#include <cmath>
#include <vector>

namespace Pinball {
using namespace std;

// ==================== Yapılar ====================
struct Ball    { float x,y,vx,vy,r; };
struct Flipper { float pivotX,pivotY,length,angle,minAngle,maxAngle,speed; bool active; };

// ==================== Değişkenler ====================
static Ball    pball;
static Flipper leftFlipper, rightFlipper;
static int     screenW=0, screenH=0;
static int     playLeft=5, playRight=165, playTop=5, playBottom=315;
static float   drainLeftX=0, drainRightX=0;
static unsigned long lastUpdateMs=0;
static const unsigned long FRAME_TIME_MS=16;
static const float PFRIC=0.985f, PBOUNCE=-0.98f, PGRAV=0.08f;
static int pScore=0;

// Flipper pinleri → select ve back butonları (dikey oyun için)
static const int LEFT_FLIP  = bck_btn;     // 6
static const int RIGHT_FLIP = select_btn;  // 7

// ==================== Yardımcılar ====================
static float pdeg(float d) { return d*(M_PI/180.0f); }

static void resetBallFromTop() {
  pball.r=4;
  int minX=playLeft+pball.r+2, maxX=playRight-pball.r-2;
  pball.x=random(minX,maxX); pball.y=playTop+pball.r+2;
  float rs=(random(-100,101))/100.0f;
  pball.vx=rs*1.5f; pball.vy=1.5f;
}

static void setupFlippers() {
  float w=playRight-playLeft, L=w*0.25f, y=playBottom-25;
  leftFlipper  = {(float)(playLeft+playRight)/2-L*1.1f, y-20.f, L, pdeg(20),pdeg(20),pdeg(-30),pdeg(8),false};
  rightFlipper = {(float)(playLeft+playRight)/2+L*1.1f, y-20.f, L, pdeg(160),pdeg(160),pdeg(210),pdeg(8),false};
  drainLeftX =leftFlipper.pivotX;
  drainRightX=rightFlipper.pivotX;
}

static void flipperEnd(const Flipper& f, float& x1, float& y1) {
  x1=f.pivotX+cosf(f.angle)*f.length;
  y1=f.pivotY+sinf(f.angle)*f.length;
}

static float distPtSeg(float px,float py,float x0,float y0,float x1,float y1,float& cx,float& cy) {
  float dx=x1-x0,dy=y1-y0,len2=dx*dx+dy*dy;
  if (len2<=0.00001f) { cx=x0;cy=y0; return sqrtf((px-x0)*(px-x0)+(py-y0)*(py-y0)); }
  float t=((px-x0)*dx+(py-y0)*dy)/len2;
  if(t<0)t=0; if(t>1)t=1;
  cx=x0+t*dx; cy=y0+t*dy;
  return sqrtf((px-cx)*(px-cx)+(py-cy)*(py-cy));
}

static void handleFlipColl(const Flipper& f) {
  float fx1,fy1; flipperEnd(f,fx1,fy1);
  float cx,cy,dist=distPtSeg(pball.x,pball.y,f.pivotX,f.pivotY,fx1,fy1,cx,cy);
  if (dist<=pball.r+1.0f) {
    float nx=pball.x-cx,ny=pball.y-cy,len=sqrtf(nx*nx+ny*ny);
    if(len<0.0001f){float dx=fx1-f.pivotX,dy=fy1-f.pivotY;len=sqrtf(dx*dx+dy*dy);if(len<0.0001f)return;nx=-dy/len;ny=dx/len;}
    else{nx/=len;ny/=len;}
    float ov=(pball.r+1.0f)-dist;
    pball.x+=nx*ov;pball.y+=ny*ov;
    float dot=pball.vx*nx+pball.vy*ny;
    if(dot>0){pball.vx-=2*dot*nx;pball.vy-=2*dot*ny;}
    if(f.active){pball.vx+=nx*1.5f;pball.vy+=ny*1.5f;}
  }
}

static bool hitCircle(int bx,int by,int cx,int cy,int r){int dx=bx-cx,dy=by-cy;return dx*dx+dy*dy<=r*r;}

// ==================== Giriş & Fizik ====================
static void handleInput() {
  leftFlipper.active  = (digitalRead(LEFT_FLIP)==LOW);
  rightFlipper.active = (digitalRead(RIGHT_FLIP)==LOW);
  if (leftFlipper.active)  { leftFlipper.angle -= leftFlipper.speed;   if(leftFlipper.angle<leftFlipper.maxAngle)   leftFlipper.angle=leftFlipper.maxAngle; }
  else                     { leftFlipper.angle += leftFlipper.speed;   if(leftFlipper.angle>leftFlipper.minAngle)   leftFlipper.angle=leftFlipper.minAngle; }
  if (rightFlipper.active) { rightFlipper.angle+= rightFlipper.speed;  if(rightFlipper.angle>rightFlipper.maxAngle) rightFlipper.angle=rightFlipper.maxAngle;}
  else                     { rightFlipper.angle-= rightFlipper.speed;  if(rightFlipper.angle<rightFlipper.minAngle) rightFlipper.angle=rightFlipper.minAngle;}
}

static void updatePhysics() {
  pball.vy+=PGRAV;
  pball.x+=pball.vx; pball.y+=pball.vy;
  pball.vx*=PFRIC;   pball.vy*=PFRIC;
  if(pball.x-pball.r<playLeft+1) { pball.x=playLeft+1+pball.r; pball.vx*=PBOUNCE; }
  if(pball.x+pball.r>playRight-1){ pball.x=playRight-1-pball.r;pball.vx*=PBOUNCE; }
  if(pball.y-pball.r<playTop+1)  { pball.y=playTop+1+pball.r;  pball.vy*=PBOUNCE; }
  if(pball.y+pball.r>playBottom-1){
    if(pball.x<drainLeftX||pball.x>drainRightX){ pball.y=playBottom-1-pball.r;pball.vy*=PBOUNCE; }
    else { resetBallFromTop(); return; }
  }
  int bx=(int)pball.x,by=(int)pball.y;
  int cx1=playLeft+(playRight-playLeft)/3;
  int cx2=playLeft+2*(playRight-playLeft)/3;
  int cy1=playTop+40,cy2=playTop+80,cy3=playTop+120;
  bool bh=false;
  if(hitCircle(bx,by,cx1,cy1,10)) bh=true;
  if(hitCircle(bx,by,cx2,cy1,10)) bh=true;
  if(hitCircle(bx,by,(playLeft+playRight)/2,cy2,12)) bh=true;
  if(hitCircle(bx,by,cx1,cy3,9)) bh=true;
  if(hitCircle(bx,by,cx2,cy3,9)) bh=true;
  if(bh){ pball.vy=-pball.vy*1.1f;pball.vx=-pball.vx*1.1f;pScore+=5; }
  handleFlipColl(leftFlipper);
  handleFlipColl(rightFlipper);
}

// ==================== Çizim ====================
static void drawPlayfield() {
  img.fillSprite(0x026A);
  img.drawLine(playLeft,playTop,playLeft,playBottom,TFT_WHITE);
  img.drawLine(playRight,playTop,playRight,playBottom,TFT_WHITE);
  img.drawLine(playLeft,playTop,playRight,playTop,TFT_WHITE);
  img.drawLine(playLeft,playBottom,(int)drainLeftX,playBottom,TFT_WHITE);
  img.drawLine((int)drainRightX,playBottom,playRight,playBottom,TFT_WHITE);
  img.drawLine((int)drainLeftX,playBottom,(int)drainLeftX,playBottom-15,TFT_WHITE);
  img.drawLine((int)drainRightX,playBottom,(int)drainRightX,playBottom-15,TFT_WHITE);
  int cx1=playLeft+(playRight-playLeft)/3,cx2=playLeft+2*(playRight-playLeft)/3;
  int cy1=playTop+40,cy2=playTop+80,cy3=playTop+120;
  img.fillCircle(cx1,cy1,6,TFT_BLUE); img.fillCircle(cx2,cy1,6,TFT_BLUE);
  img.fillCircle((playLeft+playRight)/2,cy2,7,TFT_RED);
  img.fillCircle(cx1,cy3,5,TFT_GREEN); img.fillCircle(cx2,cy3,5,TFT_GREEN);
  img.setTextColor(TFT_WHITE,0x026A); img.setTextSize(1);
  img.setCursor(55,2); img.print("PINBALL");
  img.setCursor(110,2); img.setTextColor(TFT_YELLOW,0x026A);
  img.print(pScore);
}

static void drawFlipper(const Flipper& f,uint16_t col) {
  float x1,y1; flipperEnd(f,x1,y1);
  float dx=x1-f.pivotX,dy=y1-f.pivotY,len=sqrtf(dx*dx+dy*dy);
  if(len<=0.0001f)return;
  dx/=len;dy/=len;
  float t=5.0f,nx=-dy,ny=dx;
  float px0=f.pivotX+nx*(t/2),py0=f.pivotY+ny*(t/2);
  float px1=f.pivotX-nx*(t/2),py1=f.pivotY-ny*(t/2);
  float ex0=x1+nx*(t/2),ey0=y1+ny*(t/2);
  float ex1=x1-nx*(t/2),ey1=y1-ny*(t/2);
  img.fillTriangle((int)px0,(int)py0,(int)px1,(int)py1,(int)ex0,(int)ey0,col);
  img.fillTriangle((int)px1,(int)py1,(int)ex0,(int)ey0,(int)ex1,(int)ey1,col);
}

// ==================== GameHub Arayüzü ====================
void pinballSetup() {
  tft.init(); tft.setRotation(4);
  img.setColorDepth(16); img.createSprite(170,320);
  screenW=img.width(); screenH=img.height();
  playLeft=5;playRight=screenW-5;playTop=5;playBottom=screenH-5;
  pinMode(LEFT_FLIP,INPUT_PULLUP);
  pinMode(RIGHT_FLIP,INPUT_PULLUP);
  setupFlippers(); resetBallFromTop(); pScore=0;
  lastUpdateMs=millis();
}

void pinballUpdate() {
  unsigned long now=millis();
  if(now-lastUpdateMs<FRAME_TIME_MS)return;
  lastUpdateMs=now;
  drawPlayfield();
  handleInput(); updatePhysics();
  drawFlipper(leftFlipper,TFT_ORANGE);
  drawFlipper(rightFlipper,TFT_ORANGE);
  img.fillCircle((int)pball.x,(int)pball.y,(int)pball.r,TFT_WHITE);
  img.pushSprite(0,0);
}

void setup()  { pinballSetup();  }
void update() { pinballUpdate(); }

} // namespace Pinball
