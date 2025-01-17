#define ENCODER_CLK 10  // Rotary encoder'ın CLK (A fazı) pini
#define ENCODER_DT 11   // Rotary encoder'ın DT (B fazı) pini

volatile int counter = 0;  // Encoder sayacı
volatile int lastStateCLK; // Önceki CLK durumu
volatile int currentStateCLK; // Şimdiki CLK durumu
volatile int currentStateDT;  // Şimdiki DT durumu

void IRAM_ATTR handleEncoder() {
  // CLK'nin mevcut durumunu oku
  currentStateCLK = digitalRead(ENCODER_CLK);

  // Eğer CLK'nin durumu değiştiyse
  if (currentStateCLK != lastStateCLK) {
    // DT'nin mevcut durumunu oku
    currentStateDT = digitalRead(ENCODER_DT);

    // Yön kontrolü
    if (currentStateCLK != currentStateDT) {
      counter++;  // Saat yönünde çevirme
    } else {
      counter--;  // Saat yönünün tersine çevirme
    }
  }

  // CLK'nin önceki durumunu güncelle
  lastStateCLK = currentStateCLK;
}

void setup() {
  pinMode(ENCODER_CLK, INPUT);
  pinMode(ENCODER_DT, INPUT);

  // Encoder için kesme tanımlama
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), handleEncoder, CHANGE);

  Serial.begin(115200);

  // CLK'nin başlangıç durumunu oku
  lastStateCLK = digitalRead(ENCODER_CLK);
}

void loop() {
  static int lastCounter = 0;

  // Encoder sayacındaki değişikliği kontrol et
  if (counter != lastCounter) {
    Serial.print("Encoder Counter: ");
    Serial.println(counter);
    lastCounter = counter;
  }

  delay(10);  // Titreşim önlemek için kısa bir gecikme
}

