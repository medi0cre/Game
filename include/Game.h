#pragma once
#include <stdbool.h>
#include <Arena.h>

#define ArenaSize 33554432 // 32 Megabytes

typedef struct {
    bool Z;
    bool X;
    bool C;
    bool Up;
    bool Down;
    bool Left;
    bool Right;
} Input;

extern Arena GameArena;

void GameInit(void);
Input GetUserInput(void);
