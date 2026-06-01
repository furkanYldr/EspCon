// game_pacman.h — GameHub Pacman wrapper
// Tüm pacman kodu namespace Pacman içinde
// resource.h verileri ayrı game_pacman_res.h dosyasından gelir
#pragma once
#include <TFT_eSPI.h>
#include <cmath>
#include <vector>
#include <map>
#include <utility>
#include <queue>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include "game_pacman_res.h"   // coin_matrix, sprite verileri, labirent

namespace Pacman {
using namespace std;

// ==================== Sabitler ====================
static const int pacmanColor  = 0xFFE0;
static const int frightened   = 0x001F;
static const int BlinkyColor  = 0xF800;
static const int PinkyColor   = 0xFB56;
static const int InkyColor    = 0x07FF;
static const int ClydeColor   = 0xFC60;
static const int COLOR_0      = 0x0000;
static const int COLOR_1      = 0x00AA;
static const int CELL_SIZE    = 6;
static const int MATRIX_ROWS  = PC_MATRIX_ROWS;   // 32 — bmatrix ile eşleşmeli
static const int MATRIX_COLS  = PC_MATRIX_COLS;   // 29

// ==================== Enum ====================
enum gameStateEnum { start, game, gameOver, pauseGame, win };
enum animDirection { AUP, ADOWN, ALEFT, ARIGHT };
enum pacmanDirection { PUP, PDOWN, PLEFT, PRIGHT, PEMPTY };
enum states { CHASE, SCATTER, FRIGHTENED, EATEN };
enum ghostDirection { GUP, GDOWN, GLEFT, GRIGHT, GEMPTY };

// ==================== Structs ====================
struct STRUCTPACMAN { int px,py,pvx,pvy; bool up,down,front,back,open; pacmanDirection wantedDirection; animDirection ANIM; };
struct STRUCTGHOST  { int px,py,pvx,pvy; bool up,down,front,back; int color,prevCol,prevRow; states STATE; ghostDirection wantedDirection; };

// ==================== Globals ====================
static gameStateEnum gameState = start;
static unsigned long previousMillisMove = 0;
static const unsigned long movementInterval = 10;
static int  ptimer=0, prevTimer=0, STATETimer=0, prevScore=0, countDown=4, frightenedCountDown=0;
static int  health=3, score=0, px=0, py_=0, pacmanAnim=1, pacmanSpeed=2, animCounter=0, gc=0;
static const int xPadding=76, yPadding=-3;
static int  ghostAnim=0;
static bool pUP=false,pDOWN=false,pLEFT=false,pRIGHT=false,pSELECT=false,pCATCH=false,btnPressed=false;
static bool ghostINKY=false,ghostCLYDE=false;
static bool goBack=false; static int strike=0,pinkyTimer=0;
static bool pR=true;

static STRUCTPACMAN pacman={77,209,0,0,false,false,true,true,true,PEMPTY,ARIGHT};
static STRUCTGHOST  blinky={77,137,0,0,false,false,false,false,BlinkyColor,0,0,SCATTER,GEMPTY};
static STRUCTGHOST  inky  ={90,155,0,0,false,false,false,false,InkyColor,  0,0,SCATTER,GEMPTY};
static STRUCTGHOST  clyde ={70,155,0,0,false,false,false,false,ClydeColor, 0,0,SCATTER,GEMPTY};
static STRUCTGHOST  ppinky={80,155,0,0,false,false,false,false,PinkyColor, 0,0,SCATTER,GEMPTY};
static vector<STRUCTGHOST> vecGHOST;

// ==================== Ghost AI ====================
static pair<int,int> blinyChase(pair<int,int> prev,pair<int,int> trgt,vector<pair<int,int>>& dirs){
  int dist=0,dis; pair<int,int> way;
  for(const auto& d:dirs){ if(d!=prev&&PC_coin_matrix[d.first][d.second]!=0){
    dis=((d.second-trgt.second)*(d.second-trgt.second))+((d.first-trgt.first)*(d.first-trgt.first));
    if(dist==0){dist=dis;way=d;}else if(dist>=dis){dist=dis;way=d;}}}
  return way;
}
static pair<int,int> eatenPath(pair<int,int> prev,vector<pair<int,int>>& dirs){
  pair<int,int> trgt={13,14};
  return blinyChase(prev,trgt,dirs);
}
static pair<int,int> frightenPath(pair<int,int> prev,vector<pair<int,int>>& dirs){
  if(goBack){goBack=false;return prev;}
  vector<pair<int,int>> ways;
  for(const auto& d:dirs) if(d!=prev&&PC_coin_matrix[d.first][d.second]!=0) ways.push_back(d);
  if(ways.empty())return prev;
  return ways[rand()%ways.size()];
}
static pair<int,int> searchTRY(STRUCTGHOST& ghost,int sx,int sy,int ex,int ey,int pr,int pc){
  pair<int,int> rght{sx+1,sy},lft{sx-1,sy},up{sx,sy-1},dwn{sx,sy+1};
  vector<pair<int,int>> dirs={rght,lft,up,dwn};
  pair<int,int> prev{pr,pc},trgt{ex,ey};
  if(ghost.STATE==EATEN)return eatenPath(prev,dirs);
  if(ghost.STATE==FRIGHTENED)return frightenPath(prev,dirs);
  if(ghost.STATE==SCATTER){
    switch(ghost.color){
      case BlinkyColor:trgt={0,26};break;case PinkyColor:trgt={0,0};break;
      case InkyColor:  trgt={30,29};break;case ClydeColor:trgt={30,3};break;
    }
  }
  return blinyChase(prev,trgt,dirs);
}
static void goThatWay(STRUCTGHOST& ghost){
  int spd=(ghost.STATE==EATEN)?3:1;
  switch(ghost.wantedDirection){
    case GRIGHT:ghost.pvx=spd; ghost.pvy=0;break;
    case GLEFT: ghost.pvx=-spd;ghost.pvy=0;break;
    case GDOWN: ghost.pvx=0;  ghost.pvy=spd;break;
    case GUP:   ghost.pvx=0;  ghost.pvy=-spd;break;
    default:    ghost.pvx=0;  ghost.pvy=0;break;
  }
}
static void blinkyMOVEMENT(STRUCTGHOST& ghost,pair<int,int> vec,int bx,int by,int mpx,int mpy){
  int dx=vec.first-bx,dy=vec.second-by;
  if(mpx==bx&&mpy==by){ghost.wantedDirection=GEMPTY;}
  else{
    if(dx==0&&dy==1)ghost.wantedDirection=GRIGHT;
    else if(dx==0&&dy==-1)ghost.wantedDirection=GLEFT;
    if(dx==1&&dy==0)ghost.wantedDirection=GDOWN;
    else if(dx==-1&&dy==0)ghost.wantedDirection=GUP;
  }
  goThatWay(ghost);
}
static void getAlive(){ for(auto& g:vecGHOST) if(g.px<90&&g.px>70&&g.py==137&&g.STATE==EATEN) g.STATE=CHASE; }
static void getFreak(){ for(auto& g:vecGHOST) g.STATE=FRIGHTENED; }
static void catchPacMan(){
  for(auto& g:vecGHOST) if(g.STATE==CHASE||g.STATE==SCATTER){
    float dx=pacman.px-g.px,dy=pacman.py-g.py;
    if((dx*dx+dy*dy)<81) pCATCH=true;
  }
}
static void eaten_check(){
  for(auto& g:vecGHOST) if(g.STATE==FRIGHTENED&&g.STATE!=EATEN){
    float dx=pacman.px-g.px,dy=pacman.py-g.py;
    if((dx*dx+dy*dy)<81){
      g.STATE=EATEN;
      int pts[]={200,400,800,1600};
      if(strike<4){score+=pts[strike];strike++;}
    }
  }
}
static void spawnGhost(){
  if(ptimer>prevTimer&&pinkyTimer<=8)pinkyTimer++;
  if(pinkyTimer==8){vecGHOST[1].px=77;vecGHOST[1].py=137;pinkyTimer++;}
  if(ghostINKY){vecGHOST[2].px=77;vecGHOST[2].py=137;ghostINKY=false;}
  if(ghostCLYDE){vecGHOST[3].px=77;vecGHOST[3].py=137;ghostCLYDE=false;}
}
static void pathFinder(int gx,int gy){
  for(auto& ghost:vecGHOST){
    int cpx=gx+4,cpy=gy+5,cgx=ghost.px+4,cgy=ghost.py+5;
    int rowP=(cpy-xPadding)/6,colP=(cpx-yPadding)/6;
    int rowG=(cgy-xPadding)/6,colG=(cgx-yPadding)/6;
    if((cgy-xPadding)%6==0&&(cgx-yPadding)%6==0){
      if(rowG==14&&ghost.px+4<3){ghost.px=161;ghost.wantedDirection=GLEFT;goThatWay(ghost);ghost.prevRow=14;ghost.prevCol=32;continue;}
      if(rowG==14&&ghost.px+4>165){ghost.px=-1;ghost.wantedDirection=GRIGHT;goThatWay(ghost);ghost.prevRow=14;ghost.prevCol=1;continue;}
      int ex=rowP,ey=colP;
      if(ghost.color==ppinky.color){switch(pacman.ANIM){case AUP:ex-=4;break;case ADOWN:ex+=4;break;case ALEFT:ey-=4;break;case ARIGHT:ey+=4;break;}}
      else if(ghost.color==inky.color){switch(pacman.ANIM){case AUP:ex-=2;break;case ADOWN:ex+=2;break;case ALEFT:ey-=2;break;case ARIGHT:ey+=2;break;} ex+=(ex-blinky.prevRow);ey+=(ey-blinky.prevCol);}
      else if(ghost.color==clyde.color){float dx=ex-rowG,dy=ey-colG;if((dx*dx+dy*dy)<64){ex=30;ey=1;}}
      pair<int,int> path=searchTRY(ghost,rowG,colG,ex,ey,ghost.prevRow,ghost.prevCol);
      blinkyMOVEMENT(ghost,path,rowG,colG,rowP,colP);
      ghost.prevRow=rowG;ghost.prevCol=colG;
    }
  }
}
static void ghostStateManager(){
  if(STATETimer>=20)STATETimer=0;
  for(auto& g:vecGHOST){
    if(g.STATE==FRIGHTENED||g.STATE==EATEN){
      if(g.STATE==FRIGHTENED&&frightenedCountDown<=0){g.STATE=SCATTER;strike=0;}
      continue;
    }
    g.STATE=(STATETimer<7||(STATETimer>=14&&STATETimer<20))?SCATTER:CHASE;
  }
}

// ==================== Pacman Çizim ====================
static void drawBlinky(){
  for(auto& gR:vecGHOST){
    uint8_t matrix[10][10];
    switch(ghostAnim){
      case 0:memcpy(matrix,PC_ghost,sizeof(matrix));break;
      case 1:memcpy(matrix,PC_ghost1,sizeof(matrix));break;
      case 2:memcpy(matrix,PC_ghost2,sizeof(matrix));break;
      case 3:memcpy(matrix,PC_ghost3,sizeof(matrix));break;
    }
    img.fillRect(gR.px,gR.py+1,10,9,COLOR_0);
    gR.px+=gR.pvx;gR.py+=gR.pvy;
    for(int row=0;row<10;row++) for(int col=0;col<10;col++){
      int x=col+gR.px,y=row+gR.py;
      if(matrix[row][col]==1&&!(gR.STATE==EATEN)){
        uint16_t dc=(gR.STATE==FRIGHTENED)?((frightenedCountDown<=2&&(millis()/250)%2==0)?TFT_WHITE:frightened):gR.color;
        img.drawPixel(x,y,dc);
      }else if(matrix[row][col]==2) img.drawPixel(x,y,TFT_WHITE);
      else if(matrix[row][col]==3)  img.drawPixel(x,y,TFT_BLACK);
    }
  }
}

static void checkWin(){
  for(int r=0;r<PC_coin_rows;r++) for(int c=0;c<PC_coin_cols;c++)
    if(PC_coin_matrix[r][c]==1||PC_coin_matrix[r][c]==2)return;
  gameState=win;
}

static void collectFood(int row,int col){
  if(PC_coin_matrix[row][col]==1){
    PC_coin_matrix[row][col]=3;score++;
    if(score>prevScore){if(gc<40)gc++;if(gc==20){ghostINKY=true;gc++;}if(gc==40){ghostCLYDE=true;gc++;}
    if(score==70&&PC_fruitEaten==0)PC_fruitVisible=true;}
    prevScore=score;checkWin();
  }else if(PC_coin_matrix[row][col]==2){
    PC_coin_matrix[row][col]=3;score+=20;getFreak();frightenedCountDown=7;goBack=true;
  }else if(PC_coin_matrix[row][col]==4){
    PC_coin_matrix[row][col]=3;score+=100;PC_fruitVisible=false;PC_fruitEaten++;
  }
}

static void pacMOVEMENT(){
  if(pUP){pacman.wantedDirection=PUP;pUP=false;}
  else if(pDOWN){pacman.wantedDirection=PDOWN;pDOWN=false;}
  else if(pRIGHT){pacman.wantedDirection=PRIGHT;pRIGHT=false;}
  else if(pLEFT){pacman.wantedDirection=PLEFT;pLEFT=false;}
  int centerx=pacman.px+4,centery=pacman.py+5;
  int row=(centery-xPadding)/CELL_SIZE,col=(centerx-yPadding)/CELL_SIZE;
  if((centerx-yPadding)%CELL_SIZE==0){
    if(pacman.wantedDirection==PUP&&pacman.up){pacman.pvx=0;pacman.pvy=-pacmanSpeed;pacman.wantedDirection=PEMPTY;pacman.ANIM=AUP;btnPressed=true;}
    else if(pacman.wantedDirection==PDOWN&&pacman.down){pacman.pvx=0;pacman.pvy=pacmanSpeed;pacman.wantedDirection=PEMPTY;pacman.ANIM=ADOWN;btnPressed=true;}
  }
  if((centery-xPadding)%CELL_SIZE==0){
    if(pacman.wantedDirection==PRIGHT&&pacman.front){pacman.pvx=pacmanSpeed;pacman.pvy=0;pacman.wantedDirection=PEMPTY;pacman.ANIM=ARIGHT;btnPressed=true;}
    else if(pacman.wantedDirection==PLEFT&&pacman.back){pacman.pvx=-pacmanSpeed;pacman.ANIM=ALEFT;pacman.pvy=0;pacman.wantedDirection=PEMPTY;btnPressed=true;}
  }
}

static void canMove(){
  int centerx=pacman.px+4,centery=pacman.py+5;
  int row=(centery-xPadding)/CELL_SIZE,col=(centerx-yPadding)/CELL_SIZE;
  if((centery-xPadding)%CELL_SIZE==0){
    if(row==14&&pacman.px+4<3)pacman.px=161;
    else if(row==14&&pacman.px+4>165)pacman.px=-1;
    pacman.up  =(row>0&&PC_coin_matrix[row-1][col]!=0&&PC_coin_matrix[row-1][col]!=7);
    pacman.down=(row<PC_coin_rows-1&&PC_coin_matrix[row+1][col]!=0&&PC_coin_matrix[row+1][col]!=7);
    if(!pacman.up&&pacman.pvy==-pacmanSpeed)pacman.pvy=0;
    if(!pacman.down&&pacman.pvy==pacmanSpeed)pacman.pvy=0;
    collectFood(row,col);
  }else{pacman.up=false;pacman.down=false;}
  if((centerx-yPadding)%CELL_SIZE==0){
    pacman.back =(col>0&&PC_coin_matrix[row][col-1]!=0&&PC_coin_matrix[row][col-1]!=7);
    pacman.front=(col<PC_coin_cols-1&&PC_coin_matrix[row][col+1]!=0&&PC_coin_matrix[row][col+1]!=7);
    if(!pacman.front&&pacman.pvx==pacmanSpeed)pacman.pvx=0;
    if(!pacman.back&&pacman.pvx==-pacmanSpeed)pacman.pvx=0;
    collectFood(row,col);
  }else{pacman.back=false;pacman.front=false;}
  pacMOVEMENT();
}

static void drawMaze(){
  // ── Hizalama analizi ──────────────────────────────────────────────
  // bmatrix[32][29]: row*6+70  → row0=y70, row31=y256  (32 satır)
  // coin_matrix[31][30]: row*6+76 → row0=y76, row30=y256  (31 satır)
  // İkisi y=256'da biter. bmatrix 6px (1 hücre) YUKARIDAN başlar:
  // bmatrix[0] = dış üst duvar çerçevesi (coin matrisinde karşılık yok)
  // bmatrix[1] ↔ coin[0] hizasında → y76. Bu kasıtlı orijinal tasarım.
  // ─────────────────────────────────────────────────────────────────
  for(int row=0;row<PC_MATRIX_ROWS;row++) for(int col=0;col<PC_MATRIX_COLS;col++){
    px=col*CELL_SIZE-2; py_=row*CELL_SIZE+70;  // orijinal: +70
    img.fillRect(px,py_,CELL_SIZE,CELL_SIZE,(PC_bmatrix[row][col]==1)?COLOR_1:COLOR_0);
  }
  for(int row=0;row<PC_coin_rows;row++) for(int col=0;col<PC_coin_cols;col++){
    px=col*CELL_SIZE+yPadding; py_=row*CELL_SIZE+xPadding;  // xPadding=76
    if(PC_coin_matrix[row][col]==1)      img.drawPixel(px,py_,pacmanColor);
    else if(PC_coin_matrix[row][col]==2) img.fillCircle(px,py_,3,TFT_WHITE);
  }
}

static void drawFruit(){
  for(int row=0;row<10;row++) for(int col=0;col<11;col++){
    px=col+75;py_=row+275;
    switch(PC_cherry[row][col]){
      case 1:img.drawPixel(px,py_,0x9226);break;case 2:img.drawPixel(px,py_,0x4DC3);break;
      case 3:img.drawPixel(px,py_,0xF840);break;case 4:img.drawPixel(px,py_,0xC006);break;
      case 5:img.drawPixel(px,py_,0xE947);break;
    }
  }
}

static void drawPacman(uint8_t matrix[10][10]){
  canMove();
  img.fillRect(pacman.px,pacman.py+1,10,9,COLOR_0);
  pacman.px+=pacman.pvx;pacman.py+=pacman.pvy;
  for(int row=0;row<10;row++) for(int col=0;col<10;col++){
    px=col+pacman.px;py_=row+pacman.py;
    if(pacman.open){ if(matrix[row][col]==1)img.drawPixel(px,py_,pacmanColor); else img.drawPixel(px,py_,COLOR_0); }
    else{ if(PC_pacman2[row][col]==1)img.drawPixel(px,py_,pacmanColor); else img.drawPixel(px,py_,COLOR_0); }
  }
}

static void healthTracker(){
  for(int i=0;i<health&&i<3;i++) for(int row=0;row<10;row++) for(int col=0;col<10;col++){
    if(PC_pacmanFront[row][col]==1)img.drawPixel(col+10+(i*15),row+265,pacmanColor);
  }
}

// PAC-MAN piksel-art logo (TM olmadan) — sütun bazlı 7 renk
// Turuncu arka plan kutusunda sarı + mavi gölgeli harfler
static void drawLogo(){
  // Arka plan turuncu kutu
  img.fillRoundRect(2,2,168,38,4,0xFCC0);  // turuncu
  img.drawRoundRect(2,2,168,38,4,0xC800);  // koyu kırmızı kenar
  img.drawRoundRect(3,3,166,36,3,0xFFFF);  // beyaz iç kenar
  // "PAC-MAN" büyük sarı yazı, mavi gölgeli
  // Gölge (mavi-mor)
  img.setCursor(11,10); img.setTextSize(3); img.setTextColor(0x30D8); img.print("PAC-MAN");
  img.setCursor(10,9);  img.setTextSize(3); img.setTextColor(0x30D8); img.print("PAC-MAN");
  // Ana yazı (parlak sarı)
  img.setCursor(10,8);  img.setTextSize(3); img.setTextColor(0xFFE0); img.print("PAC-MAN");
  // Beyaz üst şerit highlight
  img.setCursor(10,8);  img.setTextSize(3); img.setTextColor(0xFFFF);
  // Sadece üst kısım beyaz — harflerin üst çizgisini aydınlatmak için
  // Çerçeve dışı köşe detayı
  img.fillRect(0,0,2,2,0x0000); img.fillRect(168,0,4,2,0x0000);
  img.fillRect(0,38,2,2,0x0000); img.fillRect(168,38,4,2,0x0000);
}

static void drawInit(){
  img.fillSprite(TFT_BLACK);
  drawLogo();
  drawMaze();
  if(PC_fruitVisible)drawFruit();
  drawBlinky();
  switch(pacman.ANIM){case ALEFT:drawPacman(PC_pacmanBack);break;case ARIGHT:drawPacman(PC_pacmanFront);break;case AUP:drawPacman(PC_pacmanUp);break;case ADOWN:drawPacman(PC_pacmanDown);break;}
  img.setCursor(110,266);img.setTextSize(1);img.setTextColor(TFT_WHITE);img.print("score:");img.println(score);
  img.setCursor(150,55);img.setTextSize(1);img.setTextColor(TFT_WHITE);img.print(ptimer);
  healthTracker();pathFinder(pacman.px,pacman.py);img.pushSprite(0,0);
}

static void buttonControl(){
  int bD=digitalRead(dwn_btn),bU=digitalRead(up_btn),bR=digitalRead(rgh_btn),bL=digitalRead(lft_btn),bP=digitalRead(bck_btn),bS=digitalRead(select_btn);
  if(!btnPressed){
    if(bU==LOW){pUP=true;btnPressed=true;}
    else if(bD==LOW){pDOWN=true;btnPressed=true;}
    else if(bR==LOW){pRIGHT=true;btnPressed=true;}
    else if(bL==LOW){pLEFT=true;btnPressed=true;}
    else if(bP==LOW){if(gameState==game){gameState=pauseGame;countDown=4;}btnPressed=true;}
    else if(bS==LOW){if(gameState==pauseGame)gameState=game;else pSELECT=true;btnPressed=true;}
  }
  if(bU==HIGH&&bD==HIGH&&bR==HIGH&&bL==HIGH&&bP==HIGH&&bS==HIGH)btnPressed=false;
}

static void setGameStart(){
  pacman.wantedDirection=PEMPTY;pacman.ANIM=ARIGHT;pacman.px=77;pacman.py=209;pacman.pvy=0;pacman.pvx=0;
  vecGHOST[0].px=77;vecGHOST[0].py=137;vecGHOST[1].px=80;vecGHOST[1].py=155;
  vecGHOST[2].px=70;vecGHOST[2].py=155;vecGHOST[3].px=90;vecGHOST[3].py=155;
  pinkyTimer=0;STATETimer=0;
  for(auto& g:vecGHOST){g.STATE=SCATTER;g.wantedDirection=GEMPTY;g.pvx=0;g.pvy=0;}
  gc=0;
}

static void gameSetup_(){
  if(pCATCH){ if(health>=1){health--;pCATCH=false;setGameStart();}else gameState=gameOver; }
  if(PC_fruitVisible){int dx=pacman.px-75,dy=pacman.py-275;if(dx*dx+dy*dy<100){score+=100;PC_fruitVisible=false;PC_fruitEaten++;}}
}

// ==================== Tab Ekranları ====================
static void startAnim(){
  static int sax=0,sav=2,spx=0; spx+=sav;
  for(int row=0;row<10;row++) for(int col=0;col<10;col++){
    if(sax>300){pR=false;sav=-2;}else if(sax<-130){pR=true;sav=2;}
    sax=col+spx;int sy=row+100;
    if(pacman.open){
      if(pR){if(PC_pacmanFront[row][col]==1)img.drawPixel(sax,sy,pacmanColor);else img.drawPixel(sax,sy,COLOR_0);}
      else{if(PC_pacmanBack[row][col]==1)img.drawPixel(sax,sy,pacmanColor);else img.drawPixel(sax,sy,COLOR_0);}
    }else{if(PC_pacman2[row][col]==1)img.drawPixel(sax,sy,pacmanColor);else img.drawPixel(sax,sy,COLOR_0);}
  }
  uint8_t mat[10][10];
  switch(ghostAnim){case 0:memcpy(mat,PC_ghost,sizeof(mat));break;case 1:memcpy(mat,PC_ghost1,sizeof(mat));break;case 2:memcpy(mat,PC_ghost2,sizeof(mat));break;case 3:memcpy(mat,PC_ghost3,sizeof(mat));break;}
  img.fillRect(sax,101,10,9,COLOR_0);
  for(int row=0;row<10;row++) for(int col=0;col<10;col++){
    int gx=col+sax,gy=row+100;
    if(mat[row][col]==1){uint16_t gc2=(sav==2)?BlinkyColor:frightened;img.drawPixel(gx-60,gy,gc2);img.drawPixel(gx-75,gy,PinkyColor);img.drawPixel(gx-90,gy,InkyColor);img.drawPixel(gx-105,gy,ClydeColor);}
    else if(mat[row][col]==2){img.drawPixel(gx-60,gy,TFT_WHITE);img.drawPixel(gx-75,gy,TFT_WHITE);img.drawPixel(gx-90,gy,TFT_WHITE);img.drawPixel(gx-105,gy,TFT_WHITE);}
  }
}

static void startTAB(){
  img.fillSprite(TFT_BLACK);
  drawLogo();
  img.setTextColor(TFT_WHITE);
  img.fillRoundRect(48,168,74,24,7,TFT_YELLOW);
  img.fillRoundRect(50,170,70,20,5,0x2C38);
  img.setCursor(67,175);img.setTextSize(1);img.print(" START ");
  startAnim();img.pushSprite(0,0);
  if(pSELECT){gameState=game;pSELECT=false;}
}

static void fullReset(){
  score=0;health=3;ptimer=0;prevTimer=0;STATETimer=0;prevScore=0;countDown=4;frightenedCountDown=0;
  ghostINKY=false;ghostCLYDE=false;gc=0;
  PC_resetCoinMatrix();setGameStart();
}

static void gameOverTAB(){
  img.fillSprite(TFT_BLACK);
  img.setTextSize(3);img.setTextColor(TFT_RED);img.setCursor(8,20);img.print("GAME OVER");
  img.setTextSize(2);img.setTextColor(TFT_YELLOW);img.setCursor(35,70);img.print("Score");
  img.setCursor(55,95);img.setTextColor(TFT_WHITE);img.print(score);
  img.fillRoundRect(38,150,96,28,7,TFT_RED);img.fillRoundRect(40,152,92,24,5,0x6000);
  img.setCursor(44,160);img.setTextSize(1);img.setTextColor(TFT_WHITE);img.print(" TRY AGAIN");
  img.pushSprite(0,0);
  if(digitalRead(select_btn)==LOW){delay(200);fullReset();gameState=game;}
}

static void winTAB(){
  img.fillSprite(TFT_BLACK);
  img.setTextSize(2);img.setTextColor(TFT_YELLOW);img.setCursor(22,20);img.print("YOU WIN!");
  img.setTextSize(1);img.setTextColor(TFT_WHITE);img.setCursor(35,60);img.print("Final Score:");
  img.setTextSize(2);img.setTextColor(TFT_YELLOW);img.setCursor(45,78);img.print(score);
  img.fillRoundRect(38,150,96,28,7,TFT_YELLOW);img.fillRoundRect(40,152,92,24,5,0x2C38);
  img.setCursor(40,160);img.setTextSize(1);img.setTextColor(TFT_WHITE);img.print("  PLAY AGAIN");
  img.pushSprite(0,0);
  if(digitalRead(select_btn)==LOW){delay(200);fullReset();gameState=game;}
}

// ==================== GameHub Arayüzü ====================
void pacmanSetup(){
  tft.init();tft.setRotation(4);
  img.createSprite(172,320);
  vecGHOST.clear();
  vecGHOST={blinky,ppinky,inky,clyde};
  PC_resetCoinMatrix();
  fullReset();
  gameState=start;
  img.pushSprite(0,0);
}

void pacmanUpdate(){
  unsigned long now=millis();
  buttonControl();
  if(gameState==start){
    if(now-previousMillisMove>=movementInterval){previousMillisMove=now;animCounter+=pacmanAnim;if(animCounter>=4){pacman.open=!pacman.open;animCounter=0;ghostAnim=(ghostAnim+1)%4;}}
    startTAB();
  }else if(gameState==game){
    if(countDown==0){
      if(now-previousMillisMove>=movementInterval){previousMillisMove=now;getAlive();eaten_check();catchPacMan();drawInit();gameSetup_();animCounter+=pacmanAnim;if(animCounter>=4){pacman.open=!pacman.open;animCounter=0;ghostAnim=(ghostAnim+1)%4;}}
      static unsigned long pt2=0;if(now-pt2>=1000){pt2=now;prevTimer=ptimer;ptimer++;STATETimer++;if(frightenedCountDown>0)frightenedCountDown--;ghostStateManager();spawnGhost();}
    }else if(countDown>=0){
      static unsigned long pcd=0;if(now-pcd>=1000){pcd=now;countDown--;drawMaze();drawBlinky();img.setCursor(77,150);img.setTextSize(3);img.setTextColor(TFT_RED);img.print(countDown);img.pushSprite(0,0);}
    }
  }else if(gameState==pauseGame){
    img.fillRect(0,95,170,32,TFT_BLACK);img.setCursor(35,100);img.setTextSize(3);img.setTextColor(TFT_RED);img.print("PAUSED");
    img.fillRoundRect(48,168,74,24,7,TFT_YELLOW);img.fillRoundRect(50,170,70,20,5,0x2C38);
    img.setCursor(62,175);img.setTextSize(1);img.setTextColor(TFT_WHITE);img.print("CONTINUE");
    img.pushSprite(0,0);
  }else if(gameState==gameOver){gameOverTAB();}
  else if(gameState==win){winTAB();}
}

void setup()  { pacmanSetup();  }
void update() { pacmanUpdate(); }

} // namespace Pacman
