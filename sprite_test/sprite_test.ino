#include <TFT_eSPI.h>
#include "sprite.h" // Berry_Garden burada tanımlı
#include "tiles.h"
TFT_eSPI tft = TFT_eSPI(); 
// Sprite tanımlaması
TFT_eSprite sprite = TFT_eSprite(&tft); // Sprite nesnesi
uint16_t buffer[BERRY_GARDEN_WIDTH * BERRY_GARDEN_HEIGHT];

void setup() {
  tft.init();
  tft.setRotation(1); // Cihaza göre değiştirilebilir
  tft.fillScreen(TFT_BLACK);

  drawBerryGarden();


}

void loop() {

}







void drawBerryGarden() {
  // Sprite'ı oluştur ve boyutlandır
  sprite.createSprite(BERRY_GARDEN_WIDTH * 2, BERRY_GARDEN_HEIGHT * 2);  

  // Ekranı temizle (eski verilerin kalmaması için)
  sprite.fillScreen(TFT_BLACK);  

  // PROGMEM'den veriyi kopyala ve sprite'a aktar
  for (int y = 0; y < BERRY_GARDEN_HEIGHT; y++) {    // Y koordinatını bul
    for (int x = 0; x < BERRY_GARDEN_WIDTH; x++) {   // X koordinatını bul
      int i = y * BERRY_GARDEN_WIDTH + x;             // Düzgün indeks hesaplama
uint16_t rawColor = pgm_read_word(&Berry_Garden[i]);
uint16_t color = (rawColor >> 8) | (rawColor << 8);
       // 2x2 kareyi çiz
      sprite.fillRect(x * 2, y * 2 , 2, 2, color);  // 2x2 büyüklüğünde bir kare çiziyoruz

    }
  }

  // Sprite'ı ekrana çiz
  sprite.pushSprite(100 , 0);
}
