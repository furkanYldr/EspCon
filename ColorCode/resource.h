#ifndef RESOURCES_H
#define RESOURCES_H

#include <map>
#include <vector>

#define YELLOW 0xF6AD
#define RED 0xF840
#define BLUE 0x0257
#define GREEN 0x0400
#define ORANGE 0xFAC4
#define PINK 0xEC1D
#define PUPLE 0x818F
#define BLACK 0x0000
#define WHITE 0xFFFD
#define BROWN 0x3860
#define LIGHT_BLUE 0x8E7D
#define LIGHT_GREEN 0xA615
#define DARK_BLUE 0x4AA4
#define BACKGROUND 0x0208
#define EMPTY_SPACE 0xA617
#define BUTTON_HIGHLIGHT 0xDD1B

inline std::map<int, std::vector<int>> levels = {
    {1,  {6, 3, 4, 2, 1, 5}},
    {2,  {3, 6, 4, 1, 5, 2}},
    {3,  {3, 5, 6, 4, 2, 1}},
    {4,  {3, 6, 4, 1, 2, 5}},
    {5,  {2, 1, 6, 4, 3, 5}},
    {6,  {4, 5, 2, 6, 1, 3}},
    {7,  {5, 2, 4, 1, 6, 3}},
    {8,  {5, 2, 1, 4, 3, 6}},
    {9,  {4, 1, 6, 5, 2, 3}},
    {10, {2, 4, 5, 6, 1, 3}},
    {11, {2, 3, 1, 6, 4, 5}},
    {12, {6, 1, 3, 4, 2, 5}},
    {13, {1, 3, 2, 5, 4, 6}},
    {14, {3, 4, 5, 6, 2, 1}},
    {15, {2, 5, 4, 1, 3, 6}},
    {16, {1, 3, 4, 5, 2, 6}},
    {17, {6, 3, 5, 2, 4, 1}},
    {18, {1, 4, 3, 6, 2, 5}}
};

#endif