#pragma once
#include <stdbool.h>
#include <Arena.h>

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
extern uint16_t PlayerID;

void GameInit(void);
void GameLoop(void);
Input GetUserInput(void);
void LoadLevel(const char* File);
