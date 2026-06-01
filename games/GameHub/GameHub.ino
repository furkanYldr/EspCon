// ============================================================
//  GAME HUB — ESP32 / TFT_eSPI
//  Tüm oyunlar tek projede — encoder + butonlarla seçilir
//
//  Pin Düzeni:
//    Encoder : 47 (A), 48 (B)
//    up_btn  : 4    dwn_btn: 3
//    lft_btn : 5    rgh_btn: 1
//    bck_btn : 6    select : 7
//    R_SHLD  : 2  (sağ shoulder — seç / vur)
//    L_SHLD  : 39 (sol shoulder — hızlı adım)
//
//  Menüye dönüş: bck_btn (6) 1 saniye basılı tut
// ============================================================

#include <TFT_eSPI.h>
#include <RotaryEncoder.h>
#include <vector>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <map>
#include <utility>
#include <queue>
#include <algorithm>
#include <ctime>
using namespace std;

// ==================== Paylaşımlı Donanım ====================
// TEK tft ve img tanımı — tüm oyunlar bu global nesneleri kullanır
TFT_eSPI     tft = TFT_eSPI();
TFT_eSprite  img = TFT_eSprite(&tft);

// TEK encoder tanımı — menü + 8pool paylaşır
RotaryEncoder hubEncoder(48, 47, RotaryEncoder::LatchMode::TWO03);

// ==================== Pin Tanımları ====================
#include "pins.h"

// ==================== Oyun Wrapperları ====================
#include "game_pacman_res.h"   // pacman resource (game_pacman.h'den önce!)
#include "game_pacman.h"       // namespace Pacman
#include "game_mines.h"        // namespace Mines
#include "game_flappy.h"       // namespace Flappy
#include "game_8pool.h"        // namespace Pool
#include "game_pinball.h"      // namespace Pinball
#include "game_snake.h"        // namespace Snake
#include "game_tetris.h"       // namespace Tetris
#include "game_colorcode.h"    // namespace ColorCode
#include "game_zelda.h"        // namespace Zelda

// ==================== Menü ====================
int hubSelectedGame = -1;
int hubLaunchGame   = -1;
#include "menu.h"

// ==================== Aktif Oyun ====================
enum ActiveGame {
  AG_MENU      = -1,
  AG_PACMAN    =  0,
  AG_MINES     =  1,
  AG_FLAPPY    =  2,
  AG_POOL      =  3,
  AG_PINBALL   =  4,
  AG_SNAKE     =  5,
  AG_TETRIS    =  6,
  AG_COLORCODE =  7,
  AG_ZELDA     =  8,
};
static int activeGame = AG_MENU;

// Menüye dön — bck_btn uzun basış
static unsigned long bckHoldStart  = 0;
static bool          bckHolding    = false;
static const unsigned long BCK_HOLD_MS = 1000;

// ==================== Oyun Başlatma ====================
static void launchGame(int idx) {
  activeGame = idx;
  switch (idx) {
    case AG_PACMAN:    Pacman::pacmanSetup();          break;
    case AG_MINES:     Mines::mineSetup();             break;
    case AG_FLAPPY:    Flappy::flappySetup();          break;
    case AG_POOL:      Pool::poolSetup(&hubEncoder);   break;
    case AG_PINBALL:   Pinball::pinballSetup();        break;
    case AG_SNAKE:     Snake::snakeSetup();            break;
    case AG_TETRIS:    Tetris::setup();                break;
    case AG_COLORCODE: ColorCode::setup();             break;
    case AG_ZELDA:     Zelda::setup();                 break;
  }
}

static void returnToMenu() {
  // Zelda interrupt'larını temizle (menüde kilitlenme olmaması için)
  detachInterrupt(digitalPinToInterrupt(48));
  detachInterrupt(digitalPinToInterrupt(47));

  activeGame = AG_MENU;
  // Portrait moda geri dön (ColorCode landscape'ten sonra da doğru çalışır)
  tft.init();
  tft.setRotation(4);
  tft.setSwapBytes(true);
  tft.fillScreen(TFT_BLACK);
  img.deleteSprite();
  img.createSprite(172, 320);
  img.setTextDatum(0);
  menuSetup();
}

// ==================== SETUP ====================
void setup() {
  // Güç pini (varsa)
  pinMode(15, OUTPUT);
  digitalWrite(15, 1);

  Serial.begin(115200);

  // Tüm buton pinleri
  pinMode(up_btn,     INPUT_PULLUP);
  pinMode(dwn_btn,    INPUT_PULLUP);
  pinMode(lft_btn,    INPUT_PULLUP);
  pinMode(rgh_btn,    INPUT_PULLUP);
  pinMode(bck_btn,    INPUT_PULLUP);
  pinMode(select_btn, INPUT_PULLUP);
  pinMode(R_SHOULDER, INPUT_PULLUP);
  pinMode(L_SHOULDER, INPUT_PULLUP);
  pinMode(ENC_A,      INPUT_PULLUP);
  pinMode(ENC_B,      INPUT_PULLUP);

  // TFT başlat
  tft.init();
  tft.setRotation(4);
  tft.setSwapBytes(true);
  tft.fillScreen(TFT_BLACK);

  // Sprite oluştur
  img.createSprite(172, 320);
  img.setTextDatum(0);

  // Menüyü başlat
  menuSetup();

  // Açılış animasyonu
  tft.fillScreen(0x0841);
  tft.setTextColor(TFT_YELLOW);
  tft.setTextSize(3);
  tft.setCursor(20, 120);
  tft.print("GAME HUB");
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.setCursor(30, 170);
  tft.print("Loading games...");
  delay(800);
}

// ==================== LOOP ====================
void loop() {
  // ---- Menüye dönüş: bck_btn uzun basış ----
  if (activeGame != AG_MENU) {
    int bckNow = digitalRead(bck_btn);
    if (bckNow == LOW) {
      if (!bckHolding) { bckHolding = true; bckHoldStart = millis(); }
      else if (millis() - bckHoldStart >= BCK_HOLD_MS) {
        bckHolding = false;
        returnToMenu();
        return;
      }
    } else {
      bckHolding = false;
    }
  }

  // ---- Oyun veya menü güncelle ----
  switch (activeGame) {
    case AG_MENU: {
      int sel = menuUpdate(hubEncoder);
      if (sel >= 0) {
        launchGame(sel);
      }
      break;
    }
    case AG_PACMAN:    Pacman::pacmanUpdate();   break;
    case AG_MINES:     Mines::mineUpdate();      break;
    case AG_FLAPPY:    Flappy::flappyUpdate();   break;
    case AG_POOL:      Pool::poolUpdate();       break;
    case AG_PINBALL:   Pinball::pinballUpdate(); break;
    case AG_SNAKE:     Snake::snakeUpdate();     break;
    case AG_TETRIS:    Tetris::tetUpdate();      break;
    case AG_COLORCODE: ColorCode::ccUpdate();    break;
    case AG_ZELDA:     Zelda::update();          break;
  }
}
