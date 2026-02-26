#include "mineSweeper.h"
#include "esp32-hal-gpio.h"
#include <cmath>
#include <vector>

// --- Nesne Tanımları ---
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite img = TFT_eSprite(&tft);

// Global Oyun Durumu
int gameState = start; 

// --- Değişkenler ---
unsigned long previousMillisMove = 0;
int timer = 0;

// Oyun İçi Değişkenler
int cursorX = 0;
int cursorY = 0;

// --- ZORLUK AYARI ---
// 50 mayın (~%15 yoğunluk) ideal zorluktur.
int totalMines = 50; 

int flagsPlaced = 0;
bool firstMove = true;

// --- BUTON ZAMANLAMA DEĞİŞKENLERİ (KAYMA İÇİN) ---
unsigned long lastButtonPressTime = 0;
unsigned long buttonRepeatDelay = 300; // Basılı tutunca ne kadar süre sonra kaymaya başlasın (ms)
unsigned long buttonRepeatRate = 80;   // Kayma hızı (ms)
int lastBtnPressed = -1; // 0:UP, 1:DOWN, 2:LEFT, 3:RIGHT

// Gridler: (Grid=Mayınlar/Sayılar, View=Görünüm)
uint8_t Grid[areaRow][areaColmn];
uint8_t View[areaRow][areaColmn];

// --- FONKSİYON PROTOTİPLERİ ---
void buttonControl();
void handleMovement(int direction);
void startTAB();
void drawInit();
void drawGrid();
void placeMines(int firstR, int firstC);
void calculateNumbers();
void reveal(int r, int c);
void toggleFlag();
void checkWin();

// --- KURULUM (SETUP) ---
void mineSetup() {
  tft.init();
  tft.setRotation(4);
  
  // Pin Modları
  pinMode(bck_btn, INPUT_PULLUP);
  pinMode(lft_btn, INPUT_PULLUP);
  pinMode(rgh_btn, INPUT_PULLUP);
  pinMode(up_btn, INPUT_PULLUP);
  pinMode(dwn_btn, INPUT_PULLUP);
  pinMode(select_btn, INPUT_PULLUP);
  
  tft.fillScreen(0x0130); // Arkaplan rengi
  img.createSprite(172, 320);
  
  // Değişkenleri Sıfırla
  memset(Grid, 0, sizeof(Grid));
  memset(View, 0, sizeof(View));
  cursorX = areaColmn / 2;
  cursorY = areaRow / 2;
  flagsPlaced = 0;
  timer = 0;
  firstMove = true;
  gameState = start;
  
  img.pushSprite(0, 0);
}

// --- DÖNGÜ (LOOP) ---
void mineUpdate() {
  unsigned long currentMillis = millis();
  
  // Tuşları kontrol et
  buttonControl();

  if (gameState == start) {
    startTAB();
  } 
  else if (gameState == game) {
    
    // Zamanlayıcı (Oyun süresi)
    static unsigned long previousMillisTime = 0;
    if (currentMillis - previousMillisTime >= 1000) {
        previousMillisTime = currentMillis;
        timer++;
        drawInit();
    } 
  } 
  else if (gameState == gameOver || gameState == win) {
     drawInit(); 
     // Oyun bittiyse Select ile yeniden başlat
     if(digitalRead(select_btn) == LOW) {
         delay(200); // Debounce
         mineSetup(); 
         gameState = game;
     }
  }
}

// --- GİRİŞ EKRANI ---
void startTAB() {
  img.fillSprite(DARK_GREY);
  
  img.setTextSize(2);
  img.setTextColor(yellow);
  img.setCursor(20, 100);
  img.print("MINESWEEPER");
  
  img.setTextSize(1);
  img.setTextColor(white);
  img.setCursor(40, 140);
  img.print("Press SELECT");
  
  img.pushSprite(0, 0);

  if (digitalRead(select_btn) == LOW) {
    delay(200);
    gameState = game;
    drawInit();
  }
}

// --- ÇİZİM FONKSİYONLARI ---
void drawInit() {
  img.fillSprite(0xA615);  // Arkaplan
  img.fillRect(0, 0, 172, 50, DARK_GREY);   // Üst bar
  img.fillRect(0, 250, 172, 70, DARK_GREY); // Alt bar
  
  img.setCursor(41, 10);
  img.setTextSize(2);
  img.setTextColor(yellow);
  img.print("MINES");

  img.setCursor(9, 250);
  img.setTextSize(1);
  img.setTextColor(white);
  
  if (gameState == gameOver) {
      img.setTextColor(red);
      img.print("GAME OVER");
  } else if (gameState == win) {
      img.setTextColor(green);
      img.print("YOU WIN!");
  } else {
      img.print("Mines: ");
      img.print(totalMines - flagsPlaced);
      img.setCursor(100, 250);
      img.print("Time: ");
      img.print(timer);
  }

  drawGrid();
  img.pushSprite(0, 0);
}

void drawGrid() {
  // Sınırlar
  if (cursorX >= areaColmn) cursorX = 0;
  if (cursorX < 0) cursorX = areaColmn - 1;
  if (cursorY >= areaRow) cursorY = 0;
  if (cursorY < 0) cursorY = areaRow - 1;

  // Grid Çizimi
  for (int i = 0; i < areaRow; i++) {
    for (int j = 0; j < areaColmn; j++) {
      int Y = i * CELL_SIZE + 50;
      int X = j * CELL_SIZE + 1;
      
      // Hücre Çizimi
      if (View[i][j] == 1) { // AÇIK
          img.fillRect(X + 1, Y + 1, CELL_SIZE - 2, CELL_SIZE - 2, 0xDEDB); 
          
          if (Grid[i][j] == 9) { // Mayın
             img.fillCircle(X + CELL_SIZE/2, Y + CELL_SIZE/2, 3, black);
          } else if (Grid[i][j] > 0) { // Sayı
             if(Grid[i][j]==1) img.setTextColor(blue);
             else if(Grid[i][j]==2) img.setTextColor(green);
             else if(Grid[i][j]==3) img.setTextColor(red);
             else if(Grid[i][j]==4) img.setTextColor(purple);
             else img.setTextColor(black);
             
             img.setCursor(X + 3, Y + 2);
             img.setTextSize(1);
             img.print(Grid[i][j]);
          }
      } 
      else if (View[i][j] == 2) { // BAYRAK
          img.fillRect(X + 1, Y + 1, CELL_SIZE - 2, CELL_SIZE - 2, GREY);
          img.fillCircle(X + CELL_SIZE/2, Y + CELL_SIZE/2, 2, red);
      } 
      else { // KAPALI
          img.fillRect(X + 1, Y + 1, CELL_SIZE - 2, CELL_SIZE - 2, GREY);
      }

      // İmleç (Sarı Çerçeve)
      if (i == cursorY && j == cursorX) {
        img.drawRect(X, Y, CELL_SIZE, CELL_SIZE, yellow);
        img.drawRect(X+1, Y+1, CELL_SIZE-2, CELL_SIZE-2, yellow);
      }
    }
  }
}

// --- OYUN MANTIĞI ---
void reveal(int r, int c) {
    if (r < 0 || r >= areaRow || c < 0 || c >= areaColmn || View[r][c] != 0) return;

    if (firstMove) {
        placeMines(r, c);
        calculateNumbers();
        firstMove = false;
    }

    View[r][c] = 1; // Aç

    if (Grid[r][c] == 9) {
        gameState = gameOver;
        // Tüm mayınları göster
        for(int i=0; i<areaRow; i++)
            for(int j=0; j<areaColmn; j++)
                if(Grid[i][j] == 9) View[i][j] = 1;
    } else if (Grid[r][c] == 0) {
        // Boşsa etrafını aç (Flood Fill)
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                reveal(r + i, c + j);
            }
        }
    }
    checkWin();
}

void toggleFlag() {
    if (View[cursorY][cursorX] == 0) {
        View[cursorY][cursorX] = 2;
        flagsPlaced++;
    } else if (View[cursorY][cursorX] == 2) {
        View[cursorY][cursorX] = 0;
        flagsPlaced--;
    }
}

// --- MAYIN YERLEŞTİRME (GÜVENLİ BAŞLANGIÇ) ---
void placeMines(int firstR, int firstC) {
    int count = 0;
    int attempts = 0;
    while(count < totalMines && attempts < 5000) {
        int r = rand() % areaRow;
        int c = rand() % areaColmn;
        
        // İlk tıklanan karenin etrafındaki 3x3 alana mayın koyma
        bool isSafeZone = (abs(r - firstR) <= 1 && abs(c - firstC) <= 1);
        
        if (Grid[r][c] != 9 && !isSafeZone) {
            Grid[r][c] = 9;
            count++;
        }
        attempts++;
    }
}

void calculateNumbers() {
    for(int i=0; i<areaRow; i++){
        for(int j=0; j<areaColmn; j++){
            if(Grid[i][j] == 9) continue;
            int n = 0;
            for(int di=-1; di<=1; di++){
                for(int dj=-1; dj<=1; dj++){
                    int ni = i+di, nj = j+dj;
                    if(ni>=0 && ni<areaRow && nj>=0 && nj<areaColmn && Grid[ni][nj]==9) n++;
                }
            }
            Grid[i][j] = n;
        }
    }
}

void checkWin() {
    int covered = 0;
    for(int i=0; i<areaRow; i++)
        for(int j=0; j<areaColmn; j++)
            if(View[i][j] == 0 || View[i][j] == 2) covered++;
    if(covered == totalMines) gameState = win;
}

// --- HAREKET SİSTEMİ ---
void handleMovement(int direction) {
    // 0=UP, 1=DOWN, 2=LEFT, 3=RIGHT
    if (direction == 0) cursorY--;
    else if (direction == 1) cursorY++;
    else if (direction == 2) cursorX--;
    else if (direction == 3) cursorX++;
    drawInit(); 
}

// --- GÜNCELLENMİŞ BUTON KONTROL (KAYMA ÖZELLİĞİ) ---
bool selectPressed = false;
bool backPressed = false;

void buttonControl() {
  int btnDOWN = digitalRead(dwn_btn);
  int btnUP = digitalRead(up_btn);
  int btnRIGHT = digitalRead(rgh_btn);
  int btnLEFT = digitalRead(lft_btn);
  int btnBACK = digitalRead(bck_btn);   
  int btnSELECT = digitalRead(select_btn); 
  
  unsigned long currentMillis = millis();

  // --- YÖN TUŞLARI (SÜREKLİ BASMA MANTIĞI) ---
  int currentBtn = -1; 
  if (btnUP == LOW) currentBtn = 0;
  else if (btnDOWN == LOW) currentBtn = 1;
  else if (btnLEFT == LOW) currentBtn = 2;
  else if (btnRIGHT == LOW) currentBtn = 3;

  if (currentBtn != -1) {
    if (lastBtnPressed == -1 || lastBtnPressed != currentBtn) {
      // Tuşa YENİ basıldı (İlk hareket)
      lastBtnPressed = currentBtn;
      lastButtonPressTime = currentMillis;
      if(gameState == game) handleMovement(currentBtn);
    } 
    else if (currentMillis - lastButtonPressTime > buttonRepeatDelay) {
       // Tuş HALA basılı ve bekleme süresi geçti (Hızlı kayma)
       static unsigned long lastRepeatTime = 0;
       if (currentMillis - lastRepeatTime > buttonRepeatRate) {
           lastRepeatTime = currentMillis;
           if(gameState == game) handleMovement(currentBtn);
       }
    }
  } else {
    lastBtnPressed = -1; // Tuş bırakıldı
  }

  // --- AKSİYON TUŞLARI ---
  
  // SELECT
  if (btnSELECT == LOW && !selectPressed) {
    selectPressed = true;
    if (gameState == game) {
        reveal(cursorY, cursorX);
        drawInit();
    }
  } else if (btnSELECT == HIGH) {
    selectPressed = false;
  }

  // FLAG
  if (btnBACK == LOW && !backPressed) {
    backPressed = true;
    if (gameState == game) {
        toggleFlag();
        drawInit();
    }
  } else if (btnBACK == HIGH) {
    backPressed = false;
  }
}