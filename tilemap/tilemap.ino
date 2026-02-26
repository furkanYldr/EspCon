#include <TFT_eSPI.h>
#include "cavesofgallet.h" // Resim dosyan

TFT_eSPI tft = TFT_eSPI();

// Daha önceki sohbetimizde belirttiğin pinler
#define BTN_UP    1
#define BTN_DOWN  5
#define BTN_LEFT  4
#define BTN_RIGHT 3
#define BACKLIGHT_PIN 8
// Harita üzerindeki anlık kamera pozisyonumuz (Sol üst köşe)
int camX = 0;
int camY = 0;

// Hareket hızı (Piksel cinsinden)
const int speed = 10; 

void setup() {
  Serial.begin(115200);

  // Butonları input olarak ayarla (Dahili pull-up direnci ile)
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);

  tft.init();
  tft.setRotation(1); // Ekran yönü (Yatay mod)
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true);

  // İlk çizim
  drawMap();
}

void loop() {
  bool moved = false;

  // Butonları kontrol et (INPUT_PULLUP olduğu için basılınca LOW olur)
  // !digitalRead(...) mantığı buton basılıysa true döner.
  
  if (digitalRead(BTN_RIGHT) == LOW) {
    camX += speed;
    moved = true;
  }
  if (digitalRead(BTN_LEFT) == LOW) {
    camX -= speed;
    moved = true;
  }
  if (digitalRead(BTN_DOWN) == LOW) {
    camY += speed;
    moved = true;
  }
  if (digitalRead(BTN_UP) == LOW) {
    camY -= speed;
    moved = true;
  }

  // Eğer hareket varsa haritayı güncelle
  if (moved) {
    // Harita sınırlarının dışına çıkmayı engelle (Clamping)
    // Harita genişliği - Ekran genişliği = Gidilebilecek maksimum sağ nokta
    if (camX < 0) camX = 0;
    if (camX > CAVESOFGALLET_WIDTH - tft.width()) camX = CAVESOFGALLET_WIDTH - tft.width();
    
    if (camY < 0) camY = 0;
    if (camY > CAVESOFGALLET_HEIGHT - tft.height()) camY = CAVESOFGALLET_HEIGHT - tft.height();

    drawMap();
  }
  
  // Çok hızlı döngüyü engellemek için ufak bir gecikme (Debounce için de işe yarar)
  delay(20);
}

void drawMap() {
  // Mantık: Resmi eksi koordinatlara çizdirerek "kaydırma" efekti yaratıyoruz.
  // Örneğin camX=100 ise, resmi x=-100'e çizeriz, böylece resmin 100. pikseli ekranın 0. pikseline denk gelir.
// (const uint16_t*) ekleyerek veriyi kütüphanenin istediği türe zorluyoruz.
tft.pushImage(-camX, -camY, CAVESOFGALLET_WIDTH, CAVESOFGALLET_HEIGHT, (const uint16_t*)cavesofgallet);
}