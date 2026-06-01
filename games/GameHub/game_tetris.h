// game_tetris.h — Tetris, namespace Tetris içinde, tam inline
#pragma once
#include <TFT_eSPI.h>
#include <vector>
#include <cstring>
#include <cmath>

namespace Tetris {

// ==================== Renkler & Sabitler ====================
#define T_BG   0x0208
#define T_GRID 0x0841
#define T_PNL  0x18C3

static const int T_COLS  = 10;
static const int T_ROWS  = 20;
static const int T_BLK   = 13;
static const int T_OX    = 3;
static const int T_OY    = 20;

static const uint16_t tBlockColor[7] = {
  0x5D32, // I cyan
  0xFECB, // O yellow
  0x712E, // T purple
  0x6C64, // S green
  0xE127, // Z red
  0xCBA4, // L orange
  0x0334  // J blue
};

static const uint8_t tShapes[7][4][4][4] = {
  {{{1,1,1,1},{0,0,0,0},{0,0,0,0},{0,0,0,0}},{{1,0,0,0},{1,0,0,0},{1,0,0,0},{1,0,0,0}},{{1,1,1,1},{0,0,0,0},{0,0,0,0},{0,0,0,0}},{{1,0,0,0},{1,0,0,0},{1,0,0,0},{1,0,0,0}}},
  {{{1,1,0,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}},{{1,1,0,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}},{{1,1,0,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}},{{1,1,0,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}}},
  {{{0,1,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},{{0,1,0,0},{0,1,1,0},{0,1,0,0},{0,0,0,0}},{{0,0,0,0},{1,1,1,0},{0,1,0,0},{0,0,0,0}},{{0,1,0,0},{1,1,0,0},{0,1,0,0},{0,0,0,0}}},
  {{{0,1,1,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}},{{0,1,0,0},{0,1,1,0},{0,0,1,0},{0,0,0,0}},{{0,0,0,0},{0,1,1,0},{1,1,0,0},{0,0,0,0}},{{1,0,0,0},{1,1,0,0},{0,1,0,0},{0,0,0,0}}},
  {{{1,1,0,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},{{0,0,1,0},{0,1,1,0},{0,1,0,0},{0,0,0,0}},{{0,0,0,0},{1,1,0,0},{0,1,1,0},{0,0,0,0}},{{0,1,0,0},{1,1,0,0},{1,0,0,0},{0,0,0,0}}},
  {{{0,0,1,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},{{0,1,0,0},{0,1,0,0},{0,1,1,0},{0,0,0,0}},{{0,0,0,0},{1,1,1,0},{1,0,0,0},{0,0,0,0}},{{1,1,0,0},{0,1,0,0},{0,1,0,0},{0,0,0,0}}},
  {{{1,0,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},{{0,1,1,0},{0,1,0,0},{0,1,0,0},{0,0,0,0}},{{0,0,0,0},{1,1,1,0},{0,0,1,0},{0,0,0,0}},{{0,1,0,0},{0,1,0,0},{1,1,0,0},{0,0,0,0}}}
};

// ==================== Particle ====================
struct TPt { float x,y,vx,vy; uint8_t life,maxLife,sz; uint16_t col; };
static TPt tpt[24];
static void tptClear(){ memset(tpt,0,sizeof(tpt)); }
static void tptSpawn(float ox,float oy,uint16_t col,float vxr=2.0f,float vyb=0.0f,int n=4){
  for(int k=0;k<n;k++) for(int i=0;i<24;i++){
    if(tpt[i].life) continue;
    tpt[i]={ox+(float)(rand()%20-10)/10.f, oy,
             (float)(rand()%200-100)/100.f*vxr,
             (float)(rand()%100-50)/100.f*1.5f+vyb,
             0,0,(uint8_t)(rand()%2+1),col};
    tpt[i].maxLife=(uint8_t)(rand()%20+15);
    tpt[i].life=tpt[i].maxLife; break;
  }
}
static void tptUpdate(){
  for(auto& p:tpt){ if(!p.life)continue; p.x+=p.vx; p.y+=p.vy; p.vy+=0.1f; p.life--; }
}
static void tptDraw(){
  for(auto& p:tpt){
    if(!p.life)continue;
    float a=(float)p.life/p.maxLife;
    uint16_t c=p.col;
    int r=(int)(((c>>11)&0x1F)*a),g=(int)(((c>>5)&0x3F)*a),b=(int)((c&0x1F)*a);
    img.drawPixel((int)p.x,(int)p.y,(uint16_t)((r<<11)|(g<<5)|b));
  }
}

// ==================== Oyun Durumu ====================
static uint16_t tBoard[T_ROWS][T_COLS];
static int tScore=0,tLevel=0,tLines=0;
static bool tGameOver=false;
struct TBlk{ int type,rot,gx,gy; };
static TBlk tCur,tNxt;
static int  tHoldType=-1; static bool tHoldUsed=false;
static int  tDiff=1;
static bool tStarted=false;
static unsigned long tPrevMs=0, tBtnMs=0;
static const unsigned long T_IVLS[]={800,500,250};
static const unsigned long T_BTNRPT=120;

// ==================== Yardımcılar ====================
static uint16_t tLighten(uint16_t c){
  return (uint16_t)((min(31,(int)(((c>>11)&0x1F)+8))<<11)|(min(63,(int)(((c>>5)&0x3F)+12))<<5)|min(31,(int)((c&0x1F)+8)));
}
static uint16_t tDarken(uint16_t c){
  return (uint16_t)((max(0,(int)(((c>>11)&0x1F)-6))<<11)|(max(0,(int)(((c>>5)&0x3F)-8))<<5)|max(0,(int)((c&0x1F)-6)));
}
static bool tFits(int tp,int rot,int gx,int gy){
  for(int y=0;y<4;y++) for(int x=0;x<4;x++){
    if(!tShapes[tp][rot][y][x])continue;
    int nx=gx+x,ny=gy+y;
    if(nx<0||nx>=T_COLS||ny>=T_ROWS)return false;
    if(ny>=0&&tBoard[ny][nx])return false;
  }
  return true;
}
static void tPlace(){
  for(int y=0;y<4;y++) for(int x=0;x<4;x++){
    if(!tShapes[tCur.type][tCur.rot][y][x])continue;
    int nx=tCur.gx+x,ny=tCur.gy+y;
    if(ny>=0&&ny<T_ROWS&&nx>=0&&nx<T_COLS) tBoard[ny][nx]=tBlockColor[tCur.type];
    tptSpawn(T_OX+nx*T_BLK+T_BLK/2,T_OY+ny*T_BLK+T_BLK,tBlockColor[tCur.type],1.0f,-0.3f,1);
  }
}
static int tClearLines(){
  int cl=0;
  for(int r=T_ROWS-1;r>=0;r--){
    bool full=true;
    for(int c=0;c<T_COLS;c++) if(!tBoard[r][c]){full=false;break;}
    if(full){
      for(int c=0;c<T_COLS;c++) tptSpawn(T_OX+c*T_BLK+T_BLK/2,T_OY+r*T_BLK+T_BLK/2,tBoard[r][c],2.5f,-1.0f,2);
      for(int rr=r;rr>0;rr--) for(int c=0;c<T_COLS;c++) tBoard[rr][c]=tBoard[rr-1][c];
      for(int c=0;c<T_COLS;c++) tBoard[0][c]=0;
      cl++; r++;
    }
  }
  return cl;
}
static void tNewBlock(){
  tCur=tNxt; tNxt={(int)(rand()%7),0,3,0};
  if(!tFits(tCur.type,tCur.rot,tCur.gx,tCur.gy)) tGameOver=true;
}
static void tInitGame(){
  memset(tBoard,0,sizeof(tBoard));
  tScore=0;tLevel=0;tLines=0;tGameOver=false;tHoldType=-1;tHoldUsed=false;
  tptClear(); tNxt={(int)(rand()%7),0,3,0}; tNewBlock();
}

// ==================== Çizim ====================
static void tDrawBlk(int gx,int gy,uint16_t col,bool ghost=false){
  int px=T_OX+gx*T_BLK,py=T_OY+gy*T_BLK;
  if(py+T_BLK<=T_OY||py>=T_OY+T_ROWS*T_BLK)return;
  if(ghost){img.drawRect(px,py,T_BLK,T_BLK,col);img.drawRect(px+1,py+1,T_BLK-2,T_BLK-2,tDarken(col));}
  else{
    img.fillRect(px+1,py+1,T_BLK-2,T_BLK-2,col);
    img.drawFastHLine(px+2,py+2,T_BLK-4,tLighten(col));
    img.drawFastVLine(px+2,py+3,T_BLK-5,tLighten(col));
    img.drawRect(px,py,T_BLK,T_BLK,tDarken(col));
  }
}
static void tDrawBoard(){
  img.fillRect(T_OX,T_OY,T_COLS*T_BLK,T_ROWS*T_BLK,T_BG);
  for(int r=0;r<=T_ROWS;r++) img.drawFastHLine(T_OX,T_OY+r*T_BLK,T_COLS*T_BLK,T_GRID);
  for(int c=0;c<=T_COLS;c++) img.drawFastVLine(T_OX+c*T_BLK,T_OY,T_ROWS*T_BLK,T_GRID);
  for(int r=0;r<T_ROWS;r++) for(int c=0;c<T_COLS;c++) if(tBoard[r][c]) tDrawBlk(c,r,tBoard[r][c]);
  // Ghost
  TBlk gh=tCur;
  while(tFits(gh.type,gh.rot,gh.gx,gh.gy+1))gh.gy++;
  if(gh.gy!=tCur.gy) for(int y=0;y<4;y++) for(int x=0;x<4;x++)
    if(tShapes[gh.type][gh.rot][y][x]) tDrawBlk(gh.gx+x,gh.gy+y,tBlockColor[gh.type],true);
  // Aktif
  for(int y=0;y<4;y++) for(int x=0;x<4;x++)
    if(tShapes[tCur.type][tCur.rot][y][x]) tDrawBlk(tCur.gx+x,tCur.gy+y,tBlockColor[tCur.type]);
}
static void tDrawPanel(){
  img.fillRect(133,0,39,320,T_PNL);
  img.drawFastVLine(133,0,320,0x39E7);
  img.setTextFont(2); img.setTextColor(TFT_YELLOW); img.setCursor(136,4); img.print("TET");
  img.setTextColor(0xF81F); img.print("RIS");
  img.setTextSize(1); img.setTextColor(0x8C71);
  img.setCursor(138,26); img.print("HOLD");
  img.fillRoundRect(136,35,33,33,3,0x0841); img.drawRoundRect(135,34,35,35,4,TFT_GOLD);
  if(tHoldType>=0) for(int y=0;y<4;y++) for(int x=0;x<4;x++){
    if(!tShapes[tHoldType][0][y][x])continue;
    img.fillRect(137+x*8,37+y*8,6,6,tBlockColor[tHoldType]);
  }
  img.setCursor(138,74); img.print("NEXT");
  img.fillRoundRect(136,83,33,55,3,0x0841); img.drawRoundRect(135,82,35,57,4,TFT_GOLD);
  for(int y=0;y<4;y++) for(int x=0;x<4;x++){
    if(!tShapes[tNxt.type][tNxt.rot][y][x])continue;
    img.fillRect(137+x*8,85+y*8,6,6,tBlockColor[tNxt.type]);
  }
  img.drawFastHLine(135,144,37,0x39E7);
  img.setTextColor(0x8C71); img.setCursor(136,148); img.print("LINES");
  img.setTextColor(TFT_WHITE); img.setCursor(136,158); img.print(tLines);
  img.setTextColor(0x8C71); img.setCursor(136,174); img.print("LEVEL");
  img.setTextColor(TFT_YELLOW); img.setCursor(136,184); img.print(tLevel);
  img.setTextColor(0x8C71); img.setCursor(136,200); img.print("SCORE");
  img.setTextColor(TFT_WHITE); img.setCursor(136,210); img.print(tScore);
  img.drawFastHLine(135,237,37,0x39E7);
  static const char* DN[]={"EASY","NORM","HARD"};
  static const uint16_t DC[]={0x07E0,0xFFE0,0xF800};
  img.setTextColor(DC[tDiff]); img.setCursor(136,242); img.print(DN[tDiff]);
}
static void tDrawGameOver(){
  img.fillRect(2,90,130,70,0x6000); img.drawRect(2,90,130,70,0xF800);
  img.setTextColor(0xF800); img.setTextFont(4);
  img.setCursor(8,96); img.print("GAME"); img.setCursor(8,120); img.print("OVER");
  img.setTextFont(1); img.setTextColor(TFT_WHITE);
  img.setCursor(8,150); img.print("SELECT=retry");
}

// ==================== Zorluk Seçimi ====================
static bool tLastSelD=HIGH,tLastUD=HIGH,tLastDD=HIGH;
static void tDrawDiff(){
  img.fillSprite(T_BG);
  img.setTextColor(TFT_YELLOW); img.setTextFont(4);
  img.setCursor(15,12); img.print("TETRIS");
  img.drawFastHLine(4,46,164,TFT_YELLOW);
  static const char* DN[]={"EASY","NORMAL","HARD"};
  static const uint16_t DC[]={0x07E0,0xFFE0,0xF800};
  static const char* DS[]={"800ms drop","500ms drop","250ms drop"};
  for(int i=0;i<3;i++){
    int y=56+i*80; bool sel=(i==tDiff);
    img.fillRoundRect(8,y,156,68,6,sel?DC[i]:0x18C3);
    img.drawRoundRect(7,y-1,158,70,7,sel?TFT_WHITE:0x39E7);
    img.setTextColor(sel?T_BG:TFT_WHITE); img.setTextFont(4);
    img.setCursor(18,y+8); img.print(DN[i]);
    img.setTextFont(1); img.setTextColor(sel?0x18C3:0x8C71);
    img.setCursor(18,y+36); img.print(DS[i]);
    for(int d=0;d<3;d++){ img.fillRect(18+d*30,y+50,24,10,(d<=i)?DC[i]:0x2945); }
  }
  img.setTextColor(0x39E7); img.setTextFont(1);
  img.setCursor(10,308); img.print("UP/DWN=select  SELECT=start");
  img.pushSprite(0,0);
  int bU=digitalRead(up_btn),bD=digitalRead(dwn_btn),bS=digitalRead(select_btn);
  if(bU==LOW&&tLastUD==HIGH) tDiff=max(0,tDiff-1);
  if(bD==LOW&&tLastDD==HIGH) tDiff=min(2,tDiff+1);
  if(bS==LOW&&tLastSelD==HIGH){ tInitGame(); tStarted=true; }
  tLastUD=bU; tLastDD=bD; tLastSelD=bS;
}

// ==================== Input ====================
static int tLastL=HIGH,tLastR=HIGH,tLastD_=HIGH,tLastU_=HIGH,tLastS=HIGH,tLastB=HIGH;
static void tHandleInput(){
  unsigned long now=millis();
  int bL=digitalRead(lft_btn),bR=digitalRead(rgh_btn),bD=digitalRead(dwn_btn);
  int bU=digitalRead(up_btn),bS=digitalRead(select_btn),bB=digitalRead(bck_btn);
  
  if(bL==LOW){if(tLastL==HIGH||now-tBtnMs>T_BTNRPT){if(tFits(tCur.type,tCur.rot,tCur.gx-1,tCur.gy))tCur.gx--;tBtnMs=now;}}
  if(bR==LOW){if(tLastR==HIGH||now-tBtnMs>T_BTNRPT){if(tFits(tCur.type,tCur.rot,tCur.gx+1,tCur.gy))tCur.gx++;tBtnMs=now;}}
  
  // DOWN Button (bD) -> Hard Drop (direkt indir)
  if(bD==LOW&&tLastD_==HIGH){while(tFits(tCur.type,tCur.rot,tCur.gx,tCur.gy+1)){tCur.gy++;tScore+=2;}}
  
  // UP Button (bU) -> Rotate (şekli döndür)
  if(bU==LOW&&tLastU_==HIGH){int nr=(tCur.rot+1)%4;if(tFits(tCur.type,nr,tCur.gx,tCur.gy))tCur.rot=nr;}
  
  // SELECT Button (bS) -> Soft Drop (yavaşça indir)
  if(bS==LOW){if(tLastS==HIGH||now-tBtnMs>50){if(tFits(tCur.type,tCur.rot,tCur.gx,tCur.gy+1)){tCur.gy++;tScore++;}tBtnMs=now;}}
  
  if(bB==LOW&&tLastB==HIGH&&!tHoldUsed){
    if(tHoldType<0){tHoldType=tCur.type;tNewBlock();}
    else{int t=tHoldType;tHoldType=tCur.type;tCur={t,0,3,0};}
    tHoldUsed=true;
  }
  tLastL=bL;tLastR=bR;tLastD_=bD;tLastU_=bU;tLastS=bS;tLastB=bB;
}

// ==================== GameHub Arayüzü ====================
void setup(){
  tft.init(); tft.setRotation(4); tft.fillScreen(T_BG);
  img.deleteSprite(); img.createSprite(172,320);
  tStarted=false; tDiff=1;
  img.pushSprite(0,0);
}

void tetUpdate(){
  if(!tStarted){tDrawDiff();return;}
  if(tGameOver){
    if(rand()%3==0) tptSpawn((float)(rand()%130+3),0.0f,0xF800,0.5f,1.5f,1);
    tptUpdate();
    img.fillSprite(T_BG); tDrawBoard(); tDrawPanel(); tptDraw(); tDrawGameOver();
    img.pushSprite(0,0);
    if(digitalRead(select_btn)==LOW){delay(150);tInitGame();}
    return;
  }
  tHandleInput();
  unsigned long now=millis();
  if(now-tPrevMs>=T_IVLS[tDiff]){
    tPrevMs=now;
    if(tFits(tCur.type,tCur.rot,tCur.gx,tCur.gy+1)){tCur.gy++;}
    else{
      tPlace();
      int cl=tClearLines();
      if(cl>0){
        static const int pts[]={0,100,300,500,800};
        tScore+=pts[min(cl,4)]*(tLevel+1); tLines+=cl; tLevel=tLines/10;
        if(cl>=4){tptSpawn(65,160,TFT_YELLOW,3.0f,-2.0f,8);tptSpawn(65,160,TFT_WHITE,2.0f,-1.5f,6);}
      }
      tHoldUsed=false; tNewBlock();
    }
  }
  tptUpdate();
  img.fillSprite(T_BG); tDrawBoard(); tDrawPanel(); tptDraw();
  img.pushSprite(0,0);
}

} // namespace Tetris
