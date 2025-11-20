#ifndef RESOURCES_H
#define RESOURCES_H
#include <cstdint>

#define CELL_SIZE 6


#define MATRIX_COLS 10
#define MATRIX_ROWS 22



// back 12 pause 1 select 10
// button definition
//#define up_btn 13
//#define dwn_btn 11
//#define lft_btn 44
//#define rgh_btn 9
//#define bck_btn 7
//#define select_btn 8






#define yellow 0xFECB
#define green 0x6C64
#define pupple 0x712E
#define orange 0xCBA4
#define cyan 0x5D32
#define red 0xE127
#define blue 0x0334


extern const uint8_t tetrominoShapes[7][4][4][4];
extern const uint16_t blockColor[7];




#endif