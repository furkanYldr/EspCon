#define ENCODER_CLK 48 // A fazı
#define ENCODER_DT  47  // B fazı

volatile long counter = 0;

volatile int lastCLK = 0;
volatile unsigned long lastInterruptTime = 0;  // debounce için

void IRAM_ATTR handleEncoder() {
  unsigned long now = micros();

  // Çok hızlı gelen titreşimleri engelle (ör: 1000 µs = 1 ms altında gelenleri yok say)
  if (now - lastInterruptTime < 1000) {
    return;
  }
  lastInterruptTime = now;

  int clkState = digitalRead(ENCODER_CLK);
  int dtState  = digitalRead(ENCODER_DT);

  // Yön kontrolü
  if (clkState == dtState) {
    counter++;   // örneğin saat yönü
  } else {
    counter--;   // ters yön
  }

  lastCLK = clkState;
}

void setup() {
  Serial.begin(115200);

  pinMode(ENCODER_CLK, INPUT_PULLDOWN);
  pinMode(ENCODER_DT,  INPUT_PULLDOWN);

  lastCLK = digitalRead(ENCODER_CLK);

  // CLK hattındaki değişimde interrupt
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), handleEncoder, CHANGE);

  Serial.println("Encoder testi basladi babacum.");
}

void loop() {
  static long lastCounter = 0;

  // ISR ile ortak değişken, kopyasını alalım
  noInterrupts();
  long current = counter;
  interrupts();

  if (current != lastCounter) {
    Serial.print("Encoder Counter: ");
    Serial.println(current);
    lastCounter = current;
  }


}
