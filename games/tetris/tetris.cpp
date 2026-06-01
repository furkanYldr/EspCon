// tetris.cpp — Particle efektli, zorluk seçimli Tetris
#include "tetris.h"
#include "esp32-hal-gpio.h"
#include <cmath>
#include <vector>
#include "resource.h"
using namespace std;

// ==================== Pin Tanımları ====================
#ifndef up_btn
#define up_btn      4
#define dwn_btn     3
#define lft_btn     5
#define rgh_btn     1
#define bck_btn     6
#define select_btn  7
#define R_SHOULDER  2
#define L_SHOULDER  39
#endif

// ==================== Renkler ====================
#define T_BG     0x0208
#define T_GRID   0x0841
#define T_PANEL  0x18C3

// ==================== Particle Sistemi (küçük, sadece tetris için) ====================
struct TPart { float x,y,vx,vy; uint8_t life,maxLife,sz; uint16_t col; };
static TPart tParts[24];

static void tpClear() { memset(tParts,0,sizeof(tParts)); }

static void tpSpawn(float ox,float oy,uint16_t col,float vxr=2.0f,float vyb=0.0f,int n=4) {
  for(int k=0;k<n;k++){
    for(int i=0;i<24;i++){ if(tParts[i].life>0) continue;
      tParts[i].x=ox+(float)(rand()%20-10)/10.0f;
      tParts[i].y=oy;
      tParts[i].vx=(float)(rand()%200-100)/100.0f*vxr;
      tParts[i].vy=(float)(rand()%100-50)/100.0f*1.5f+vyb;
      tParts[i].sz=(uint8_t)(rand()%2+1);
      tParts[i].maxLife=(uint8_t)(rand()%20+15);
      tParts[i].life=tParts[i].maxLife;
      tParts[i].col=col; break;
    }
  }
}

static void tpUpdate() {
  for(int i=0;i<24;i++){
    if(!tParts[i].life) continue;
    tParts[i].x+=tParts[i].vx;
    tParts[i].y+=tParts[i].vy;
    tParts[i].vy+=0.1f;
    tParts[i].life--;
  }
}

static void tpDraw() {
  for(int i=0;i<24;i++){
    if(!tParts[i].life) continue;
    float a=(float)tParts[i].life/tParts[i].maxLife;
    uint16_t c=tParts[i].col;
    int r=(int)(((c>>11)&0x1F)*a), g=(int)(((c>>5)&0x3F)*a), b=(int)((c&0x1F)*a);
    c=(uint16_t)((r<<11)|(g<<5)|b);
    int px=(int)tParts[i].x, py=(int)tParts[i].py_;  // not: py_ -> y kullan
    img.drawPixel((int)tParts[i].x,(int)tParts[i].y,c);
  }
}

// ==================== Oyun Değişkenleri ====================
static const int COLS      = 10;
static const int ROWS      = 20;
static const int BLOCK     = 13;
static const int OX        = 3;
static const int OY        = 20;

static uint16_t board[ROWS][COLS];   // renk haritası (0=boş)
static int score  = 0;
static int level  = 0;
static int lines  = 0;
static bool gameOver = false;

// Zorluk
static int  tdiff = 1;             // 0=EASY 1=NORMAL 2=HARD
static bool tStarted = false;
static const unsigned long T_INTERVALS[] = {800,500,250};

// Blok
struct TBlock { int type,rot,gx,gy; };
static TBlock curBlock, nextBlock;

static unsigned long tPrevMillis = 0;
static unsigned long tBtnMove    = 0;
static const unsigned long BTN_RPT = 120;

// ==================== Yardımcılar ====================
static uint16_t lighten(uint16_t c){
  int r=min(31,(int)(((c>>11)&0x1F)+8));
  int g=min(63,(int)(((c>>5)&0x3F)+12));
  int b=min(31,(int)((c&0x1F)+8));
  return (uint16_t)((r<<11)|(g<<5)|b);
}
static uint16_t darken(uint16_t c){
  int r=max(0,(int)(((c>>11)&0x1F)-6));
  int g=max(0,(int)(((c>>5)&0x3F)-8));
  int b=max(0,(int)((c&0x1F)-6));
  return (uint16_t)((r<<11)|(g<<5)|b);
}

static bool fits(int type,int rot,int gx,int gy){
  for(int y=0;y<4;y++) for(int x=0;x<4;x++){
    if(!tetrominoShapes[type][rot][y][x]) continue;
    int nx=gx+x, ny=gy+y;
    if(nx<0||nx>=COLS||ny>=ROWS) return false;
    if(ny>=0&&board[ny][nx]) return false;
  }
  return true;
}

static void placeBlock(){
  for(int y=0;y<4;y++) for(int x=0;x<4;x++){
    if(!tetrominoShapes[curBlock.type][curBlock.rot][y][x]) continue;
    int nx=curBlock.gx+x, ny=curBlock.gy+y;
    if(ny>=0&&ny<ROWS&&nx>=0&&nx<COLS) board[ny][nx]=blockColor[curBlock.type];
  }
  // Toz efekti
  for(int x=0;x<4;x++) for(int y=0;y<4;y++){
    if(!tetrominoShapes[curBlock.type][curBlock.rot][y][x]) continue;
    int px=OX+(curBlock.gx+x)*BLOCK+BLOCK/2;
    int py=OY+(curBlock.gy+y)*BLOCK+BLOCK;
    tpSpawn(px,py,blockColor[curBlock.type],1.0f,-0.3f,2);
  }
}

static int clearLines(){
  int cleared=0;
  for(int r=ROWS-1;r>=0;r--){
    bool full=true;
    for(int c=0;c<COLS;c++) if(!board[r][c]){full=false;break;}
    if(full){
      // Spark efekti
      for(int c=0;c<COLS;c++) tpSpawn(OX+c*BLOCK+BLOCK/2, OY+r*BLOCK+BLOCK/2, board[r][c], 2.5f, -1.0f, 2);
      // Satırı sil, üstteki indir
      for(int rr=r;rr>0;rr--) for(int c=0;c<COLS;c++) board[rr][c]=board[rr-1][c];
      for(int c=0;c<COLS;c++) board[0][c]=0;
      cleared++; r++; // aynı satırı tekrar kontrol et
    }
  }
  return cleared;
}

static void newBlock(){
  curBlock  = nextBlock;
  nextBlock = {(int)(rand()%7),0,3,0};
  if(!fits(curBlock.type,curBlock.rot,curBlock.gx,curBlock.gy)){
    gameOver=true;
  }
}

static void initGame(){
  memset(board,0,sizeof(board));
  score=0; level=0; lines=0; gameOver=false;
  tpClear();
  nextBlock={rand()%7,0,3,0};
  newBlock();
}

// ==================== Çizim ====================
static void drawSingleBlock(int gx,int gy,uint16_t col,bool ghost=false){
  int px=OX+gx*BLOCK, py=OY+gy*BLOCK;
  if(py+BLOCK<=OY||py>=OY+ROWS*BLOCK) return;
  if(ghost){
    img.drawRect(px,py,BLOCK,BLOCK,col);
    img.drawRect(px+1,py+1,BLOCK-2,BLOCK-2,darken(col));
  } else {
    img.fillRect(px+1,py+1,BLOCK-2,BLOCK-2,col);
    img.drawFastHLine(px+2,py+2,BLOCK-4,lighten(col));
    img.drawFastVLine(px+2,py+3,BLOCK-5,lighten(col));
    img.drawRect(px,py,BLOCK,BLOCK,darken(col));
  }
}

static void drawBoard(){
  // Arka plan + grid çizgileri
  img.fillRect(OX,OY,COLS*BLOCK,ROWS*BLOCK,T_BG);
  for(int r=0;r<=ROWS;r++) img.drawFastHLine(OX,OY+r*BLOCK,COLS*BLOCK,T_GRID);
  for(int c=0;c<=COLS;c++) img.drawFastVLine(OX+c*BLOCK,OY,ROWS*BLOCK,T_GRID);

  // Yerleşmiş bloklar
  for(int r=0;r<ROWS;r++) for(int c=0;c<COLS;c++)
    if(board[r][c]) drawSingleBlock(c,r,board[r][c]);

  // Ghost (düşeceği yer)
  TBlock gh=curBlock;
  while(fits(gh.type,gh.rot,gh.gx,gh.gy+1)) gh.gy++;
  if(gh.gy!=curBlock.gy){
    for(int y=0;y<4;y++) for(int x=0;x<4;x++){
      if(!tetrominoShapes[gh.type][gh.rot][y][x]) continue;
      drawSingleBlock(gh.gx+x,gh.gy+y,blockColor[gh.type],true);
    }
  }

  // Aktif blok
  for(int y=0;y<4;y++) for(int x=0;x<4;x++){
    if(!tetrominoShapes[curBlock.type][curBlock.rot][y][x]) continue;
    drawSingleBlock(curBlock.gx+x,curBlock.gy+y,blockColor[curBlock.type]);
  }
}

static void drawPanel(){
  img.fillRect(133,0,39,320,T_PANEL);
  img.drawFastVLine(133,0,320,0x39E7);

  // Başlık
  img.setTextColor(TFT_YELLOW); img.setTextFont(2);
  img.setCursor(136,4); img.print("TET");
  img.setTextColor(0xF81F);    img.print("RIS");

  // HOLD kutusu
  img.setTextColor(0x8C71); img.setTextSize(1);
  img.setCursor(138,24); img.print("HOLD");
  img.fillRoundRect(136,33,33,33,3,0x0841);
  img.drawRoundRect(135,32,35,35,4,TFT_GOLD);

  // NEXT kutusu
  img.setCursor(138,72); img.print("NEXT");
  img.fillRoundRect(136,81,33,55,3,0x0841);
  img.drawRoundRect(135,80,35,57,4,TFT_GOLD);

  // NEXT bloğu göster
  for(int y=0;y<4;y++) for(int x=0;x<4;x++){
    if(!tetrominoShapes[nextBlock.type][nextBlock.rot][y][x]) continue;
    int px=137+x*8, py=83+y*8;
    img.fillRect(px+1,py+1,6,6,blockColor[nextBlock.type]);
    img.drawRect(px,py,8,8,darken(blockColor[nextBlock.type]));
  }

  // Score / Level / Lines
  img.drawFastHLine(135,142,37,0x39E7);
  img.setTextColor(0x8C71); img.setCursor(136,146); img.print("LINES");
  img.setTextColor(TFT_WHITE); img.setCursor(136,156); img.print(lines);
  img.setTextColor(0x8C71); img.setCursor(136,172); img.print("LEVEL");
  img.setTextColor(TFT_YELLOW); img.setCursor(136,182); img.print(level);
  img.setTextColor(0x8C71); img.setCursor(136,198); img.print("SCORE");
  img.setTextColor(TFT_WHITE); img.setCursor(136,208);
  if(score<10000) img.print(score); else { img.print(score/1000); img.print("K"); }

  // Zorluk
  img.drawFastHLine(135,235,37,0x39E7);
  static const char* DNAMES[]={"EASY","NORM","HARD"};
  static const uint16_t DCOLS[]={0x07E0,0xFFE0,0xF800};
  img.setTextColor(DCOLS[tdiff]); img.setCursor(136,240); img.print(DNAMES[tdiff]);
}

static void drawGameOver(){
  img.fillRect(2,90,130,70,0x6000);
  img.drawRect(2,90,130,70,0xF800);
  img.setTextColor(0xF800); img.setTextFont(4);
  img.setCursor(8,96); img.print("GAME");
  img.setCursor(8,120); img.print("OVER");
  img.setTextFont(1); img.setTextColor(TFT_WHITE);
  img.setCursor(8,148); img.print("SELECT=retry");
}

// ==================== Zorluk Seçim Ekranı ====================
static bool lastSelD=HIGH, lastBckD=HIGH, lastUD=HIGH, lastDD=HIGH;

static void drawDiffScreen(){
  img.fillSprite(T_BG);

  // Başlık
  img.setTextColor(TFT_YELLOW); img.setTextFont(4);
  img.setCursor(15,12); img.print("TETRIS");
  img.drawFastHLine(4,46,164,TFT_YELLOW);

  // 3 zorluk kutusu
  static const char* DNAMES[]={"EASY","NORMAL","HARD"};
  static const uint16_t DCOLS[]={0x07E0,0xFFE0,0xF800};
  static const char* DSUBS[]={"800ms drop","500ms drop","250ms drop"};

  for(int i=0;i<3;i++){
    int y=56+i*80;
    bool sel=(i==tdiff);
    img.fillRoundRect(8,y,156,68,6,sel?DCOLS[i]:0x18C3);
    img.drawRoundRect(7,y-1,158,70,7,sel?TFT_WHITE:0x39E7);
    img.setTextColor(sel?T_BG:TFT_WHITE); img.setTextFont(4);
    img.setCursor(18,y+8); img.print(DNAMES[i]);
    img.setTextFont(1); img.setTextColor(sel?0x18C3:0x8C71);
    img.setCursor(18,y+36); img.print(DSUBS[i]);
    // Bar
    for(int d=0;d<3;d++){
      uint16_t bc=(d<=i)?DCOLS[i]:0x2945;
      img.fillRect(18+d*30,y+50,24,10,bc);
    }
  }

  img.setTextColor(0x39E7); img.setTextFont(1);
  img.setCursor(10,308); img.print("UP/DWN=select  SELECT=start");
  img.pushSprite(0,0);

  // Buton okuma
  int bU=digitalRead(up_btn), bD=digitalRead(dwn_btn), bS=digitalRead(select_btn);
  if(bU==LOW&&lastUD==HIGH) tdiff=max(0,tdiff-1);
  if(bD==LOW&&lastDD==HIGH) tdiff=min(2,tdiff+1);
  if(bS==LOW&&lastSelD==HIGH){ initGame(); tStarted=true; }
  lastUD=bU; lastDD=bD; lastSelD=bS;
}

// ==================== Giriş Kontrolü ====================
static int  holdType=-1, holdRot=0;
static bool holdUsed=false;

static void handleInput(){
  unsigned long now=millis();
  int bL=digitalRead(lft_btn), bR=digitalRead(rgh_btn);
  int bD=digitalRead(dwn_btn), bU=digitalRead(up_btn);
  int bS=digitalRead(select_btn), bB=digitalRead(bck_btn);

  static int lastL=HIGH,lastR=HIGH,lastD=HIGH,lastU=HIGH,lastS=HIGH,lastB=HIGH;

  // Sol/Sağ hareket (defalarca bas)
  if(bL==LOW){ if(lastL==HIGH||now-tBtnMove>BTN_RPT){ if(fits(curBlock.type,curBlock.rot,curBlock.gx-1,curBlock.gy)) curBlock.gx--; tBtnMove=now; } }
  if(bR==LOW){ if(lastR==HIGH||now-tBtnMove>BTN_RPT){ if(fits(curBlock.type,curBlock.rot,curBlock.gx+1,curBlock.gy)) curBlock.gx++; tBtnMove=now; } }
  // Hızlı düşüş
  if(bD==LOW){ if(lastD==HIGH||now-tBtnMove>50){ if(fits(curBlock.type,curBlock.rot,curBlock.gx,curBlock.gy+1)){curBlock.gy++;score++;} tBtnMove=now; } }
  // Anlık düşüş
  if(bU==LOW&&lastU==HIGH){ while(fits(curBlock.type,curBlock.rot,curBlock.gx,curBlock.gy+1)){curBlock.gy++;score+=2;} }
  // Döndür
  if(bS==LOW&&lastS==HIGH){ int nr=(curBlock.rot+1)%4; if(fits(curBlock.type,nr,curBlock.gx,curBlock.gy)) curBlock.rot=nr; }
  // Hold
  if(bB==LOW&&lastB==HIGH&&!holdUsed){
    if(holdType<0){ holdType=curBlock.type; holdRot=0; newBlock(); }
    else{ int t=holdType; holdType=curBlock.type; curBlock={t,0,3,0}; }
    holdUsed=true;
  }

  lastL=bL;lastR=bR;lastD=bD;lastU=bU;lastS=bS;lastB=bB;
}

// ==================== Tetris Setup/Update ====================
void tetrisSetup(){
  tft.init(); tft.setRotation(4); tft.fillScreen(T_BG);
  img.createSprite(172,320);
  tStarted=false; tdiff=1;
  img.pushSprite(0,0);
}

void tetrisUpdate(){
  if(!tStarted){ drawDiffScreen(); return; }

  if(gameOver){
    // Yağmur efekti
    if(rand()%3==0){
      tpSpawn((float)(rand()%130+3), 0.0f, 0xF800, 0.5f, 1.5f, 1);
    }
    tpUpdate();
    img.fillSprite(T_BG);
    drawBoard(); drawPanel();
    tpDraw();
    drawGameOver();
    img.pushSprite(0,0);
    if(digitalRead(select_btn)==LOW){ delay(150); initGame(); }
    return;
  }

  handleInput();

  unsigned long now=millis();
  if(now-tPrevMillis>=T_INTERVALS[tdiff]){
    tPrevMillis=now;
    if(fits(curBlock.type,curBlock.rot,curBlock.gx,curBlock.gy+1)){
      curBlock.gy++;
    } else {
      placeBlock();
      int cl=clearLines();
      if(cl>0){
        static const int pts[]={0,100,300,500,800};
        score+=pts[min(cl,4)]*( level+1);
        lines+=cl;
        level=lines/10;
        if(cl>=4){ // Tetris! — büyük patlama
          tpSpawn(65,160,TFT_YELLOW,3.0f,-2.0f,8);
          tpSpawn(65,160,TFT_WHITE, 2.0f,-1.5f,6);
        }
      }
      holdUsed=false;
      newBlock();
    }
  }

  tpUpdate();
  img.fillSprite(T_BG);
  drawBoard(); drawPanel();
  tpDraw();

  // HOLD kutusuna blok çiz
  if(holdType>=0){
    for(int y=0;y<4;y++) for(int x=0;x<4;x++){
      if(!tetrominoShapes[holdType][0][y][x]) continue;
      int px=137+x*8, py=35+y*8;
      img.fillRect(px+1,py+1,6,6,blockColor[holdType]);
      img.drawRect(px,py,8,8,darken(blockColor[holdType]));
    }
  }

  img.pushSprite(0,0);
}

// ==================== Wrapper (standalone uyum) ====================
void drawTetromino(int type,int rot,int gx,int gy){
  for(int y=0;y<4;y++) for(int x=0;x<4;x++)
    if(tetrominoShapes[type][rot][y][x]) drawSingleBlock(gx+x,gy+y,blockColor[type]);
}
void drawBlock(int gx,int gy,uint16_t col){ drawSingleBlock(gx,gy,col); }
void spawnBlock(){ /* handled internally */ }
void moveBlock(){ /* handled internally */ }
void rotateBlock(){ int nr=(curBlock.rot+1)%4; if(fits(curBlock.type,nr,curBlock.gx,curBlock.gy)) curBlock.rot=nr; }
void holdBlock(){  /* handled in handleInput */ }
void addBlocks(){ /* handled internally */ }
