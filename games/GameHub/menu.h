// menu.h — GameHub ana menüsü
// Encoder ile scroll, R_SHOULDER ile seç
#pragma once
#include <TFT_eSPI.h>
#include <RotaryEncoder.h>

// GameHub aktif oyun enum'u (GameHub.ino'da tanımlı)
extern int hubSelectedGame;     // seçili kart indeksi (0-8)
extern int hubLaunchGame;       // -1 = menüde, 0+ = oyun başlatılacak

// Oyun tanımları
struct GameCard {
  const char* name;
  const char* line2;
  uint16_t    accentColor;
  bool        locked;
};

static const GameCard GAMES[] = {
  { "PAC",    "MAN",    0xFFE0, false },  // 0 - Pacman
  { "MINE",   "SWEEP",  0x52AA, false },  // 1 - Minesweeper
  { "FLAPPY", "BALL",   0x07E0, false },  // 2 - Flappy Bird
  { "8 BALL", "POOL",   0x0340, false },  // 3 - 8Pool
  { "PIN",    "BALL",   0xF81F, false },  // 4 - Pinball
  { "SNAKE",  "",       0x04CC, false },  // 5 - Snake
  { "TETRIS", "",      0x001F, false },  // 6 - Tetris
  { "COLOR",  "CODE",   0xFCA0, false },  // 7 - ColorCode
  { "ZELDA",  "QUEST",  0xFC60, false },  // 8 - Zelda Macera Oyunu
};
static const int NUM_GAMES = 9;

// Menü durumu
static int  menuSelected  = 0;
static int  menuPrevEnc   = 0;
static int  menuLastBtn   = HIGH;
static int  menuLastSnap  = HIGH;
static unsigned long menuAnim = 0;

// ==================== Menü Çizimi ====================
static void drawMenuCard(int idx, bool selected) {
  int col  = idx % 3;
  int row  = idx / 3;
  int cw   = 56;   // kart genişliği
  int ch   = 86;   // kart yüksekliği
  int padX = 2;
  int padY = 2;
  int x    = col * (cw + padX) + padX;
  int y    = row * (ch + padY) + 32 + padY;

  const GameCard& g = GAMES[idx];
  uint16_t bg  = selected ? g.accentColor : 0x18C3;
  uint16_t fg  = selected ? 0x0000 : 0xCE79;

  // Kart zemin
  img.fillRoundRect(x, y, cw, ch, 6, bg);

  // Seçili animasyonu: parlak border
  if (selected) {
    img.drawRoundRect(x-1, y-1, cw+2, ch+2, 7, TFT_WHITE);
    img.drawRoundRect(x-2, y-2, cw+4, ch+4, 8, (uint16_t)(g.accentColor | 0x3186));
  } else {
    img.drawRoundRect(x, y, cw, ch, 6, 0x39E7);
  }

  // Kilitli overlay
  if (g.locked) {
    img.fillRoundRect(x+2, y+2, cw-4, ch-4, 4, 0x0861);
    img.drawRoundRect(x+2, y+2, cw-4, ch-4, 4, 0x39E7);
    img.setTextColor(0x39E7); img.setTextSize(1);
    img.setCursor(x+18, y+30); img.print("LOCK");
    // Kilit ikonu (küçük dikdörtgen)
    img.fillRect(x+20, y+46, 16, 12, 0x39E7);
    img.fillRect(x+22, y+43, 12, 6, 0x0861);
    img.fillCircle(x+28, y+43, 5, 0x39E7);
    img.fillCircle(x+28, y+43, 3, 0x0861);
  } else {
    // Oyun adı
    img.setTextColor(fg); img.setTextSize(1);
    img.setCursor(x+4, y+8); img.print(g.name);
    img.setCursor(x+4, y+18); img.print(g.line2);
    // Küçük dekorasyon çizgi
    img.drawFastHLine(x+4, y+30, cw-8, selected ? 0x0000 : g.accentColor);
    // İkon — her oyun için basit şekil
    switch(idx) {
      case 0: // Pacman - sarı daire
        img.fillCircle(x+28, y+56, 14, 0xFFE0);
        img.fillTriangle(x+28,y+56, x+42,y+48, x+42,y+64, bg);
        break;
      case 1: // Minesweeper - kare grid
        for(int r=0;r<3;r++) for(int c=0;c<3;c++) {
          img.fillRect(x+12+c*12,y+44+r*12,10,10,0x52AA);
          img.drawRect(x+12+c*12,y+44+r*12,10,10,0x18C3);
        }
        img.fillCircle(x+22,y+56,3,0xF800); // mayın
        break;
      case 2: // Flappy - küçük daire + sütunlar
        img.fillRect(x+38,y+42,6,15,0x2444); img.fillRect(x+38,y+67,6,15,0x2444);
        img.fillRect(x+14,y+42,6,10,0x2444); img.fillRect(x+14,y+66,6,10,0x2444);
        img.fillCircle(x+25,y+56,6,TFT_YELLOW);
        break;
      case 3: // 8 Pool - yeşil dikdörtgen + toplar
        img.fillRect(x+8,y+42,40,28,0x0340);
        img.drawRect(x+8,y+42,40,28,0x03E0);
        img.fillCircle(x+22,y+56,4,TFT_WHITE);
        img.fillCircle(x+36,y+52,3,0xF800);
        img.fillCircle(x+32,y+62,3,0xFFE0);
        img.fillCircle(x+28,y+56,3,0x0000);
        break;
      case 4: // Pinball - küçük top + flipper
        img.drawLine(x+10,y+68,x+26,y+58,TFT_ORANGE);
        img.drawLine(x+46,y+68,x+30,y+58,TFT_ORANGE);
        img.fillCircle(x+28,y+48,5,TFT_WHITE);
        break;
      case 5: // Snake - yılan kare dizisi
        for(int s=0;s<4;s++) img.fillRect(x+8+s*10,y+50,8,8,TFT_DARKCYAN);
        img.fillRect(x+8,y+60,8,8,TFT_DARKCYAN);
        img.fillRect(x+8,y+70,8,8,TFT_DARKCYAN);
        img.fillCircle(x+32,y+42,3,TFT_YELLOW); // yiyecek
        break;
      case 6: // Tetris - Tetris Blokları
        img.fillRect(x+14, y+52, 16, 8, 0xF800); // Kırmızı Yatay
        img.fillRect(x+22, y+44, 8, 8, 0xF800);  // Kırmızı Tık
        img.fillRect(x+22, y+52, 16, 8, 0x07E0); // Yeşil Yatay
        img.fillRect(x+22, y+60, 8, 8, 0x07E0);
        break;
      case 7: // ColorCode - Renk boncukları
        img.fillCircle(x+16, y+56, 6, 0xF800); // Kırmızı
        img.fillCircle(x+28, y+48, 6, 0xFFE0); // Sarı
        img.fillCircle(x+40, y+56, 6, 0x07E0); // Yeşil
        break;
      case 8: // Zelda - Triforce
        img.fillTriangle(x+28, y+40, x+16, y+64, x+40, y+64, 0xFDE0); // Dış Altın Üçgen
        img.fillTriangle(x+28, y+64, x+22, y+52, x+34, y+52, bg);    // İç Oyuk Üçgen (Arkaplan rengi)
        break;
    }
    // Numara
    img.setTextColor(selected ? 0x4208 : 0x4A49);
    img.setTextSize(1); img.setCursor(x+cw-14, y+ch-10);
    img.print(idx+1);
  }
}

static void menuSetup() {
  menuSelected = 0;
  menuPrevEnc  = 0;
  menuAnim     = 0;
}

static void menuDraw() {
  // Arkaplan
  img.fillSprite(0x0841);

  // Başlık bar
  img.fillRect(0, 0, 172, 30, 0x0000);
  img.drawFastHLine(0, 30, 172, 0x3186);
  img.setTextColor(TFT_YELLOW); img.setTextSize(2);
  img.setCursor(18, 6); img.print("GAME");
  img.setTextColor(TFT_WHITE); img.setTextSize(2);
  img.setCursor(76, 6); img.print("HUB");
  // Küçük joystick ikonu
  img.fillCircle(155, 15, 9, 0x18C3);
  img.fillCircle(155, 15, 5, TFT_YELLOW);

  // Tüm kartlar
  for (int i = 0; i < NUM_GAMES; i++) {
    drawMenuCard(i, i == menuSelected);
  }

  // Alt bilgi
  img.fillRect(0, 307, 172, 13, 0x0000);
  img.setTextColor(0x8410); img.setTextSize(1);
  img.setCursor(2, 309);
  if (!GAMES[menuSelected].locked) {
    img.print(">> "); img.print(GAMES[menuSelected].name);
    img.print(" "); img.print(GAMES[menuSelected].line2);
    img.print(" <<");
  } else {
    img.print("   --- LOCKED ---");
  }

  img.pushSprite(0, 0);
}

static int menuUpdate(RotaryEncoder& enc) {
  // Encoder navigasyon
  enc.tick();
  int newPos = enc.getPosition();
  if (newPos != menuPrevEnc) {
    int delta = newPos - menuPrevEnc;
    menuSelected = (menuSelected + delta + NUM_GAMES) % NUM_GAMES;
    menuPrevEnc  = newPos;
  }

  // D-pad yön butonları ile navigasyon (3x3 grid)
  static bool menuLastUp=HIGH, menuLastDwn=HIGH, menuLastLft=HIGH, menuLastRgh=HIGH;
  int bU = digitalRead(up_btn);
  int bD = digitalRead(dwn_btn);
  int bL = digitalRead(lft_btn);
  int bR = digitalRead(rgh_btn);

  if (bU == LOW && menuLastUp == HIGH) {
    menuSelected = (menuSelected - 3 + NUM_GAMES) % NUM_GAMES;
  }
  if (bD == LOW && menuLastDwn == HIGH) {
    menuSelected = (menuSelected + 3) % NUM_GAMES;
  }
  if (bL == LOW && menuLastLft == HIGH) {
    menuSelected = (menuSelected - 1 + NUM_GAMES) % NUM_GAMES;
  }
  if (bR == LOW && menuLastRgh == HIGH) {
    menuSelected = (menuSelected + 1) % NUM_GAMES;
  }
  menuLastUp = bU; menuLastDwn = bD; menuLastLft = bL; menuLastRgh = bR;

  // SELECT (select_btn) — oyunu başlat
  int btnNow = digitalRead(select_btn);
  if (btnNow == LOW && menuLastBtn == HIGH) {
    if (!GAMES[menuSelected].locked) {
      menuLastBtn = btnNow;
      return menuSelected;  // seçilen oyun index'i döner
    }
  }
  menuLastBtn = btnNow;

  menuDraw();
  return -1;  // hala menüde
}
