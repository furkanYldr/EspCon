#ifndef RESOURCES_H
#define RESOURCES_H
#include <cstdint>

#define CELL_SIZE 6


#define MATRIX_COLS 29
#define MATRIX_ROWS 32
#define coin_collums 28
#define coin_rows 31
// back 12 pause 1 select 10 
// button definition
#define up_btn 11
#define dwn_btn 3
#define lft_btn 2
#define rgh_btn 13 
///// COLOR
#define pacmanColor 0xFFA0
#define COLOR_1 0x347C
#define COLOR_0 0x0000
#define Blinky 0xF800
#define Pinky 0xF81F
#define Inky 0x8E7D
#define Clyde 0xFCE0




extern uint8_t coin_matrix[coin_rows][coin_collums];
extern uint8_t bmatrix[MATRIX_ROWS][MATRIX_COLS] ;
extern uint8_t pacman1[10][10] ;
extern uint8_t pacman2[10][10] ;

extern uint8_t ghost[10][10];



#endif