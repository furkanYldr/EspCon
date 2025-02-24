#ifndef RESOURCES_H
#define RESOURCES_H
#include <cstdint>

#define CELL_SIZE 6


#define MATRIX_COLS 29
#define MATRIX_ROWS 32
#define coin_collums 30
#define coin_rows 31


// back 12 pause 1 select 10 
// button definition
#define up_btn 5
#define dwn_btn 4
#define lft_btn 6
#define rgh_btn 2
#define bck_btn 7
#define select_btn 3


///// COLOR
#define pacmanColor 0xFFA0
#define COLOR_1 0x347C
#define COLOR_0 0x0000
#define BlinkyColor 0xF800
#define PinkyColor 0xF81F
#define InkyColor 0x8E7D
#define ClydeColor 0xFCE0
#define frightened 0x1BBA



// mazes
extern uint8_t coin_matrix[coin_rows][coin_collums];
extern uint8_t bmatrix[MATRIX_ROWS][MATRIX_COLS] ;

// PACMAN'S SPRITES
extern uint8_t pacmanFront[10][10] ;
extern uint8_t pacmanBack[10][10] ;
extern uint8_t pacmanUp[10][10] ;
extern uint8_t pacmanDown[10][10] ;
extern uint8_t pacman2[10][10] ;

// GHOSTS ANİMS
extern uint8_t ghost[10][10];
extern uint8_t ghost1[10][10];
extern uint8_t ghost2[10][10];
extern uint8_t ghost3[10][10];


// FRUİTS
extern uint8_t cherry[10][11];



#endif