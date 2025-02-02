#include <TFT_eSPI.h>
#include <Wire.h>
#include <vector>
using namespace std;

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite img = TFT_eSprite(&tft);

#define jumpButton 1

int lastButtonState = HIGH;
int buttonState = HIGH;

unsigned long previousMillis = 0;
const unsigned long interval = 33;
float elapsedTime = 0;



const int SCREEN_WIDTH = 172;
const int SCREEN_HEIGHT = 320;
int top = 40;
int ground = 290;
int gravity = 20;
int score = 0;
int highScore = 0;
int logicManager = 2;
int fullXScreen = 172;
int fullYScreen = 320;
bool hasTouched = false;


float fStable = 0.5f;

struct sBall {

  float px, py;
  float ox, oy;
  float vx, vy;
  float ax, ay;
  float radius;
  
  unsigned int bColor;
  float mass;
  int scale;
  int id;
};

struct sLineSegment {
  float sx, sy;
  float ex, ey;
  float radius = 0;
};

vector<sLineSegment> vecLines;
vector<sBall> vecBalls;
vector<sBall *> vecFakeBalls;
vector<pair<sBall *, sBall *>> vecCollidingPairs;
vector<int> BallRadius = {
  4,   //0
  8,   //1
  11,   //2
  15,  //3
  19,  //4
  25,  //5
  30,  //6
  35,  //7
  40 / 8

};
vector<unsigned int> ballColors{
  0xBF3A,
  0x218B,
  0xEEF9,
  0x7337,
  0xFC60,
  0xB8A1,
  0xD8D0,
  0xEDC9,
  0x4AA4

};

void addLine(float x1, float y1, float x2, float y2) {

  sLineSegment l;
  l.sx = x1;
  l.sy = y1;
  l.ex = x2;
  l.ey = y2;

  vecLines.emplace_back(l);
}
void addBall(float x, float y, float r) {
  sBall b;
  b.px = x;
  b.py = y;
  b.vx = 0;
  b.vy = 0;
  b.ax = 0;
  b.ay = 0;
   b.scale = r ; 
  b.bColor = ballColors[b.scale];
 
  b.radius = BallRadius[b.scale];
  b.mass = 1;
 
  b.id = vecBalls.size();
  vecBalls.emplace_back(b);
  
};

void setup() {
  pinMode(15, OUTPUT);
  digitalWrite(15, 1);

  Serial.begin(9600);
  pinMode(jumpButton, INPUT_PULLUP);
  tft.init();
  tft.setRotation(2);
  tft.setSwapBytes(true);

  tft.fillScreen(TFT_BLACK);
  img.createSprite(172, 320);
  img.setTextDatum(4);
  img.setTextColor(TFT_WHITE, TFT_BLACK);
  img.pushSprite(0, 0);

  addLine(0, 0, 0, 320);
  addLine(170, 0, 170, 320);
  addLine(0, 320, 170, 320);
  addLine(0, 0, 170, 0);
 // addBall(85, 160, 8);
// addBall(85, 160, 7);
// addBall(85, 160, 6);
// addBall(85, 160, 5);
// addBall(85, 160, 4);
// addBall(85, 160, 3);
// addBall(85, 160, 2);
// addBall(85, 160, 1);
// addBall(85, 160, 0);
}

void drawInit() {
  img.fillSprite(0x04CC);


  // img.drawFastHLine(0, top, 172, TFT_SILVER);
  for (auto ball : vecBalls) {
    img.fillCircle(ball.px, ball.py, ball.radius, ball.bColor);
  }
  for (auto line : vecLines) {
    img.drawLine(line.sx, line.sy, line.ex, line.ey, TFT_BLACK);
  }


  img.pushSprite(0, 0);
}

void GameOver(boolean n) {
  if (n) {
    img.setCursor(20, 100);
    img.setTextColor(TFT_BLACK, TFT_RED);
    img.fillRect(0, 97, 170, 20, TFT_RED);
    img.setTextSize(2);
    img.print(" GAME OVER ");
  }
}

void Score() {

  img.setCursor(10, 10);
  img.setTextSize(2);
  img.setTextDatum(4);
  img.setTextColor(0xEC84);  //0xEC84
  img.println(score);
}

void startGame() {

  img.setCursor(10, 72);
  img.setTextSize(2);
  img.setTextDatum(4);
  img.setTextColor(0xEC84);  //0xEC84
  img.print(String("MERGE BAL"));
  img.setCursor(100, 95);
  img.setTextSize(1);
  img.print("By FYldrr");
}
void collisionDetect() {
  auto doCircelOverlap = [](float x1, float y1, float r1, float x2, float y2, float r2) {
    return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) <= (r1 + r2) * (r1 + r2);
  };

  for (auto &ball : vecBalls) {

    ball.ox = ball.px;
    ball.oy = ball.py;
    // Add Drag to emulate rolling friction
    ball.ax = -ball.vx * 0.6f;  // Apply drag and gravity
    ball.ay = -ball.vy * 0.6f + 500;

    // Update ball physics
    ball.vx += ball.ax * 0.01;
    ball.vy += ball.ay * 0.01;
    ball.px += ball.vx * 0.01;
    ball.py += ball.vy * 0.01;


    // Clamp velocity near zero
    if (fabs(ball.vx * ball.vx + ball.vy * ball.vy) < fStable) {
      ball.vx = 0;
      ball.vy = 0;
    }
  }

  for (auto &ball : vecBalls) {

    for (auto &edge : vecLines) {
      // Check that line formed by velocity vector, intersects with line segment
      float fLineX1 = edge.ex - edge.sx;
      float fLineY1 = edge.ey - edge.sy;

      float fLineX2 = ball.px - edge.sx;
      float fLineY2 = ball.py - edge.sy;

      float fEdgeLength = fLineX1 * fLineX1 + fLineY1 * fLineY1;

      float t = max(0.0f, min(fEdgeLength, (fLineX1 * fLineX2 + fLineY1 * fLineY2))) / fEdgeLength;

      float fClosestPointX = edge.sx + t * fLineX1;
      float fClosestPointY = edge.sy + t * fLineY1;

      float fDistance = sqrtf((ball.px - fClosestPointX) * (ball.px - fClosestPointX) + (ball.py - fClosestPointY) * (ball.py - fClosestPointY));

      if (fDistance <= (ball.radius + edge.radius)) {

        sBall *fakeball = new sBall();
        fakeball->radius = edge.radius;
        fakeball->mass = ball.mass * 0.8f;
        fakeball->px = fClosestPointX;
        fakeball->py = fClosestPointY;
        fakeball->vx = -ball.vx;
        fakeball->vy = -ball.vy;


        vecFakeBalls.push_back(fakeball);

        vecCollidingPairs.push_back({ &ball, fakeball });

        float fOverlap = 1.0f * (fDistance - ball.radius - fakeball->radius);

        ball.px -= fOverlap * (ball.px - fakeball->px) / fDistance;
        ball.py -= fOverlap * (ball.py - fakeball->py) / fDistance;
      }
    }
    for (auto &target : vecBalls) {
      if (ball.id != target.id) {
        if (doCircelOverlap(ball.px, ball.py, ball.radius, target.px, target.py, target.radius)) {
         if(ball.scale == target.scale){
          ball.scale += 1;
          ball.radius = BallRadius[ball.scale];
           ball.bColor = ballColors[ball.scale];
          target.radius = 0 ;
          vecBalls.erase( vecBalls.begin()+ target.id );
          
          }else {
          
          vecCollidingPairs.push_back({ &ball, &target });

          // Distance between ball centers
          float fDistance = sqrtf((ball.px - target.px) * (ball.px - target.px) + (ball.py - target.py) * (ball.py - target.py));

          // Calculate displacement required
          float fOverlap = (fDistance - ball.radius - target.radius);

          ball.px -= fOverlap * (ball.px - target.px) / fDistance;
          ball.py -= fOverlap * (ball.py - target.py) / fDistance;

          target.px += fOverlap * (ball.px - target.px) / fDistance;
          target.py += fOverlap * (ball.py - target.py) / fDistance;
          }
        }
      }
    }
    //silebilirm her an
    float fIntendedSpeed = sqrtf(ball.vx * ball.vx + ball.vy * ball.vy);
    float fIntendedDistance = fIntendedSpeed * 0.001;
    float fActualDistance = sqrtf((ball.px - ball.ox) * (ball.px - ball.ox) + (ball.py - ball.oy) * (ball.py - ball.oy));
    float fActualTime = fActualDistance / fIntendedSpeed;
  }
  // fatal function
  for (auto c : vecCollidingPairs) {
    sBall *b1 = c.first;
    sBall *b2 = c.second;

    // Distance between balls
    float fDistance = sqrtf((b1->px - b2->px) * (b1->px - b2->px) + (b1->py - b2->py) * (b1->py - b2->py));

    // Normal
    float nx = (b2->px - b1->px) / fDistance;
    float ny = (b2->py - b1->py) / fDistance;

    // Tangent
    float tx = -ny;
    float ty = nx;

    // Dot Product Tangent
    float dpTan1 = b1->vx * tx + b1->vy * ty;
    float dpTan2 = b2->vx * tx + b2->vy * ty;

    // Dot Product Normal
    float dpNorm1 = b1->vx * nx + b1->vy * ny;
    float dpNorm2 = b2->vx * nx + b2->vy * ny;

    // Conservation of momentum in 1D
    float m1 = (dpNorm1 * (b1->mass - b2->mass) + 2.0f * b2->mass * dpNorm2) / (b1->mass + b2->mass);
    float m2 = (dpNorm2 * (b2->mass - b1->mass) + 2.0f * b1->mass * dpNorm1) / (b1->mass + b2->mass);

    // Update ball velocities
    b1->vx = tx * dpTan1 + nx * m1;
    b1->vy = ty * dpTan1 + ny * m1;
    b2->vx = tx * dpTan2 + nx * m2;
    b2->vy = ty * dpTan2 + ny * m2;
  }

  for (auto &b : vecFakeBalls) delete b;
  vecFakeBalls.clear();
  vecCollidingPairs.clear();
}


void loop() {

  buttonState = digitalRead(jumpButton);

  if (buttonState == LOW && lastButtonState == HIGH) {
    if (logicManager == 1) {
      logicManager = 2;
    } else if (logicManager == 2) {
      int randomValue =  2;//rand() % 9;
      addBall(85,20,randomValue);
    } else if (logicManager == 3) {
    }
  }

  lastButtonState = buttonState;


  unsigned long currentMillis = millis();
  elapsedTime = (currentMillis - previousMillis) / 1000.0f;
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    collisionDetect();
    drawInit();
  }
}
