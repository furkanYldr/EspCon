#include <TFT_eSPI.h>
#include <Wire.h>
#include <vector>

using namespace std;

// --- EKRAN ---
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite img = TFT_eSprite(&tft);

// --- TUŞLAR ---
#define BTN_RIGHT 1
#define BTN_LEFT  5
#define BTN_DROP  3

// --- OYUN AYARLARI ---
const int SCREEN_WIDTH = 172;
const int SCREEN_HEIGHT = 320;
const int DEAD_LINE = 50; 

// --- BASİT VE HIZLI FİZİK AYARLARI ---
const int SUB_STEPS = 2;        // 4 veya 8 değil, sadece 2. Hem hızlı hem yeterli.
const float GRAVITY = 0.35f;    // Yerçekimi
const float FRICTION = 0.92f;   // Sürtünme
const float WALL_BOUNCE = 0.2f; // Duvar sekmesi

int score = 0;
bool isGameOver = false;

// Tehlike Sayacı
unsigned long dangerStartTime = 0; 
bool inDanger = false;             

struct sBall {
  float px, py;       
  float vx, vy;       
  float radius;       
  unsigned int bColor;
  float mass;         
  int scale;          
  int id;             
  bool isDead = false;
};

vector<sBall> vecBalls;

float dropperX = 85;    
float moveSpeed = 5.0;  // Sağa sola gidiş hızlandı
int nextBallScale = 0;
bool canDrop = true;
unsigned long lastDropTime = 0;

// Yarıçaplar
vector<int> BallRadius = { 6, 11, 16, 21, 27, 33, 40, 48, 56 }; 
vector<unsigned int> ballColors = {
  0xBF3A, 0x218B, 0xEEF9, 0x7337, 0xFC60,
  0xB8A1, 0xD8D0, 0xEDC9, 0x4AA4
};

void addBall(float x, float y, int scaleVal) {
  sBall b;
  b.px = x;
  b.py = y;
  b.vx = 0;
  b.vy = 0;
  b.scale = scaleVal;
  
  if (b.scale >= BallRadius.size()) b.scale = BallRadius.size() - 1;
  
  b.bColor = ballColors[b.scale];
  b.radius = BallRadius[b.scale];
  b.mass = b.radius * b.radius; // Kütle hala r^2 (büyükler ağır olsun)
  
  b.id = millis() + random(0, 9999);
  b.isDead = false;
  vecBalls.emplace_back(b);
}

void setup() {
  Serial.begin(115200);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_DROP, INPUT_PULLUP);

  tft.init();
  tft.setRotation(0); 
  tft.setSwapBytes(true);
  tft.fillScreen(TFT_BLACK);

  img.createSprite(SCREEN_WIDTH, SCREEN_HEIGHT);
  img.setTextDatum(MC_DATUM);
  img.setTextColor(TFT_WHITE, TFT_BLACK);
  
  nextBallScale = random(0, 5); 
}

// --- HIZLI FİZİK MOTORU ---
void updatePhysics() {
  if (isGameOver) return;

  // Döngüyü az tutuyoruz ki işlemci yorulmasın
  float dt = 1.0f / SUB_STEPS; 

  for (int step = 0; step < SUB_STEPS; step++) {
    
    // 1. Hareket
    for (auto &ball : vecBalls) {
      if (ball.isDead) continue;

      ball.vx *= pow(FRICTION, dt); 
      ball.vy *= pow(FRICTION, dt); 
      ball.vy += GRAVITY * dt;      

      ball.px += ball.vx * dt; 
      ball.py += ball.vy * dt;

      // Duvarlar
      if (ball.px - ball.radius < 0) {
        ball.px = ball.radius;
        ball.vx *= -WALL_BOUNCE;
      } else if (ball.px + ball.radius > SCREEN_WIDTH) {
        ball.px = SCREEN_WIDTH - ball.radius;
        ball.vx *= -WALL_BOUNCE;
      }
      
      if (ball.py + ball.radius > SCREEN_HEIGHT) {
        ball.py = SCREEN_HEIGHT - ball.radius;
        ball.vy *= -WALL_BOUNCE; 
        // Yerde titremesin diye ufak hızları sıfırla
        if (abs(ball.vy) < 0.5f) ball.vy = 0;
      }
    }

    // 2. Çarpışma (Basit Yöntem - Tek geçiş)
    for (size_t i = 0; i < vecBalls.size(); i++) {
      for (size_t j = i + 1; j < vecBalls.size(); j++) {
        sBall &b1 = vecBalls[i];
        sBall &b2 = vecBalls[j];

        if (b1.isDead || b2.isDead) continue;

        float dx = b1.px - b2.px;
        float dy = b1.py - b2.py;
        float distSq = dx*dx + dy*dy;
        float minDist = b1.radius + b2.radius;

        if (distSq < minDist * minDist) {
          float dist = sqrt(distSq);
          if (dist == 0) dist = 0.1f;

          // MERGE
          if (step == 0 && b1.scale == b2.scale && b1.scale < 8) { 
             b1.isDead = true;
             b2.isDead = true;
             score += (b1.scale + 1) * 10;
             float midX = (b1.px + b2.px) / 2;
             float midY = (b1.py + b2.py) / 2;
             addBall(midX, midY, b1.scale + 1);
             continue;
          }

          // FİZİK (Basit İtme)
          float overlap = (minDist - dist);
          float nx = dx / dist;
          float ny = dy / dist;

          // Kütle oranına göre itme (Büyük olan az kaysın)
          float totalMass = b1.mass + b2.mass;
          float r1 = b2.mass / totalMass;
          float r2 = b1.mass / totalMass;
          
          // Pozisyon düzeltme (Yumuşak değil, direkt ayır)
          float separationFactor = 0.8f; // %80 ayır (Titremeyi azaltır)
          b1.px += nx * overlap * r1 * separationFactor;
          b1.py += ny * overlap * r1 * separationFactor;
          b2.px -= nx * overlap * r2 * separationFactor;
          b2.py -= ny * overlap * r2 * separationFactor;
          
          // Hız Tepkisi
          float k = 0.1f; // Zıplama katsayısı (Düşük tutuldu)
          float dvx = b1.vx - b2.vx;
          float dvy = b1.vy - b2.vy;
          float vn = dvx * nx + dvy * ny;

          if (vn < 0) {
             float impulse = -(1 + k) * vn;
             b1.vx += nx * impulse * r1;
             b1.vy += ny * impulse * r1;
             b2.vx -= nx * impulse * r2;
             b2.vy -= ny * impulse * r2;
          }
        }
      }
    }
  }

  for (int i = vecBalls.size() - 1; i >= 0; i--) {
    if (vecBalls[i].isDead) vecBalls.erase(vecBalls.begin() + i);
  }
}

void checkGameOver() {
  if (isGameOver) return;
  bool currentlyOverLine = false;
  for (auto &b : vecBalls) {
    // Dead line üstünde ve hızı düşükse
    if (b.py < DEAD_LINE && abs(b.vy) < 0.5f) {
      currentlyOverLine = true;
      break; 
    }
  }
  if (currentlyOverLine) {
    if (!inDanger) {
      inDanger = true;
      dangerStartTime = millis();
    } else {
      if (millis() - dangerStartTime > 3000) isGameOver = true;
    }
  } else {
    inDanger = false;
  }
}

void draw() {
  img.fillSprite(0x2104); 
  
  // Tehlike Çizgisi
  if (inDanger) {
    if ((millis() / 200) % 2 == 0) img.drawFastHLine(0, DEAD_LINE, SCREEN_WIDTH, TFT_YELLOW);
    else img.drawFastHLine(0, DEAD_LINE, SCREEN_WIDTH, TFT_RED);
  } else {
    img.drawFastHLine(0, DEAD_LINE, SCREEN_WIDTH, TFT_RED);
  }

  // Topları Çiz
  for (auto &ball : vecBalls) {
    img.fillCircle((int)ball.px, (int)ball.py, (int)ball.radius, ball.bColor);
    img.drawCircle((int)ball.px, (int)ball.py, (int)ball.radius, TFT_BLACK);
    
    // Basit Yüz
    if(ball.scale >= 2) {
       img.drawPixel(ball.px - 4, ball.py - 2, TFT_BLACK);
       img.drawPixel(ball.px + 4, ball.py - 2, TFT_BLACK);
    }
  }

  if (!isGameOver) {
      // Dropper Çizgisi ve Topu
      img.drawLine(dropperX, 20, dropperX, DEAD_LINE, 0x5A5A); 
      img.fillCircle(dropperX, 25, BallRadius[nextBallScale], ballColors[nextBallScale]);
      
      // Sarı Süre Barı
      if (inDanger) {
         int barWidth = map(millis() - dangerStartTime, 0, 3000, 0, SCREEN_WIDTH);
         img.fillRect(0, DEAD_LINE - 5, barWidth, 4, TFT_YELLOW);
      }
  }

  // Skor
  img.setTextSize(1);
  img.setTextColor(TFT_WHITE, 0x2104);
  img.setCursor(5, 5);
  img.printf("Skor: %d", score);

  // Game Over Ekranı
  if (isGameOver) {
    img.fillRect(20, 100, 132, 60, TFT_WHITE);
    img.drawRect(20, 100, 132, 60, TFT_RED);
    img.setTextColor(TFT_RED, TFT_WHITE);
    img.drawString("KAYBETTIN!", 86, 120);
    img.drawString("Skor: " + String(score), 86, 140);
  }

  img.pushSprite(0, 0);
}

void loop() {
  if (!isGameOver) {
    // Kontroller
    if (digitalRead(BTN_LEFT) == LOW) {
      dropperX -= moveSpeed;
      if (dropperX < 10) dropperX = 10;
    }
    if (digitalRead(BTN_RIGHT) == LOW) {
      dropperX += moveSpeed;
      if (dropperX > SCREEN_WIDTH - 10) dropperX = SCREEN_WIDTH - 10;
    }
    if (digitalRead(BTN_DROP) == LOW && canDrop) {
      // Ufak bir rastgelelik (noise) ekle ki kule oluşmasın
      float noise = random(-20, 20) / 10.0f; 
      addBall(dropperX + noise, 25, nextBallScale);
      nextBallScale = random(0, 5); // 0-4 arası rastgele boyut
      canDrop = false;
      lastDropTime = millis();
    }
    if (!canDrop && millis() - lastDropTime > 500) canDrop = true;
    
    checkGameOver();
  }

  // Fiziği artık sadece 1 kere çağırıyoruz, bu hızı 2 kat artırır.
  updatePhysics(); 
  
  draw();
}