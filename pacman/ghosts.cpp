#include "HWCDC.h"

#include <map>
#include "pacman.h"
#include "ghosts.h"
#include <vector>
#include <utility>
#include <queue>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <ctime>

using namespace std;

bool goBack = false;


STRUCTGHOST blinky = { 5, 77, 0, 0, false, false, false, false, BlinkyColor, 0, 0, SCATTER, GEMPTY };
STRUCTGHOST inky = { 5, 83, 0, 0, false, false, false, false, InkyColor, 0, 0, SCATTER, GEMPTY };
STRUCTGHOST clyde = { 17, 77, 0, 0, false, false, false, false, ClydeColor, 0, 0, SCATTER, GEMPTY };
STRUCTGHOST pinky = { 11, 77, 0, 0, false, false, false, false, PinkyColor, 0, 0, SCATTER, GEMPTY };

vector<STRUCTGHOST> vecGHOST = { clyde, inky, pinky, blinky };

pair<int, int> searchTRY(STRUCTGHOST& ghost, int sx, int sy, int ex, int ey, int prevr, int prevc) {

  pair<int, int> rght(sx + 1, sy);
  pair<int, int> lft(sx - 1, sy);
  pair<int, int> up(sx, sy - 1);
  pair<int, int> dwn(sx, sy + 1);
  vector<pair<int, int>> directions = { rght, lft, up, dwn };

  pair<int, int> prev(prevr, prevc);
  pair<int, int> trgt(ex, ey);
  if (ghost.STATE == EATEN) {

    return eatenPath(prev, directions);

  } else if (ghost.STATE == CHASE) {

    return blinyChase(prev, trgt, directions);
  } else if (ghost.STATE == FRIGHTENED) {

    return frightenPath(prev, directions);
  } else if (ghost.STATE == SCATTER) {
    switch (ghost.STATE) {
      case BlinkyColor:
        trgt = make_pair(0, 26);
        break;
      case PinkyColor:

        trgt = make_pair(30, 3);

        break;
    }

    return blinyChase(prev, trgt, directions);
  }
}

pair<int, int> blinyChase(pair<int, int> prev, pair<int, int> trgt, vector<pair<int, int>>& directions) {

  int distance = 0;
  int dis;
  pair<int, int> way;

  for (const auto& dir : directions) {
    if (dir != prev && coin_matrix[dir.first][dir.second] != 0) {
      dis = ((dir.second - trgt.second) * (dir.second - trgt.second)) + ((dir.first - trgt.first) * (dir.first - trgt.first));

      if (distance == 0) {
        distance = dis;
        way = dir;
      } else if (distance < dis) {
        continue;
      } else if (distance >= dis) {
        distance = dis;
        way = dir;
      }
    }
  }

  return way;
}

pair<int, int> eatenPath(pair<int, int> prev, vector<pair<int, int>>& directions) {

  int distance = 0;
  int dis;
  pair<int, int> way;
  pair<int, int> trgt;
  trgt = make_pair(13, 14);


  for (const auto& dir : directions) {
    if (dir != prev && coin_matrix[dir.first][dir.second] != 0) {
      dis = ((dir.second - trgt.second) * (dir.second - trgt.second)) + ((dir.first - trgt.first) * (dir.first - trgt.first));

      if (distance == 0 ) {
        distance = dis;
        way = dir;
      } else if (distance < dis) {
        continue;
      } else if (distance >= dis) {
        distance = dis;
        way = dir;
      }
    }
  }

  return way;
}

void getAlive() {
  for (STRUCTGHOST& ghost : vecGHOST) {
    if (ghost.px < 90 && ghost.px > 70 && ghost.py == 137 && ghost.STATE == EATEN) {

      ghost.STATE = CHASE;
      Serial.print("ulaştı ");
      
    }
  }
}

pair<int, int> frightenPath(pair<int, int> prev, vector<pair<int, int>>& directions) {

  int distance = 0;
  int dis;
  pair<int, int> way;
  vector<pair<int, int>> ways;
  if (goBack) {
    way = prev;
    goBack = false;
  } else {

    for (const auto& dir : directions) {
      if (dir != prev && coin_matrix[dir.first][dir.second] != 0) {
        ways.push_back(dir);
      }
    }
    srand(static_cast<unsigned>(time(0)));
    int randomIndex = rand() % ways.size();
    way = ways[randomIndex];
  }

  return way;
}

void eaten() {
  for (STRUCTGHOST& ghost : vecGHOST) {
    if (ghost.STATE == FRIGHTENED) {
      float dx = pacman.px - ghost.px;
      float dy = pacman.py - ghost.py;
      float db = 81;
      float distance = (dx * dx) + (dy * dy);

      if (db > distance) { ghost.STATE = EATEN; }
    }
  }
}

void catchPacMan() {
  for (STRUCTGHOST& ghost : vecGHOST) {
    if (ghost.STATE == CHASE|| ghost.STATE == SCATTER) {
      float dx = pacman.px - ghost.px;
      float dy = pacman.py - ghost.py;
      float db = 81;
      float distance = (dx * dx) + (dy * dy);
      if (db > distance) { 
      CATCH = true;
      paused = true;}
    }
  }
}

void blinkyMOVEMENT(STRUCTGHOST& ghost, pair<int, int> vector, int bx, int by, int px, int py) {
  int targetX = vector.first;
  int targetY = vector.second;

  int dx = targetX - bx;
  int dy = targetY - by;



  if (px == bx && py == by) {

    ghost.wantedDirection = GEMPTY;

  } else {

    if (dx == 0 && dy == 1) {
      ghost.wantedDirection = GRIGHT;

    } else if (dx == 0 && dy == -1) {
      ghost.wantedDirection = GLEFT;
    }


    if (dx == 1 && dy == 0) {

      ghost.wantedDirection = GDOWN;


    } else if (dx == -1 && dy == 0) {


      ghost.wantedDirection = GUP;
    }
  }
  goThatWay(ghost);
}
void goThatWay(STRUCTGHOST& ghost) {
  int blinkySpeed;
  if (ghost.STATE == EATEN) {

    blinkySpeed = 3;

  } else {
    blinkySpeed = 1;
  }

  switch (ghost.wantedDirection) {
    case GRIGHT:
      ghost.pvx = blinkySpeed;
      ghost.pvy = 0;
      break;
    case GLEFT:
      ghost.pvx = -blinkySpeed;
      ghost.pvy = 0;
      break;
    case GDOWN:
      ghost.pvx = 0;
      ghost.pvy = blinkySpeed;
      break;
    case GUP:
      ghost.pvx = 0;
      ghost.pvy = -blinkySpeed;
      break;
    case GEMPTY:

      ghost.pvx = 0;
      ghost.pvy = 0;
      break;
  }
}
void drawBlinky() {
  for (STRUCTGHOST& ghostR : vecGHOST) {

    static int x;
    static int y;
    uint8_t matrix[10][10];  // Geçici matris

    // Hangi animasyon kullanılacaksa onu geçici matrise kopyala
    switch (ghostAnim) {
      case 0:
        memcpy(matrix, ghost, sizeof(matrix));  // ghost dizisini matrix'e kopyala
        break;
      case 1:
        memcpy(matrix, ghost1, sizeof(matrix));  // ghost1 dizisini matrix'e kopyala
        break;
      case 2:
        memcpy(matrix, ghost2, sizeof(matrix));  // ghost2 dizisini matrix'e kopyala
        break;
      case 3:
        memcpy(matrix, ghost3, sizeof(matrix));  // ghost3 dizisini matrix'e kopyala
        break;
    }

    // Ghost'un hareketini temizle ve güncelle
    img.fillRect(ghostR.px, ghostR.py + 1, 10, 9, COLOR_0);

    ghostR.px += ghostR.pvx;
    ghostR.py += ghostR.pvy;


    // Ghost'u çizer
    for (int row = 0; row < 10; row++) {
      for (int col = 0; col < 10; col++) {
        x = col + ghostR.px;
        y = row + ghostR.py;

        if (matrix[row][col] == 1 && !(ghostR.STATE == EATEN)) {
          img.drawPixel(x, y, ghostR.STATE == FRIGHTENED ? frightened : ghostR.color);
        } else if (matrix[row][col] == 2) {
          img.drawPixel(x, y, TFT_WHITE);
        } else if (matrix[row][col] == 3) {
          img.drawPixel(x, y, TFT_BLACK);
        }
      }
    }
  }
}
void getFreak() {
  for (STRUCTGHOST& ghost : vecGHOST) {

    ghost.STATE = FRIGHTENED;
  }
}
void pathFinder(int x, int y) {
  for (STRUCTGHOST& ghost : vecGHOST) {


    int centerPacmanx = x + 4;
    int centerPacmany = y + 5;

    int centerghostx = ghost.px + 4;
    int centerghosty = ghost.py + 5;

    int rowpacman = (centerPacmany - xPadding) / 6;
    int colpacman = (centerPacmanx - yPadding) / 6;

    int rowghost = (centerghosty - xPadding) / 6;
    int colghost = (centerghostx - yPadding) / 6;

    if ((centerghosty - xPadding) % 6 == 0 && (centerghostx - yPadding) % 6 == 0) {

      if (rowghost == 14 && ghost.px + 4 < 3) {
        ghost.px = 161;
        ghost.wantedDirection = GLEFT;
        goThatWay(ghost);
        ghost.prevRow = 14;
        ghost.prevCol = 32;
        continue;
      } else if (rowghost == 14 && ghost.px + 4 > 165) {
        Serial.println("çalışıyor2");
        ghost.px = -1;
        ghost.wantedDirection = GRIGHT;
        goThatWay(ghost);

        ghost.prevRow = 14;
        ghost.prevCol = 1;
        continue;
      }

      int startX = rowghost, startY = colghost;
      int endX = rowpacman, endY = colpacman;

      if (ghost.color == pinky.color) {
        switch (pacman.ANIM) {
          case AUP:
            endX -= 4;
            break;
          case ADOWN:
            endX += 4;
            break;
          case ALEFT:
            endY -= 4;
            break;
          case ARIGHT:
            endY += 4;
            break;
        }
      } else if (ghost.color == inky.color) {
        switch (pacman.ANIM) {
          case AUP:
            endX -= 2;
            break;
          case ADOWN:
            endX += 2;
            break;
          case ALEFT:
            endY -= 2;
            break;
          case ARIGHT:
            endY += 2;
            break;
        }
        int dx = endX - blinky.prevRow;
        int dy = endY - blinky.prevCol;
        endX += dx;
        endY += dy;
      } else if (ghost.color == clyde.color) {
        float dx = endX - startX;
        float dy = endY - startY;
        float db = 64;
        float distance = (dx * dx) + (dy * dy);
        if (distance < db) {
          endX = 30;
          endY = 1;
        }
      }


      pair<int, int> Path = searchTRY(ghost, startX, startY, endX, endY, ghost.prevRow, ghost.prevCol);

      blinkyMOVEMENT(ghost, Path, startX, startY, rowpacman, colpacman);


      ghost.prevRow = rowghost;
      ghost.prevCol = colghost;
    }
  }
}
