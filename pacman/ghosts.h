#ifndef GHOSTS_H
#define GHOSTS_H

#include <cstdint>
#include <vector>
#include "pacman.h"
#include "resource.h"

using namespace std;

// ======================== ENUM TANIMLARI ========================
enum states {
    CHASE,
    SCATTER,
    FRIGHTENED,
    EATEN
};

enum ghostDirection { 
    GUP,
    GDOWN,
    GLEFT,
    GRIGHT,
    GEMPTY 
};

// ======================== STRUCT TANIMLARI ========================
struct STRUCTGHOST {
    int px, py;     // Konum
    int pvx, pvy;   // Hız
    bool up, down, front, back;
    int color;
    int prevCol, prevRow;
    states STATE;
    ghostDirection wantedDirection;
};

// ======================== HAYALETLERİN DEĞİŞKENLERİ ========================
extern STRUCTGHOST blinky;
extern STRUCTGHOST inky;
extern STRUCTGHOST clyde;
extern STRUCTGHOST pinky;
extern vector<STRUCTGHOST> vecGHOST ;
extern bool goBack;
extern int strike ;
extern int pinkyTimer;

// ======================== FONKSİYON PROTOTİPLERİ ========================
void spawnGhost();
void drawBlinky();
void pathFinder(int x, int y);
void blinkyMOVEMENT(STRUCTGHOST& ghost, vector<pair<int, int>> vector, int bx, int by, int px, int py);
void goThatWay(STRUCTGHOST& ghost);
void eaten();
void catchPacMan();
void getAlive();
void drawPath(const vector<pair<int, int>>& path);
void getFreak();

pair<int, int> frightenPath(pair<int, int> prev, vector<pair<int, int>>& directions);
pair<int, int> searchTRY(STRUCTGHOST& ghost, int sx, int sy, int ex, int ey, int prevr, int prevc);
pair<int, int> blinyChase(pair<int, int> prev, pair<int, int> trgt, vector<pair<int, int>>& directions);
pair<int, int> eatenPath(pair<int, int> prev, vector<pair<int, int>>& directions);

#endif 