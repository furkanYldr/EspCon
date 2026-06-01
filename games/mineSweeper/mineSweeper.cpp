// mineSweeper.cpp — Map boyutu seçimli Minesweeper
// Boyutlar: TINY(8x10,10) | SMALL(11x14,20) | MEDIUM(14x17,35) | LARGE(17x20,50) ← mevcut
#include "mineSweeper.h"
#include "esp32-hal-gpio.h"
#include <cmath>
#include <vector>
#include <cstring>

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
#define M_YELLOW    0xFECB
#define M_GREEN     0x6C64
#define M_PURPLE    0x712E
#define M_RED       0xE127
#define M_BLUE      0x0334
#define M_WHITE     0xFFFF
#define M_BLACK     0x0000
#define M_GREY      0x52AA
#define M_DARK      0x218B
#define M_BG        0x0208
#define M_ACCENT    0x07E0

// ==================== Map Konfigürasyonları ====================
struct MapConfig {
  const char* name;
  const char* subtitle;
  int         cols;
  int         rows;
  int         mines;
  int         cellSize;
  uint16_t    themeColor;
};

static const MapConfig MAP_CONFIGS[] = {
  { "TINY",   "8x10  10 mines",  8,  10, 10, 16, 0x07E0 },  // yeşil
  { "SMALL",  "11x14 20 mines", 11,  14, 20, 13, 0xFFE0 },  // sarı
  { "MEDIUM", "14x17 35 mines", 14,  17, 35, 11, 0xFC60 },  // turuncu
  { "LARGE",  "17x20 50 mines", 17,  20, 50, 10, 0xF800 },  // kırmızı
};
static const int NUM_MAP_CONFIGS = 4;

// ==================== Dinamik Parametreler ====================
static int COLS      = 17;
static int ROWS      = 20;
static int CELL_SIZE = 10;
static int MINES     = 50;
static int selectedMap = 3;   // default LARGE (mevcut)

// ==================== Oyun Durumu ====================
enum MineState { MS_MAP_SELECT, MS_START, MS_GAME, MS_GAMEOVER, MS_WIN };
static MineState gameState = MS_MAP_SELECT;

// ==================== Zamanlama & Grid ====================
static unsigned long prevMillisMove = 0;
static int mTimer = 0;
static int cursorX = 0, cursorY = 0;
static int flagsPlaced = 0;
static bool firstMove = true;

// Max boyut: 17 col × 20 row (LARGE)
static uint8_t Grid[20][17];
static uint8_t View[20][17];

// Buton state
static bool bSel = false, bBck = false;
static int  lastBtnPressed = -1;
static unsigned long lastBtnTime = 0;
static const unsigned long BTN_DELAY = 280;
static const unsigned long BTN_RATE  = 80;

// Map seçim ekranı encoder
static int msLastSel = HIGH, msLastSnap = HIGH;

// ==================== Yardımcı ====================
// Izgara başlangıç Y offset (50px üst bar)
static int gridOffsetY() { return 50; }
static int gridOffsetX() { return (172 - COLS * CELL_SIZE) / 2; }

// ==================== Mayın Mantığı ====================
static void placeMines(int fr, int fc) {
  int count = 0, attempts = 0;
  while (count < MINES && attempts < 5000) {
    int r = rand() % ROWS;
    int c = rand() % COLS;
    bool safe = (abs(r - fr) <= 1 && abs(c - fc) <= 1);
    if (Grid[r][c] != 9 && !safe) { Grid[r][c] = 9; count++; }
    attempts++;
  }
}

static void calcNumbers() {
  for (int i = 0; i < ROWS; i++)
    for (int j = 0; j < COLS; j++) {
      if (Grid[i][j] == 9) continue;
      int n = 0;
      for (int di = -1; di <= 1; di++)
        for (int dj = -1; dj <= 1; dj++) {
          int ni = i+di, nj = j+dj;
          if (ni>=0&&ni<ROWS&&nj>=0&&nj<COLS&&Grid[ni][nj]==9) n++;
        }
      Grid[i][j] = n;
    }
}

static void checkWin() {
  int covered = 0;
  for (int i = 0; i < ROWS; i++)
    for (int j = 0; j < COLS; j++)
      if (View[i][j] == 0 || View[i][j] == 2) covered++;
  if (covered == MINES) gameState = MS_WIN;
}

static void reveal(int r, int c) {
  if (r<0||r>=ROWS||c<0||c>=COLS||View[r][c]!=0) return;
  if (firstMove) { placeMines(r,c); calcNumbers(); firstMove=false; }
  View[r][c] = 1;
  if (Grid[r][c] == 9) {
    gameState = MS_GAMEOVER;
    for (int i=0;i<ROWS;i++) for (int j=0;j<COLS;j++) if(Grid[i][j]==9) View[i][j]=1;
  } else if (Grid[r][c] == 0) {
    for (int i=-1;i<=1;i++) for (int j=-1;j<=1;j++) reveal(r+i,c+j);
  }
  checkWin();
}

static void toggleFlag() {
  if      (View[cursorY][cursorX]==0) { View[cursorY][cursorX]=2; flagsPlaced++; }
  else if (View[cursorY][cursorX]==2) { View[cursorY][cursorX]=0; flagsPlaced--; }
}

// ==================== Çizim ====================
static void drawGrid() {
  int ox = gridOffsetX(), oy = gridOffsetY();
  // Sınır düzeltmesi
  if (cursorX >= COLS) cursorX = 0;   if (cursorX < 0) cursorX = COLS-1;
  if (cursorY >= ROWS) cursorY = 0;   if (cursorY < 0) cursorY = ROWS-1;

  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLS; j++) {
      int Y = oy + i * CELL_SIZE;
      int X = ox + j * CELL_SIZE;
      bool sel = (i==cursorY && j==cursorX);

      if (View[i][j] == 1) {          // Açık
        img.fillRect(X+1,Y+1,CELL_SIZE-2,CELL_SIZE-2,0xDEDB);
        if (Grid[i][j] == 9) {
          img.fillCircle(X+CELL_SIZE/2,Y+CELL_SIZE/2,CELL_SIZE/3,M_RED);
        } else if (Grid[i][j] > 0) {
          const uint16_t numColors[] = {M_BLUE,M_GREEN,M_RED,M_PURPLE,0x8000,0x5D32,M_BLACK,M_GREY};
          img.setTextColor(numColors[Grid[i][j]-1]);
          img.setTextSize(CELL_SIZE>=12 ? 1 : 0);
          img.setCursor(X+CELL_SIZE/4, Y+1);
          img.print(Grid[i][j]);
        }
      } else if (View[i][j] == 2) {   // Bayrak
        img.fillRect(X+1,Y+1,CELL_SIZE-2,CELL_SIZE-2,M_GREY);
        // Bayrak ikonu
        img.fillTriangle(X+CELL_SIZE/2-1,Y+2, X+CELL_SIZE-3,Y+CELL_SIZE/3, X+CELL_SIZE/2-1,Y+CELL_SIZE*2/3, M_RED);
        img.drawFastVLine(X+CELL_SIZE/2-1,Y+2,CELL_SIZE-4,M_WHITE);
      } else {                         // Kapalı
        img.fillRect(X+1,Y+1,CELL_SIZE-2,CELL_SIZE-2,M_GREY);
        // 3D efekti
        img.drawFastHLine(X+1,Y+1,CELL_SIZE-2,0x8C71); // üst highlight
        img.drawFastVLine(X+1,Y+1,CELL_SIZE-2,0x8C71); // sol highlight
      }
      // Cursor
      if (sel) {
        img.drawRect(X,Y,CELL_SIZE,CELL_SIZE,M_YELLOW);
        img.drawRect(X+1,Y+1,CELL_SIZE-2,CELL_SIZE-2,M_YELLOW);
      }
    }
  }
}

static void drawHUD() {
  const MapConfig& mc = MAP_CONFIGS[selectedMap];
  // Üst bar
  img.fillRect(0,0,172,48,M_BG);
  img.drawFastHLine(0,48,172,mc.themeColor);

  // Başlık
  img.setTextColor(mc.themeColor); img.setTextSize(2);
  img.setCursor(4,4); img.print("MINE");
  img.setTextColor(M_WHITE); img.print("SWEEPER");

  // Map adı + kalan mayın
  img.setTextSize(1); img.setTextColor(M_WHITE);
  img.setCursor(4,26); img.print(mc.name);
  img.print("  ");
  img.setTextColor(mc.themeColor); img.print(MINES-flagsPlaced); img.print(" x");

  // Süre
  img.setTextColor(0x8C71); img.setCursor(130,26);
  int mm = mTimer/60, ss = mTimer%60;
  if(mm<10) img.print("0"); img.print(mm); img.print(":"); if(ss<10) img.print("0"); img.print(ss);

  // Bayrak sayısı göstergesi (küçük ikonlar)
  img.setCursor(4,38); img.setTextColor(M_RED); img.print("F:"); img.print(flagsPlaced);
  img.setTextColor(0x8C71); img.print("/"); img.print(MINES);
}

static void drawScreen() {
  img.fillSprite(M_BG);
  drawHUD();
  drawGrid();

  if (gameState == MS_GAMEOVER) {
    img.fillRect(20,120,132,50,0x6000);
    img.drawRect(20,120,132,50,M_RED);
    img.setTextColor(M_RED); img.setTextSize(2);
    img.setCursor(28,126); img.print("GAME OVER");
    img.setTextSize(1); img.setTextColor(M_WHITE);
    img.setCursor(30,148); img.print("SELECT = retry");
  } else if (gameState == MS_WIN) {
    img.fillRect(20,120,132,50,0x0320);
    img.drawRect(20,120,132,50,M_GREEN);
    img.setTextColor(M_GREEN); img.setTextSize(2);
    img.setCursor(40,126); img.print("YOU WIN!");
    img.setTextSize(1); img.setTextColor(M_WHITE);
    img.setCursor(30,148); img.print("SELECT = play again");
  }

  img.pushSprite(0,0);
}

// ==================== Map Seçim Ekranı ====================
static void drawMapSelect() {
  img.fillSprite(M_BG);

  // Başlık
  img.setTextColor(M_WHITE); img.setTextSize(2);
  img.setCursor(20,8); img.print("MAP SIZE");
  img.drawFastHLine(4,28,164,0x3186);

  // 4 kart
  for (int i = 0; i < NUM_MAP_CONFIGS; i++) {
    const MapConfig& mc = MAP_CONFIGS[i];
    int y = 35 + i * 68;
    bool sel = (i == selectedMap);

    // Kart zemin
    uint16_t bg = sel ? mc.themeColor : 0x18C3;
    img.fillRoundRect(8, y, 156, 60, 6, bg);
    img.drawRoundRect(7, y-1, 158, 62, 7, sel ? M_WHITE : 0x39E7);
    if (sel) img.drawRoundRect(6, y-2, 160, 64, 8, mc.themeColor);

    // Metin
    uint16_t tc = sel ? M_BLACK : M_WHITE;
    img.setTextColor(tc); img.setTextSize(2);
    img.setCursor(18, y+8); img.print(mc.name);

    img.setTextSize(1); img.setTextColor(sel ? 0x18C3 : 0x8C71);
    img.setCursor(18, y+28); img.print(mc.subtitle);

    // Grid preview (küçük ızgara)
    int previewX = 110, previewCellW = (sel?4:3);
    int previewCols = min(mc.cols, 14);
    int previewRows = min(mc.rows, 12);
    for (int r = 0; r < previewRows; r++)
      for (int c = 0; c < previewCols; c++)
        img.fillRect(previewX + c*(previewCellW+1), y+8 + r*(previewCellW+1),
                     previewCellW, previewCellW,
                     sel ? 0x18C3 : 0x39E7);

    // Difficulty bar (4 segment)
    for (int d = 0; d < 4; d++) {
      uint16_t bc = (d <= i) ? mc.themeColor : 0x2945;
      img.fillRect(18 + d*18, y+44, 14, 8, bc);
      img.drawRect(17 + d*18, y+43, 16, 10, sel ? M_WHITE : 0x39E7);
    }
  }

  // Alt bilgi
  img.setTextColor(0x39E7); img.setTextSize(1);
  img.setCursor(10,308); img.print("ENC=select  R_SHLD=start");

  img.pushSprite(0,0);
}

static void handleMapSelect() {
  // R_SHOULDER başlat
  int selNow = digitalRead(R_SHOULDER);
  if (selNow == LOW && msLastSel == HIGH) {
    COLS = MAP_CONFIGS[selectedMap].cols;
    ROWS = MAP_CONFIGS[selectedMap].rows;
    MINES = MAP_CONFIGS[selectedMap].mines;
    CELL_SIZE = MAP_CONFIGS[selectedMap].cellSize;
    gameState = MS_START;
  }
  msLastSel = selNow;

  // L_SHOULDER = bir üst
  int snapNow = digitalRead(L_SHOULDER);
  if (snapNow == LOW && msLastSnap == HIGH)
    selectedMap = (selectedMap - 1 + NUM_MAP_CONFIGS) % NUM_MAP_CONFIGS;
  msLastSnap = snapNow;

  // bck_btn = bir alt (aşağı kaydır)
  // up/dwn ile navigasyon
  int bU = digitalRead(up_btn), bD = digitalRead(dwn_btn);
  static int lastU=HIGH, lastD=HIGH;
  if (bU==LOW && lastU==HIGH) selectedMap=(selectedMap-1+NUM_MAP_CONFIGS)%NUM_MAP_CONFIGS;
  if (bD==LOW && lastD==HIGH) selectedMap=(selectedMap+1)%NUM_MAP_CONFIGS;
  lastU=bU; lastD=bD;

  drawMapSelect();
}

// ==================== Başlangıç Ekranı ====================
static void drawStartScreen() {
  img.fillSprite(M_BG);
  const MapConfig& mc = MAP_CONFIGS[selectedMap];

  img.setTextColor(mc.themeColor); img.setTextSize(2);
  img.setCursor(12,80); img.print("MINESWEEPER");
  img.setTextSize(1); img.setTextColor(M_WHITE);
  img.setCursor(30,115); img.print("Map: "); img.print(mc.name);
  img.setCursor(20,130); img.print(mc.subtitle);
  img.setTextColor(mc.themeColor);
  img.setCursor(35,165); img.print("SELECT to start");
  img.setCursor(20,185); img.setTextColor(0x8C71);
  img.print("BACK = change map");

  img.pushSprite(0,0);

  if (digitalRead(select_btn)==LOW) { delay(150); gameState=MS_GAME; }
  if (digitalRead(bck_btn)==LOW)    { delay(150); gameState=MS_MAP_SELECT; }
}

// ==================== Sıfırlama ====================
static void resetGame() {
  memset(Grid,0,sizeof(Grid));
  memset(View,0,sizeof(View));
  cursorX=COLS/2; cursorY=ROWS/2;
  flagsPlaced=0; mTimer=0; firstMove=true;
  gameState=MS_GAME;
}

// ==================== Buton Kontrolü ====================
static bool bSelP=false, bBckP=false;
static void buttonControl() {
  int bU=digitalRead(up_btn), bD=digitalRead(dwn_btn);
  int bL=digitalRead(lft_btn), bR=digitalRead(rgh_btn);
  int bB=digitalRead(bck_btn), bS=digitalRead(select_btn);
  unsigned long now=millis();

  // Yön tuşları — kayma özelliği
  int cur=-1;
  if(bU==LOW)cur=0; else if(bD==LOW)cur=1; else if(bL==LOW)cur=2; else if(bR==LOW)cur=3;
  if(cur!=-1){
    if(lastBtnPressed==-1||lastBtnPressed!=cur){
      lastBtnPressed=cur; lastBtnTime=now;
      if(cur==0)cursorY--; else if(cur==1)cursorY++; else if(cur==2)cursorX--; else cursorX++;
    } else if(now-lastBtnTime>BTN_DELAY){
      static unsigned long lr=0;
      if(now-lr>BTN_RATE){lr=now;
        if(cur==0)cursorY--; else if(cur==1)cursorY++; else if(cur==2)cursorX--; else cursorX++;
      }
    }
  } else { lastBtnPressed=-1; }

  // Sınır sarma
  if(cursorX<0)cursorX=COLS-1; if(cursorX>=COLS)cursorX=0;
  if(cursorY<0)cursorY=ROWS-1; if(cursorY>=ROWS)cursorY=0;

  // SELECT — aç
  if(bS==LOW&&!bSelP){ bSelP=true; if(gameState==MS_GAME){reveal(cursorY,cursorX);} }
  else if(bS==HIGH) bSelP=false;

  // BACK — bayrak
  if(bB==LOW&&!bBckP){ bBckP=true; if(gameState==MS_GAME){toggleFlag();} }
  else if(bB==HIGH) bBckP=false;

  // Oyun bitti → SELECT ile tekrar
  if((gameState==MS_GAMEOVER||gameState==MS_WIN)&&bS==LOW&&!bSelP){
    bSelP=true; delay(150); resetGame();
  }
}

// ==================== GameHub Arayüzü ====================
void mineSetup() {
  tft.init();
  tft.setRotation(4);
  tft.fillScreen(M_BG);
  img.createSprite(172,320);
  gameState = MS_MAP_SELECT;
  selectedMap = 3; // default LARGE
  img.pushSprite(0,0);
}

void mineUpdate() {
  unsigned long now = millis();

  if (gameState == MS_MAP_SELECT) {
    handleMapSelect();
    return;
  }
  if (gameState == MS_START) {
    drawStartScreen();
    return;
  }

  buttonControl();

  if (gameState == MS_GAME) {
    static unsigned long prevT = 0;
    if (now - prevT >= 1000) { prevT=now; mTimer++; }
    drawScreen();
  } else {
    // GAMEOVER veya WIN — ekranı güncelle
    drawScreen();
  }
}