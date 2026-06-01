// game_8pool.h — GameHub 8 Ball Pool wrapper
// Tüm kod 8pool.ino'dan namespace Pool içine alındı
#pragma once
#include <TFT_eSPI.h>
#include <cmath>
#include <vector>
#include <RotaryEncoder.h>

// Global encoder pointer — poolSetup'ta atanır
static RotaryEncoder* pEncPtr = nullptr;

namespace Pool {
using namespace std;

// ==================== Encoder (GameHub ile paylaşım) ====================
// Encoder nesnesi GameHub.ino'da tanımlı — pool kendi local encoder state'ini tutar
static int  p_prevEncoderPos = 0;
static int  p_newEncoderPos  = 0;
static float aimAngle        = (float)M_PI;

// ==================== Masa Sabitleri ====================
static const int TABLE_L=10, TABLE_R=162, TABLE_T=10, TABLE_B=310, POCKET_R=7;

// ==================== Renkler ====================
#define P_TABLE    0x0340
#define P_FELT     0x0540
#define P_RAIL     0x1A00
#define P_RAIL2    0xA540
#define P_CUE      0xFDA0
#define P_POCKET   0x0000
#define P_POWER_BG 0x18C3
#define P_DOTLINE  0x8C71

// Top renkleri
#define PB1  0xFFE0
#define PB2  0x001F
#define PB3  0xF800
#define PB4  0x780F
#define PB5  0xFC60
#define PB6  0x03E0
#define PB7  0x8000
#define PB8  0x0000

// ==================== Yapılar ====================
struct PBall { float px,py,ox,oy,vx,vy,ax,ay,radius,mass; int id; bool pocketed,isCue,isStripe; uint16_t bColor; };
struct PLine { float sx,sy,ex,ey,radius; };
struct PPocket { float px,py,pradius; };

// ==================== Vektörler ====================
static vector<PBall>               vecBalls;
static vector<PLine>               vecLines;
static vector<PPocket>             vecPockets;
static vector<PBall*>              vecFake;
static vector<pair<PBall*,PBall*>> vecPairs;

// ==================== Oyun Durumu ====================
enum PoolState { P_AIMING, P_ROLLING, P_GAMEOVER };
static PoolState poolState = P_AIMING;

// Güç
static bool          pBtnHeld    = false;
static unsigned long pHoldStart  = 0;
static float         pShotPower  = 0.0f;
static const float   P_MAX_POWER = 1300.0f;
static const float   P_HOLD_TIME = 1800.0f;

static int pScore=0;
static unsigned long pPrevMillis=0;
static const unsigned long P_FRAME=30;
static int pLastJump=HIGH, pLastSnap=HIGH;
static const float P_STABLE = 1.8f;

// ==================== Yardımcılar ====================
static bool pAllStopped() {
  for (const auto& b:vecBalls) { if(b.pocketed)continue; if(b.vx*b.vx+b.vy*b.vy>0.08f)return false; }
  return true;
}
static bool pAllColorPocketed() {
  for (const auto& b:vecBalls) if(!b.isCue&&!b.pocketed)return false;
  return true;
}
static PBall* getCue() {
  for (auto& b:vecBalls) if(b.isCue&&!b.pocketed)return &b;
  return nullptr;
}

// ==================== Top Ekle ====================
static void addPBall(float x,float y,float r,uint16_t col,bool cue=false,bool stripe=false) {
  PBall b; b.px=x;b.py=y;b.vx=0;b.vy=0;b.ax=0;b.ay=0;
  b.radius=r;b.mass=r*10;b.bColor=col;b.id=vecBalls.size();
  b.pocketed=false;b.isCue=cue;b.isStripe=stripe;
  vecBalls.push_back(b);
}
static void addPLine(float x1,float y1,float x2,float y2) {
  PLine l; l.sx=x1;l.sy=y1;l.ex=x2;l.ey=y2;l.radius=0;
  vecLines.push_back(l);
}
static void addPocket(float x,float y) {
  PPocket p; p.px=x;p.py=y;p.pradius=POCKET_R;
  vecPockets.push_back(p);
}

// ==================== Masa Kurulumu ====================
static void resetPool() {
  vecBalls.clear(); vecLines.clear(); vecPockets.clear();
  pScore=0; poolState=P_AIMING; aimAngle=(float)M_PI;
  addPBall(86,238,5,TFT_WHITE,true,false);

  const float bx=86,by=152,rs=11.6f;
  struct BD{uint16_t c;bool s;};
  const BD rack[15]={
    {PB1,false},{PB7,false},{PB4,true},{0xF800,true},{PB8,false},
    {PB6,false},{PB4,false},{PB3,false},{PB2,true},{0x03E0,true},
    {PB7,true},{PB2,false},{PB5,true},{PB5,false},{PB3,true}
  };
  int k=0;
  for(int row=0;row<5;row++) for(int col=0;col<=row;col++) {
    float x=bx+(col-row*0.5f)*rs, y=by-row*rs*0.866f;
    addPBall(x,y,5,rack[k].c,false,rack[k].s); k++;
  }
  int mid=(TABLE_T+TABLE_B)/2;
  addPLine(TABLE_L,TABLE_T+POCKET_R+2,TABLE_L,mid-POCKET_R-2);
  addPLine(TABLE_L,mid+POCKET_R+2,TABLE_L,TABLE_B-POCKET_R-2);
  addPLine(TABLE_R,TABLE_T+POCKET_R+2,TABLE_R,mid-POCKET_R-2);
  addPLine(TABLE_R,mid+POCKET_R+2,TABLE_R,TABLE_B-POCKET_R-2);
  addPLine(TABLE_L+POCKET_R+2,TABLE_T,TABLE_R-POCKET_R-2,TABLE_T);
  addPLine(TABLE_L+POCKET_R+2,TABLE_B,TABLE_R-POCKET_R-2,TABLE_B);
  addPocket(TABLE_L,TABLE_T); addPocket(TABLE_R,TABLE_T);
  addPocket(TABLE_L,mid);     addPocket(TABLE_R,mid);
  addPocket(TABLE_L,TABLE_B); addPocket(TABLE_R,TABLE_B);
}

// ==================== Fizik ====================
static void pCollision() {
  auto overlap=[](float x1,float y1,float r1,float x2,float y2,float r2){
    float dx=x1-x2,dy=y1-y2;return dx*dx+dy*dy<=(r1+r2)*(r1+r2);};
  for(auto& b:vecBalls){
    if(b.pocketed)continue;
    b.ox=b.px;b.oy=b.py;
    b.ax=-b.vx*0.6f;b.ay=-b.vy*0.6f;
    b.vx+=b.ax*0.01f;b.vy+=b.ay*0.01f;
    b.px+=b.vx*0.01f;b.py+=b.vy*0.01f;
    if(fabs(b.vx)<P_STABLE)b.vx=0;
    if(fabs(b.vy)<P_STABLE)b.vy=0;
  }
  for(auto& b:vecBalls){
    if(b.pocketed)continue;
    for(auto& e:vecLines){
      float lx1=e.ex-e.sx,ly1=e.ey-e.sy,lx2=b.px-e.sx,ly2=b.py-e.sy;
      float el=lx1*lx1+ly1*ly1,t=max(0.f,min(el,lx1*lx2+ly1*ly2))/el;
      float cx=e.sx+t*lx1,cy=e.sy+t*ly1,dx=b.px-cx,dy=b.py-cy;
      float dist=sqrtf(dx*dx+dy*dy);
      if(dist<=b.radius+e.radius){
        PBall* fb=new PBall();fb->radius=e.radius;fb->mass=b.mass*0.8f;fb->px=cx;fb->py=cy;fb->vx=-b.vx;fb->vy=-b.vy;
        vecFake.push_back(fb);vecPairs.push_back({&b,fb});
        float ov=1.0f*(dist-b.radius-fb->radius);b.px-=ov*(b.px-fb->px)/dist;b.py-=ov*(b.py-fb->py)/dist;
      }
    }
    for(auto& t:vecBalls){
      if(b.id==t.id||t.pocketed)continue;
      if(overlap(b.px,b.py,b.radius,t.px,t.py,t.radius)){
        vecPairs.push_back({&b,&t});
        float dx=b.px-t.px,dy=b.py-t.py,dist=sqrtf(dx*dx+dy*dy);
        if(dist<0.001f)dist=0.001f;
        float ov=(dist-b.radius-t.radius)*0.5f;
        b.px-=ov*dx/dist;b.py-=ov*dy/dist;t.px+=ov*dx/dist;t.py+=ov*dy/dist;
      }
    }
  }
  for(auto c:vecPairs){
    PBall*b1=c.first,*b2=c.second;
    float dist=sqrtf(pow(b1->px-b2->px,2)+pow(b1->py-b2->py,2));
    if(dist<0.001f)dist=0.001f;
    float nx=(b2->px-b1->px)/dist,ny=(b2->py-b1->py)/dist,tx=-ny,ty=nx;
    float dt1=b1->vx*tx+b1->vy*ty,dt2=b2->vx*tx+b2->vy*ty;
    float dn1=b1->vx*nx+b1->vy*ny,dn2=b2->vx*nx+b2->vy*ny;
    float m1=(dn1*(b1->mass-b2->mass)+2*b2->mass*dn2)/(b1->mass+b2->mass);
    float m2=(dn2*(b2->mass-b1->mass)+2*b1->mass*dn1)/(b1->mass+b2->mass);
    b1->vx=tx*dt1+nx*m1;b1->vy=ty*dt1+ny*m1;b2->vx=tx*dt2+nx*m2;b2->vy=ty*dt2+ny*m2;
  }
  vecPairs.clear();for(auto f:vecFake)delete f;vecFake.clear();
}

static void pCheckPockets(){
  for(auto& b:vecBalls){
    if(b.pocketed)continue;
    for(const auto& p:vecPockets){
      float dx=b.px-p.px,dy=b.py-p.py,thr=p.pradius+b.radius-2;
      if(dx*dx+dy*dy<=thr*thr){
        if(b.isCue){b.px=86;b.py=238;b.vx=0;b.vy=0;}
        else{b.pocketed=true;b.vx=0;b.vy=0;pScore+=10;}
        break;
      }
    }
  }
}

// ==================== Çizim ====================
static void pDrawBall(const PBall& b){
  int cx=(int)b.px,cy=(int)b.py,r=(int)b.radius;
  if(b.isCue){img.fillCircle(cx,cy,r,TFT_WHITE);img.drawCircle(cx,cy,r,0xC618);return;}
  if(!b.isStripe){
    img.fillCircle(cx,cy,r,b.bColor);
    if(r>=4)img.fillCircle(cx-1,cy-1,1,0xFFFF);
    img.drawCircle(cx,cy,r,0x4208);
  } else {
    img.fillCircle(cx,cy,r,TFT_WHITE);
    img.fillCircle(cx,cy,max(2,(int)(r*0.55f)),b.bColor);
    img.fillCircle(cx-1,cy-1,1,TFT_WHITE);
    img.drawCircle(cx,cy,r,0xC618);
  }
}

static void pDrawTable(){
  img.fillSprite(P_RAIL);
  img.fillRect(TABLE_L-3,TABLE_T-3,TABLE_R-TABLE_L+6,TABLE_B-TABLE_T+6,P_RAIL2);
  img.fillRect(TABLE_L,TABLE_T,TABLE_R-TABLE_L,TABLE_B-TABLE_T,P_TABLE);
  img.drawRect(TABLE_L,TABLE_T,TABLE_R-TABLE_L,TABLE_B-TABLE_T,P_FELT);
  int mid=(TABLE_T+TABLE_B)/2;
  img.fillCircle(86,mid,2,0x3186);img.fillCircle(86,TABLE_T+(TABLE_B-TABLE_T)/4,2,0x3186);
  for(const auto& p:vecPockets){
    img.fillCircle((int)p.px,(int)p.py,(int)p.pradius+3,0x1082);
    img.fillCircle((int)p.px,(int)p.py,(int)p.pradius+1,0x0861);
    img.fillCircle((int)p.px,(int)p.py,(int)p.pradius,P_POCKET);
  }
}

static void pDrawAimLine(const PBall& c){
  float dx=cosf(aimAngle),dy=sinf(aimAngle);
  for(int s=9;s<85;s+=9){
    float lx=c.px+dx*s,ly=c.py+dy*s;
    if(lx>TABLE_L+1&&lx<TABLE_R-1&&ly>TABLE_T+1&&ly<TABLE_B-1)
      img.fillCircle((int)lx,(int)ly,1,P_DOTLINE);
  }
}

static void pDrawCue(const PBall& c,float pw){
  float dx=cosf(aimAngle),dy=sinf(aimAngle),pull=6+pw*20;
  float sx=c.px-dx*(c.radius+pull),sy=c.py-dy*(c.radius+pull);
  float ex=c.px-dx*(c.radius+pull+50),ey=c.py-dy*(c.radius+pull+50);
  for(int t=-2;t<=2;t++){float ox=-dy*t,oy=dx*t;img.drawLine((int)(sx+ox),(int)(sy+oy),(int)(ex+ox),(int)(ey+oy),P_CUE);}
  img.fillCircle((int)sx,(int)sy,2,TFT_WHITE);img.fillCircle((int)ex,(int)ey,3,0x6B4D);
}

static void pDrawPower(float pw){
  int bx=2,by=255,bw=6,bh=45;img.fillRect(bx,by,bw,bh,P_POWER_BG);
  int f=(int)(pw*bh);
  if(f>0){uint16_t c=pw<0.4f?0x07E0:pw<0.75f?0xFFE0:0xF800;img.fillRect(bx,by+bh-f,bw,f,c);}
  img.drawRect(bx,by,bw,bh,TFT_WHITE);
  img.setTextColor(TFT_WHITE);img.setTextSize(1);img.setCursor(1,248);img.print("P");
}

static void pDrawUI(){
  img.fillRect(0,0,172,9,0x1082);
  img.setTextColor(TFT_YELLOW);img.setTextSize(1);img.setCursor(3,1);img.print("8POOL");
  img.setTextColor(TFT_WHITE);img.setCursor(55,1);img.print("S:");img.print(pScore);
  int rem=0;for(const auto& b:vecBalls)if(!b.isCue&&!b.pocketed)rem++;
  img.setTextColor(TFT_CYAN);img.setCursor(130,1);img.print("x");img.print(rem);
}

static void pGameOver(){
  img.fillSprite(P_RAIL);img.fillRect(10,30,152,260,0x0861);img.drawRect(10,30,152,260,P_RAIL2);
  img.setTextColor(TFT_YELLOW);img.setTextSize(2);img.setCursor(22,55);img.print("YOU WIN!");
  img.setTextColor(TFT_WHITE);img.setTextSize(1);img.setCursor(42,100);img.print("Score:");
  img.setTextColor(TFT_YELLOW);img.setTextSize(3);img.setCursor(52,118);img.print(pScore);
  img.fillRoundRect(36,200,100,30,8,TFT_YELLOW);img.fillRoundRect(38,202,96,26,6,0x0861);
  img.setCursor(45,210);img.setTextSize(1);img.setTextColor(TFT_WHITE);img.print("  PLAY AGAIN");
  img.pushSprite(0,0);
}

// ==================== GameHub Arayüzü ====================
void poolSetup(RotaryEncoder* encPtr = nullptr) {
  pEncPtr = encPtr;
  tft.init();tft.setRotation(4);tft.setSwapBytes(true);tft.fillScreen(TFT_BLACK);
  img.createSprite(172,320);img.setTextDatum(0);
  p_prevEncoderPos=0;p_newEncoderPos=0;
  pLastJump=HIGH;pLastSnap=HIGH;
  resetPool();img.pushSprite(0,0);
}

void poolUpdate() {
  unsigned long now=millis();
  if(now-pPrevMillis<P_FRAME)return;
  pPrevMillis=now;

  if(poolState==P_GAMEOVER){
    pGameOver();
    int j=digitalRead(R_SHOULDER);
    if(j==LOW&&pLastJump==HIGH){vecLines.clear();vecPockets.clear();resetPool();}
    pLastJump=j;return;
  }

  // Encoder
  if(pEncPtr){
    pEncPtr->tick();
    p_newEncoderPos=pEncPtr->getPosition();
    if(p_newEncoderPos!=p_prevEncoderPos){
      int delta=p_newEncoderPos-p_prevEncoderPos;
      aimAngle+=delta*(3.0f*(float)M_PI/180.0f);
      while(aimAngle>2*(float)M_PI)aimAngle-=2*(float)M_PI;
      while(aimAngle<0)aimAngle+=2*(float)M_PI;
      p_prevEncoderPos=p_newEncoderPos;
    }
  }

  // Snap (sol shoulder)
  int snapNow=digitalRead(L_SHOULDER);
  if(snapNow==LOW&&pLastSnap==HIGH&&poolState==P_AIMING){
    aimAngle+=15.0f*(float)M_PI/180.0f;
    if(aimAngle>2*(float)M_PI)aimAngle-=2*(float)M_PI;
  }
  pLastSnap=snapNow;

  // Jump (sağ shoulder)
  int jumpNow=digitalRead(R_SHOULDER);
  bool justPressed=(jumpNow==LOW&&pLastJump==HIGH);
  bool justReleased=(jumpNow==HIGH&&pLastJump==LOW);
  pLastJump=jumpNow;

  if(poolState==P_AIMING){
    if(justPressed){pBtnHeld=true;pHoldStart=now;pShotPower=0;}
    if(pBtnHeld&&jumpNow==LOW) pShotPower=min((now-pHoldStart)/P_HOLD_TIME,1.0f);
    if(justReleased&&pBtnHeld){
      pBtnHeld=false;
      PBall* cue=getCue();
      if(cue&&pShotPower>0.02f){cue->vx=cosf(aimAngle)*pShotPower*P_MAX_POWER;cue->vy=sinf(aimAngle)*pShotPower*P_MAX_POWER;poolState=P_ROLLING;}
      pShotPower=0;
    }
  }
  if(poolState==P_ROLLING){
    pCollision();pCheckPockets();
    if(pAllStopped()){poolState=pAllColorPocketed()?P_GAMEOVER:P_AIMING;}
  }

  pDrawTable();
  for(const auto& b:vecBalls) if(!b.pocketed)pDrawBall(b);
  pDrawUI();
  if(poolState==P_AIMING){PBall* c=getCue();if(c){pDrawAimLine(*c);pDrawCue(*c,pShotPower);}pDrawPower(pShotPower);}
  if(poolState==P_ROLLING){img.setTextColor(TFT_CYAN);img.setTextSize(1);img.setCursor(62,305);img.print("rolling..");}
  img.pushSprite(0,0);
}

void setup()  { poolSetup();  }
void update() { poolUpdate(); }

} // namespace Pool
