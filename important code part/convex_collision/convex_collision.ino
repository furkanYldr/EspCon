#include <TFT_eSPI.h>
#include <Wire.h>
#include <vector>
#include <algorithm>
#include <math.h>

using namespace std;

// Eğer INFINITY tanımlı değilse tanımla
#ifndef INFINITY
#define INFINITY (1e9)
#endif

// ====== BUTON PINLERİ ======
#define up_btn     1
#define dwn_btn    5
#define lft_btn    4
#define rgh_btn    3
#define bck_btn    6   
#define select_btn 7   

// ====== ENCODER PINLERİ ======
#define ENCODER_CLK 48
#define ENCODER_DT  47

// *** ÖNEMLİ: Interrupt içinde değişen değişkenler 'volatile' olmalıdır ***
volatile int counter = 0;
volatile int lastEncoded = 0; // Encoder durumu için

// ====== TFT ======
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite img = TFT_eSprite(&tft);

// ====== ZAMANLAMA ======
unsigned long previousMillis = 0;
const unsigned long interval = 16;   // ~60 FPS

// ====== INPUT DURUMLARI ======
bool UP = false;
bool DOWN = false;
bool LEFT = false;
bool RIGHT = false;
bool SELECT1 = false;   
bool SELECT2 = false;   

// Aktif shape index (0 veya 1)
int activeShapeIndex = 0;

// ====== VEC / POLYGON YAPILARI ======
struct vec2d {
  float x;
  float y;
};

struct polygon {
  vector<vec2d> p;   // Transform edilmiş noktalar
  vec2d pos;         // Shape pozisyonu
  float angle;       // Yön (radyan)
  vector<vec2d> o;   // Model (orijinal local noktalar)
  bool overlap = false; // Çarpışma flag'i
};

vector<polygon> vecShapes;
int nMode = 0;  // 0: SAT, 1: SAT/STATIC, 2: DIAG, 3: DIAG/STATIC

// ====== İLERİ DEKLARASYONLAR ======
void buttonControl();
void updateGame(float fElapsedTime);
void updatePolygonsAndCollisions();
void Draw();

// ==== Collision fonksiyonları ====
bool ShapeOverlap_SAT(polygon &r1, polygon &r2);
bool ShapeOverlap_SAT_STATIC(polygon &r1, polygon &r2);
bool ShapeOverlap_DIAGS(polygon &r1, polygon &r2);
bool ShapeOverlap_DIAGS_STATIC(polygon &r1, polygon &r2);

// ==========================================
//  ENCODER INTERRUPT FONKSİYONU (YENİ)
// ==========================================
// IRAM_ATTR, fonksiyonun RAM'de çalışmasını sağlar (daha hızlıdır ve kilitlenmeyi önler)
void IRAM_ATTR readEncoderISR() {
  // En basit ve kararlı yöntemlerden biri (State Table mantığına benzer basitleştirilmiş)
  int MSB = digitalRead(ENCODER_CLK); // Most Significant Bit
  int LSB = digitalRead(ENCODER_DT);  // Least Significant Bit

  int encoded = (MSB << 1) | LSB; // İki pini birleştir
  int sum = (lastEncoded << 2) | encoded; // Eski durumla yeni durumu birleştir

  // Bu binary desenlere göre yönü belirle
  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) counter++;
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) counter--;

  lastEncoded = encoded; // Durumu kaydet
}

void setup() {
  Serial.begin(115200);

  // Butonları INPUT_PULLUP yap
  pinMode(up_btn,     INPUT_PULLUP);
  pinMode(dwn_btn,    INPUT_PULLUP);
  pinMode(lft_btn,    INPUT_PULLUP);
  pinMode(rgh_btn,    INPUT_PULLUP);
  pinMode(bck_btn,    INPUT_PULLUP);
  pinMode(select_btn, INPUT_PULLUP);

  // ===== ENCODER AYARLARI (DÜZELTİLDİ) =====
  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT,  INPUT_PULLUP);

  // Interrupt'ları bağla (CHANGE: sinyal değiştiği an tetiklenir)
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), readEncoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_DT),  readEncoderISR, CHANGE);

  Serial.println("Encoder test basladi (Interrupt Version)...");

  // TFT
  tft.init();
  tft.setRotation(4);
  tft.setSwapBytes(true);
  tft.fillScreen(TFT_BLUE);

  img.createSprite(170, 320);
  img.setTextDatum(TL_DATUM);
  img.setTextColor(TFT_WHITE, TFT_BLUE);

  // ====== Pentagon (Shape 0) ======
  polygon s1;
  float fTheta = 3.14159f * 2.0f / 5.0f;
  s1.pos = { 80.0f, 80.0f };
  s1.angle = 0.0f;
  for (int i = 0; i < 5; i++) {
    float x = 30.0f * cosf(fTheta * i);
    float y = 30.0f * sinf(fTheta * i);
    s1.o.push_back({ x, y });
    s1.p.push_back({ x, y });
  }

  // ====== Üçgen (Shape 1) ======
  polygon s2;
  fTheta = 3.14159f * 2.0f / 3.0f;
  s2.pos = { 120.0f, 150.0f };
  s2.angle = 0.0f;
  for (int i = 0; i < 3; i++) {
    float x = 20.0f * cosf(fTheta * i);
    float y = 20.0f * sinf(fTheta * i);
    s2.o.push_back({ x, y });
    s2.p.push_back({ x, y });
  }

  // ====== Kare (Shape 2, statik engel gibi) ======
  polygon s3;
  s3.pos = { 60.0f, 230.0f };
  s3.angle = 0.0f;
  s3.o.push_back({ -30, -30 });
  s3.o.push_back({ -30, +30 });
  s3.o.push_back({ +30, +30 });
  s3.o.push_back({ +30, -30 });
  s3.p.resize(4);

  vecShapes.push_back(s1);  // 0
  vecShapes.push_back(s2);  // 1
  vecShapes.push_back(s3);  // 2 (statik engel)

  img.pushSprite(0, 0);
}

void loop() {
  // ARTIK BURADA ENCODER OKUMUYORUZ.
  // Encoder okuması arka planda otomatik yapılıyor.
  
  // Sadece debug için yazdıralım (değer değişmişse)
  static int prevCounterPrint = 0;
  if (counter != prevCounterPrint) {
      Serial.print("Encoder Counter: ");
      Serial.println(counter);
      prevCounterPrint = counter;
  }

  buttonControl();  // butonları oku
  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    float fElapsedTime = (currentMillis - previousMillis) / 1000.0f;
    previousMillis = currentMillis;

    updateGame(fElapsedTime);         // input -> pos/angle
    updatePolygonsAndCollisions();    // p[] güncelle + collision test
    Draw();                           // ekrana çiz
  }
}

// ======================
//  BUTON OKUMA
// ======================
void buttonControl() {
  int btnDOWN   = digitalRead(dwn_btn);
  int btnUP     = digitalRead(up_btn);
  int btnRIGHT  = digitalRead(rgh_btn);
  int btnLEFT   = digitalRead(lft_btn);
  int btnPAUSE  = digitalRead(bck_btn);
  int btnSELECT = digitalRead(select_btn);

  UP      = (btnUP     == LOW);
  DOWN    = (btnDOWN   == LOW);
  RIGHT   = (btnRIGHT  == LOW);
  LEFT    = (btnLEFT   == LOW);
  SELECT2 = (btnPAUSE  == LOW);   // Mode
  SELECT1 = (btnSELECT == LOW);   // Shape değiştirme (tıklama)
}

// ======================
//  INPUT → SHAPE HAREKET
// ======================
void updateGame(float fElapsedTime) {
  static bool prevSelect2 = false;
  if (SELECT2 && !prevSelect2) {
    nMode = (nMode + 1) % 4;   
    Serial.print("Mode: ");
    Serial.println(nMode);
  }
  prevSelect2 = SELECT2;

  static bool prevSelect1 = false;
  if (SELECT1 && !prevSelect1) {
    activeShapeIndex++;
    if (activeShapeIndex > 1) activeShapeIndex = 0;
    Serial.print("Active Shape: ");
    Serial.println(activeShapeIndex);
  }
  prevSelect1 = SELECT1;

  if (activeShapeIndex < 0 || activeShapeIndex >= (int)vecShapes.size()) return;
  polygon &s = vecShapes[activeShapeIndex];

  const float moveSpeed = 60.0f;  // px/s
  const float stepAngle = 0.15f;  // ÇOK ÖNEMLİ: Interrupt çok hassastır, bu değeri düşürdüm.

  // ====== COUNTER KULLANIMI ======
  // Interrupt çok hızlı sayabilir (her "tık" için 4 değer artabilir)
  // Bu yüzden division (/ 4 veya / 2) yaparak yumuşatabilirsin.
  static int prevCounter = 0;
  
  // Anlık counter değerini yerel değişkene al (işlem sırasında değişmesin diye)
  int currentCounter = counter; 
  int delta = currentCounter - prevCounter;
  
  // Eğer çok hızlı dönüyorsa shape uçmasın diye yumuşatma
  if (delta != 0) {
      // Not: Interrupt yöntemi her tıkta 2 veya 4 artış verebilir, bu normaldir.
      // Bu yüzden delta'yı doğrudan açıya ekliyoruz.
      s.angle += delta * stepAngle;
      prevCounter = currentCounter;
  }

  // D-pad
  if (UP) {
    s.pos.x += cosf(s.angle) * moveSpeed * fElapsedTime;
    s.pos.y += sinf(s.angle) * moveSpeed * fElapsedTime;
  }
  if (DOWN) {
    s.pos.x -= cosf(s.angle) * moveSpeed * fElapsedTime;
    s.pos.y -= sinf(s.angle) * moveSpeed * fElapsedTime;
  }
}

// ======================
//  POLYGON GÜNCELLEME + COLLISION
// ======================
void updatePolygonsAndCollisions() {
  for (auto &r : vecShapes) {
    for (int i = 0; i < (int)r.o.size(); i++) {
      r.p[i] = {
        (r.o[i].x * cosf(r.angle)) - (r.o[i].y * sinf(r.angle)) + r.pos.x,
        (r.o[i].x * sinf(r.angle)) + (r.o[i].y * cosf(r.angle)) + r.pos.y
      };
    }
    r.overlap = false; 
  }

  for (int m = 0; m < (int)vecShapes.size(); m++) {
    for (int n = m + 1; n < (int)vecShapes.size(); n++) {
      switch (nMode) {
        case 0: vecShapes[m].overlap |= ShapeOverlap_SAT(vecShapes[m], vecShapes[n]); break;
        case 1: vecShapes[m].overlap |= ShapeOverlap_SAT_STATIC(vecShapes[m], vecShapes[n]); break;
        case 2: vecShapes[m].overlap |= ShapeOverlap_DIAGS(vecShapes[m], vecShapes[n]); break;
        case 3: vecShapes[m].overlap |= ShapeOverlap_DIAGS_STATIC(vecShapes[m], vecShapes[n]); break;
      }
    }
  }
}

// ======================
//  ÇİZİM
// ======================
void Draw() {
  img.fillSprite(TFT_BLUE);

  img.setTextDatum(TL_DATUM);
  img.setTextColor(TFT_YELLOW, TFT_BLUE);
  img.setCursor(4, 4);
  img.printf("SAT:%s",  (nMode == 0 ? "*" : " "));
  img.setCursor(4, 16);
  img.printf("SAT S:%s", (nMode == 1 ? "*" : " "));
  img.setCursor(4, 28);
  img.printf("DIAG:%s", (nMode == 2 ? "*" : " "));
  img.setCursor(4, 40);
  img.printf("DIAG S:%s", (nMode == 3 ? "*" : " "));

  img.setCursor(4, 60);
  img.printf("CTRL: S%d", activeShapeIndex);

  for (auto &r : vecShapes) {
    uint16_t c = r.overlap ? TFT_RED : TFT_WHITE;
    int n = (int)r.p.size();
    if (n < 2) continue;

    for (int i = 0; i < n; i++) {
      int j = (i + 1) % n;
      img.drawLine((int)r.p[i].x, (int)r.p[i].y,
                   (int)r.p[j].x, (int)r.p[j].y, c);
    }
    img.drawLine((int)r.p[0].x, (int)r.p[0].y,
                 (int)r.pos.x,   (int)r.pos.y,   c);
  }
  img.pushSprite(0, 0);
}

// COLLISION FONKSİYONLARI AYNEN KALDI (Kod kalabalığı olmasın diye tekrar yazmadım, eskisiyle aynı)
// Aşağıya kendi collision fonksiyonlarını yapıştırabilirsin.
// ... ShapeOverlap_SAT ...
// ... ShapeOverlap_SAT_STATIC ...
// ... ShapeOverlap_DIAGS ...
// ... ShapeOverlap_DIAGS_STATIC ...
bool ShapeOverlap_SAT(polygon &r1, polygon &r2) {
  polygon *poly1 = &r1;
  polygon *poly2 = &r2;

  for (int shape = 0; shape < 2; shape++) {
    if (shape == 1) {
      poly1 = &r2;
      poly2 = &r1;
    }

    for (int a = 0; a < (int)poly1->p.size(); a++) {
      int b = (a + 1) % poly1->p.size();
      vec2d axisProj = { -(poly1->p[b].y - poly1->p[a].y),
                           poly1->p[b].x - poly1->p[a].x };
      float d = sqrtf(axisProj.x * axisProj.x + axisProj.y * axisProj.y);
      axisProj = { axisProj.x / d, axisProj.y / d };

      float min_r1 = INFINITY, max_r1 = -INFINITY;
      for (int p = 0; p < (int)poly1->p.size(); p++) {
        float q = (poly1->p[p].x * axisProj.x + poly1->p[p].y * axisProj.y);
        min_r1 = std::min(min_r1, q);
        max_r1 = std::max(max_r1, q);
      }

      float min_r2 = INFINITY, max_r2 = -INFINITY;
      for (int p = 0; p < (int)poly2->p.size(); p++) {
        float q = (poly2->p[p].x * axisProj.x + poly2->p[p].y * axisProj.y);
        min_r2 = std::min(min_r2, q);
        max_r2 = std::max(max_r2, q);
      }

      if (!(max_r2 >= min_r1 && max_r1 >= min_r2))
        return false;
    }
  }

  return true;
}

bool ShapeOverlap_SAT_STATIC(polygon &r1, polygon &r2) {
  polygon *poly1 = &r1;
  polygon *poly2 = &r2;

  float overlap = INFINITY;

  for (int shape = 0; shape < 2; shape++) {
    if (shape == 1) {
      poly1 = &r2;
      poly2 = &r1;
    }

    for (int a = 0; a < (int)poly1->p.size(); a++) {
      int b = (a + 1) % poly1->p.size();
      vec2d axisProj = { -(poly1->p[b].y - poly1->p[a].y),
                           poly1->p[b].x - poly1->p[a].x };

      float min_r1 = INFINITY, max_r1 = -INFINITY;
      for (int p = 0; p < (int)poly1->p.size(); p++) {
        float q = (poly1->p[p].x * axisProj.x + poly1->p[p].y * axisProj.y);
        min_r1 = std::min(min_r1, q);
        max_r1 = std::max(max_r1, q);
      }

      float min_r2 = INFINITY, max_r2 = -INFINITY;
      for (int p = 0; p < (int)poly2->p.size(); p++) {
        float q = (poly2->p[p].x * axisProj.x + poly2->p[p].y * axisProj.y);
        min_r2 = std::min(min_r2, q);
        max_r2 = std::max(max_r2, q);
      }

      overlap = std::min(std::min(max_r1, max_r2) - std::max(min_r1, min_r2), overlap);

      if (!(max_r2 >= min_r1 && max_r1 >= min_r2))
        return false;
    }
  }

  vec2d d = { r2.pos.x - r1.pos.x, r2.pos.y - r1.pos.y };
  float s = sqrtf(d.x * d.x + d.y * d.y);
  if (s != 0.0f) {
    r1.pos.x -= overlap * d.x / s;
    r1.pos.y -= overlap * d.y / s;
  }
  return false; 
}

bool ShapeOverlap_DIAGS(polygon &r1, polygon &r2) {
  polygon *poly1 = &r1;
  polygon *poly2 = &r2;

  for (int shape = 0; shape < 2; shape++) {
    if (shape == 1) {
      poly1 = &r2;
      poly2 = &r1;
    }

    for (int p = 0; p < (int)poly1->p.size(); p++) {
      vec2d line_r1s = poly1->pos;
      vec2d line_r1e = poly1->p[p];

      for (int q = 0; q < (int)poly2->p.size(); q++) {
        vec2d line_r2s = poly2->p[q];
        vec2d line_r2e = poly2->p[(q + 1) % poly2->p.size()];

        float h = (line_r2e.x - line_r2s.x) * (line_r1s.y - line_r1e.y) -
                  (line_r1s.x - line_r1e.x) * (line_r2e.y - line_r2s.y);
        float t1 = ((line_r2s.y - line_r2e.y) * (line_r1s.x - line_r2s.x) +
                    (line_r2e.x - line_r2s.x) * (line_r1s.y - line_r2s.y)) / h;
        float t2 = ((line_r1s.y - line_r1e.y) * (line_r1s.x - line_r2s.x) +
                    (line_r1e.x - line_r1s.x) * (line_r1s.y - line_r2s.y)) / h;

        if (t1 >= 0.0f && t1 < 1.0f && t2 >= 0.0f && t2 < 1.0f)
          return true;
      }
    }
  }
  return false;
}

bool ShapeOverlap_DIAGS_STATIC(polygon &r1, polygon &r2) {
  polygon *poly1 = &r1;
  polygon *poly2 = &r2;

  for (int shape = 0; shape < 2; shape++) {
    if (shape == 1) {
      poly1 = &r2;
      poly2 = &r1;
    }

    for (int p = 0; p < (int)poly1->p.size(); p++) {
      vec2d line_r1s = poly1->pos;
      vec2d line_r1e = poly1->p[p];
      vec2d displacement = { 0, 0 };

      for (int q = 0; q < (int)poly2->p.size(); q++) {
        vec2d line_r2s = poly2->p[q];
        vec2d line_r2e = poly2->p[(q + 1) % poly2->p.size()];

        float h = (line_r2e.x - line_r2s.x) * (line_r1s.y - line_r1e.y) -
                  (line_r1s.x - line_r1e.x) * (line_r2e.y - line_r2s.y);
        float t1 = ((line_r2s.y - line_r2e.y) * (line_r1s.x - line_r2s.x) +
                    (line_r2e.x - line_r2s.x) * (line_r1s.y - line_r2s.y)) / h;
        float t2 = ((line_r1s.y - line_r1e.y) * (line_r1s.x - line_r2s.x) +
                    (line_r1e.x - line_r1s.x) * (line_r1s.y - line_r2s.y)) / h;

        if (t1 >= 0.0f && t1 < 1.0f && t2 >= 0.0f && t2 < 1.0f) {
          displacement.x += (1.0f - t1) * (line_r1e.x - line_r1s.x);
          displacement.y += (1.0f - t1) * (line_r1e.y - line_r1s.y);
        }
      }

      r1.pos.x += displacement.x * (shape == 0 ? -1.0f : +1.0f);
      r1.pos.y += displacement.y * (shape == 0 ? -1.0f : +1.0f);
    }
  }
  return false;
}