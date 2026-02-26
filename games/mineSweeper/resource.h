#ifndef RESOURCES_H
#define RESOURCES_H
#include <cstdint>

// Tetris'ten kalanlar (Zararı yok, kalabilir)
#define CELL_SIZE 10
#define areaRow 20
#define areaColmn 17

// --- BUTON TANIMLARI (Bunları aktif etmelisin) ---
#define up_btn 4
#define dwn_btn 3
#define lft_btn 5
#define rgh_btn 1
#define bck_btn 6
#define select_btn 7

// Renkler (Kalabilir)
#define yellow 0xFECB
#define green 0x6C64
#define purple 0x712E
#define orange 0xCBA4
#define cyan 0x5D32
#define red 0xE127
#define blue 0x0334
#define white 0xFFFF    // Hatayı çözen satır
#define black 0x0000    // Hatayı çözen satır
#define GREY 0x52AA     // Hatayı çözen satır
#define DARK_GREY 0x218B 

extern const uint8_t tetrominoShapes[7][4][4][4];
extern const uint16_t blockColor[7];

#endif