#include <TFT_eSPI.h>  // TFT LCD kütüphanesi


TFT_eSPI tft = TFT_eSPI();  // TFT ekran nesnesi

// Renk değerleri
uint8_t r = 0, g = 0, b = 0;
int8_t dr = 1, dg = 2, db = 3; // Renk artış hızları

void setup() {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
}

void loop() {
  // Renk değerlerini yavaşça değiştir
  r += dr;
  g += dg;
  b += db;

  // Renk sınırlarını kontrol et ve yön değiştir
  if (r <= 0 || r >= 255) dr = -dr;
  if (g <= 0 || g >= 255) dg = -dg;
  if (b <= 0 || b >= 255) db = -db;

  // 16-bit renk oluştur
  uint16_t color = tft.color565(r, g, b);

  // Tüm ekranı tek renkle doldur
  tft.fillScreen(color);

  delay(10); // Geçiş hızını ayarlar
}
