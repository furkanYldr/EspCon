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
const unsigned long interval = 50;
float elapsedTime = 0;




int top = 40;
int ground = 290;
int gravity = 20;
int score = 0;
int highScore = 0;
int logicManager = 2;
int fullXScreen = 172;
int fullYScreen = 320;
bool hasTouched = false;

struct sBall {

  float px, py;
  float ox,oy;
  float vx, vy;
  float ax, ay;
  float radius;
  unsigned int bColor;
  float mass;

  int id;
};

struct sLineSegment
{
	float sx, sy;
	float ex, ey;
	float radius;
};

vector<sLineSegment> vecLines ;
vector<sBall> vecBalls;
vector<pair<sBall *, sBall *>> vecCollidingPairs;

void addLine(float x1 , float y1 , float x2 , float y2 );
{
 sline l ;
 l.xs = x1 ;
 l.ys = y1 ;
 l.xe = x2 ;
 l.ye = y2 ;

vecBalls.emplace_back(l);

}
void addBall(float x, float y, float r, unsigned int color) {
  sBall b;
  b.px = x;
  b.py = y;
  b.vx = 0;
  b.vy = 0;
  b.ax = 0;
  b.ay = 0;
  b.bColor = color;

  b.radius = r;
  b.mass = r * 10.0f;
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
  const int SCREEN_WIDTH = 172;
  const int SCREEN_HEIGHT = 320;
  addBall(85, 30, 10, 0xBD76);

  addBall(80, 120, 15, 0x49E6);

  addBall(80, 150, 15, 0x49E6);
  addBall(80, 70, 15, 0x49E6);
  addLine(250,20,150,150);
  vecBalls[0].vy = 50;
  vecBalls[0].vx =40 ;
}

void drawInit() {
  img.fillSprite(0xFEFE);
  // img.fillCircle(85, 160, 45, 0xBF3A);
  // img.fillCircle(85, 160, 40, 0x897B);
  // img.fillCircle(85, 160, 34, 0xAD98);
  // img.fillCircle(85, 160, 29, 0xA145);
  // img.fillCircle(85, 160, 22, 0x036D);
  // img.fillCircle(85, 160, 17, 0x9497);
  // img.fillCircle(85, 160, 13, 0x5B8D);
  // img.fillCircle(85, 160, 10, 0xCC53);
  // img.fillCircle(85, 160,  8, 0xB8A1);
  // img.fillCircle(85, 160,  3, 0xFDB8);
  img.fillRect(0, ground, 172, 30, 0x89C8);
  img.drawFastHLine(0, top, 172, TFT_SILVER);
  for (auto ball : vecBalls) {
    img.fillCircle(ball.px, ball.py, ball.radius, ball.bColor);
  }
  for(auto line : vecLines){
    img.drawLine(line.xs,line.ys,line.xe,line.ye , TFT_BLACK )
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


    // Update ball physics
    ball.vx += ball.ax * 0.01;
    ball.vy += ball.ay * 0.01;
    ball.px += ball.vx * 0.01;
    ball.py += ball.vy * 0.01;

    // Wrap the balls around screen
    if (ball.px < 0) ball.px += 170;
    if (ball.px >= 170) ball.px -= 170;
    if (ball.py < 0) ball.py += 320;
    if (ball.py >= 320) ball.py -= 320;
    // Clamp velocity near zero
	
  }


 for (auto &ball : vecBalls)
		{
			for (auto &target : vecBalls)
			{
				if (ball.id != target.id)
				{
					if (doCircelOverlap(ball.px, ball.py, ball.radius, target.px, target.py, target.radius))
					{
						// Collision has occured
						vecCollidingPairs.push_back({ &ball, &target });

						// Distance between ball centers
						float fDistance = sqrtf((ball.px - target.px)*(ball.px - target.px) + (ball.py - target.py)*(ball.py - target.py));

						// Calculate displacement required
						float fOverlap = (fDistance - ball.radius - target.radius);

						
						ball.px -= fOverlap * (ball.px - target.px) / fDistance;
						ball.py -= fOverlap * (ball.py - target.py) / fDistance;

					  target.px += fOverlap * (ball.px - target.px) / fDistance;
					  target.py += fOverlap * (ball.py - target.py) / fDistance;
            

					}
				}
			}
      //silebilirm her an 
      float fIntendedSpeed = sqrtf( ball.vx * ball.vx + ball.vy*ball.vy);
      float fIntendedDistance = fIntendedSpeed * 0.001; 
      float fActualDistance	= sqrtf((ball.px - ball.ox)*(ball.px - ball.ox) + (ball.py - ball.oy)*(ball.py - ball.oy));
			float fActualTime = fActualDistance / fIntendedSpeed;



		}
  // fatal function
 	for (auto c : vecCollidingPairs)
		{
			sBall *b1 = c.first;
			sBall *b2 = c.second;

			// Distance between balls
			float fDistance = sqrtf((b1->px - b2->px)*(b1->px - b2->px) + (b1->py - b2->py)*(b1->py - b2->py));

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
     vecCollidingPairs.clear();


}


void loop() {

  buttonState = digitalRead(jumpButton);

  if (buttonState == LOW && lastButtonState == HIGH) {
    if (logicManager == 1) {
      logicManager = 2;
    } else if (logicManager == 2) {
      collisionDetect();
    } else if (logicManager == 3) {
      // Diğer oyun mekanikleri burada işlenebilir
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
